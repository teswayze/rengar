# include "endgames.hpp"
# include "eval_param.hpp"

bool is_insufficient_material(const Board &board){
	return (not board.White.Pawn) and (not board.Black.Pawn) and (board.EvalInfo.phase_count <= 17);
}

inline bool only_has_minor(const HalfBoard &side){
	return (not side.Rook) and (not side.Queen) and (__builtin_popcountll(side.Bishop | side.Knight) <= 1);
}

inline bool is_opposite_color_bishops(const Board &board){
	if ((not board.White.Rook) and (not board.Black.Rook) and (not board.White.Knight) and (not board.Black.Knight) and
		(not board.White.Queen) and (not board.Black.Queen)) {
			int white_bishop_sign = ((board.White.Bishop & LIGHT_SQUARES) ? 1 : 0) 
				- ((board.White.Bishop & DARK_SQUARES) ? 1 : 0);
			int black_bishop_sign = ((board.Black.Bishop & LIGHT_SQUARES) ? 1 : 0) 
				- ((board.Black.Bishop & DARK_SQUARES) ? 1 : 0);
			return white_bishop_sign * black_bishop_sign == -1;
		}
	return false;
}

EVAL_PARAM(better_side_pawnless, 70)
EVAL_PARAM(better_side_one_pawn, 156)
EVAL_PARAM(better_side_two_pawn, 212)
EVAL_PARAM(ocb_endgame, 203)

int make_endgame_adjustment(int raw_eval, const Board &board){
	if (is_opposite_color_bishops(board)) raw_eval = (raw_eval * ocb_endgame) / 256;

	auto &better_side = (raw_eval > 0) ? board.White : board.Black;
	int multiplier = 256;

	if (not better_side.Pawn){
		// Better side only has a single minor - checkmate can't be forced
		if (only_has_minor(better_side)) multiplier = 0;
		// Better side may be able to checkmate, but without pawns it's more difficult
		else multiplier = better_side_pawnless;
	}
	if (__builtin_popcountll(better_side.Pawn) == 1){
		multiplier = better_side_one_pawn;
	}
	if (__builtin_popcountll(better_side.Pawn) == 2){
		multiplier = better_side_two_pawn;
	}

	return (raw_eval * multiplier) / 256;
}
