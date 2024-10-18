from argparse import ArgumentParser
import asyncio
from dataclasses import dataclass
import datetime
import hashlib
import math
import random
from pathlib import Path
import typing as ty

from chess import Board, Move, engine
import pandas as pd


class ChessClock:
    def __init__(self, start_time_min: float, increment_sec: float):
        self._white_time = start_time_min * 60
        self._black_time = start_time_min * 60
        self._increment = increment_sec

    def get_limit(self) -> engine.Limit:
        return engine.Limit(
            white_clock=self._white_time,
            black_clock=self._black_time,
            white_inc=self._increment,
            black_inc=self._increment,
        )

    def update_clock(self, turn: bool, ms_elapsed: int):
        if turn:
            self._white_time -= ms_elapsed - self._increment
        else:
            self._black_time -= ms_elapsed - self._increment

    def is_flag_up(self) -> bool:
        return self._white_time < self._increment or self._black_time < self._increment


async def play_move(board: Board, engine_: engine.Protocol, clock: ChessClock) -> pd.Series:
    fen = board.fen()
    result = await engine_.play(board, clock.get_limit(), info=engine.INFO_ALL)
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


def write_game_output(output_dir: Path, game_name: str, info: pd.DataFrame, message: str, pgn: str):
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


def compute_elo_diff(score_pct: float) -> float:
    if score_pct == 0.0:
        return -math.inf
    if score_pct == 1.0:
        return math.inf
    return -400 * math.log10(1 / score_pct - 1)


def shuffled_openings(openings_path: Path, seed_obj):
    with open(openings_path) as f:
        openings = f.readlines()
    random.seed(int(hashlib.shake_128(str(seed_obj).encode()).hexdigest(4), base=16))
    random.shuffle(openings)
    return openings


@dataclass(frozen=True)
class GameSpec:
    opening: str
    main_color: bool


@dataclass
class EngineCrash:
    error: RuntimeError
    crashing_engine: str
    fen: str


