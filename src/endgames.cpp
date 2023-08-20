# include "endgames.hpp"

bool is_insufficient_material(const Board &board){
	return (not board.White.Pawn) and (not board.Black.Pawn) and ((board.EvalInfo.phase_count <= 4) or (board.EvalInfo.phase_count == 12));
}

inline bool only_has_minor(const HalfBoard &side){
	return (not side.Rook) and (not side.Queen) and (__builtin_popcountll(side.Bishop | side.Knight) <= 1);
}

const int better_side_pawnless = 89;
const int better_side_one_pawn = 174;

int make_endgame_adjustment(int raw_eval, const Board &board){
	if ((not board.White.Pawn) and (raw_eval > 0)){
		if (only_has_minor(board.White)){
			// White only has a single minor - checkmate can't be forced
			return 0;
		}
		// White may be able to checkmate, but without pawns it's more difficult
		return (raw_eval * better_side_pawnless) / 256;
	}
	if ((__builtin_popcountll(board.White.Pawn) == 1) and (raw_eval > 0)){
		return (raw_eval * better_side_one_pawn) / 256;
	}

	if ((not board.Black.Pawn) and (raw_eval < 0)){
		if (only_has_minor(board.Black)){
			// Black only has a single minor - checkmate can't be forced
			return 0;
		}
		// Black may be able to checkmate, but without pawns it's more difficult
		return (raw_eval * better_side_pawnless) / 256;
	}
	if ((__builtin_popcountll(board.Black.Pawn) == 1) and (raw_eval < 0)){
		return (raw_eval * better_side_one_pawn) / 256;
	}

	return raw_eval;
}
