# pragma once

# include <array>

# include "bitboard.hpp"

const int MG_PAWN = 84;
const int MG_KNIGHT = 349;
const int MG_BISHOP = 374;
const int MG_ROOK = 478;
const int MG_QUEEN = 1030;

const int EG_PAWN = 102;
const int EG_KNIGHT = 282;
const int EG_BISHOP = 298;
const int EG_ROOK = 512;
const int EG_QUEEN = 941;

const int PC_KNIGHT = 1;
const int PC_BISHOP = 1;
const int PC_ROOK = 2;
const int PC_QUEEN = 4;

const int PC_TOTAL = 4 * PC_KNIGHT + 4 * PC_BISHOP + 4 * PC_ROOK + 2 * PC_QUEEN;

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
	-38,  -5, -91, -46, -34, -51,  -2, -17,
    -35,   7, -27, -22,  21,  50,   9, -56,
    -25,  28,  34,  31,  26,  41,  28, -11,
    -13,  -4,  10,  41,  28,  28,  -2, -11,
    -15,   4,   4,  17,  25,   3,   1,  -5,
 	 -9,   6,   6,   6,   5,  18,   9,   1,
	 -5,   6,   7,  -9,  -2,  12,  24,  -8,
    -42, -12, -23, -30, -22, -21, -48, -30,
};

const std::array<int, 64> eg_bishop_table = {
   -15, -22, -12,  -9,  -8, -10, -18, -25,
	-9,  -5,   6, -13,  -4, -14,  -5, -15,
	 1,  -9,  -1,  -2,  -3,   5,  -1,   3,
	-4,   8,  11,   8,  13,   9,   2,   1,
	-7,   2,  12,  18,   6,   9,  -4, -10,
   -13,  -4,   7,   9,  12,   2,  -8, -16,
   -15, -19,  -8,  -2,   3, -10, -16, -28,
   -24, -10, -24,  -6, -10, -17,  -6, -18,
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
};

template <bool white>
constexpr PstEvalInfo adjust_eval(const PstEvalInfo old, const PstEvalInfo diff){
	const int sign = white ? 1 : -1;
	return PstEvalInfo{old.mg + sign * diff.mg, old.eg + sign * diff.eg, old.phase_count + diff.phase_count, old.hash ^ diff.hash};
}

int eval_from_info(PstEvalInfo info);

constexpr PstEvalInfo half_to_full_eval_info(const PstEvalInfo w, const PstEvalInfo b){
	return PstEvalInfo{w.mg - b.mg, w.eg - b.eg, static_cast<uint8_t>(w.phase_count + b.phase_count), w.hash ^ b.hash};
}

bool operator<(const PstEvalInfo, const PstEvalInfo);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const BitMask king, const BitMask castle);
