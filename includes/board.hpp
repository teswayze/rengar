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

	PstEvalInfo EvalInfo;
};

template <bool white>
HalfBoard from_masks(BitMask p, BitMask n, BitMask b, BitMask r, BitMask q, BitMask k, BitMask castle);

struct Board {
	HalfBoard White;
	HalfBoard Black;
	BitMask Occ;
	BitMask EPMask;

	constexpr Board(const HalfBoard white, const HalfBoard black) :
		White(white), Black(black), Occ(Black.All | White.All), EPMask(EMPTY_BOARD) { }

	constexpr Board(const HalfBoard white, const HalfBoard black, const Square ep_square) :
		White(white), Black(black), Occ(Black.All | White.All), EPMask(ToMask(ep_square)) { }

	Board() : White(HalfBoard()), Black(HalfBoard()), Occ(0), EPMask(0) { }
};

template <bool white>
constexpr HalfBoard get_side(const Board board){
	return white ? board.White : board.Black;
}

template <bool white>
constexpr Board from_sides(const HalfBoard friendly, const HalfBoard enemy){
	return white ? Board(friendly, enemy) : Board(enemy, friendly);
}

template <bool white>
constexpr Board from_sides_ep(const HalfBoard friendly, const HalfBoard enemy, const Square ep){
	return white ? Board(friendly, enemy, ep) : Board(enemy, friendly, ep);
}

template <bool white>
HalfBoard remove_piece(const HalfBoard board, const Square square);

constexpr bool operator==(const HalfBoard x, const HalfBoard y){
	return x.Pawn == y.Pawn and x.Knight == y.Knight and x.Bishop == y.Bishop and x.Rook == y.Rook
			and x.Queen == y.Queen and x.King == y.King;
}

constexpr bool operator==(const Board x, const Board y){
	return x.Occ == y.Occ and x.White == y.White and x.Black == y.Black;
}
