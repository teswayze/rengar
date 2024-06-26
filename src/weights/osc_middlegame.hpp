# pragma once

# include <array>

const int osc_mg_pawn = 86;
const int osc_mg_knight = 321;
const int osc_mg_bishop = 292;
const int osc_mg_rook = 349;
const int osc_mg_queen = 761;
const int osc_mg_bishop_pair = 23;

// All tables can be viewed from white's perspective where white is castled queenside and black is castled kingside

const std::array<int, 64> osc_mg_pawn_table = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 90, 128,  65,  91,  71, 120,  26, -14,
	 -6,   5,  28,  28,  60,  55,  23, -21,
	-11,   8,   5,  14,  13,   4,  13, -28,
	-31,  -2,   0,  12,  16,  -1, -13, -27,
	-13,  31,  -1,   8,  -7,  -3,   0, -20,
	-30,  37,  26, -17, -30, -23,  -8, -27,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const std::array<int, 64> osc_mg_knight_table = {
	-179,  -98,  -49,  -55,   48, -111,  -26, -120,
	 -80,  -45,   55,   29,   10,   45,   -3,  -27,
	 -59,   49,   22,   57,   74,  120,   62,   32,
	 -13,   -1,   12,   55,   16,   64,   -1,   15,
	 -21,  -10,    2,   -4,   13,    6,    9,  -20,
	 -39,   14,    1,   11,   -4,   -5,  -12,  -30,
	 -27,  -27,    1,  -17,    0,  -23,  -60,  -44,
	 -36,  -33,  -39,  -40,  -49,  -63,  -30, -119,
};
const std::array<int, 64> osc_mg_bishop_table = {
	-37, -14, -85, -48, -21, -52,  -6, -18,
	-22,   2, -14,  -7,  17,  37,   4, -43,
	 -4,  19,  20,  25,  16,  43,  26,  -8,
	-26, -10,   6,  45,  30,  14,  -3,  -9,
	-10,  13,  -5,  15,  25,  -4,  -1,  -7,
	 12,  15,   8,  -5,  -6,   3,  11,   5,
	  0,  30,   4,   1,  -6,   2,   3,  -5,
	-15, -38, -12,  -7, -31, -16, -13, -34,
};
const std::array<int, 64> osc_mg_rook_table = {
	 18,  47,  30,  45,  52,  27,  34,  48,
	 24,  38,  59,  69,  71,  72,  31,  47,
	 -7,  22,  26,  40,  21,  48,  57,  14,
	-23,  -6,   9,  27,  31,  37, -10, -19,
	-30, -28, -19,  -7,   9,  -2,   2, -23,
	-47, -28, -14, -11,   0,   2,  -7, -33,
	-44, -14, -20,  -7, -15,   5,  -7, -71,
	-17, -36,  -4,   5,   3,  -1, -10,  -7,
};
const std::array<int, 64> osc_mg_queen_table = {
	-30, -19,  22,   1,  39,  43,  38,  43,
	-23, -47, -16,   1, -12,  57,  25,  46,
	-25, -22,  -3,   5,  24,  63,  48,  54,
	-30, -33, -20, -22,  -6,  10,  -3,  -2,
	-20, -37, -15, -11, -16, -11,  -2,  -5,
	-23, -15, -13,  -7, -22,  -6,  13,   4,
	-40, -15,   6,   2,   9,  18,  -6,  -8,
	-56, -40, -30, -19,   3,  -2, -18,  -3,
};
const std::array<int, 64> osc_mg_king_table = {
	-76,  17,   8, -23,   0,   0,   0,   0,
	 19, -14, -14, -21,   0,   0,   0,   0,
	-19,  15, -15, -25,   0,   0,   0,   0,
	-21, -31, -16, -30,   0,   0,   0,   0,
	-62,  -1, -44, -52,   0,   0,   0,   0,
	-25, -23, -35, -62,   0,   0,   0,   0,
	-18,  -6, -24, -82,   0,   0,   0,   0,
	-33,  20,  -7, -80,   0,   0,   0,   0,
};
