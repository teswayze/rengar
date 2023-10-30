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

	if (__builtin_popcountll(bishop) > 1){
		mg_kk += ssc_mg_bishop_pair;
		mg_qk += osc_mg_bishop_pair;
		mg_kq += osc_mg_bishop_pair;
		mg_qq += ssc_mg_bishop_pair;
		eg += eg_bishop_pair;
	}
    
    return PstEvalInfo{mg_kk, mg_qk, mg_kq, mg_qq, eg, phase_count, hash};
}

PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b){
	return PstEvalInfo{w.mg_kk - b.mg_kk, w.mg_qk - b.mg_qk, w.mg_kq - b.mg_kq, w.mg_qq - b.mg_qq,
	 w.eg - b.eg, pc_intercept + w.phase_count + b.phase_count, w.hash ^ b.hash};
}

template PstEvalInfo static_eval_info<true>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const Square, const BitMask);
template PstEvalInfo static_eval_info<false>(const BitMask, const BitMask, const BitMask, const BitMask, const BitMask, const Square, const BitMask);

constexpr uint64_t hash_diff(std::array<uint64_t, 64> table, Square from, Square to){
	return table[from] ^ table[to];
}

# define DEFINE_MOVE_FUNCTION(piece) \
template <bool white> \
void PstEvalInfo::move_##piece(const Square from, const Square to){ \
	const int sign = white ? 1 : -1; \
	mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, to)] - ssc_mg_##piece##_table[FlipIf(white, from)]); \
	mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, to ^ 7)] - osc_mg_##piece##_table[RotIf(white, from ^ 7)]); \
	mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, to)] - osc_mg_##piece##_table[RotIf(white, from)]); \
	mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, to ^ 7)] - ssc_mg_##piece##_table[FlipIf(white, from ^ 7)]); \
	eg += sign * (eg_##piece##_table[FlipIf(white, to)] - eg_##piece##_table[FlipIf(white, from)]); \
	hash ^= hash_diff(white ? white_##piece##_hash : black_##piece##_hash, from, to); \
} \
template void PstEvalInfo::move_##piece<true>(const Square, const Square); \
template void PstEvalInfo::move_##piece<false>(const Square, const Square);

DEFINE_MOVE_FUNCTION(pawn);
DEFINE_MOVE_FUNCTION(knight);
DEFINE_MOVE_FUNCTION(bishop);
DEFINE_MOVE_FUNCTION(rook);
DEFINE_MOVE_FUNCTION(queen);
DEFINE_MOVE_FUNCTION(king);

# define DEFINE_REMOVE_FUNCTION(piece) \
template <bool white> \
void PstEvalInfo::remove_##piece(const Square square){ \
	const int sign = white ? -1 : 1; \
	mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, square)] + ssc_mg_##piece); \
	mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, square ^ 7)] + osc_mg_##piece); \
	mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, square)] + osc_mg_##piece); \
	mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, square ^ 7)] + ssc_mg_##piece); \
	eg += sign * (eg_##piece##_table[FlipIf(white, square)] + eg_##piece); \
	phase_count -= pc_##piece; \
	hash ^= (white ? white_##piece##_hash : black_##piece##_hash)[square]; \
} \
template void PstEvalInfo::remove_##piece<true>(const Square); \
template void PstEvalInfo::remove_##piece<false>(const Square);

DEFINE_REMOVE_FUNCTION(pawn);
DEFINE_REMOVE_FUNCTION(knight);
DEFINE_REMOVE_FUNCTION(bishop);
DEFINE_REMOVE_FUNCTION(rook);
DEFINE_REMOVE_FUNCTION(queen);

# define DEFINE_PROMOTE_FUNCTION(piece) \
template <bool white> \
void PstEvalInfo::promote_pawn_to_##piece(const Square from, const Square to){ \
	const int sign = white ? 1 : -1; \
	mg_kk += sign * (ssc_mg_##piece##_table[FlipIf(white, to)] + ssc_mg_##piece -  \
		ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn); \
	mg_qk += sign * (osc_mg_##piece##_table[RotIf(white, to ^ 7)] + osc_mg_##piece -  \
		osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn); \
	mg_kq += sign * (osc_mg_##piece##_table[RotIf(white, to)] + osc_mg_##piece -  \
		osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn); \
	mg_qq += sign * (ssc_mg_##piece##_table[FlipIf(white, to ^ 7)] + ssc_mg_##piece - \
		ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn); \
	eg += sign * (eg_##piece##_table[FlipIf(white, to)] + eg_##piece - eg_pawn_table[FlipIf(white, from)] - eg_pawn); \
	phase_count += pc_##piece - pc_pawn; \
	hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_##piece##_hash : black_##piece##_hash)[to]; \
} \
template void PstEvalInfo::promote_pawn_to_##piece<true>(const Square, const Square); \
template void PstEvalInfo::promote_pawn_to_##piece<false>(const Square, const Square);

DEFINE_PROMOTE_FUNCTION(knight);
DEFINE_PROMOTE_FUNCTION(bishop);
DEFINE_PROMOTE_FUNCTION(rook);
DEFINE_PROMOTE_FUNCTION(queen);

template <bool white>
void PstEvalInfo::castle_queenside(){
	move_king<white>(white ? E1 : E8, white ? C1 : C8);
	move_rook<white>(white ? A1 : A8, white ? D1 : D8);
}
template void PstEvalInfo::castle_queenside<true>();
template void PstEvalInfo::castle_queenside<false>();
template <bool white>
void PstEvalInfo::castle_kingside(){
	move_king<white>(white ? E1 : E8, white ? G1 : G8);
	move_rook<white>(white ? H1 : H8, white ? F1 : F8);
}
template void PstEvalInfo::castle_kingside<true>();
template void PstEvalInfo::castle_kingside<false>();

template <bool white>
void PstEvalInfo::remove_bishop_pair_bonus(){
	auto sign = white ? 1 : -1;
	mg_kk -= ssc_mg_bishop_pair * sign;
	mg_qk -= osc_mg_bishop_pair * sign;
	mg_kq -= osc_mg_bishop_pair * sign;
	mg_qq -= ssc_mg_bishop_pair * sign;
	eg -= eg_bishop_pair * sign;
}
template void PstEvalInfo::remove_bishop_pair_bonus<true>();
template void PstEvalInfo::remove_bishop_pair_bonus<false>();
