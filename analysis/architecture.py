from __future__ import annotations

from dataclasses import dataclass

import torch

from data import EvaluationInputData

def _clamp(x: torch.FloatTensor) -> torch.FloatTensor:
    threshold = 127 / 128
    return torch.clamp(x, -threshold, threshold)


@dataclass
class FirstLayerData:
    full_symm: torch.FloatTensor  # shape=(N, S1)
    vert_asym: torch.FloatTensor  # shape=(N, S1)
    horz_asym: torch.FloatTensor  # shape=(N, S2)
    rotl_asym: torch.FloatTensor  # shape=(N, S2)

    def clamp(self) -> FirstLayerData:
        return FirstLayerData(
            full_symm=_clamp(self.full_symm),
            vert_asym=_clamp(self.vert_asym),
            horz_asym=_clamp(self.horz_asym),
            rotl_asym=_clamp(self.rotl_asym),
        )


class InputLayer(torch.nn.Module):
    def __init__(self, s1: int, s2: int):
        super().__init__()
        self.pst_fs = torch.nn.EmbeddingBag(185, s1, mode='sum', padding_idx=184)
        self.pst_va = torch.nn.EmbeddingBag(185, s1, mode='sum', padding_idx=184)
        self.pst_ha = torch.nn.EmbeddingBag(185, s2, mode='sum', padding_idx=184)
        self.pst_ra = torch.nn.EmbeddingBag(185, s2, mode='sum', padding_idx=184)

        for p in self.parameters():
            # Before rescaling them, they're all N(0, 1)
            # With 16 inputs on average, dividing by 4 achieves unit output stdev
            p /= 4

        self.tempo_va = torch.nn.Linear(1, s1, bias=False)
        torch.nn.init.zeros_(self.tempo_va.weight)

    def forward(self, input: EvaluationInputData) -> FirstLayerData:
        return FirstLayerData(
            self.pst_fs.forward(input.pst_idx),
            self.pst_va.forward(input.pst_idx, per_sample_weights=input.color_sign) + self.tempo_va(input.wtm),
            self.pst_ha.forward(input.pst_idx, per_sample_weights=input.sob_sign),
            self.pst_ra.forward(input.pst_idx, per_sample_weights=input.color_sign * input.sob_sign),
        )


@dataclass
class SecondLayerData:
    full_symm: torch.FloatTensor  # shape=(N, S3)
    vert_asym: torch.FloatTensor  # shape=(N, S3)

    def clamp(self) -> FirstLayerData:
        return SecondLayerData(
            full_symm=_clamp(self.full_symm),
            vert_asym=_clamp(self.vert_asym),
        )


class HiddenLayer(torch.nn.Module):
    def __init__(self, s1: int, s2: int, s3: int):
        super().__init__()
        n_symm_terms = 1 + 2 * s1 + 2 * s2
        n_asym_terms = 2 * s1 + s2
        input_scale = 0.72135  # STDEV of a normal clipped to [-1, 1]

        self.fs = torch.nn.Linear(s1, s3, bias=True)
        self.absva = torch.nn.Linear(s1, s3, bias=False)
        self.absha = torch.nn.Linear(s2, s3, bias=False)
        self.absra = torch.nn.Linear(s2, s3, bias=False)

        for p in self.parameters:
            torch.nn.init.normal_(p, std=1 / input_scale / n_symm_terms ** 0.5)

        self.va = torch.nn.Linear(s1, s3, bias=False)
        torch.nn.init.normal_(self.va.weight, std=1 / input_scale / n_asym_terms ** 0.5)
        self.fsxva = torch.nn.Linear(s1, s3, bias=False)
        torch.nn.init.normal_(self.fsxva.weight, std=1 / input_scale ** 2 / n_asym_terms ** 0.5)
        self.haxra = torch.nn.Linear(s2, s3, bias=False)
        torch.nn.init.normal_(self.haxra.weight, std=1 / input_scale ** 2 / n_asym_terms ** 0.5)
    
    def forward(self, input: FirstLayerData) -> SecondLayerData:
        return SecondLayerData(
            (
                self.fs(input.full_symm) + 
                self.absva(torch.abs(input.vert_asym)) + 
                self.absha(torch.abs(input.horz_asym)) + 
                self.absra(torch.abs(input.rotl_asym))
            ),
            (
                self.va(input.vert_asym) +
                self.fsxva(input.full_symm * input.vert_asym) +
                self.haxra(input.horz_asym * input.rotl_asym)
            ),
        )


class OutputLayer(torch.nn.Module):
    def __init__(self, s3: int):
        super().__init__()
        n_terms = 2 * s3
        input_scale = 0.72135  # STDEV of a normal clipped to [-1, 1]

        self.va = torch.nn.Linear(16, 1, bias=False)
        torch.nn.init.normal_(self.va.weight, std=1 / input_scale / n_terms ** 0.5)
        self.fsxva = torch.nn.Linear(16, 1, bias=False)
        torch.nn.init.normal_(self.fsxva.weight, std=1 / input_scale ** 2 / n_terms ** 0.5)
    
    def forward(self, input: SecondLayerData) -> torch.Tensor:
        return torch.squeeze(self.va(input.vert_asym) + self.fsxva(input.full_symm * input.vert_asym), dim=1)


class RengarNetwork(torch.nn.Module):
    def __init__(
        self, 
        s1: int,
        s2: int,
        s3: int,
    ):
        super().__init__()

        self.l0 = InputLayer(s1, s2)
        self.l1 = HiddenLayer(s1, s2, s3)
        self.l2 = OutputLayer(s3)

    def forward(self, input: EvaluationInputData) -> torch.Tensor:
        return self.l2(self.l1(self.l0(input).clamp()).clamp())
