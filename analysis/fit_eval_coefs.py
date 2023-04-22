from pathlib import Path

import numpy as np
import pandas as pd
from chess import Board
from tqdm import tqdm

from translate_constants import translate_constants, Constants


def mask_to_bool_array(mask: int) -> np.ndarray:
    return np.array([bool(mask & (1<<n)) for n in range(64)]).reshape(8, 8)


def flip_if_white(board: np.ndarray, color: str):
    if color == 'W':
        return board[::-1]
    if color == 'B':
        return board
    raise ValueError(color)


def features_from_fen(fen: str, constants: Constants) -> pd.Series:
    b = Board(fen)
    labeled_pieces = [('P', 'pawn'), ('N', 'knight'), ('B', 'bishop'), ('R', 'rook'), ('Q', 'queen'), ('K', 'king')]

    masks = {
        f'{color_name}{label}': mask_to_bool_array(b.occupied_co[color_index] & getattr(b, piece + 's'))
        for color_name, color_index in [('W', True), ('B', False)]
        for label, piece in labeled_pieces
    }

    counts = {f'#{label}': masks[label].sum() for label in ['WP', 'BP', 'WN', 'BN', 'WB', 'BB', 'WR', 'BR', 'WQ', 'BQ']}
    tables = {
        f'{color}{label}{phase[0].upper()}T': np.sum(masks[f'{color}{label}'] * flip_if_white(constants.board_arrays[f'{phase}_{piece}_table'], color))
        for label, piece in labeled_pieces
        for color in 'WB'
        for phase in ['mg', 'eg']
    }

    return pd.Series({**counts, **tables})


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
    
    constants = translate_constants('includes/pst.hpp')
    x = pd.DataFrame({row.name: features_from_fen(row['fen'], constants) for _, row in quiet.iterrows()}).T
    y = quiet['score'] * quiet['fen'].apply(side_to_move_sign)

    return x, y


def get_data_for_fitting(tournament_name: str):
    tourney_path = Path('games') / tournament_name
    xs = []
    ys = []
    for game_dir in tqdm(list(tourney_path.iterdir())):
        x, y = load_features_and_target_for_game(game_dir / 'info.csv')
        xs.append(x)
        ys.append(y)

    return xs, ys
