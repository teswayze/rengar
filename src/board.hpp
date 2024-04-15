# pragma once

# include "bitboard.hpp"
# include "pst.hpp"
# include "attacks.hpp"

struct Board {
	HalfBoard White;
	HalfBoard Black;
	BitMask Occ;
	BitMask EPMask;

	PstEvalInfo EvalInfo;

	Attacks WtAtk;
	Attacks BkAtk;

	Board() = default;
	Board(const Board&) = delete;

	Board copy() const {
		return Board{ White.copy(), Black.copy(), Occ, EPMask, EvalInfo.copy(), WtAtk.copy(), BkAtk.copy() };
	}
};

Board from_sides_without_eval(const HalfBoard &white, const HalfBoard &black);
Board from_sides_without_eval_ep(const HalfBoard &white, const HalfBoard &black, const Square ep);

template <bool white>
const HalfBoard& get_side(const Board &board){
	return white ? board.White : board.Black;
}

template <bool white>
HalfBoard& get_side(Board &board){
	return white ? board.White : board.Black;
}

template <bool white>
void make_move_with_new_eval(Board &board, const Move move, const PstEvalInfo new_eval);

template <bool white>
int make_move(Board &board, const Move move);

bool is_irreversible(const Board &board, const Move move);

BitMask rook_attacks(const BitMask rooks, const BitMask occ);
BitMask bishop_attacks(const BitMask bishops, const BitMask occ);
