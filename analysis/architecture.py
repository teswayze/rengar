from __future__ import annotations

from dataclasses import dataclass
import typing as ty

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


@dataclass
class SecondLayerData:
    full_symm: torch.FloatTensor  # shape=(N, S3)
    vert_asym: torch.FloatTensor  # shape=(N, S3)

    def clamp(self) -> FirstLayerData:
        return SecondLayerData(
            full_symm=_clamp(self.full_symm),
            vert_asym=_clamp(self.vert_asym),
        )


LayerDataT = ty.TypeVar('LayerDataT', FirstLayerData, SecondLayerData)


class ModuleMap(torch.nn.Module):
    def __init__(self, module: torch.nn.Module, *exclude_keys: str):
        super().__init__()
        self.exclude_keys = exclude_keys
        self.module = module
    
    def forward(self, input: LayerDataT) -> LayerDataT:
        return type(input)(**{k: v if k in self.exclude_keys else self.module(v) for k, v in input.__dict__.items()})


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
            p.data /= 4

        self.tempo_va = torch.nn.Linear(1, s1, bias=False)
        torch.nn.init.zeros_(self.tempo_va.weight)

    def forward(self, input: EvaluationInputData) -> FirstLayerData:
        return FirstLayerData(
            self.pst_fs.forward(input.pst_idx),
            self.pst_va.forward(input.pst_idx, per_sample_weights=input.color_sign) + self.tempo_va(input.wtm),
            self.pst_ha.forward(input.pst_idx, per_sample_weights=input.sob_sign),
            self.pst_ra.forward(input.pst_idx, per_sample_weights=input.color_sign * input.sob_sign),
        )


class HiddenLayer(torch.nn.Module):
    def __init__(self, s1: int, s2: int, s3: int):
        super().__init__()

        self.fs = torch.nn.Linear(s1, s3, bias=True)
        self.absva = torch.nn.Linear(s1, s3, bias=False)
        self.absha = torch.nn.Linear(s2, s3, bias=False)
        self.absra = torch.nn.Linear(s2, s3, bias=False)

        self.va = torch.nn.Linear(s1, s3, bias=False)
        self.fsxva = torch.nn.Linear(s1, s3, bias=False)
        self.haxra = torch.nn.Linear(s2, s3, bias=False)

        n_symm_terms = 1 + 2 * s1 + 2 * s2
        n_asym_terms = 2 * s1 + s2
        params = list(self.parameters())
        assert len(params) == 8
        for p in params[:5]:
            torch.nn.init.normal_(p, std=1 / n_symm_terms ** 0.5)
        for p in params[5:]:
            torch.nn.init.normal_(p, std=1 / n_asym_terms ** 0.5)

    
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

        self.va = torch.nn.Linear(s3, 1, bias=False)
        self.fsxva = torch.nn.Linear(s3, 1, bias=False)

        n_terms = 2 * s3
        for p in self.parameters():
            torch.nn.init.normal_(p, std=1 / n_terms ** 0.5)
    
    def forward(self, input: SecondLayerData) -> torch.Tensor:
        return torch.squeeze(self.va(input.vert_asym) + self.fsxva(input.full_symm * input.vert_asym), dim=1)


def initialize_rengar_network(s1: int, s2: int, s3: int, l1_dropout: float, l2_dropout: float) -> torch.nn.Sequential:
    return torch.nn.Sequential(
        InputLayer(s1, s2),
        ModuleMap(torch.nn.Hardtanh(-127/128, 127/128)),
        ModuleMap(torch.nn.Dropout(l1_dropout)),
        HiddenLayer(s1, s2, s3),
        ModuleMap(torch.nn.Hardtanh(-127/128, 127/128), 'vert_asym'),
        ModuleMap(torch.nn.Dropout(l2_dropout)),
        OutputLayer(s3),
    )


def clamp_weights(net: torch.nn.Module):
    for p in net.parameters():
        torch.clamp(p.data, -127/64, 127/64, out=p.data)
