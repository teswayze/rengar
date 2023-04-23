from pathlib import Path

import numpy as np
import pandas as pd
from chess import Board
from tqdm import tqdm
from statsmodels.regression.quantile_regression import QuantReg

from translate_constants import pst
from utils import print_nicely

LABELED_PIECES = [('P', 'pawn'), ('N', 'knight'), ('B', 'bishop'), ('R', 'rook'), ('Q', 'queen'), ('K', 'king')]
PC_TOTAL = pst.PC_KNIGHT * 4 + pst.PC_BISHOP * 4 + pst.PC_ROOK * 4 + pst.PC_QUEEN * 2


def mask_to_bool_array(mask: int) -> np.ndarray:
    return np.array([bool(mask & (1<<n)) for n in range(64)]).reshape(8, 8)


def flip_if_white(board: np.ndarray, color: str):
    if color == 'W':
        return board[::-1]
    if color == 'B':
        return board
    raise ValueError(color)


def features_from_fen(fen: str) -> pd.Series:
    b = Board(fen)

    masks = {
        f'{color_name}{label}mask': b.occupied_co[color_index] & getattr(b, piece + 's')
        for color_name, color_index in [('W', True), ('B', False)]
        for label, piece in LABELED_PIECES
    }
    arrays = {k: mask_to_bool_array(v) for k, v in masks.items()}

    counts = {f'#{label}': arrays[label + 'mask'].sum() for label in ['WP', 'BP', 'WN', 'BN', 'WB', 'BB', 'WR', 'BR', 'WQ', 'BQ']}
    tables = {
        f'{color}{label}{phase}': np.sum(arrays[f'{color}{label}mask'] * flip_if_white(getattr(pst, f'{phase}_{piece}_table'), color))
        for label, piece in LABELED_PIECES
        for color in 'WB'
        for phase in ['mg', 'eg']
    }

    return pd.Series({**counts, **tables, **masks})


def side_to_move_sign(fen: str) -> int:
    match fen.split()[1]:
        case 'b': return -1
        case 'w': return 1
    raise ValueError(fen)


def is_check(move_san: str) -> bool:
    return move_san[-1] in '+#'


def is_forcing(move_san: str) -> bool:
    return any(char in 'x+=#' for char in move_san)


def load_features_and_target_for_game(path: Path) -> tuple[pd.DataFrame, pd.Series]:
    df = pd.read_csv(path, index_col=0)
    quiet = df[~(df['move'].apply(is_check).shift(fill_value=False) | df['move'].apply(is_forcing))]
    
    x = pd.DataFrame({row.name: features_from_fen(row['fen']) for _, row in quiet.iterrows()}).T
    y = quiet['score'] * quiet['fen'].apply(side_to_move_sign)

    return x, y


def get_data_for_fitting(tournament_name: str) -> tuple[pd.DataFrame, pd.Series]:
    tourney_path = Path('games') / tournament_name
    xs = []
    ys = []
    for game_dir in tqdm(sorted(tourney_path.iterdir())):
        x, y = load_features_and_target_for_game(game_dir / 'info.csv')
        xs.append(x)
        ys.append(y)

    return pd.concat(xs), pd.concat(ys)


def compute_table_bias(x: pd.DataFrame) -> dict[str, float]:
    phase_weights = pd.Series(0, index=x.columns)
    for label, piece in LABELED_PIECES[1:-1]:
        phase_weights[f'#W{label}'] = phase_weights[f'#B{label}'] = getattr(pst, f'PC_{piece.upper()}')

    phase = x @ phase_weights
    mg_mean = (x.T @ phase) / phase.sum()
    eg_mean = (x.T @ (PC_TOTAL - phase)) / (PC_TOTAL - phase).sum()
    mg_mean['#WK'] = mg_mean['#BK'] = eg_mean['#WK'] = eg_mean['#BK'] = 1

    output = {}
    for label, piece in LABELED_PIECES:
        output[f'mg_{piece}_table'] = (mg_mean[f'W{label}mg'] + mg_mean[f'B{label}mg']) / (mg_mean[f'#W{label}'] + mg_mean[f'#B{label}'])
        output[f'eg_{piece}_table'] = (eg_mean[f'W{label}eg'] + eg_mean[f'B{label}eg']) / (eg_mean[f'#W{label}'] + eg_mean[f'#B{label}'])

    return output


