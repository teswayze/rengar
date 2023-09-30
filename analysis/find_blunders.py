from argparse import ArgumentParser
from pathlib import Path

from chess import engine, pgn, WHITE, BLACK
from tqdm import tqdm


def read_all_games(path: Path) -> list[pgn.Game]:
    print('Loading games...')
    games = []
    done = False
    with open(path) as f:
        while not done:
            game = pgn.read_game(f)
            if game is None:
                done = True
            else:
                games.append(game)
    print(f'Found {len(games)} games')
    return games


def find_mistakes(game: pgn.Game, num_nodes: int) -> list[str]:
    if game.headers.get('White').startswith('Rengar'):
        side = WHITE
    elif game.headers.get('Black').startswith('Rengar'):
        side = BLACK
    else:
        return []

    game_result = game.headers.get('Result')
    if game_result == '1-0':
        last_score = 200 if side else -200
    elif game_result == '0-1':
        last_score = -200 if side else 200
    elif game_result == '1/2-1/2':
        last_score = 0
    else:
        raise ValueError(f'{game_result = }')

    board = game.end().board()
    stockfish = engine.SimpleEngine.popen_uci('stockfish')
    mistake_positions = []

    while len(board.move_stack) > 0:
        board.pop()
        result = stockfish.play(board, engine.Limit(nodes=num_nodes), info=engine.INFO_SCORE)
        score = max(min(result.info['score'].pov(side).score(mate_score=200), 200), -200)
        
        if (board.turn == side) and (score >= last_score + 50):
            mistake_positions.append(board.fen())

        last_score = score

    stockfish.quit()
    return mistake_positions


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--pgn-file', type=Path, required=True)
    parser.add_argument('--output-file', type=Path, required=True)
    parser.add_argument('--sf-nodes', type=int, required=True)
    options = parser.parse_args()

    games = read_all_games(options.pgn_file)
    with open(options.output_file, 'w') as f:
        for game in tqdm(games):
            mistakes = find_mistakes(game, options.sf_nodes)
            for fen in mistakes:
                f.write(fen + '\n')
