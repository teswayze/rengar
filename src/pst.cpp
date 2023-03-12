# include "pst.hpp"

template <bool white>
PstEvalInfo static_eval_info(
		const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const BitMask king
		){
    int mg = 0;
    int eg = 0;
    int phase_count = 0;

    Bitloop(pawn, x){
    	mg += mg_pawn_table[FlipIf(white, SquareOf(x))] + MG_PAWN;
    	eg += eg_pawn_table[FlipIf(white, SquareOf(x))] + EG_PAWN;
    }
    Bitloop(knight, x){
    	mg += mg_knight_table[FlipIf(white, SquareOf(x))] + MG_KNIGHT;
    	eg += eg_knight_table[FlipIf(white, SquareOf(x))] + EG_KNIGHT;
    	phase_count += 1;
    }
    Bitloop(bishop, x){
    	mg += mg_bishop_table[FlipIf(white, SquareOf(x))] + MG_BISHOP;
    	eg += eg_bishop_table[FlipIf(white, SquareOf(x))] + EG_BISHOP;
    	phase_count += 1;
    }
    Bitloop(rook, x){
    	mg += mg_rook_table[FlipIf(white, SquareOf(x))] + MG_ROOK;
    	eg += eg_rook_table[FlipIf(white, SquareOf(x))] + EG_ROOK;
    	phase_count += 2;
    }
    Bitloop(queen, x){
    	mg += mg_queen_table[FlipIf(white, SquareOf(x))] + MG_QUEEN;
    	eg += eg_queen_table[FlipIf(white, SquareOf(x))] + EG_QUEEN;
    	phase_count += 4;
    }
	mg += mg_king_table[FlipIf(white, SquareOf(king))];
	eg += eg_king_table[FlipIf(white, SquareOf(king))];
    
    return PstEvalInfo{mg, eg, phase_count};
}

template PstEvalInfo static_eval_info<true>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask);
template PstEvalInfo static_eval_info<false>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const BitMask);
