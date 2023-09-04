import datetime
from abc import ABC, abstractmethod
from argparse import ArgumentParser
from pathlib import Path
from random import shuffle, seed

from chess import Board, Move
import chess.engine as ce
import numpy as np
import pandas as pd


class ChessClock:
    def __init__(self, start_time_min: float, increment_sec: float):
        self._white_time = start_time_min * 60
        self._black_time = start_time_min * 60
        self._increment = increment_sec

    def get_limit(self, node_limit: int | None) -> ce.Limit:
        return ce.Limit(
            white_clock=self._white_time,
            black_clock=self._black_time,
            white_inc=self._increment,
            black_inc=self._increment,
            nodes=node_limit,
        )

    def update_clock(self, turn: bool, ms_elapsed: int):
        if turn:
            self._white_time -= ms_elapsed - self._increment
        else:
            self._black_time -= ms_elapsed - self._increment

    def is_flag_up(self) -> bool:
        return self._white_time < self._increment or self._black_time < self._increment


def play_move(board: Board, engine: ce.SimpleEngine, clock: ChessClock, node_limit: int | None) -> pd.Series:
    fen = board.fen()
    result = engine.play(board, clock.get_limit(node_limit), info=ce.INFO_ALL)
    time_ms = result.info['time']
    clock.update_clock(board.turn, time_ms)
    pov_score = result.info['score']
    score = pov_score.pov(pov_score.turn)
    move_san = board.san(result.move)
    board.push(result.move)
    return pd.Series({
        'fen': fen,
        'move': move_san,
        'depth': result.info['depth'],
        'time': time_ms,
        'nodes': result.info['nodes'],
        'score': score.score(mate_score=100000),
    })


class Matchup(ABC):
    @abstractmethod
    def initialize_engines(self):
        pass

    @abstractmethod
    def white(self) -> ce.SimpleEngine:
        pass

    @abstractmethod
    def black(self) -> ce.SimpleEngine:
        pass

    def node_limit(self, turn: bool) -> int | None:
        return None

    @abstractmethod
    def quit(self):
        pass


class Selfplay(Matchup):
    def __init__(self, branch: str):
        self._branch = branch
        self._engine = None

    def initialize_engines(self):
        if self._engine is not None:
            raise RuntimeError("Engines already initialized")
        self._engine = ce.SimpleEngine.popen_uci('./bin/' + self._branch + '/uci')

    def white(self) -> ce.SimpleEngine:
        if self._engine is None:
            raise RuntimeError("Must initialize engines first")
        return self._engine

    def black(self) -> ce.SimpleEngine:
        if self._engine is None:
            raise RuntimeError("Must initialize engines first")
        return self._engine

    def quit(self):
        self.white().quit()
        self._engine = None

    def __str__(self):
        return f'{self._branch}-selfplay'


class TwoPlayer(Matchup):
    def __init__(self, white_branch: str, black_branch: str):
        self.white_branch = white_branch
        self.black_branch = black_branch
        self._white_engine = None
        self._black_engine = None

    def initialize_engines(self):
        if (self._white_engine is not None) or (self._black_engine is not None):
            raise RuntimeError("Engines already initialized")
        self._white_engine = ce.SimpleEngine.popen_uci('./bin/' + self.white_branch + '/uci')
        self._black_engine = ce.SimpleEngine.popen_uci('./bin/' + self.black_branch + '/uci')

    def white(self) -> ce.SimpleEngine:
        if self._white_engine is None:
            raise RuntimeError("Must initialize engines first")
        return self._white_engine

    def black(self) -> ce.SimpleEngine:
        if self._black_engine is None:
            raise RuntimeError("Must initialize engines first")
        return self._black_engine

    def quit(self):
        self.white().quit()
        self.black().quit()
        self._white_engine = None
        self._black_engine = None

    def __str__(self):
        return f'{self.white_branch}-vs-{self.black_branch}'


