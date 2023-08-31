# include "pst.hpp"
# include "hashing.hpp"
# include "bmi2_fallback.hpp"
# include "weights/endgame.hpp"
# include "weights/osc_middlegame.hpp"
# include "weights/ssc_middlegame.hpp"
# include "weights/phase_count.hpp"

# include <stdexcept>

template <bool white>
PstEvalInfo static_eval_info(
		const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const Square king, const BitMask castle
		){
    int mg_kk = 0;
    int mg_qk = 0;
    int mg_kq = 0;
    int mg_qq = 0;
    int eg = 0;
    int phase_count = 0;
    uint64_t hash = 0ull;

    Bitloop(pawn, x){
    	mg_kk += ssc_mg_pawn_table[FlipIf(white, TZCNT(x))] + ssc_mg_pawn;
    	mg_qk += osc_mg_pawn_table[RotIf(white, TZCNT(x) ^ 7)] + osc_mg_pawn;
    	mg_kq += osc_mg_pawn_table[RotIf(white, TZCNT(x))] + osc_mg_pawn;
    	mg_qq += ssc_mg_pawn_table[FlipIf(white, TZCNT(x) ^ 7)] + ssc_mg_pawn;
    	eg += eg_pawn_table[FlipIf(white, TZCNT(x))] + eg_pawn;
    	phase_count += pc_pawn;
    	hash ^= (white ? white_pawn_hash : black_pawn_hash)[TZCNT(x)];
    }
    Bitloop(knight, x){
    	mg_kk += ssc_mg_knight_table[FlipIf(white, TZCNT(x))] + ssc_mg_knight;
    	mg_qk += osc_mg_knight_table[RotIf(white, TZCNT(x) ^ 7)] + osc_mg_knight;
    	mg_kq += osc_mg_knight_table[RotIf(white, TZCNT(x))] + osc_mg_knight;
    	mg_qq += ssc_mg_knight_table[FlipIf(white, TZCNT(x) ^ 7)] + ssc_mg_knight;
    	eg += eg_knight_table[FlipIf(white, TZCNT(x))] + eg_knight;
    	phase_count += pc_knight;
    	hash ^= (white ? white_knight_hash : black_knight_hash)[TZCNT(x)];
    }
    Bitloop(bishop, x){
    	mg_kk += ssc_mg_bishop_table[FlipIf(white, TZCNT(x))] + ssc_mg_bishop;
    	mg_qk += osc_mg_bishop_table[RotIf(white, TZCNT(x) ^ 7)] + osc_mg_bishop;
    	mg_kq += osc_mg_bishop_table[RotIf(white, TZCNT(x))] + osc_mg_bishop;
    	mg_qq += ssc_mg_bishop_table[FlipIf(white, TZCNT(x) ^ 7)] + ssc_mg_bishop;
    	eg += eg_bishop_table[FlipIf(white, TZCNT(x))] + eg_bishop;
    	phase_count += pc_bishop;
    	hash ^= (white ? white_bishop_hash : black_bishop_hash)[TZCNT(x)];
    }
    Bitloop(rook, x){
    	mg_kk += ssc_mg_rook_table[FlipIf(white, TZCNT(x))] + ssc_mg_rook;
    	mg_qk += osc_mg_rook_table[RotIf(white, TZCNT(x) ^ 7)] + osc_mg_rook;
    	mg_kq += osc_mg_rook_table[RotIf(white, TZCNT(x))] + osc_mg_rook;
    	mg_qq += ssc_mg_rook_table[FlipIf(white, TZCNT(x) ^ 7)] + ssc_mg_rook;
    	eg += eg_rook_table[FlipIf(white, TZCNT(x))] + eg_rook;
    	phase_count += pc_rook;
    	hash ^= (white ? white_rook_hash : black_rook_hash)[TZCNT(x)];
    }
    Bitloop(queen, x){
    	mg_kk += ssc_mg_queen_table[FlipIf(white, TZCNT(x))] + ssc_mg_queen;
    	mg_qk += osc_mg_queen_table[RotIf(white, TZCNT(x) ^ 7)] + osc_mg_queen;
    	mg_kq += osc_mg_queen_table[RotIf(white, TZCNT(x))] + osc_mg_queen;
    	mg_qq += ssc_mg_queen_table[FlipIf(white, TZCNT(x) ^ 7)] + ssc_mg_queen;
    	eg += eg_queen_table[FlipIf(white, TZCNT(x))] + eg_queen;
    	phase_count += pc_queen;
    	hash ^= (white ? white_queen_hash : black_queen_hash)[TZCNT(x)];
    }
	mg_kk += ssc_mg_king_table[FlipIf(white, king)];
	mg_qk += osc_mg_king_table[RotIf(white, king ^ 7)];
	mg_kq += osc_mg_king_table[RotIf(white, king)];
	mg_qq += ssc_mg_king_table[FlipIf(white, king ^ 7)];
	eg += eg_king_table[FlipIf(white, king)];
	hash ^= (white ? white_king_hash : black_king_hash)[king];

	if (castle & ToMask(white ? A1 : A8)){ hash ^= (white ? white_cqs_hash : black_cqs_hash); }
	if (castle & ToMask(white ? H1 : H8)){ hash ^= (white ? white_cks_hash : black_cks_hash); }
    
    return PstEvalInfo{mg_kk, mg_qk, mg_kq, mg_qq, eg, phase_count, hash};
}

PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b){
	return PstEvalInfo{w.mg_kk - b.mg_kk, w.mg_qk - b.mg_qk, w.mg_kq - b.mg_kq, w.mg_qq - b.mg_qq,
	 w.eg - b.eg, pc_intercept + w.phase_count + b.phase_count, w.hash ^ b.hash};
}

template PstEvalInfo static_eval_info<true>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const Square, const BitMask);
template PstEvalInfo static_eval_info<false>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const Square, const BitMask);
