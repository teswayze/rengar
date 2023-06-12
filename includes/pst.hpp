# pragma once

# include <array>

# include "bitboard.hpp"

const int mg_pawn = 63;
const int mg_knight = 311;
const int mg_bishop = 297;
const int mg_rook = 372;
const int mg_queen = 814;

const int eg_pawn = 141;
const int eg_knight = 372;
const int eg_bishop = 359;
const int eg_rook = 637;
const int eg_queen = 1162;

const int pc_pawn = 2;
const int pc_knight = 5;
const int pc_bishop = 13;
const int pc_rook = 17;
const int pc_queen = 40;
const int pc_intercept = -1;

const std::array<int, 64> mg_pawn_table = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 90, 128,  65,  91,  71, 120,  26, -14,
	 -6,   5,  28,  28,  60,  55,  23, -21,
	-11,   8,   5,  14,  13,   4,  13, -28,
	-27, -13,  -1,  16,  12,   0,  -2, -31,
	-20,   0,  -3,  -7,   8,  -1,  31, -13,
	-27,  -8, -23, -30, -17,  26,  37, -30,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const std::array<int, 64> eg_pawn_table = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	154, 159, 155, 125, 142, 122, 150, 172,
	 84,  92,  86,  65,  43,  44,  71,  74,
	 40,  17,  12,  -3, -12, -12,   3,   8,
	 11,   8,  -4, -17, -18, -13,  -8,  -8,
	  7,  -6, -11,   2,  -9, -10,  -4, -20,
	  5,  13,   9,   0,  -8,  -5,   1, -16,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const std::array<int, 64> mg_knight_table = {
	-179,  -98,  -49,  -55,   48, -111,  -26, -120,
	 -80,  -45,   55,   29,   10,   45,   -3,  -27,
	 -59,   49,   22,   57,   74,  120,   62,   32,
	 -13,   -1,   12,   55,   16,   64,   -1,   15,
	 -21,  -10,    2,   -4,   13,    6,    9,  -20,
	 -30,  -12,   -5,   -4,   11,    1,   14,  -39,
	 -44,  -60,  -23,    0,  -17,    1,  -27,  -27,
	-119,  -30,  -63,  -49,  -40,  -39,  -33,  -36,
};
const std::array<int, 64> eg_knight_table = {
	 -58,  -38,  -16,  -26,  -30,  -29,  -65, -101,
	 -22,   -8,  -31,    1,  -14,  -34,  -16,  -51,
	 -25,  -19,   18,   12,    0,   -7,  -22,  -36,
	  -6,   -1,   18,   27,   31,   18,    4,  -10,
	 -22,   -7,   12,   18,   15,   18,    7,  -21,
	 -29,   -2,   -6,    7,    6,   -6,  -24,  -25,
	 -42,  -17,  -12,  -13,   -7,  -12,  -22,  -39,
	 -31,  -71,  -21,  -19,  -17,  -19,  -57,  -65,
};
const std::array<int, 64> mg_bishop_table = {
	-37, -14, -85, -48, -21, -52,  -6, -18,
	-22,   2, -14,  -7,  17,  37,   4, -43,
	 -4,  19,  20,  25,  16,  43,  26,  -8,
	-26, -10,   6,  45,  30,  14,  -3,  -9,
	-10,  13,  -5,  15,  25,  -4,  -1,  -7,
	  5,  11,   3,  -6,  -5,   8,  15,  12,
	 -5,   3,   2,  -6,   1,   4,  30,   0,
	-34, -13, -16, -31,  -7, -12, -38, -15,
};
const std::array<int, 64> eg_bishop_table = {
	 -2,  -6, -11,   2,   0,  -9, -21, -25,
	-18,  -3,   2, -19,  -1,   1,  -5, -23,
	 -4,  -6,  11,   7,   7,  17,   6,  10,
	  7,  29,  10,   4,  15,  12,   5,   1,
	-15,  -3,  17,  13,  14,   1,  -3, -17,
	-18,  -5,  23,  11,  11,   5, -14, -21,
	-13, -30,  -6,  -5,  -6,  -9,  -7, -39,
	-35, -16, -47,  -8, -15, -19, -16, -25,
};
const std::array<int, 64> mg_rook_table = {
	 18,  47,  30,  45,  52,  27,  34,  48,
	 24,  38,  59,  69,  71,  72,  31,  47,
	 -7,  22,  26,  40,  21,  48,  57,  14,
	-23,  -6,   9,  27,  31,  37, -10, -19,
	-30, -28, -19,  -7,   9,  -2,   2, -23,
	-47, -28, -14, -11,   0,   2,  -7, -33,
	-44, -14, -20,  -7, -15,   5,  -7, -71,
	 -7, -10,  -1,   3,   5,  -4, -36, -17,
};
const std::array<int, 64> eg_rook_table = {
	  1,   5,  16,   9,   8,  18,   8,   4,
	 18,  16,   6,  13,  -3,   8,  14,   1,
	 23,  14,  15,  11,  12,  11,  -5,   8,
	 19,  14,  16,   5,   2,   5,   4,   8,
	 14,  14,  14,  13,  -5,   2, -11,  -6,
	 -3,  -5,  -2,   9,   3,  -9, -16, -10,
	-14,  -2,  -6,  -5, -16, -21, -10,  -4,
	-17, -21, -17, -10, -13, -16,  -6, -66,
};
const std::array<int, 64> mg_queen_table = {
	-30, -19,  22,   1,  39,  43,  38,  43,
	-23, -47, -16,   1, -12,  57,  25,  46,
	-25, -22,  -3,   5,  24,  63,  48,  54,
	-30, -33, -20, -22,  -6,  10,  -3,  -2,
	-20, -37, -15, -11, -16, -11,  -2,  -5,
	-23, -15, -13,  -7, -22,  -6,  13,   4,
	-40, -15,   6,   2,   9,  18,  -6,  -8,
	 -3, -18,  -2,   3, -19, -30, -40, -56,
};
const std::array<int, 64> eg_queen_table = {
	-12,   5,  16,  15,  16,  16,   4,  15,
	-18,  25,  21,  35,  59,  18,  24,  -4,
	 -8,  12,  -3,  38,  41,  32,  14,   8,
	  4,  21,  22,  44,  43,  38,  41,  21,
	-16,   6,  11,  36,  16,  28,  24,   5,
	-19, -12,  15,  -9,  11,   1,  -4, -14,
	-24, -32, -33, -26, -12, -32, -44, -39,
	-29, -21, -33, -53,  -3, -35, -27, -51,
};
const std::array<int, 64> mg_king_table = {
	-76,  17,   8, -23, -64, -33,  -7,  -3,
	 19, -14, -14, -21, -23, -24, -46, -36,
	-19,  15, -15, -25, -20, -11,  -9, -23,
	-21, -31, -16, -30, -25, -37, -29, -51,
	-62,  -1, -44, -52, -54, -54, -46, -54,
	-25, -23, -35, -62, -61, -46, -28, -41,
	-18,  -6, -24, -82, -65, -43,   4,   5,
	-33,  20,  -7, -80, -25, -45,  25,  12,
};
const std::array<int, 64> eg_king_table = {
	-73, -32, -17, -15, -10,  21,   4, -21,
	-10,  15,  25,  15,  17,  31,  28,  15,
	 13,  23,  21,  24,  23,  33,  31,  18,
	  0,  25,  22,  27,  28,  29,  25,   6,
	-18,   4,  18,  21,  26,  23,  10,  -7,
	-19,   1,   6,  22,  18,  14,  10, -10,
	-34,  -7,   9,  11,   8,   3, -11, -11,
	-60, -37, -13, -18, -55, -14, -43, -48,
};

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

constexpr PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b){
	return PstEvalInfo{w.mg - b.mg, w.eg - b.eg, pc_intercept + w.phase_count + b.phase_count, w.hash ^ b.hash};
}

bool operator<(const PstEvalInfo, const PstEvalInfo);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const Square king, const BitMask castle);