class RengarVsStockfish(Matchup):
    def __init__(self, sf_side: bool, sf_nodes: int):
        self.sf_side = sf_side
        self.sf_nodes = sf_nodes
        self._rengar = None
        self._stockfish = None

    def initialize_engines(self):
        if (self._rengar is not None) or (self._stockfish is not None):
            raise RuntimeError("Engines already initialized")
        self._rengar = ce.SimpleEngine.popen_uci('./uci')
        self._stockfish = ce.SimpleEngine.popen_uci('stockfish')

    def white(self) -> ce.SimpleEngine:
        engine = self._stockfish if self.sf_side else self._rengar
        if engine is None:
            raise RuntimeError("Must initialize engines first")
        return engine

    def black(self) -> ce.SimpleEngine:
        engine = self._rengar if self.sf_side else self._stockfish
        if engine is None:
            raise RuntimeError("Must initialize engines first")
        return engine

    def node_limit(self, turn: bool) -> int | None:
        if turn == self.sf_side:
            return self.sf_nodes
        return None

    def quit(self):
        self.white().quit()
        self.black().quit()
        self._rengar = None
        self._stockfish = None

    def __str__(self):
        if self.sf_nodes < 1_000_000:
            nodes_str = str(self.sf_nodes // 1_000) + 'k'
        else:
            nodes_str = str(self.sf_nodes // 1_000_000) + 'M'

        if self.sf_side:
            return 'stockfish' + nodes_str + '-vs-rengar'
        return 'rengar-vs-stockfish' + nodes_str


def play_game(board: Board, matchup: Matchup, start_time_min: float, increment_sec: float, pgn: str) -> tuple[pd.DataFrame, str, str]:
    matchup.initialize_engines()
    info = []
    game_over_message = None
    clock = ChessClock(start_time_min, increment_sec)
    while game_over_message is None:
        info.append(play_move(board, matchup.white() if board.turn else matchup.black(), clock, matchup.node_limit(board.turn)))
        pgn = pgn + ' ' + info[-1]['move']

        if clock.is_flag_up():
            if board.turn:
                game_over_message = 'White won by timeout'
            else:
                game_over_message = 'Black won by timeout'
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

    matchup.quit()
    return pd.DataFrame(info), game_over_message, pgn


def write_game_output(output_dir: Path, game_name: str, info: pd.DataFrame, message: str, pgn: str):
    print(f'{datetime.datetime.now().strftime("%H:%M:%S")} {game_name}: {message}')
    path = output_dir / game_name
    path.mkdir(exist_ok=True, parents=True)
    info.to_csv(path / 'info.csv')
    with open(path / 'game.pgn', 'w') as g:
        g.write(pgn)
    with open(path / 'result.txt', 'w') as f:
        f.write(message)


def read_game_result(output_dir: Path, game_name: str) -> str | None:
    file_path = output_dir / game_name / 'result.txt'
    if file_path.exists():
        with open(file_path) as f:
            return f.read()
    return None


def setup_board(move_seq: str) -> tuple[Board, str]:
    board = Board()
    moves_san = []
    for move_str in move_seq.split():
        move = Move.from_uci(move_str)
        moves_san.append(board.san(move))
        board.push(move)
    return board, ' '.join(moves_san)


def compute_score_stats(wdl_dict: dict[str, int]) -> pd.Series:
    w = wdl_dict['Win']
    d = wdl_dict['Draw']
    l = wdl_dict['Loss']
    return pd.Series({
        'Score': (w + d / 2),
        'T-Stat': float('nan') if (w + l == 0) else round((w - l) / (w + l) ** 0.5, 2),
        **wdl_dict,
    })


def compute_bayes_elo(scores: pd.Series) -> pd.Series:
    score_pct = scores / (2 * scores.mean())
    n = len(scores)
    weights = pd.Series(1, index=scores.index)
    for _ in range(10):
        expected = weights.apply(lambda x: ((x / (x + weights)).sum() - 0.5) / (n-1))
        weights = weights * score_pct / expected
    elo_unnorm = np.log10(weights) * 400
    return (elo_unnorm - elo_unnorm.iloc[0]).round()
    

def play_tournament(openings_path: Path, output_dir: Path, start_time_min: float, increment_sec: float, players: list[str], sf_nodes: int | None):
    with open(openings_path) as f:
        openings = f.readlines()
    seed(0)
    shuffle(openings)

    seen = set()
    for opn in openings:
        name = opn.split('|')[1].lstrip().rstrip()
        assert name not in seen, f"Duplicate opening: {name}"
        seen.add(name)

    if sf_nodes is not None:
        matchups = [RengarVsStockfish(True, sf_nodes), RengarVsStockfish(False, sf_nodes)]
        scores = {engine: {'Win': 0, 'Draw': 0, 'Loss': 0} for engine in str(matchups[0]).split('-vs-')}
    elif len(players) == 1:
        matchups = [Selfplay(players[0])]
        scores = {color: {'Win': 0, 'Draw': 0, 'Loss': 0} for color in ['White', 'Black']}
    else:
        matchups = [TwoPlayer(x, y) for x in players for y in players if x != y]
        scores = {player: {'Win': 0, 'Draw': 0, 'Loss': 0} for player in players}

    for opening in openings:
        move_seq, opening_name = opening.split('|')
        move_seq = move_seq.rstrip()
        opening_name = opening_name.lstrip().rstrip()

        for matchup in matchups:
            dir_name = opening_name + '-' + str(matchup)
            message = read_game_result(output_dir, dir_name)
            if message is None:
                board, partial_pgn = setup_board(move_seq)
                info, message, pgn = play_game(board, matchup, start_time_min, increment_sec, partial_pgn)
                write_game_output(output_dir, dir_name, info, message, pgn)

            if isinstance(matchup, Selfplay):
                w, b = 'White', 'Black'
            else:
                w, b = str(matchup).split('-vs-')

            if message == 'White won by checkmate':
                scores[w]['Win'] += 1
                scores[b]['Loss'] += 1
            elif message == 'Black won by checkmate':
                scores[w]['Loss'] += 1
                scores[b]['Win'] += 1
            else:
                scores[w]['Draw'] += 1
                scores[b]['Draw'] += 1

        df = pd.DataFrame({k: compute_score_stats(v) for k, v in scores.items()}).T
        df['Elo'] = compute_bayes_elo(df['Score'])
        print(df.sort_values('Score', ascending=False))


def main():
    parser = ArgumentParser()
    parser.add_argument('--book', required=True)
    parser.add_argument('--start-time-min', type=float, required=True)
    parser.add_argument('--increment-sec', type=float, required=True)
    parser.add_argument('--output-dir', required=True)
    parser.add_argument('--players', default=['main'], nargs='+')
    parser.add_argument('--sf-nodes', type=int)
    options = parser.parse_args()

    play_tournament(Path('openings') / f'{options.book}.book', Path('games') / options.output_dir, options.start_time_min, options.increment_sec, options.players, options.sf_nodes)


if __name__ == '__main__':
    main()
