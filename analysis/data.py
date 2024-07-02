from dataclasses import dataclass

import numpy as np
import pandas as pd
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
    wtm: torch.FloatTensor  # shape=(N, 1)


def _load_csv_to_tensor(path: str, dtype=np.float32) -> torch.Tensor:
    return torch.from_numpy(pd.read_csv(path, index_col=0, dtype=dtype).values)


def load_data_and_labels(path: str) -> tuple[EvaluationInputData, torch.FloatTensor]:
    pst_idx = _load_csv_to_tensor(path + '.pst_idx.csv', dtype=np.int32)
    color_sign = _load_csv_to_tensor(path + '.color_sign.csv')
    sob_sign = _load_csv_to_tensor(path + '.sob_sign.csv')
    wtm = _load_csv_to_tensor(path + '.wtm.csv')
    game_result = _load_csv_to_tensor(path + '.game_result.csv')
    return EvaluationInputData(
        pst_idx=pst_idx,
        color_sign=color_sign,
        sob_sign=sob_sign,
        wtm=wtm,
    ), game_result.squeeze(dim=1)
