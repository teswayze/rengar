from __future__ import annotations

import asyncio
import datetime
from argparse import ArgumentParser
from dataclasses import dataclass
from pathlib import Path

from chess import engine
import numpy as np
import pandas as pd

from sprt_async import shuffled_openings, setup_board, ChessClock, play_move


@dataclass
class SpsaTrial:
    params_white: dict[str, float]
    params_black: dict[str, float]
    perturbation: np.ndarray

    def mirror(self) -> SpsaTrial:
        return SpsaTrial(
            params_white=self.params_black,
            params_black=self.params_white,
            perturbation=1 / self.perturbation,
        )



class SimpleOptimizer:
    def __init__(
        self, 
        params: dict[str, dict[str, float]],
        max_ratio: float = 1.25,
        learning_rate: float = 0.1,
    ):
        df = pd.DataFrame(params)
        self.param_names = list(df.columns)
        self.param_values = df.loc['default'].values
        self.param_min = df.loc['min'].values
        self.param_max = df.loc['max'].values

        self.step = max_ratio
        self.lr = learning_rate

    def generate_trial(self) -> SpsaTrial:
        diff = self.step ** np.random.uniform(-1, 1, size=len(self.param_names))
        params_white = np.clip(self.param_values * diff, self.param_min, self.param_max).round().astype(int)
        params_black = np.clip(self.param_values / diff, self.param_min, self.param_max).round().astype(int)
        trial = SpsaTrial(
            params_white=dict(zip(self.param_names, params_white)),
            params_black=dict(zip(self.param_names, params_black)),
            perturbation=diff ** self.lr,
        )
        return trial

    def record_trial(self, trial: SpsaTrial, result: float):
        self.param_values = np.clip(self.param_values * trial.perturbation ** result, self.param_min, self.param_max)

    def current_params(self) -> dict[str, float]:
        return dict(zip(self.param_names, self.param_values.round().astype(int)))


class TuningRunner:
    def __init__(self):
        parser = ArgumentParser()
        parser.add_argument('--book', default='J')
        parser.add_argument('--start-time-min', type=float, default=2.0)
        parser.add_argument('--increment-sec', type=float, default=1.0)
        parser.add_argument('--num-workers', type=int, default=1)
        parser.add_argument('--uci-option', nargs='+', required=True)
        options = parser.parse_args()

        self._openings_path = Path('openings') / f'{options.book}.book'
        self._start_time_min = options.start_time_min
        self._increment_sec = options.increment_sec
        self._num_workers = options.num_workers

        rengar = engine.SimpleEngine.popen_uci('./uci')
        uci_options = pd.DataFrame(list(rengar.options.values())).set_index('name')[['default', 'min', 'max']].T.to_dict()
        tuning_options = {k: v for k, v in uci_options.items() if any(k.startswith(prefix) for prefix in options.uci_option)}
        assert len(tuning_options) > 0, f"No options matched {options.uci_option}\nAvailable: {list(uci_options)}"
        self.optimizer = SimpleOptimizer(tuning_options)

        self._opening_queue = asyncio.Queue()
        self._mirror_queue = asyncio.Queue()

    async def main(self):

        def _log(message: str):
            print(datetime.datetime.now().strftime("%H:%M:%S") + '\t(main)\t' + message)

        _log(f'Tuning {len(self.optimizer.param_names)} options:\n\t' + '\n\t'.join(self.optimizer.param_names))
        _log(f'TC: {self._start_time_min}+{self._increment_sec}')
        openings = shuffled_openings(self._openings_path, str(datetime.datetime.now()))
        for opening in openings:
            self._opening_queue.put_nowait(opening)

        workers = [self.worker(i) for i in range(self._num_workers)]
        await asyncio.gather(*workers)

    async def worker(self, id: int):
        
        def _log(message: str):
            print(datetime.datetime.now().strftime("%H:%M:%S") + '\t' + f'(worker {id})' + '\t' + message)

        while True:
            try:
                opening, trial = self._mirror_queue.get_nowait()
            except asyncio.QueueEmpty:
                try:
                    opening = self._opening_queue.get_nowait()
                except asyncio.QueueEmpty:
                    return
                trial = self.optimizer.generate_trial()
                self._mirror_queue.put_nowait((opening, trial.mirror()))
            
            move_seq, opening_name = opening.split('|')
            move_seq = move_seq.rstrip()
            opening_name = opening_name.lstrip().rstrip()

            _log(f'Starting game from opening {opening_name}')
            game_result = await self.play_game(move_seq, trial)
            _log(game_result)
            
            if game_result.startswith('White won'):
                self.optimizer.record_trial(trial, 1.0)
                _log(f'Best params: {self.optimizer.current_params()}')
            elif game_result.startswith('Black won'):
                self.optimizer.record_trial(trial, -1.0)
                _log(f'Best params: {self.optimizer.current_params()}')
            elif not game_result.startswith('Draw'):
                raise ValueError(game_result)

    async def play_game(self, move_seq: str, trial: SpsaTrial) -> str:
        board, _ = setup_board(move_seq)

        _, white = await engine.popen_uci('./uci')
        await white.configure(trial.params_white)
        _, black = await engine.popen_uci('./uci')
        await black.configure(trial.params_black)
        
        clock = ChessClock(self._start_time_min, self._increment_sec)
        game_over_message = None
        error = None
        while game_over_message is None:
            try:
                move_info = await play_move(board, white if board.turn else black, clock)
                if clock.is_flag_up():
                    error = RuntimeError(
                        'Engine timed out!'
                        f'\nmove = {move_info["move"]}\n'
                        f'\ndepth = {move_info["depth"]}\n'
                        f'\ntime = {move_info["time"]}\n'
                        f'\nnodes = {move_info["nodes"]}\n'
                        f'\nscore = {move_info["score"]}\n'
                    )
            except (engine.EngineError, engine.EngineTerminatedError) as error:
                pass
            if error is not None:
                if isinstance(error, engine.EngineTerminatedError):
                    await (black if board.turn else white).quit()
                else:
                    await white.quit()
                    await black.quit()
                winning_engine = 'Black' if board.turn else 'White'
                crashing_params = trial.params_white if board.turn else trial.params_black
                print(f'ERROR: crashed on {board.fen()} using params {crashing_params}')
                print(error)
                return winning_engine + ' won by crash'

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
        return game_over_message


if __name__ == '__main__':
    asyncio.run(TuningRunner().main())
