# include "pst.hpp"
# include "hashing.hpp"

# include <exception>

int eval_from_info(const PstEvalInfo &info){
	return info.mg * info.phase_count + info.eg * (PC_TOTAL - info.phase_count);
}

template <bool white>
PstEvalInfo static_eval_info(
		const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const BitMask king, const BitMask castle
		){
    int mg = 0;
    int eg = 0;
    int phase_count = 0;
    uint64_t hash = 0ull;

    Bitloop(pawn, x){
    	mg += mg_pawn_table[FlipIf(white, SquareOf(x))] + MG_PAWN;
    	eg += eg_pawn_table[FlipIf(white, SquareOf(x))] + EG_PAWN;
    	hash ^= (white ? white_pawn_hash : black_pawn_hash)[SquareOf(x)];
    }
    Bitloop(knight, x){
    	mg += mg_knight_table[FlipIf(white, SquareOf(x))] + MG_KNIGHT;
    	eg += eg_knight_table[FlipIf(white, SquareOf(x))] + EG_KNIGHT;
    	phase_count += PC_KNIGHT;
    	hash ^= (white ? white_knight_hash : black_knight_hash)[SquareOf(x)];
    }
    Bitloop(bishop, x){
    	mg += mg_bishop_table[FlipIf(white, SquareOf(x))] + MG_BISHOP;
    	eg += eg_bishop_table[FlipIf(white, SquareOf(x))] + EG_BISHOP;
    	phase_count += PC_BISHOP;
    	hash ^= (white ? white_bishop_hash : black_bishop_hash)[SquareOf(x)];
    }
    Bitloop(rook, x){
    	mg += mg_rook_table[FlipIf(white, SquareOf(x))] + MG_ROOK;
    	eg += eg_rook_table[FlipIf(white, SquareOf(x))] + EG_ROOK;
    	phase_count += PC_ROOK;
    	hash ^= (white ? white_rook_hash : black_rook_hash)[SquareOf(x)];
    }
    Bitloop(queen, x){
    	mg += mg_queen_table[FlipIf(white, SquareOf(x))] + MG_QUEEN;
    	eg += eg_queen_table[FlipIf(white, SquareOf(x))] + EG_QUEEN;
    	phase_count += PC_QUEEN;
    	hash ^= (white ? white_queen_hash : black_queen_hash)[SquareOf(x)];
    }
	mg += mg_king_table[FlipIf(white, SquareOf(king))];
	eg += eg_king_table[FlipIf(white, SquareOf(king))];
	hash ^= (white ? white_king_hash : black_king_hash)[SquareOf(king)];

	if (castle & ToMask(white ? A1 : A8)){ hash ^= (white ? white_cqs_hash : black_cqs_hash); }
	if (castle & ToMask(white ? H1 : H8)){ hash ^= (white ? white_cks_hash : black_cks_hash); }
    
    return PstEvalInfo{mg, eg, phase_count, hash};
}

bool operator<(const PstEvalInfo, const PstEvalInfo){
	throw std::logic_error("Should never compare eval infos - is there a duplicate move in the MoveQueue?");
}

template PstEvalInfo static_eval_info<true>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask);
template PstEvalInfo static_eval_info<false>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask);
