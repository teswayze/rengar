import functools
from pathlib import Path

import numpy as np
import pandas as pd
from chess import Board, SquareSet, BB_EMPTY
from tqdm import tqdm

from golden_section_search import golden_section_search
from translate_constants import pst, endgames, eval_
from utils import print_cpp_2d_array_code, print_cpp_constant_code, root_dir

LABELED_PIECES = [('P', 'pawn'), ('N', 'knight'), ('B', 'bishop'), ('R', 'rook'), ('Q', 'queen'), ('K', 'king')]
PC_TOTAL = 256


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


def features_from_board(b: Board) -> tuple[pd.Series, pd.Series]:
    arrays = {
        f'{color_name}_{piece}': mask_to_bool_array(b.occupied_co[color_index] & getattr(b, piece + 's'))
        for color_name, color_index in [('wt', True), ('bk', False)]
        for _, piece in LABELED_PIECES
    }

    counts = {k: v.sum() for k, v in arrays.items()}
    attacks = {
        'wt_bishop_atk': bishop_attack_count(b, True),
        'wt_rook_atk': rook_attack_count(b, True),
        'wt_queen_atk': queen_attack_count(b, True),
        'bk_bishop_atk': bishop_attack_count(b, False),
        'bk_rook_atk': rook_attack_count(b, False),
        'bk_queen_atk': queen_attack_count(b, False),
    }

    tables = pd.concat([
        pd.Series(
            arr[::-1].flatten() if label.startswith('wt') else arr.flatten(),
            index=[f'{label}_table_{square_num}' for square_num in range(64)],
        ) for label, arr in arrays.items()
    ])

    return pd.Series({'tempo': 1 if b.turn else -1, **counts, **attacks}).astype(int), tables


def load_features_for_game(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path, index_col=0)
    board = Board(df['fen'].iloc[0])

    regular = {}
    table = {}
    for name, row in df.iterrows():
        move = board.parse_san(row['move'])
        if not (board.is_check() or board.is_capture(move) or move.promotion is not None):
            s1, s2 = features_from_board(board)
            regular[name] = s1
            table[name] = s2
        board.push(move)
    
    x = pd.concat([pd.DataFrame(regular).T, pd.DataFrame(table).T], axis=1)

    return x


def get_data_for_fitting(*tournament_names: str) -> tuple[pd.DataFrame, pd.Series]:
    all_game_dirs = []
    for tournament_name in tournament_names:
        tourney_path = root_dir() / 'games' / tournament_name
        all_game_dirs.extend(tourney_path.iterdir())
    
    xs = []
    zs = []
    game_id = 0
    for game_dir in tqdm(all_game_dirs):
        x = load_features_for_game(game_dir / 'info.csv')
        x.index = pd.MultiIndex.from_product([[game_id], x.index], names=['game', 'move'])
        xs.append(x)

        with open(game_dir / 'result.txt') as f:
            result_text = f.read()
        res = 0
        if 'White won' in result_text:
            res = 1
        elif 'Black won' in result_text:
            res = -1
        zs.append(pd.Series(res, index=x.index))

        game_id += 1

    return pd.concat(xs), pd.concat(zs)


# TODO: this is very broken
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


def table_diff(x: pd.DataFrame, piece: str, metric: str) -> pd.DataFrame:
    white_idx = [f'wt_{piece}_table_{i}' for i in range(64)]
    black_idx = [f'bk_{piece}_table_{i}' for i in range(64)]
    metric_idx = [f'{metric}_{piece}_table_{i}' for i in range(64)]

    white_df = x[white_idx]
    white_df.columns = metric_idx
    black_df = x[black_idx]
    black_df.columns = metric_idx

    return white_df.astype(int) - black_df.astype(int)


def extract_features_for_metric(x: pd.DataFrame, metric: str) -> pd.Series:
    if metric in ['mg', 'eg']:
        black_sign = -1
    elif metric == 'pc':
        black_sign = 1
    else:
        raise ValueError(metric)

    output = pd.DataFrame(index=x.index)
    expected = pd.Series(dtype=int)
    for _, piece in LABELED_PIECES[:-1]:
        count_attr_name = f'{metric}_{piece}'
        expected[count_attr_name] = getattr(pst, count_attr_name)
        output[count_attr_name] = x[f'wt_{piece}'] + black_sign * x[f'bk_{piece}']
    if black_sign == 1:
        output[f'{metric}_intercept'] = 1
        expected[f'{metric}_intercept'] = 0
    else:
        for piece in ['bishop', 'rook', 'queen']:
            attack_attr_name = f'{metric}_{piece}_atk'
            output[attack_attr_name] = x[f'wt_{piece}_atk'] - x[f'bk_{piece}_atk']
            expected[attack_attr_name] = getattr(eval_, attack_attr_name)

        tempo_attr_name = f'{metric}_tempo'
        output[tempo_attr_name] = x['tempo']
        expected[tempo_attr_name] = 0

        table_dfs = []
        expected_sers = []
        for _, piece in LABELED_PIECES:
            table_attr_name = f'{metric}_{piece}_table'
            table_df = table_diff(x, piece, metric)
            table_dfs.append(table_df)
            expected_sers.append(pd.Series(getattr(pst, f'{metric}_{piece}_table').flatten(), index=table_df.columns))

        output = pd.concat([output, *table_dfs], axis=1)
        expected = pd.concat([expected, *expected_sers])


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


