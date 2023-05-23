import functools
from pathlib import Path

import numpy as np
import pandas as pd
from chess import Board, SquareSet, BB_EMPTY
from tqdm import tqdm

from golden_section_search import golden_section_search
from translate_constants import pst
from utils import print_cpp_2d_array_code, print_cpp_constant_code

LABELED_PIECES = [('P', 'pawn'), ('N', 'knight'), ('B', 'bishop'), ('R', 'rook'), ('Q', 'queen'), ('K', 'king')]
PC_TOTAL = pst.pc_knight * 4 + pst.pc_bishop * 4 + pst.pc_rook * 4 + pst.pc_queen * 2


def bishop_attack_count(b: Board, color: bool) -> int:
    ss = SquareSet(BB_EMPTY)
    for bishop in SquareSet(b.bishops & b.occupied_co[color]):
        ss |= b.attacks(bishop)
    return len(ss)


def rook_attack_count(b: Board, color: bool) -> int:
    ss = SquareSet(BB_EMPTY)
    for bishop in SquareSet(b.rooks & b.occupied_co[color]):
        ss |= b.attacks(bishop)
    return len(ss)


def queen_attack_count(b: Board, color: bool) -> int:
    ss = SquareSet(BB_EMPTY)
    for bishop in SquareSet(b.queens & b.occupied_co[color]):
        ss |= b.attacks(bishop)
    return len(ss)


def lookup_label(piece: str):
    return {y: x for x, y in LABELED_PIECES}[piece]


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
    attacks = {
        'WBA': bishop_attack_count(b, True),
        'WRA': rook_attack_count(b, True),
        'WQA': queen_attack_count(b, True),
        'BBA': bishop_attack_count(b, False),
        'BRA': rook_attack_count(b, False),
        'BQA': queen_attack_count(b, False),
    }

    return pd.Series({**counts, **tables, **attacks, **masks})


def side_to_move_sign(fen: str) -> int:
    match fen.split()[1]:
        case 'b': return -1
        case 'w': return 1
    raise ValueError(fen)


def is_check(move_san: str) -> bool:
    return move_san[-1] in '+#'


def is_forcing(move_san: str) -> bool:
    return any(char in 'x+=#' for char in move_san)


def load_features_for_game(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path, index_col=0)
    quiet = df[~(df['move'].apply(is_check).shift(fill_value=False) | df['move'].apply(is_forcing))]
    
    x = pd.DataFrame({row.name: features_from_fen(row['fen']) for _, row in quiet.iterrows()}).T

    return x


def get_data_for_fitting(*tournament_names: str) -> tuple[pd.DataFrame, pd.Series]:
    all_game_dirs = []
    for tournament_name in tournament_names:
        tourney_path = Path(__file__).parent.parent / 'games' / tournament_name
        all_game_dirs.extend(tourney_path.iterdir())
    
    xs = []
    zs = []
    for game_dir in tqdm(all_game_dirs):
        x = load_features_for_game(game_dir / 'info.csv')
        xs.append(x)

        with open(game_dir / 'result.txt') as f:
            result_text = f.read()
        res = 0
        if 'White won' in result_text:
            res = 1
        elif 'Black won' in result_text:
            res = -1
        zs.append(pd.Series(res, index=x.index))

    return pd.concat(xs), pd.concat(zs)


def compute_table_bias(x: pd.DataFrame) -> dict[str, float]:
    phase_weights = pd.Series(0, index=x.columns)
    for label, piece in LABELED_PIECES[1:-1]:
        phase_weights[f'#W{label}'] = phase_weights[f'#B{label}'] = getattr(pst, f'pc_{piece}')

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
    for label, piece in LABELED_PIECES[:-1]:
        count_attr_name = f'{metric}_{piece}'
        if hasattr(pst, count_attr_name):
            expected[f'#{label}{metric}'] = getattr(pst, count_attr_name)
        else:
            expected[f'#{label}{metric}'] = 0
        output[f'#{label}{metric}'] = x[f'#W{label}'] + black_sign * x[f'#B{label}']
    if black_sign == 1:
        output[f'{metric}_intercept'] = 1
        expected[f'{metric}_intercept'] = 0
    else:
        for label, piece in LABELED_PIECES:
            if f'W{label}{metric}' in x.columns:
                output[f'{label}{metric}_table'] = x[f'W{label}{metric}'] + black_sign * x[f'B{label}{metric}']
            else:
                output[f'{label}{metric}_table'] = 0
            expected[f'{label}{metric}_table'] = 1
        
        output[f'B{metric}_atk'] = x['WBA'] - x['BBA']
        output[f'R{metric}_atk'] = x['WRA'] - x['BRA']
        output[f'Q{metric}_atk'] = x['WQA'] - x['BQA']
        expected[f'B{metric}_atk'] = expected[f'R{metric}_atk'] = expected[f'Q{metric}_atk'] = 0

    return output.astype(int), expected


