from argparse import ArgumentParser
from pathlib import Path

from chess import pgn
from tqdm import tqdm

from utils import root_dir


def migrate_game(game_path: Path):
    with open(game_path / 'game.pgn') as pgn_file:
        game = pgn.read_game(pgn_file)

    with open(game_path / 'game.uci', 'w') as uci_file:
        for move in game.mainline_moves():
            uci_file.write(move.uci() + ' ')


def migrate_tournaments(*tournament_names: str):
    all_game_dirs = []
    for tournament_name in tournament_names:
        tourney_path = root_dir() / 'games' / tournament_name
        all_game_dirs.extend(tourney_path.iterdir())

    for game_dir in tqdm(all_game_dirs):
        migrate_game(game_dir)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--tournaments', nargs='+')
    options = parser.parse_args()

    assert options.tournaments, "No tournaments to train! Use --tournaments argument"

    migrate_tournaments(*options.tournaments)
