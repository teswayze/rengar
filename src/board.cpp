# include "board.hpp"

template <bool white>
HalfBoard from_masks(const BitMask p, const BitMask n, const BitMask b, const BitMask r, 
		const BitMask q, const BitMask k, const BitMask castle){
	return HalfBoard{p, n, b, r, q, k, p | n | b | r | q | k, castle, static_eval_info<white>(p, n, b, r, q, k)};
}

template HalfBoard from_masks<true>(const BitMask, const BitMask, const BitMask, const BitMask, 
		const BitMask, const BitMask, const BitMask);
template HalfBoard from_masks<false>(const BitMask, const BitMask, const BitMask, const BitMask, 
		const BitMask, const BitMask, const BitMask);


template <bool white>
HalfBoard remove_piece(const HalfBoard board, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return board; }
	
	return from_masks<white>(
			board.Pawn & (~mask),
			board.Knight & (~mask),
			board.Bishop & (~mask),
			board.Rook & (~mask),
			board.Queen & (~mask),
			board.King,
			board.Castle & (~mask)
			);
}

template HalfBoard remove_piece<true>(const HalfBoard, const Square);
template HalfBoard remove_piece<false>(const HalfBoard, const Square);


# include "doctest.h"

TEST_CASE("Removing pieces from white starting position"){
	HalfBoard h = from_masks<true>(RANK_2, ToMask(B1) | ToMask(G1), ToMask(C1) | ToMask(F1),
			ToMask(A1) | ToMask(H1), ToMask(D1), ToMask(E1), ToMask(A1) | ToMask(H1));
	
	HalfBoard no_g2 = remove_piece<true>(h, G2);
	CHECK(no_g2.Pawn == (RANK_2 & ~ToMask(G2)));
	CHECK(no_g2.All == (RANK_1 | RANK_2 & ~ToMask(G2)));
	CHECK(no_g2.EvalInfo.phase_count == 12);
	CHECK(h.EvalInfo.mg - no_g2.EvalInfo.mg == 120);
	CHECK(h.EvalInfo.eg - no_g2.EvalInfo.eg == 96);
	
	HalfBoard no_b1 = remove_piece<true>(h, B1);
	CHECK(no_b1.Knight == ToMask(G1));
	CHECK(no_b1.All == (RANK_2 | RANK_1 & ~ToMask(B1)));
	CHECK(no_b1.EvalInfo.phase_count == 11);
	CHECK(h.EvalInfo.mg - no_b1.EvalInfo.mg == 316);
	CHECK(h.EvalInfo.eg - no_b1.EvalInfo.eg == 230);
	
	HalfBoard no_c1 = remove_piece<true>(h, C1);
	CHECK(no_c1.Bishop == ToMask(F1));
	CHECK(no_c1.All == (RANK_2 | RANK_1 & ~ToMask(C1)));
	CHECK(no_c1.EvalInfo.phase_count == 11);
	CHECK(h.EvalInfo.mg - no_c1.EvalInfo.mg == 351);
	CHECK(h.EvalInfo.eg - no_c1.EvalInfo.eg == 274);
		
	HalfBoard no_a1 = remove_piece<true>(h, A1);
	CHECK(no_a1.Rook == ToMask(H1));
	CHECK(no_a1.All == (RANK_2 | RANK_1 & ~ToMask(A1)));
	CHECK(no_a1.Castle == ToMask(H1));
	CHECK(no_a1.EvalInfo.phase_count == 10);
	CHECK(h.EvalInfo.mg - no_a1.EvalInfo.mg == 458);
	CHECK(h.EvalInfo.eg - no_a1.EvalInfo.eg == 503);
	
	HalfBoard no_d1 = remove_piece<true>(h, D1);
	CHECK(no_d1.Queen == EMPTY_BOARD);
	CHECK(no_d1.All == (RANK_2| RANK_1 & ~ToMask(D1)));
	CHECK(no_d1.EvalInfo.phase_count == 8);
	CHECK(h.EvalInfo.mg - no_d1.EvalInfo.mg == 1035);
	CHECK(h.EvalInfo.eg - no_d1.EvalInfo.eg == 893);
}

TEST_CASE("Removing pieces from black starting position"){
	HalfBoard h = from_masks<false>(RANK_7, ToMask(B8) | ToMask(G8), ToMask(C8) | ToMask(F8),
			ToMask(A8) | ToMask(H8), ToMask(D8), ToMask(E8), ToMask(A8) | ToMask(H8));
	
	HalfBoard no_g7 = remove_piece<false>(h, G7);
	CHECK(no_g7.Pawn == (RANK_7 & ~ToMask(G7)));
	CHECK(no_g7.All == (RANK_8 | RANK_7 & ~ToMask(G7)));
	CHECK(no_g7.EvalInfo.phase_count == 12);
	CHECK(h.EvalInfo.mg - no_g7.EvalInfo.mg == 120);
	CHECK(h.EvalInfo.eg - no_g7.EvalInfo.eg == 96);
	
	HalfBoard no_g8 = remove_piece<false>(h, G8);
	CHECK(no_g8.Knight == ToMask(B8));
	CHECK(no_g8.All == (RANK_7 | RANK_8 & ~ToMask(G8)));
	CHECK(no_g8.EvalInfo.phase_count == 11);
	CHECK(h.EvalInfo.mg - no_g8.EvalInfo.mg == 318);
	CHECK(h.EvalInfo.eg - no_g8.EvalInfo.eg == 231);
	
	HalfBoard no_c8 = remove_piece<false>(h, C8);
	CHECK(no_c8.Bishop == ToMask(F8));
	CHECK(no_c8.All == (RANK_7 | RANK_8 & ~ToMask(C8)));
	CHECK(no_c8.EvalInfo.phase_count == 11);
	CHECK(h.EvalInfo.mg - no_c8.EvalInfo.mg == 351);
	CHECK(h.EvalInfo.eg - no_c8.EvalInfo.eg == 274);
		
	HalfBoard no_h8 = remove_piece<false>(h, H8);
	CHECK(no_h8.Rook == ToMask(A8));
	CHECK(no_h8.All == (RANK_7 | RANK_8 & ~ToMask(H8)));
	CHECK(no_h8.Castle == ToMask(A8));
	CHECK(no_h8.EvalInfo.phase_count == 10);
	CHECK(h.EvalInfo.mg - no_h8.EvalInfo.mg == 451);
	CHECK(h.EvalInfo.eg - no_h8.EvalInfo.eg == 492);
	
	HalfBoard no_d8 = remove_piece<false>(h, D8);
	CHECK(no_d8.Queen == EMPTY_BOARD);
	CHECK(no_d8.All == (RANK_7 | RANK_8 & ~ToMask(D8)));
	CHECK(no_d8.EvalInfo.phase_count == 8);
	CHECK(h.EvalInfo.mg - no_d8.EvalInfo.mg == 1035);
	CHECK(h.EvalInfo.eg - no_d8.EvalInfo.eg == 893);
}