def compute_eval(x: pd.DataFrame) -> pd.Series:
    mg_feat, mg_coef = extract_features_for_metric(x, 'mg')
    mg = mg_feat @ mg_coef
    eg_feat, eg_coef = extract_features_for_metric(x, 'eg')
    eg = eg_feat @ eg_coef
    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef

    return eg + (mg - eg) * pc / PC_TOTAL


def game_score_mse(evaluations: pd.Series, z: pd.Series, magic_number: int) -> float:
    e_y = np.exp(evaluations / magic_number)
    z_hat = (e_y - 1) / (e_y + 1)
    return ((z - z_hat) ** 2).mean()


def compute_magic_number(evaluations: pd.Series, z: pd.Series) -> int:
    return golden_section_search(0, 1000, functools.partial(game_score_mse, evaluations, z))


def compute_pawnless_adjustment(x: pd.DataFrame, z: pd.Series) -> float:
    ev = compute_eval(x)
    better_side_zero_pawn = ((ev > 0) & (x['#WP'] == 0)) | ((ev < 0) & (x['#BP'] == 0))
    better_side_single_pawn = ((ev > 0) & (x['#WP'] == 1)) | ((ev < 0) & (x['#BP'] == 1))
    better_side_multi_pawn = ((ev > 0) & (x['#WP'] >= 2)) | ((ev < 0) & (x['#BP'] >= 2))

    baseline = compute_magic_number(ev[better_side_multi_pawn], z[better_side_multi_pawn])
    zero_pawn = baseline / compute_magic_number(ev[better_side_zero_pawn], z[better_side_zero_pawn])
    one_pawn = baseline / compute_magic_number(ev[better_side_single_pawn], z[better_side_single_pawn])
    return zero_pawn, one_pawn


def fit_eval_coefs(x: pd.DataFrame, z: pd.Series) -> pd.Series:
    x_mg, mg_coef = extract_features_for_metric(x, 'mg')
    x_eg, eg_coef = extract_features_for_metric(x, 'eg')
    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef

    x_combined = pd.concat([x_mg.multiply(pc, axis=0), x_eg.multiply(PC_TOTAL - pc, axis=0)], axis=1) / PC_TOTAL
    table_columns = [c for c in x_combined.columns if c.endswith('_table')]
    x_no_table = x_combined[x_combined.columns.difference(table_columns)]
    table_sum = x_combined[table_columns].sum(axis=1)
    coef_no_table = pd.concat([mg_coef, eg_coef])[x_no_table.columns]
    
    magic_number = None
    for i in tqdm(range(10)):
        raw_eval = x_no_table @ coef_no_table + table_sum
        adjust = pd.Series(1, index=raw_eval.index)
        adjust.loc[(raw_eval > 0) & (x['#WP'] == 0)] = 0.5
        adjust.loc[(raw_eval < 0) & (x['#BP'] == 0)] = 0.5
        adjust.loc[(raw_eval > 0) & (x['#WP'] == 1)] = 0.75
        adjust.loc[(raw_eval < 0) & (x['#BP'] == 1)] = 0.75
        adjusted_eval = adjust * raw_eval

        if magic_number is None:
            magic_number = compute_magic_number(adjusted_eval, z)
        eval_e = np.exp(adjusted_eval / magic_number)
        win_p_modeled = (eval_e - 1) / (eval_e + 1)
        win_p_grad = (1 - win_p_modeled ** 2) / (2 * magic_number) * adjust
        error = z - win_p_modeled
        x_w = x_no_table.multiply(win_p_grad, axis='index')
        innovation = np.linalg.solve(x_w.T @ x_w, x_w.T @ error)
        coef_no_table = coef_no_table + innovation
    
    return coef_no_table


def fit_pc_coefs(x: pd.DataFrame, z: pd.Series) -> pd.Series:
    mg_feat, mg_coef = extract_features_for_metric(x, 'mg')
    mg = mg_feat @ mg_coef
    eg_feat, eg_coef = extract_features_for_metric(x, 'eg')
    eg = eg_feat @ eg_coef
    x_pc, pc_coef = extract_features_for_metric(x, 'pc')

    magic_number = None
    for i in tqdm(range(10)):
        pc = x_pc @ pc_coef
        raw_eval = eg + (mg - eg) * pc / PC_TOTAL
        adjust = pd.Series(1, index=raw_eval.index)
        adjust.loc[(raw_eval > 0) & (x['#WP'] == 0)] = 0.5
        adjust.loc[(raw_eval < 0) & (x['#BP'] == 0)] = 0.5
        adjust.loc[(raw_eval > 0) & (x['#WP'] == 1)] = 0.75
        adjust.loc[(raw_eval < 0) & (x['#BP'] == 1)] = 0.75
        adjusted_eval = adjust * raw_eval

        if magic_number is None:
            magic_number = compute_magic_number(adjusted_eval, z)
        eval_e = np.exp(adjusted_eval / magic_number)
        win_p_modeled = (eval_e - 1) / (eval_e + 1)
        win_p_grad = (1 - win_p_modeled ** 2) / (2 * magic_number) * adjust * (mg - eg) / PC_TOTAL
        error = z - win_p_modeled
        x_w = x_pc.multiply(win_p_grad, axis='index')
        innovation = np.linalg.solve(x_w.T @ x_w, x_w.T @ error)
        pc_coef = pc_coef + innovation

    return pc_coef


