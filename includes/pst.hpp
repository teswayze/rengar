# pragma once

# include "bitboard.hpp"

struct PstEvalInfo{
	int mg;
	int eg;
	int phase_count;

	uint64_t hash;

	PstEvalInfo() = default;
	PstEvalInfo(const PstEvalInfo&) = delete;

	PstEvalInfo copy() const {
		return PstEvalInfo{ mg, eg, phase_count, hash };
	}
};

template <bool white>
constexpr PstEvalInfo adjust_eval(const PstEvalInfo old, const PstEvalInfo diff){
	const int sign = white ? 1 : -1;
	return PstEvalInfo{old.mg + sign * diff.mg, old.eg + sign * diff.eg, old.phase_count + diff.phase_count, old.hash ^ diff.hash};
}

PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const Square king, const BitMask castle);
