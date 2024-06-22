from dataclasses import dataclass

import torch

@dataclass
class EvaluationInputData:
    # Integers from 0-184 for 24 pawn squares + 32 * 5 piece squares
    # A value of 184 indicates padding
    # The data would fit in ByteTensor but that doesn't work with torch.Embedding
    pst_idx: torch.IntTensor  # shape=(N, 32)
    # 1.0 for white pieces; -1.0 for black pieces
    color_sign: torch.FloatTensor  # shape=(N, 32)
    # 1.0 for kingside; -1.0 for queenside
    sob_sign: torch.FloatTensor  # shape=(N, 32)
    # 1.0 for white to move; -1.0 for black to move
    wtm: torch.BoolTensor  # shape=(N, 1)


@dataclass
class FirstLayerData:
    full_symm: torch.FloatTensor  # shape=(N, 16)
    vert_asym: torch.FloatTensor  # shape=(N, 16)
    horz_asym: torch.FloatTensor  # shape=(N, 16)
    rotl_asym: torch.FloatTensor  # shape=(N, 16)


def _clamp(underlying: torch.Tensor) -> torch.Tensor:
    return torch.clamp(underlying, torch.tensor(-4.0), torch.tensor(4.0))


class InputLayer(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.pst_fs = torch.nn.EmbeddingBag(185, 16, mode='sum', padding_idx=184)
        self.pst_va = torch.nn.EmbeddingBag(185, 16, mode='sum', padding_idx=184)
        self.pst_ha = torch.nn.EmbeddingBag(185, 16, mode='sum', padding_idx=184)
        self.pst_ra = torch.nn.EmbeddingBag(185, 16, mode='sum', padding_idx=184)

        self.tempo_va = torch.nn.Linear(1, 16, bias=False)

    def forward(self, input: EvaluationInputData) -> FirstLayerData:
        return FirstLayerData(
            _clamp(self.pst_fs.forward(input.pst_idx)),
            _clamp(self.pst_va.forward(input.pst_idx, per_sample_weights=input.color_sign) + self.tempo_va(input.wtm)),
            _clamp(self.pst_ha.forward(input.pst_idx, per_sample_weights=input.sob_sign)),
            _clamp(self.pst_ra.forward(input.pst_idx, per_sample_weights=input.color_sign * input.sob_sign)),
        )


@dataclass
class SecondLayerData:
    full_symm: torch.FloatTensor  # shape=(N, 16)
    vert_asym: torch.FloatTensor  # shape=(N, 16)


class HiddenLayer(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.fs = torch.nn.Linear(16, 16, bias=True)
        self.absva = torch.nn.Linear(16, 16, bias=False)
        self.absha = torch.nn.Linear(16, 16, bias=False)
        self.absra = torch.nn.Linear(16, 16, bias=False)

        self.va = torch.nn.Linear(16, 16, bias=False)
        self.fsxva = torch.nn.Linear(16, 16, bias=False)
        self.haxra = torch.nn.Linear(16, 16, bias=False)
    
    def forward(self, input: FirstLayerData) -> SecondLayerData:
        return SecondLayerData(
            _clamp(
                self.fs(input.full_symm) + 
                self.absva(torch.abs(input.vert_asym)) + 
                self.absha(torch.abs(input.horz_asym)) + 
                self.absra(torch.abs(input.rotl_asym))
            ),
            _clamp(
                self.va(input.vert_asym) +
                self.fsxva(input.full_symm * input.vert_asym) +
                self.haxra(input.horz_asym * input.rotl_asym)
            ),
        )


class OutputLayer(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.va = torch.nn.Linear(16, 1, bias=False)
        self.fsxva = torch.nn.Linear(16, 1, bias=False)
    
    def forward(self, input: SecondLayerData) -> torch.Tensor:
        return torch.squeeze(self.va(input.vert_asym) + self.fsxva(input.full_symm * input.vert_asym), dim=1)


class RengarNetwork(torch.nn.Module):
    def __init__(self, init_std: float):
        super().__init__()
        self.l0 = InputLayer()
        self.l1 = HiddenLayer()
        self.l2 = OutputLayer()

        for p in self.parameters():
            torch.nn.init.normal_(p, std=init_std)

    def forward(self, input: EvaluationInputData) -> torch.Tensor:
        return self.l2(self.l1(self.l0(input)))