def fit_all_coefs(x: pd.DataFrame, z: pd.Series, ridge_mult: float) -> pd.Series:
    print('Extracting features for middlegame...')
    x_mg, mg_coef = extract_features_for_metric(x, 'mg')
    print('Extracting features for endgame...')
    x_eg, eg_coef = extract_features_for_metric(x, 'eg')
    print('Extracting features for phase count...')
    x_pc, pc_coef = extract_features_for_metric(x, 'pc')
    pawnless_adj = endgames.better_side_pawnless
    one_pawn_adj = endgames.better_side_one_pawn

    print('Computing evaluation...')
    mg = x_mg @ mg_coef
    eg = x_eg @ eg_coef
    pc = x_pc @ pc_coef
    raw_eval = eg + (mg - eg) * pc / PC_TOTAL

    better_side_pawnless = ((raw_eval > 0) & (x['wt_pawn'] == 0)) | ((raw_eval < 0) & (x['bk_pawn'] == 0))
    better_side_one_pawn = ((raw_eval > 0) & (x['wt_pawn'] == 1)) | ((raw_eval < 0) & (x['bk_pawn'] == 1))
    better_side_only_minor = ((raw_eval > 0) & (x['wt_pawn'] + x['wt_queen'] + x['wt_rook'] == 0) & (x['wt_bishop'] + x['wt_knight'] <= 1)) | \
                             ((raw_eval < 0) & (x['bk_pawn'] + x['bk_queen'] + x['bk_rook'] == 0) & (x['bk_bishop'] + x['bk_knight'] <= 1))
    adjust = pd.Series(1, index=raw_eval.index)
    adjust.loc[better_side_pawnless] = pawnless_adj / 256
    adjust.loc[better_side_one_pawn] = one_pawn_adj / 256
    adjust.loc[better_side_only_minor] = 0
    adjusted_eval = adjust * raw_eval

    print('Computing magic number...')
    magic_number = compute_magic_number(adjusted_eval, z)
    print('Computing win probability error...')
    eval_e = np.exp(adjusted_eval / magic_number)
    win_p_modeled = (eval_e - 1) / (eval_e + 1)
    error = z - win_p_modeled

    print('Setting up regression...')
    win_p_grad = (1 - win_p_modeled ** 2) / (2 * magic_number)
    x_w = pd.concat([
        x_mg.multiply(win_p_grad * pc / PC_TOTAL * adjust, axis=0),
        x_eg.multiply(win_p_grad * (PC_TOTAL - pc) / PC_TOTAL * adjust, axis=0),
        x_pc.multiply(win_p_grad * (mg - eg) / PC_TOTAL * adjust, axis=0),
        pd.DataFrame({
            'better_side_pawnless': win_p_grad * better_side_pawnless * raw_eval / 256,
            'better_side_one_pawn': win_p_grad * better_side_one_pawn * raw_eval / 256,
        }),
    ], axis=1)
    dropped_columns = (x_w == 0).all()
    x_w_keep = x_w.loc[:, ~dropped_columns]

    print('Making custom ridge adjustment...')
    game_w = (x_w_keep ** 2).groupby('game').sum()
    gini = (game_w ** 2).sum() / game_w.sum() ** 2
    xtx = x_w_keep.T @ x_w_keep
    print('Running regression...')
    np.fill_diagonal(xtx.values, np.diagonal(xtx) * (1 + gini.values * ridge_mult))
    innovation = pd.Series(np.linalg.solve(xtx, x_w_keep.T @ error), index=x_w_keep.columns).reindex(x_w.columns, fill_value=0)

    return innovation + pd.concat([mg_coef, eg_coef, pc_coef, pd.Series({'better_side_pawnless': pawnless_adj, 'better_side_one_pawn': one_pawn_adj})])[x_w.columns]


def suggest_new_tables(x: pd.DataFrame, y: pd.Series):
    for piece in ['pawn', 'knight', 'bishop', 'rook', 'queen', 'king']:
        for phase in ['mg', 'eg']:
            table_name = f'{phase}_{piece}_table'
            old_table = getattr(pst, table_name)
            adjustment = fit_pst_adjustment(x, y, piece, phase)
            print_cpp_2d_array_code(table_name, old_table + adjustment)
