from argparse import ArgumentParser
from dataclasses import dataclass
from pathlib import Path

import chess
import numpy as np
import pandas as pd
from tqdm import tqdm

from utils import print_cpp_2d_array_code, print_cpp_constant_code


def _transform_mult_to_additive(x: np.ndarray) -> np.ndarray:
    return np.round(100 * np.clip(np.nan_to_num(np.log(x)), -10, 10)).astype(int)


def _attacker_set(board: chess.Board, square: chess.Square, color: chess.Color) -> set[int]:
    attackers = board.attackers(color, square)
    return set(board.piece_at(a).piece_type for a in attackers)


@dataclass
class MoveOrderingInfo:
    move_to: dict[int, dict[int, float]]
    capture: dict[int, dict[int, float]]
    castle_queenside: float
    castle_kingside: float
    en_passant: float
    underpromotion: dict[int, float]
    guard: dict[int, dict[int, float]]
    support: dict[int, dict[int, float]]

    @classmethod
    def from_constant(cls, c: float):
        all_pieces = [chess.PAWN, chess.KNIGHT, chess.BISHOP, chess.ROOK, chess.QUEEN, chess.KING]
        all_squares = range(64)
        return cls(
            move_to={p: {s: c for s in range(64)} for p in all_pieces},
            capture={p: {s: c for s in all_pieces[:-1]} for p in all_pieces},
            castle_queenside=c,
            castle_kingside=c,
            en_passant=c,
            underpromotion={p: c for p in [chess.KNIGHT, chess.BISHOP, chess.ROOK]},
            guard={p: {s: c for s in all_pieces} for p in all_pieces[:-1]},
            support={p: {s: c for s in all_pieces} for p in all_pieces[:-1]},
        )

    def get_weight(self, b: chess.Board, m: chess.Move):
        if b.is_queenside_castling(m):
            return self.castle_queenside
        if b.is_kingside_castling(m):
            return self.castle_kingside
        if b.is_en_passant(m):
            return self.en_passant

        piece = b.piece_at(m.from_square).piece_type
        value = self.move_to[piece][m.to_square ^ (56 if b.turn else 0)]

        if (capture := b.piece_at(m.to_square)) is not None:
            value *= self.capture[piece][capture.piece_type]
        if piece != chess.KING:
            for guard in _attacker_set(b, m.to_square, not b.turn):
                value *= self.guard[piece][guard]
        if piece != chess.KING:
            for support in _attacker_set(b, m.to_square, b.turn):
                if support != piece or (piece == chess.PAWN and capture is None):
                    value *= self.support[piece][support]
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
        if b.is_en_passant(m):
            self.en_passant += value
            return

        piece = b.piece_at(m.from_square).piece_type
        self.move_to[piece][m.to_square ^ (56 if b.turn else 0)] += value

        if (capture := b.piece_at(m.to_square)) is not None:
            self.capture[piece][capture.piece_type] += value
        if piece != chess.KING:
            for guard in _attacker_set(b, m.to_square, not b.turn):
                self.guard[piece][guard] += value
        if piece != chess.KING:
            for support in _attacker_set(b, m.to_square, b.turn):
                if support != piece or (piece == chess.PAWN and capture is None):
                    self.support[piece][support] += value
        if m.promotion not in [None, chess.QUEEN]:
            self.underpromotion[m.promotion] += value

    def print_cpp_code(self):
        piece_names = ['pawn', 'knight', 'bishop', 'rook', 'queen', 'king']
        for to_table, name in zip(self.move_to.values(), piece_names):
            print_cpp_2d_array_code(name + '_freq', _transform_mult_to_additive(np.array(list(to_table.values()))).reshape((8, 8)))

        for attacker, dict_ in zip(piece_names, self.capture.values()):
            print_cpp_2d_array_code(f'{attacker}_capture_freq', _transform_mult_to_additive(np.array([1] + list(dict_.values()))))
        for piece, dict_ in zip(piece_names[:-1], self.guard.values()):
            print_cpp_2d_array_code(f'{piece}_guard_freq', _transform_mult_to_additive(np.array(list(dict_.values())[::-1])))
        for piece, dict_ in zip(piece_names[:-1], self.support.values()):
            print_cpp_2d_array_code(f'{piece}_support_freq', _transform_mult_to_additive(np.array(list(dict_.values())[::-1])))

        print_cpp_constant_code('castle_qs_freq', _transform_mult_to_additive(self.castle_queenside))
        print_cpp_constant_code('castle_ks_freq', _transform_mult_to_additive(self.castle_kingside))
        print_cpp_constant_code('en_passant_freq', _transform_mult_to_additive(self.en_passant))

        for up_piece, freq in zip(['knight', 'bishop', 'rook'], self.underpromotion.values()):
            print_cpp_constant_code(f'underpromote_to_{up_piece}_freq', _transform_mult_to_additive(freq))


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
        en_passant=formula.en_passant * selections.en_passant / expectations.en_passant,
        underpromotion=(pd.Series(formula.underpromotion) * pd.Series(selections.underpromotion) / pd.Series(expectations.underpromotion)).to_dict(),
        guard=(pd.DataFrame(formula.guard) * pd.DataFrame(selections.guard) / pd.DataFrame(expectations.guard)).to_dict(),
        support=(pd.DataFrame(formula.support) * pd.DataFrame(selections.support) / pd.DataFrame(expectations.support)).to_dict(),
    )


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--tournaments', nargs='+')
    options = parser.parse_args()

    assert options.tournaments, "No tournaments to train! Use --tournaments argument"

    formula = MoveOrderingInfo.from_constant(1.0)

    for i, tourney in enumerate(options.tournaments):
        print(f'Processing {tourney} ({i+1} of {len(options.tournaments)})')
        exp, sel = process_tournament(tourney, formula)
        formula = update(formula, exp, sel)

        formula.print_cpp_code()
