# pragma once

# include "bitboard.hpp"

struct PstEvalInfo{
	int mg_kk;
	int mg_qk;
	int mg_kq;
	int mg_qq;
	int eg;
	int phase_count;

	uint64_t hash;

	PstEvalInfo() = default;
	PstEvalInfo(const PstEvalInfo&) = delete;

	PstEvalInfo copy() const {
		return PstEvalInfo{ mg_kk, mg_qk, mg_kq, mg_qq, eg, phase_count, hash };
	}
};

PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const Square king, const BitMask castle);