def drop_table_columns(df: pd.DataFrame) -> pd.DataFrame:
    return df[[c for c in df.columns if not c.endswith('_table')]]


def fit_all_coefs(x: pd.DataFrame, z: pd.Series) -> pd.Series:
    x_mg, mg_coef = extract_features_for_metric(x, 'mg')
    x_eg, eg_coef = extract_features_for_metric(x, 'eg')
    x_pc, pc_coef = extract_features_for_metric(x, 'pc')
    pawnless_adj = 192
    one_pawn_adj = 256

    mg = mg_feat @ mg_coef
    eg = eg_feat @ eg_coef
    pc = x_pc @ pc_coef
    raw_eval = eg + (mg - eg) * pc / PC_TOTAL

    better_side_pawnless = ((raw_eval > 0) & (x['#WP'] == 0)) | ((raw_eval < 0) & (x['#BP'] == 0))
    better_side_one_pawn = ((raw_eval > 0) & (x['#WP'] == 1)) | ((raw_eval < 0) & (x['#BP'] == 1))
    adjust = pd.Series(1, index=raw_eval.index)
    adjust.loc[better_side_pawnless] = pawnless_adj / 256
    adjust.loc[better_side_one_pawn] = one_pawn_adj / 256
    adjusted_eval = adjust * raw_eval

    magic_number = compute_magic_number(adjusted_eval, z)
    win_p_modeled = (eval_e - 1) / (eval_e + 1)
    error = z - win_p_modeled

    win_p_grad = (1 - win_p_modeled ** 2) / (2 * magic_number)
    x_w = pd.concat([
        drop_table_columns(x_mg).multiply(win_p_grad * pc / PC_TOTAL * adjust, axis=0),
        drop_table_columns(x_eg).multiply(win_p_grad * (PC_TOTAL - pc) / PC_TOTAL * adjust, axis=0),
        x_pc.multiply(win_p_grad * (mg - eg) / PC_TOTAL * adjust, axis=0),
        pd.DataFrame({
            'pawnless': win_p_grad * better_side_pawnless * raw_eval / 256,
            'one_pawn': win_p_grad * better_side_one_pawn * raw_eval / 256,
        }),
    ], axis=1)
    innovation = np.linalg.solve(x_w.T @ x_w, x_w.T @ error)

    return innovation + pd.concat([mg_coef, eg_coef, pc_coef, pd.Series({'pawnless': pawnless_adj, 'one_pawn': one_pawn_adj})])[x_w.columns]


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
        if np.sign(grad_abs_error) != np.sign(adjustment):
            return adjustment
        


def fit_pst_adjustment(x: pd.DataFrame, y: pd.Series, piece: str, phase: str) -> np.ndarray:
    mg_feat, mg_coef = extract_features_for_metric(x, 'mg')
    mg = mg_feat @ mg_coef
    eg_feat, eg_coef = extract_features_for_metric(x, 'eg')
    eg = eg_feat @ eg_coef
    pc_feat, pc_coef = extract_features_for_metric(x, 'pc')
    pc = pc_feat @ pc_coef
    ev = eg + (mg - eg) * pc / PC_TOTAL
    
    inflation = compute_eval_inflation(x, y)
    error = y / inflation - ev

    if phase == 'mg':
        weights = pc / PC_TOTAL
        black_sign = -1
    elif phase == 'eg':
        weights = (PC_TOTAL - pc) / PC_TOTAL
        black_sign = -1
    elif phase == 'pc':
        weights = (mg - eg) / PC_TOTAL
        black_sign = 1
    else:
        raise ValueError(phase)

    white = x[f'W{lookup_label(piece)}mask'].astype('uint64')
    black = x[f'B{lookup_label(piece)}mask'].astype('uint64')

    suggested_adjustments = []
    for i in range(64):
        w_match = (white & (1 << (56 ^ i))).astype(bool)
        b_match = (black & (1 << i)).astype(bool)
        signed_weights = weights * w_match + black_sign * weights * b_match

        suggested_adjustments.append(_find_right_adjustment(error, signed_weights))

    return np.array(suggested_adjustments).reshape((8, 8))


def suggest_new_tables(x: pd.DataFrame, y: pd.Series):
    for piece in ['pawn', 'knight', 'bishop', 'rook', 'queen', 'king']:
        for phase in ['mg', 'eg']:
            table_name = f'{phase}_{piece}_table'
            old_table = getattr(pst, table_name)
            adjustment = fit_pst_adjustment(x, y, piece, phase)
            print_cpp_2d_array_code(table_name, old_table + adjustment)
