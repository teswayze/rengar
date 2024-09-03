from argparse import ArgumentParser
import asyncio
from dataclasses import dataclass
import datetime
from pathlib import Path
import typing as ty

import pandas as pd

from selfplay import compute_bayes_elo, shuffled_openings, TwoPlayer, read_game_result, setup_board, play_game, write_game_output


@dataclass(frozen=True)
class GameSpec:
    opening: str
    main_color: bool


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
        while abs(current_mov) < self._required_mov or n_running > 0:
            while n_running < self._required_mov - abs(current_mov):
                self._game_spec_queue.put_nowait(GameSpec(openings[0], True))
                self._game_spec_queue.put_nowait(GameSpec(openings[0], False))
                openings = openings[1:]
                n_running += 2

            game_result: ty.Literal['W', 'D', 'L'] = await self._game_output_queue.get()
            n_running -= 1
            results_so_far[game_result] += 1
            _log(f'{results_so_far = }')
            current_mov += {'W': 1, 'D': 0, 'L': -1}[game_result]

        for _ in range(self._num_workers):
            self._game_spec_queue.put_nowait('STOP')

        if current_mov > 0:
            _log('Change accepted!')
        else:
            _log('Change rejected')

        main_score = results_so_far['L'] + results_so_far['D'] * 0.5
        branch_score = results_so_far['W'] + results_so_far['D'] * 0.5
        elo_estimate = compute_bayes_elo(pd.Series({'main': main_score, 'branch': branch_score}))['branch']
        _log(f'{elo_estimate = }')
    
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
                matchup = TwoPlayer('main', self._branch_name)
            else:
                matchup = TwoPlayer(self._branch_name, 'main')

            dir_name = opening_name + '-' + str(matchup)
            message = read_game_result(self._output_dir, dir_name)
            if message is None:
                board, partial_pgn = setup_board(move_seq)
                _log(f'Starting game: {dir_name}')
                info, message, pgn = play_game(board, matchup, self._start_time_min, self._increment_sec, partial_pgn)
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
