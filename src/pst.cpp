# include "pst.hpp"

template <bool white>
PstEvalInfo static_eval_info(const HalfBoard side){
    int mg = 0;
    int eg = 0;
    uint8_t phase_count = 0;

    Bitloop(side.Pawn, x){
    	mg += mg_pawn_table[FlipIf(white, SquareOf(x))] + MG_PAWN;
    	eg += eg_pawn_table[FlipIf(white, SquareOf(x))] + EG_PAWN;
    }
    Bitloop(side.Knight, x){
    	mg += mg_knight_table[FlipIf(white, SquareOf(x))] + MG_KNIGHT;
    	eg += eg_knight_table[FlipIf(white, SquareOf(x))] + EG_KNIGHT;
    	phase_count += 1;
    }
    Bitloop(side.Bishop, x){
    	mg += mg_bishop_table[FlipIf(white, SquareOf(x))] + MG_BISHOP;
    	eg += eg_bishop_table[FlipIf(white, SquareOf(x))] + EG_BISHOP;
    	phase_count += 1;
    }
    Bitloop(side.Rook, x){
    	mg += mg_rook_table[FlipIf(white, SquareOf(x))] + MG_ROOK;
    	eg += eg_rook_table[FlipIf(white, SquareOf(x))] + EG_ROOK;
    	phase_count += 2;
    }
    Bitloop(side.Queen, x){
    	mg += mg_queen_table[FlipIf(white, SquareOf(x))] + MG_QUEEN;
    	eg += eg_queen_table[FlipIf(white, SquareOf(x))] + EG_QUEEN;
    	phase_count += 4;
    }
	mg += mg_king_table[FlipIf(white, SquareOf(side.King))];
	eg += eg_king_table[FlipIf(white, SquareOf(side.King))];
    
    return PstEvalInfo{mg, eg, phase_count};
}

template PstEvalInfo static_eval_info<true>(const HalfBoard);
template PstEvalInfo static_eval_info<false>(const HalfBoard);
