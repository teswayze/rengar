import random
from argparse import ArgumentParser

import pandas as pd
from chess import Board
from chess.syzygy import Tablebase

from tqdm import tqdm


def generate_pseudo_random_fen(endgame: str, color: str = 'w') -> str:
    board_chars = ['.' for _ in range(64)]
    for piece in endgame:
        collision = '-'
        idx = -1
        while collision != '.':
            idx = random.randint(0, 63)
            collision = board_chars[idx]
        board_chars[idx] = piece
    
    fen = '/'.join(''.join(board_chars[8*i:8*(i+1)]) for i in range(8)) \
        .replace('........', '8').replace('.......', '7').replace('......', '6').replace('.....', '5') \
        .replace('....', '4').replace('...', '3').replace('..', '2').replace('.', '1') + ' ' + color + ' - - 0 1'
    
    return fen


def accept_fen_for_mate_test(fen: str, min_dtz: int) -> bool:
    board = Board(fen)
    if not board.is_valid():
        return False  # Illegal position
    
    tb = Tablebase()
    tb.add_directory('/Users/thomasswayze/syzygy')
    if tb.probe_wdl(board) == 2:
        return False  # Position is not actually winning
    if tb.probe_dtz(board) < min_dtz:
        return False  # Conversion is too easy

    return True


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--endgame', required=True)
    parser.add_argument('--min-dtz', type=int, default=10)
    parser.add_argument('--num-positions', type=int, default=100)
    parser.add_argument('--seed', type=int, default=0)
    parser.add_argument('--syzygy-test', action='store_true')
    options = parser.parse_args()

    fens = []
    random.seed(options.seed)
    progress = tqdm(total=options.num_positions)
    stronger, weaker = options.endgame.split('v')
    forward = stronger + weaker.lower()
    mirrored = weaker + stronger.lower()

    if options.syzygy_test:
        tb = Tablebase()
        tb.add_directory('/Users/thomasswayze/syzygy')

        while len(fens) < options.num_positions:
            fen = generate_pseudo_random_fen(random.choice([forward, mirrored]), random.choice('wb'))
            board = Board(fen)
            if board.is_valid():
                wdl = tb.probe_wdl(board)
                dtz = tb.probe_dtz(board)
                fens.append({'fen': fen, 'wdl': wdl, 'dtz': dtz})
                progress.update()
        
        pd.DataFrame(fens).to_csv(f'syzygy_test/{options.endgame}.csv', index=False)
    else:
        while len(fens) < options.num_positions:
            fen = generate_pseudo_random_fen(forward)
            if accept_fen_for_mate_test(fen, options.min_dtz):
                fens.append(fen)
                progress.update()
        
        with open(f'mate_test_fens/{options.endgame}.txt', 'w') as f:
            f.write('\n'.join(fens))
