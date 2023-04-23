from dataclasses import dataclass
from pathlib import Path

import chess
import numpy as np
import pandas as pd
from tqdm import tqdm

from utils import print_nicely


@dataclass
class MoveOrderingInfo:
    move_to: dict[int, dict[int, float]]
    capture: dict[int, dict[int, float]]
    castle_queenside: float
    castle_kingside: float
    underpromotion: dict[int, float]

    @classmethod
    def from_constant(cls, c: float):
        all_pieces = [chess.PAWN, chess.KNIGHT, chess.BISHOP, chess.ROOK, chess.QUEEN, chess.KING]
        all_squares = range(64)
        return cls(
            move_to={p: {s: c for s in range(64)} for p in all_pieces},
            capture={p: {s: c for s in all_pieces[:-1]} for p in all_pieces},
            castle_queenside=c,
            castle_kingside=c,
            underpromotion={p: c for p in [chess.KNIGHT, chess.BISHOP, chess.ROOK]},
        )


    def get_weight(self, b: chess.Board, m: chess.Move):
        if b.is_queenside_castling(m):
            return self.castle_queenside
        if b.is_kingside_castling(m):
            return self.castle_kingside

        piece = b.piece_at(m.from_square).piece_type
        value = self.move_to[piece][m.to_square ^ (56 if b.turn else 0)]

        if (capture := b.piece_at(m.to_square)) is not None:
            value *= self.capture[piece][capture.piece_type]
        if m.promotion not in [None, chess.QUEEN]:
            value *= self.underpromotion[m.promotion]

        return value

    def increment(self, b: chess.Board, m: chess.Move, value: float):
        if b.is_queenside_castling(m):
            self.castle_queenside += value
            return
        if b.is_kingside_castling(m):
            self.castle_kingside += value
            return

        piece = b.piece_at(m.from_square).piece_type
        self.move_to[piece][m.to_square ^ (56 if b.turn else 0)] += value

        if (capture := b.piece_at(m.to_square)) is not None:
            self.capture[piece][capture.piece_type] += value
        if m.promotion not in [None, chess.QUEEN]:
            self.underpromotion[m.promotion] += value

    def print_cpp_code(self):
        piece_names = ['pawn', 'knight', 'bishop', 'rook', 'queen', 'king']
        for to_table, name in zip(self.move_to.values(), piece_names):
            print('const std::array<int, 64> ' + name + '_freq = {')
            print_nicely((100 * np.log(pd.Series(to_table))).round().fillna(0).astype(int).values.reshape((8, 8)))
            print('};')

        for attacker, dict_ in zip(piece_names, self.capture.values()):
            for victim, freq in zip(piece_names[:-1], dict_.values()):
                print(f'const int {attacker}_capture_{victim}_freq = {int(round(100 * np.log(freq)))};')

        print(f'const int castle_qs_freq = {int(round(100 * np.log(self.castle_queenside)))};')
        print(f'const int castle_ks_freq = {int(round(100 * np.log(self.castle_kingside)))};')

        for up_piece, freq in zip(['knight', 'bishop', 'rook'], self.underpromotion.values()):
            print(f'const int underpromote_to_{up_piece}_freq = {int(round(100 * np.log(freq)))};')


def process_game(formula: MoveOrderingInfo, expectation_counter: MoveOrderingInfo, selection_counter: MoveOrderingInfo, pgn: str):
    b = chess.Board()
    for move_san in pgn.split():
        moves = list(b.legal_moves)
        weights = [formula.get_weight(b, m) for m in moves]
        w_sum = sum(weights)
        for w, m in zip(weights, moves):
            expectation_counter.increment(b, m, w/w_sum)

        chosen_move = b.parse_san(move_san)
        selection_counter.increment(b, chosen_move, 1)
        b.push(chosen_move)


def process_tournament(tournament_name: str, formula: MoveOrderingInfo) -> tuple[MoveOrderingInfo, MoveOrderingInfo]:
    tourney_path = Path('games') / tournament_name
    expectation = MoveOrderingInfo.from_constant(0.0)
    selection = MoveOrderingInfo.from_constant(0.0)

    for game_dir in tqdm(sorted(tourney_path.iterdir())):
        with open(game_dir / 'game.pgn') as f:
            pgn_text = f.read()
        process_game(formula, expectation, selection, pgn_text)

    return expectation, selection


def update(formula: MoveOrderingInfo, expectations: MoveOrderingInfo, selections: MoveOrderingInfo) -> MoveOrderingInfo:
    return MoveOrderingInfo(
        move_to=(pd.DataFrame(formula.move_to) * pd.DataFrame(selections.move_to) / pd.DataFrame(expectations.move_to)).to_dict(),
        capture=(pd.DataFrame(formula.capture) * pd.DataFrame(selections.capture) / pd.DataFrame(expectations.capture)).to_dict(),
        castle_queenside=formula.castle_queenside * selections.castle_queenside / expectations.castle_queenside,
        castle_kingside=formula.castle_kingside * selections.castle_kingside / expectations.castle_kingside,
        underpromotion=(pd.Series(formula.underpromotion) * pd.Series(selections.underpromotion) / pd.Series(expectations.underpromotion)).to_dict(),
    )


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--tournament-name', required=True)
    parser.add_argument('--num-iterations', type=int, default=5)
    options = parser.parse_args()

    formula = MoveOrderingInfo.from_constant(1.0)

    for i in range(options.num_iterations):
        print(f'Starting iteration {i} of {options.num_iterations}')
        exp, sel = process_tournament(options.tournament_name, formula)
        formula = update(formula, exp, sel)

    formula.print_cpp_code()
