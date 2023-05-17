# pragma once

# include "bitboard.hpp"
# include "pst.hpp"
# include "attacks.hpp"


struct HalfBoard {
	BitMask Pawn;
	BitMask Knight;
	BitMask Bishop;
	BitMask Rook;
	BitMask Queen;
	BitMask King;

	BitMask All;

	BitMask Castle;

	HalfBoard() = default;
	HalfBoard(const HalfBoard&) = delete;

	HalfBoard copy() const {
		return HalfBoard{ Pawn, Knight, Bishop, Rook, Queen, King, All, Castle };
	}
};

constexpr HalfBoard from_masks(BitMask p, BitMask n, BitMask b, BitMask r, BitMask q, BitMask k, BitMask castle){
	return HalfBoard{p, n, b, r, q, k, p | n | b | r | q | k, castle};
}

constexpr bool side_has_non_pawn_piece(const HalfBoard &side){
	return side.All != (side.King & side.Pawn);
}

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

using Move = uint16_t;
using MoveFlags = uint16_t;

const uint16_t NULL_MOVE = 0;

const uint16_t KNIGHT_MOVE = 1;
const uint16_t BISHOP_MOVE = 2;
const uint16_t ROOK_MOVE = 3;
const uint16_t QUEEN_MOVE = 4;
const uint16_t KING_MOVE = 5;
const uint16_t CASTLE_QUEENSIDE = 6;
const uint16_t CASTLE_KINGSIDE = 7;

const uint16_t SINGLE_PAWN_PUSH = 8;
const uint16_t DOUBLE_PAWN_PUSH = 9;
const uint16_t PAWN_CAPTURE = 10;
const uint16_t EN_PASSANT_CAPTURE = 11;
const uint16_t PROMOTE_TO_KNIGHT = 12;
const uint16_t PROMOTE_TO_BISHOP = 13;
const uint16_t PROMOTE_TO_ROOK = 14;
const uint16_t PROMOTE_TO_QUEEN = 15;

constexpr Square move_source(const Move move){return move & 63;}
constexpr Square move_destination(const Move move){return move >> 6 & 63;}
constexpr MoveFlags move_flags(const Move move){return move >> 12;}

constexpr Move move_from_squares(const Square from, const Square to, const uint16_t flags){
	return ((Move) from) | ((Move) to) << 6 | flags << 12;
}

template <bool white>
void make_move_with_new_eval(Board &board, const Move move, const PstEvalInfo new_eval);

template <bool white>
int make_move(Board &board, const Move move);

bool is_irreversible(const Board &board, const Move move);

BitMask rook_attacks(const BitMask rooks, const BitMask occ);
BitMask bishop_attacks(const BitMask bishops, const BitMask occ);

# ifndef DOCTEST_CONFIG_DISABLE
void check_consistent_fb(const Board &board);
# endif