def extract_features_for_metric(x: pd.DataFrame, metric: str) -> pd.Series:
    if metric in ['mg', 'eg']:
        black_sign = -1
    elif metric == 'pc':
        black_sign = 1
    else:
        raise ValueError(metric)

    output = pd.DataFrame(index=x.index)
    expected = pd.Series(dtype=int)
    for label, piece in LABELED_PIECES:
        count_attr_name = f'{metric.upper()}_{piece.upper()}'
        if hasattr(pst, count_attr_name):
            expected[f'#{label}{metric}'] = getattr(pst, count_attr_name)
            output[f'#{label}{metric}'] = x[f'#W{label}'] + black_sign * x[f'#B{label}']
    for label, piece in LABELED_PIECES:
        if f'W{label}{metric}' in x.columns:
            expected[f'{label}{metric}_table'] = 1
            output[f'{label}{metric}_table'] = x[f'W{label}{metric}'] + black_sign * x[f'B{label}{metric}']

    return output.astype(int), expected


def compute_eval(x: pd.DataFrame) -> pd.Series:
    mg_feat, mg_coef = extract_features_for_metric(x, 'mg')
    mg = mg_feat @ mg_coef
    eg_feat, eg_coef = extract_features_for_metric(x, 'eg')
    eg = eg_feat @ eg_coef
    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef

    return eg + (mg - eg) * pc / PC_TOTAL


def compute_eval_inflation(x: pd.DataFrame, y: pd.Series) -> float:
    return QuantReg(y, compute_eval(x)).fit().params['x1']


def compute_pawnless_adjustment(x: pd.DataFrame, y: pd.Series) -> float:
    ev = compute_eval(x)
    better_side_pawnless = ((ev > 0) & (x['#WP'] == 0)) | ((ev < 0) & (x['#BP'] == 0))

    return QuantReg(y[better_side_pawnless], ev[better_side_pawnless]).fit().params['x1'] / compute_eval_inflation(x, y)


def fit_eval_coefs(x: pd.DataFrame, y: pd.Series) -> pd.Series:
    x_mg = extract_features_for_metric(x, 'mg')[0]
    x_eg = extract_features_for_metric(x, 'eg')[0]
    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef

    x_combined = pd.concat([x_mg.multiply(pc, axis=0), x_eg.multiply(24 - pc, axis=0)], axis=1)
    inflation_factor = compute_eval_inflation(x, y)
    table_columns = [c for c in x_combined.columns if c.endswith('_table')]
    y_no_table = y * PC_TOTAL / inflation_factor - x_combined[table_columns].sum(axis=1)
    x_no_table = x_combined[x_combined.columns.difference(table_columns)]
    
    return QuantReg(y_no_table, x_no_table).fit().params


def fit_pc_coefs(x: pd.DataFrame, y: pd.Series) -> pd.Series:
    mg_feat, mg_coef = extract_features_for_metric(x, 'mg')
    mg = mg_feat @ mg_coef
    eg_feat, eg_coef = extract_features_for_metric(x, 'eg')
    eg = eg_feat @ eg_coef
    x_pc = extract_features_for_metric(x, 'pc')[0]

    inflation_factor = compute_eval_inflation(x, y)
    y_no_eg = (y / inflation_factor - eg) * PC_TOTAL
    x_scaled = x_pc.multiply(mg - eg, axis=0)

    return QuantReg(y_no_eg, x_scaled).fit().params


def _find_right_adjustment(error: pd.Series, signed_weights: pd.Series, t_stat_req: float = 3, max_adj: int = 10):
    if (signed_weights == 0).all():
        return 0
    
    adjustment = 0
    weights_rms = (signed_weights ** 2).sum() ** 0.5
    while True:
        grad_abs_error = np.sign(error - adjustment * signed_weights) @ signed_weights
        if abs(grad_abs_error) < t_stat_req * weights_rms:
            return adjustment
        adjustment += int(np.sign(grad_abs_error))
        if abs(adjustment) == max_adj:
            return adjustment
        


def fit_pst_adjustment(x: pd.DataFrame, y: pd.Series, piece: str, phase: str) -> np.ndarray:
    ev = compute_eval(x)
    inflation = compute_eval_inflation(x, y)
    error = y / inflation - ev

    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef
    if phase == 'mg':
        weights = pc / PC_TOTAL
    elif phase == 'eg':
        weights = (24 - pc) / PC_TOTAL
    else:
        raise ValueError(phase)

    white = x[f'W{piece}mask'].astype('uint64')
    black = x[f'B{piece}mask'].astype('uint64')

    suggested_adjustments = []
    for i in range(64):
        w_match = (white & (1 << (56 ^ i))).astype(bool)
        b_match = (black & (1 << i)).astype(bool)
        signed_weights = weights * w_match - weights * b_match

        suggested_adjustments.append(_find_right_adjustment(error, signed_weights))

    return np.array(suggested_adjustments).reshape((8, 8))
