# pragma once

# include "bitboard.hpp"
# include "pst.hpp"


struct HalfBoard {
	BitMask Pawn;
	BitMask Knight;
	BitMask Bishop;
	BitMask Rook;
	BitMask Queen;
	BitMask King;

	BitMask All;

	BitMask Castle;
};

constexpr HalfBoard from_masks(BitMask p, BitMask n, BitMask b, BitMask r, BitMask q, BitMask k, BitMask castle){
	return HalfBoard{p, n, b, r, q, k, p | n | b | r | q | k, castle};
}

struct Board {
	HalfBoard White;
	HalfBoard Black;
	BitMask Occ;
	BitMask EPMask;

	PstEvalInfo EvalInfo;

	Board() = default;
	Board(const Board&) = delete;
};

Board from_sides_without_eval(const HalfBoard white, const HalfBoard black);
Board from_sides_without_eval_ep(const HalfBoard white, const HalfBoard black, const Square ep);

template <bool white>
constexpr HalfBoard get_side(const Board &board){
	return white ? board.White : board.Black;
}

template <bool white>
constexpr Board from_sides(const HalfBoard friendly, const HalfBoard enemy, const PstEvalInfo eval_info){
	return Board{white ? friendly : enemy, white ? enemy : friendly, friendly.All | enemy.All, EMPTY_BOARD, eval_info};
}

template <bool white>
constexpr Board from_sides_ep(const HalfBoard friendly, const HalfBoard enemy, const Square ep, const PstEvalInfo eval_info){
	return Board{white ? friendly : enemy, white ? enemy : friendly, friendly.All | enemy.All, ToMask(ep), eval_info};
}

constexpr bool operator==(const HalfBoard x, const HalfBoard y){
	return x.Pawn == y.Pawn and x.Knight == y.Knight and x.Bishop == y.Bishop and x.Rook == y.Rook
			and x.Queen == y.Queen and x.King == y.King;
}

constexpr bool operator==(const Board x, const Board y){
	return x.Occ == y.Occ and x.White == y.White and x.Black == y.Black;
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
PstEvalInfo compute_eval_diff_for_move(const Board &board, const Move move);

template <bool white>
Board make_move_with_new_eval(const Board &board, const Move move, const PstEvalInfo new_eval);

template <bool white>
Board make_move(const Board &board, const Move move);

bool is_irreversible(const Board board, const Move move);

# ifndef DOCTEST_CONFIG_DISABLE
void check_consistent_fb(const Board &board);
# endif
