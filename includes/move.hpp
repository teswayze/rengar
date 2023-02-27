# pragma once

# include <cstdint>
# include "board.hpp"

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
Board make_move(const Board board, const Move move);

bool is_irreversible(const Board board, const Move move);
