# pragma once

# include <array>

# include "bitboard.hpp"

const int mg_pawn = 64;
const int mg_knight = 311;
const int mg_bishop = 296;
const int mg_rook = 371;
const int mg_queen = 814;

const int eg_pawn = 141;
const int eg_knight = 367;
const int eg_bishop = 361;
const int eg_rook = 636;
const int eg_queen = 1163;

const int pc_pawn = 2;
const int pc_knight = 5;
const int pc_bishop = 13;
const int pc_rook = 17;
const int pc_queen = 40;
const int pc_intercept = -1;

const std::array<int, 64> mg_pawn_table = {
      0,   0,   0,   0,   0,   0,  0,   0,
	 96, 132,  59,  93,  66, 124,  32, -13,
	 -8,   5,  24,  29,  63,  54,  23, -22,
	-16,  11,   4,  19,  21,  10,  15, -25,
	-29,  -4,  -7,  10,  15,   4,   8, -27,
	-28,  -6,  -6, -12,   1,   1,  31, -14,
	-37,  -3, -22, -25, -17,  22,  36, -24,
      0,   0,   0,   0,   0,   0,  0,   0,
};

const std::array<int, 64> eg_pawn_table = {
      0,   0,   0,   0,   0,   0,   0,   0,
	170, 165, 150, 126, 139, 124, 157, 179,
	 86,  92,  77,  59,  48,  45,  74,  76,
	 24,  16,   5,  -3, -10,  -4,   9,   9,
	  5,   1, -11, -15, -15, -16,  -5,  -9,
	 -4,  -1, -14,  -7,  -8, -13,  -9, -16,
	  5,   0,   0,   2,   5,  -8,  -6, -15,
      0,   0,   0,   0,   0,   0,   0,   0,
};

const std::array<int, 64> mg_knight_table = {
	-179, -101,  -46,  -61,   49, -109,  -27, -119,
	 -85,  -53,   60,   24,   11,   50,   -5,  -29,
	 -59,   48,   25,   53,   72,  117,   61,   32,
	 -21,    5,    7,   41,   25,   57,    6,   10,
	 -25,   -8,    4,    1,   16,    7,    9,  -20,
	 -35,  -21,    0,   -2,    7,    5,   13,  -28,
	 -41,  -65,  -24,  -15,  -13,    6,  -26,  -31,
    -117,  -33,  -70,  -45,  -29,  -40,  -31,  -35,
};

const std::array<int, 64> eg_knight_table = {
	-59,  -39,  -14,  -29,  -32,  -28,  -64, -100,
	-26,   -9,  -26,   -3,  -10,  -26,  -25,  -53,
	-25,  -21,    9,    8,   -2,  -10,  -20,  -42,
	-18,    2,   21,   21,   21,   10,    7,  -19,
	-19,   -7,   15,   24,   15,   16,    3,  -19,
	-24,   -4,   -2,   14,    9,   -4,  -21,  -23,
	-43,  -21,  -11,   -6,   -3,  -21,  -24,  -45,
	-30,  -52,  -24,  -16,  -23,  -19,  -51,  -65,
};

const std::array<int, 64> mg_bishop_table = {
	-38, -15, -81, -46, -24, -51,  -2, -17,
	-25,  -1, -17, -12,  21,  40,   9, -46,
	-15,  23,  30,  25,  16,  41,  27, -11,
	-16,  -9,   8,  34,  27,  20,  -2, -11,
	 -5,   8,   2,  11,  21,   2,   7, -12,
	 -7,   9,   6,  -4,  -3,  10,  13,  11,
	 -5,   6,   4,  -4,  -3,   7,  29,   1,
	-32,  -9, -19, -30, -12, -12, -38, -20,
};
const std::array<int, 64> eg_bishop_table = {
	 -5, -12, -12,   1,  -8, -10, -18, -25,
	-19,  -5,   1, -23,   1,  -4,  -5, -25,
	 -9,  -1,   9,   8,   7,  15,   7,   3,
	  4,  16,  12,   9,  15,  19,   5,   2,
	-17,  -3,  14,  18,  14,  10,  -9,  -2,
	-15,  -8,   7,  19,  22,  12, -12, -26,
	-15, -18,  -2,  -9,   4,  -2, -23, -36,
	-34, -15, -30,  -6, -20, -27, -16, -28,
};

const std::array<int, 64> mg_rook_table = {
	31,  41,  31,  50,  62,   8,  30,  42,
	26,  31,  57,  61,  79,  66,  25,  43,
	-6,  18,  25,  35,  16,  44,  60,  15,
   -25, -12,   6,  25,  23,  34,  -9, -21,
   -37, -27, -13,  -2,   8,  -8,   5, -24,
   -46, -26, -17, -18,   2,  -1,  -6, -34,
   -45, -17, -21, -10,  -2,  10,  -7, -72,
   -20, -14,   0,  16,  15,   6, -38, -27,
};

const std::array<int, 64> eg_rook_table = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

const std::array<int, 64> mg_queen_table = {
   -33,  -5,  24,   7,  54,  39,  38,  40,
   -29, -44, -10,  -4, -21,  52,  23,  49,
   -18, -22,   2,   3,  24,  51,  42,  52,
   -32, -32, -21, -21,  -6,  12,  -7,  -4,
   -14, -31, -14, -15,  -7,  -9,  -2,  -8,
   -19,  -3, -16,  -7, -10,  -3,   9,   0,
   -40, -13,   6,  -3,   3,  10,  -8,  -4,
	-6, -23, -14,   5, -20, -30, -36, -55,
};

const std::array<int, 64> eg_queen_table = {
	-14,  17,  17,  22,  22,  14,   5,  15,
    -22,  15,  27,  36,  53,  20,  25,  -5,
    -25,   1,   4,  44,  42,  30,  14,   4,
   	 -2,  17,  19,  40,  52,  35,  52,  31,
    -23,  23,  14,  42,  26,  29,  34,  18,
    -21, -32,  10,   1,   4,  12,   5,   0,
    -27, -28, -35, -21, -21, -28, -41, -37,
    -38, -33, -27, -48, -10, -37, -25, -46,
};

const std::array<int, 64> mg_king_table = {
	-78,  10,   3, -28, -69, -47, -11,   0,
	 16, -14, -33, -20, -21, -17, -51, -42,
    -22,  11, -11, -29, -33,  -7,   9, -35,
    -30, -33, -25, -40, -43, -38, -27, -49,
    -62, -14, -40, -52, -59, -57, -46, -64,
    -27, -27, -35, -59, -57, -43, -28, -40,
    -12,  -6, -21, -77, -56, -29,  -4,  -5,
    -28,  23,  -1, -67,  -5, -41,  11,   1,
};

const std::array<int, 64> eg_king_table = {
   -75, -36, -19, -19, -12,  14,   3, -18,
   -13,  16,  13,  16,  16,  37,  22,  10,
	 9,  16,  22,  14,  19,  44,  43,  12,
	-9,  21,  23,  26,  25,  32,  25,   2,
   -19,  -5,  20,  23,  26,  22,   8, -12,
   -20,  -4,  10,  20,  22,  15,   6, -10,
   -28, -12,   3,  12,  13,   3,  -6, -18,
   -54, -35, -22, -12, -29, -15, -25, -44,
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
	return PstEvalInfo{w.mg - b.mg, w.eg - b.eg, w.phase_count + b.phase_count, w.hash ^ b.hash};
}

bool operator<(const PstEvalInfo, const PstEvalInfo);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const BitMask king, const BitMask castle);
