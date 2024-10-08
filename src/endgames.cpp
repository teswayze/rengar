# include <cmath>
# include "endgames.hpp"

bool is_insufficient_material(const Board &board){
	return (__builtin_popcountll(board.Occ) <= 3) and (not board.White.Pawn) and (not board.Black.Pawn) and 
		(not board.White.Rook) and (not board.Black.Rook) and (not board.White.Queen) and (not board.Black.Queen);
}

bool only_has_minor(const HalfBoard &side){
	return (not side.Pawn) and (not side.Rook) and (not side.Queen) and (__builtin_popcountll(side.Bishop | side.Knight) <= 1);
}

// These are chosen such that 5-piece pawnless endgames have the right result
// Drawn endgames: KNNvK=144, KRvKN=144, KQvKRN=132
// Won endgames: KRNvKB=168, KBBvKN=168, KQvKBB=180, KBNvK=192, KQvKR=204
// Linear programming confirms that the margin of 7/6 is optimal
// The choice of queen can vary from 408 to 432 keeping the same margin
const int mop_up_knight = 72;
const int mop_up_bishop = 120;
const int mop_up_rook = 216;
const int mop_up_queen = 420;

// Force the king to the edge of the board to checkmate
// With pawns off the board, the regular pst is no good anymore - we need more symmetry!
const std::array<int, 64> mop_up_king_pst = {
	18, 15, 12,  9,  9, 12, 15, 18,
	15, 12,  9,  6,  6,  9, 12, 15,
	12,  9,  6,  3,  3,  6,  9, 12,
	 9,  6,  3,  0,  0,  3,  6,  9,
	 9,  6,  3,  0,  0,  3,  6,  9,
	12,  9,  6,  3,  3,  6,  9, 12,
	15, 12,  9,  6,  6,  9, 12, 15,
	18, 15, 12,  9,  9, 12, 15, 18,
};

// For handling KBNvK (drive king into corner controlled by bishop)
// Also can help defent KRvKB (keep king out of corner controlled by bishop)
const std::array<int, 64> mop_up_king_pst_lsb = {
	 6,  7,  8,  9, 11, 14, 17, 20,
	 7,  4,  5,  6,  8, 11, 14, 17,
	 8,  5,  2,  3,  5,  8, 11, 14,
	 9,  6,  3,  0,  2,  5,  8, 11,
	11,  8,  5,  2,  0,  3,  6,  9,
	14, 11,  8,  5,  3,  2,  5,  8,
	17, 14, 11,  8,  6,  5,  4,  7,
	20, 17, 14, 11,  9,  8,  7,  6,
};
const std::array<int, 64> mop_up_king_pst_dsb = {
	20, 17, 14, 11,  9,  8,  7,  6,
	17, 14, 11,  8,  6,  5,  4,  7,
	14, 11,  8,  5,  3,  2,  5,  8,
	11,  8,  5,  2,  0,  3,  6,  9,
	 9,  6,  3,  0,  2,  5,  8, 11,
	 8,  5,  2,  3,  5,  8, 11, 14,
	 7,  4,  5,  6,  8, 11, 14, 17,
	 6,  7,  8,  9, 11, 14, 17, 20,
};

// We need to bring our king in close to assist with the checkmate
int manhattan_distance(const Square k1, const Square k2){
	return std::abs(k1 % 8 - k2 % 8) + std::abs(k1 / 8 - k2 / 8);
}

std::array<int, 64> relevant_mop_up_pst(const Board &board){
	if (__builtin_popcountll(board.White.Bishop | board.Black.Bishop) != 1) return mop_up_king_pst;
	if (__builtin_popcountll(board.Occ) == 4) {
		// If it's KRvKB, the king should avoid the square the bishop controls
		// If it's KBNvK, the attacker should drive the king to the square the bishop controls
		// Other 4-man endgames with 1 bishop are too far from the margin for this to make a difference
		return ((board.White.Bishop | board.Black.Bishop) & LIGHT_SQUARES) ? mop_up_king_pst_lsb : mop_up_king_pst_dsb;
	}
	if (__builtin_popcountll(board.Occ) == 5) {
		if ((board.White.Queen | board.Black.Queen) and (board.White.Knight | board.Black.Knight)) {
			// The only relevant one here is KQvKBN which is generally a win
			// The attacker must avoid a drawing fortress in the corner the bishop controls
			return ((board.White.Bishop | board.Black.Bishop) & LIGHT_SQUARES) ? mop_up_king_pst_dsb : mop_up_king_pst_lsb;
		}
	}
	return mop_up_king_pst;
}

// Inspired by this: https://www.chessprogramming.org/Mop-up_Evaluation#Chess_4.x
int mop_up_evaluation(const Board &board){
	const int material_imbalance
		= __builtin_popcountll(board.White.Knight) * mop_up_knight
		+ __builtin_popcountll(board.White.Bishop) * mop_up_bishop
		+ __builtin_popcountll(board.White.Rook) * mop_up_rook
		+ __builtin_popcountll(board.White.Queen) * mop_up_queen
		- __builtin_popcountll(board.Black.Knight) * mop_up_knight
		- __builtin_popcountll(board.Black.Bishop) * mop_up_bishop
		- __builtin_popcountll(board.Black.Rook) * mop_up_rook
		- __builtin_popcountll(board.Black.Queen) * mop_up_queen;
	
	const int king_distance = manhattan_distance(board.White.King, board.Black.King);

	auto pst = relevant_mop_up_pst(board);

	return material_imbalance
		+ (material_imbalance > 0) * (pst[board.Black.King] - king_distance)
		- (material_imbalance < 0) * (pst[board.White.King] - king_distance)
		- __builtin_popcountll(EDGES & board.White.All)
		+ __builtin_popcountll(EDGES & board.Black.All);
}