class SprtRunner:
    def __init__(self):
        parser = ArgumentParser()
        parser.add_argument('--output-dir', required=True)
        parser.add_argument('--branch-name', required=True)
        parser.add_argument('--book', default='J')
        parser.add_argument('--start-time-min', type=float, default=2.0)
        parser.add_argument('--increment-sec', type=float, default=1.0)
        parser.add_argument('--required-mov', type=int, default=25)
        parser.add_argument('--num-workers', type=int, default=1)
        options = parser.parse_args()

        self._openings_path = Path('openings') / f'{options.book}.book'
        self._output_dir = Path('games') / options.output_dir
        self._branch_name = options.branch_name
        self._start_time_min = options.start_time_min
        self._increment_sec = options.increment_sec
        self._required_mov = options.required_mov
        self._num_workers = options.num_workers

        self._game_spec_queue = asyncio.Queue()
        self._game_output_queue = asyncio.Queue()

    async def driver(self):

        def _log(message: str):
            print(datetime.datetime.now().strftime("%H:%M:%S") + '\t(driver)\t' + message)

        _log(f'Beginning SPRT for {self._branch_name}')
        _log(f'TC: {self._start_time_min}+{self._increment_sec}')
        n_running = 0
        results_so_far = {'W': 0, 'D': 0, 'L': 0}
        current_mov = 0
        openings = shuffled_openings(self._openings_path, self._output_dir)
        any_crash = False
        while n_running > 0 or (abs(current_mov) < self._required_mov and not any_crash and len(openings) > 0):
            while not any_crash and n_running < self._required_mov - abs(current_mov) and len(openings) > 0:
                self._game_spec_queue.put_nowait(GameSpec(openings[0], True))
                self._game_spec_queue.put_nowait(GameSpec(openings[0], False))
                openings = openings[1:]
                n_running += 2

            _log(f'{n_running = }')
            game_result: ty.Literal['W', 'D', 'L'] | EngineCrash = await self._game_output_queue.get()
            n_running -= 1
            if isinstance(game_result, EngineCrash):
                any_crash = True
                try:
                    while True:
                        self._game_spec_queue.get_nowait()
                        n_running -= 1
                except asyncio.QueueEmpty:
                    pass
            else:
                results_so_far[game_result] += 1
                _log(f'{results_so_far = }')
                current_mov += {'W': 1, 'D': 0, 'L': -1}[game_result]

        for _ in range(self._num_workers):
            self._game_spec_queue.put_nowait('STOP')
        
        if any_crash:
            return

        if current_mov > 0:
            _log('Change accepted!')
        else:
            _log('Change rejected')

        branch_score = (results_so_far['W'] + results_so_far['D'] * 0.5) / sum(results_so_far.values())
        elo_estimate = compute_elo_diff(branch_score)
        _log(f'{elo_estimate = }')

    async def play_game(self, move_seq: str, main_color: bool) -> EngineCrash | tuple[pd.DataFrame, str, str]:
        board, pgn = setup_board(move_seq)

        _, main_engine = await engine.popen_uci('./bin/main/uci')
        _, branch_engine = await engine.popen_uci(f'./bin/{self._branch_name}/uci')
        if main_color:
            white = main_engine
            black = branch_engine
        else:
            white = branch_engine
            black = main_engine
        
        clock = ChessClock(self._start_time_min, self._increment_sec)
        info = []
        game_over_message = None
        while game_over_message is None:
            try:
                move_info = await play_move(board, white if board.turn else black, clock)
            except (engine.EngineError, engine.EngineTerminatedError) as error:
                if isinstance(error, engine.EngineError):
                    await white.quit()
                    await black.quit()
                else:
                    await (black if board.turn else white).quit()
                crashing_engine = 'main' if main_color == board.turn else self._branch_name
                return EngineCrash(error, crashing_engine, board.fen())
            if clock.is_flag_up():
                await white.quit()
                await black.quit()
                crashing_engine = 'main' if main_color == board.turn else self._branch_name
                error = RuntimeError(
                    'Engine timed out!'
                    f'\nmove = {info["move"]}\n'
                    f'\ndepth = {info["depth"]}\n'
                    f'\ntime = {info["time"]}\n'
                    f'\nnodes = {info["nodes"]}\n'
                    f'\nscore = {info["score"]}\n'
                )
                return EngineCrash(error, crashing_engine, board.fen())

            info.append(move_info)
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

        await white.quit()
        await black.quit()
        return pd.DataFrame(info), game_over_message, pgn

    
    async def worker(self, id: int):
        
        def _log(message: str):
            print(datetime.datetime.now().strftime("%H:%M:%S") + '\t' + f'(worker {id})' + '\t' + message)

        while True:
            game_spec: GameSpec | ty.Literal['STOP'] = await self._game_spec_queue.get()
            if game_spec == 'STOP':
                return
            
            move_seq, opening_name = game_spec.opening.split('|')
            move_seq = move_seq.rstrip()
            opening_name = opening_name.lstrip().rstrip()

            if game_spec.main_color:
                dir_name = opening_name + '-main-vs-' + self._branch_name
            else:
                dir_name = opening_name + '-' + self._branch_name + '-vs-main'

            message = read_game_result(self._output_dir, dir_name)
            if message is None:
                _log(f'Starting game: {dir_name}')
                game_result = await self.play_game(move_seq, game_spec.main_color)
                if isinstance(game_result, EngineCrash):
                    _log(f'{game_result.crashing_engine} crashed on {game_result.fen}:\n{game_result.error}')
                    self._game_output_queue.put_nowait(game_result)
                    return
                info, message, pgn = game_result
                write_game_output(self._output_dir, dir_name, info, message, pgn)
            
            _log(f'{dir_name}: {message}')
            if message.startswith('White won'):
                self._game_output_queue.put_nowait('L' if game_spec.main_color else 'W')
            elif message.startswith('Black won'):
                self._game_output_queue.put_nowait('W' if game_spec.main_color else 'L')
            elif message.startswith('Draw'):
                self._game_output_queue.put_nowait('D')
            else:
                raise ValueError(message)

    async def main(self):
        driver = self.driver()
        workers = [self.worker(i) for i in range(self._num_workers)]
        await asyncio.gather(driver, *workers)


if __name__ == '__main__':
    asyncio.run(SprtRunner().main())
