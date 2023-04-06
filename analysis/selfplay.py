from pathlib import Path
import datetime
from argparse import ArgumentParser

from chess import Board, Move
import chess.engine as ce
import pandas as pd


def play_move(board: Board, engine: ce.SimpleEngine, limit: ce.Limit) -> pd.Series:
    fen = board.fen()
    result = engine.play(board, limit, info=ce.INFO_ALL)
    pov_score = result.info['score']
    score = pov_score.pov(pov_score.turn)
    move_san = board.san(result.move)
    board.push(result.move)
    return pd.Series({
        'fen': fen,
        'move': move_san,
        'depth': result.info['depth'],
        'time': result.info['time'],
        'nodes': result.info['nodes'],
        'score': score.score(mate_score=100000),
    })


def play_game(board: Board, white_engine: ce.SimpleEngine, black_engine: ce.SimpleEngine, limit: ce.Limit, pgn: str) -> tuple[pd.DataFrame, str, str]:
    info = []
    game_over_message = None
    while game_over_message is None:
        info.append(play_move(board, white_engine if board.turn else black_engine, limit))
        pgn = pgn + ' ' + info[-1]['move']
        
        if board.is_checkmate():
            if board.turn:
                game_over_message = 'Black won by checkmate'
            else:
                game_over_message = 'White won by checkmate'
        if board.is_stalemate():
            game_over_message = 'Draw by stalemate'
        if board.is_insufficient_material():
            game_over_message = 'Draw by insufficient material'
        if board.is_repetition():
            game_over_message = 'Draw by threefold repetition'
        if board.is_fifty_moves():
            game_over_message = 'Draw by fifty move rule'

    return pd.DataFrame(info), game_over_message, pgn


def write_game_output(output_dir: Path, game_name: str, info: pd.DataFrame, message: str, pgn: str):
    print(f'{datetime.datetime.now().strftime("%H:%M:%S")} {game_name}: {message}')
    path = output_dir / game_name
    path.mkdir(exist_ok=True, parents=True)
    info.to_csv(path / 'info.csv')
    with open(path / 'result.txt', 'w') as f:
        f.write(message)
    with open(path / 'game.pgn', 'w') as g:
        g.write(pgn)


def setup_board(move_seq: str) -> tuple[Board, str]:
    board = Board()
    moves_san = []
    for move_str in move_seq.split():
        move = Move.from_uci(move_str)
        moves_san.append(board.san(move))
        board.push(move)
    return board, ' '.join(moves_san)


def play_match(openings_path: Path, output_dir: Path, node_limit: int, exec_1: str, exec_2: str | None = None):
    limit = ce.Limit(nodes=node_limit)
    with open(openings_path) as f:
        openings = f.readlines()

    for opening in openings:
        move_seq, opening_name = opening.split('|')
        move_seq = move_seq.rstrip()
        opening_name = opening_name.lstrip().rstrip()

        board, partial_pgn = setup_board(move_seq)
        white = ce.SimpleEngine.popen_uci(exec_1)
        black = white if exec_2 is None else ce.SimpleEngine.popen_uci(exec_2)
        info, message, pgn = play_game(board, white, black, limit, partial_pgn)
        write_game_output(output_dir, opening_name, info, message, pgn)
        white.quit()

        if exec_2 is not None:
            black.quit()

            board, partial_pgn = setup_board(move_seq)
            white = ce.SimpleEngine.popen_uci(exec_2)
            black = ce.SimpleEngine.popen_uci(exec_1)
            info, message, pgn = play_game(board, white, black, limit, partial_pgn)
            write_game_output(output_dir, opening_name + '-reverse', info, message)
            white.quit()
            black.quit()


def main():
    parser = ArgumentParser()
    parser.add_argument('--book', required=True)
    parser.add_argument('--nodes', type=int, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    options = parser.parse_args()

    play_match(Path('openings') / f'{options.book}.book', Path(options.output_dir), options.nodes, './uci')


if __name__ == '__main__':
    main()
