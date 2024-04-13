import json
import random
from argparse import ArgumentParser
from time import sleep

import chess
from urllib.request import urlopen
from urllib.error import HTTPError

from tqdm import tqdm


def generate_pseudo_random_fen(endgame: str) -> str:
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
        .replace('....', '4').replace('...', '3').replace('..', '2').replace('.', '1') + ' w - - 0 1'
    
    return fen


def accept_fen_for_mate_test(fen: str, min_dtz: int) -> bool:
    board = chess.Board(fen)
    if not board.is_valid():
        return False  # Illegal position
    
    url = 'http://tablebase.lichess.ovh/standard?fen=' + fen.replace(' ', '_')
    try:
        api_result = json.loads(urlopen(url).read())
    except HTTPError as e:
        if e.code != 429:
            raise e
        sleep(60)
        api_result = json.loads(urlopen(url).read())
    if api_result['category'] != 'win':
        return False  # Position is not actually winning

    if api_result['dtz'] < min_dtz:
        return False  # Conversion is too easy

    return True


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--endgame', required=True)
    parser.add_argument('--min-dtz', type=int, required=True)
    parser.add_argument('--num-positions', type=int, default=100)
    parser.add_argument('--seed', type=int, default=0)
    options = parser.parse_args()

    fens = []
    random.seed(options.seed)
    progress = tqdm(total=options.num_positions)

    while len(fens) < options.num_positions:
        fen = generate_pseudo_random_fen(options.endgame)
        if accept_fen_for_mate_test(fen, options.min_dtz):
            fens.append(fen)
            progress.update()
    
    with open(f'mate_test_fens/{options.endgame}.txt', 'w') as f:
        f.write('\n'.join(fens))
