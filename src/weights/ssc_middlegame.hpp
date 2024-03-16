# pragma once

# include "../eval_param.hpp"

EVAL_PARAM(ssc_mg_pawn, 71)
EVAL_PARAM(ssc_mg_knight, 309)
EVAL_PARAM(ssc_mg_bishop, 291)
EVAL_PARAM(ssc_mg_rook, 396)
EVAL_PARAM(ssc_mg_queen, 846)
EVAL_PARAM(ssc_mg_bishop_pair, 5)

// All tables can be viewed from white's perspective with both kings on the kingside

EVAL_PARAM_ARRAY(64, ssc_mg_pawn_table,
	  0,   0,   0,   0,   0,   0,   0,   0,
	 90, 128,  65,  91,  71, 120,  26, -14,
	 -6,   5,  28,  28,  60,  55,  23, -21,
	-11,   8,   5,  14,  13,   4,  13, -28,
	-27, -13,  -1,  16,  12,   0,  -2, -31,
	-20,   0,  -3,  -7,   8,  -1,  31, -13,
	-27,  -8, -23, -30, -17,  26,  37, -30,
	  0,   0,   0,   0,   0,   0,   0,   0,
)
EVAL_PARAM_ARRAY(64, ssc_mg_knight_table,
	-179,  -98,  -49,  -55,   48, -111,  -26, -120,
	 -80,  -45,   55,   29,   10,   45,   -3,  -27,
	 -59,   49,   22,   57,   74,  120,   62,   32,
	 -13,   -1,   12,   55,   16,   64,   -1,   15,
	 -21,  -10,    2,   -4,   13,    6,    9,  -20,
	 -30,  -12,   -5,   -4,   11,    1,   14,  -39,
	 -44,  -60,  -23,    0,  -17,    1,  -27,  -27,
	-119,  -30,  -63,  -49,  -40,  -39,  -33,  -36,
)
EVAL_PARAM_ARRAY(64, ssc_mg_bishop_table,
	-37, -14, -85, -48, -21, -52,  -6, -18,
	-22,   2, -14,  -7,  17,  37,   4, -43,
	 -4,  19,  20,  25,  16,  43,  26,  -8,
	-26, -10,   6,  45,  30,  14,  -3,  -9,
	-10,  13,  -5,  15,  25,  -4,  -1,  -7,
	  5,  11,   3,  -6,  -5,   8,  15,  12,
	 -5,   3,   2,  -6,   1,   4,  30,   0,
	-34, -13, -16, -31,  -7, -12, -38, -15,
)
EVAL_PARAM_ARRAY(64, ssc_mg_rook_table,
	 18,  47,  30,  45,  52,  27,  34,  48,
	 24,  38,  59,  69,  71,  72,  31,  47,
	 -7,  22,  26,  40,  21,  48,  57,  14,
	-23,  -6,   9,  27,  31,  37, -10, -19,
	-30, -28, -19,  -7,   9,  -2,   2, -23,
	-47, -28, -14, -11,   0,   2,  -7, -33,
	-44, -14, -20,  -7, -15,   5,  -7, -71,
	 -7, -10,  -1,   3,   5,  -4, -36, -17,
)
EVAL_PARAM_ARRAY(64, ssc_mg_queen_table,
	-30, -19,  22,   1,  39,  43,  38,  43,
	-23, -47, -16,   1, -12,  57,  25,  46,
	-25, -22,  -3,   5,  24,  63,  48,  54,
	-30, -33, -20, -22,  -6,  10,  -3,  -2,
	-20, -37, -15, -11, -16, -11,  -2,  -5,
	-23, -15, -13,  -7, -22,  -6,  13,   4,
	-40, -15,   6,   2,   9,  18,  -6,  -8,
	 -3, -18,  -2,   3, -19, -30, -40, -56,
)
EVAL_PARAM_ARRAY(64, ssc_mg_king_table,
	  0,   0,   0,   0, -64, -33,  -7,  -3,
	  0,   0,   0,   0, -23, -24, -46, -36,
	  0,   0,   0,   0, -20, -11,  -9, -23,
	  0,   0,   0,   0, -25, -37, -29, -51,
	  0,   0,   0,   0, -54, -54, -46, -54,
	  0,   0,   0,   0, -61, -46, -28, -41,
	  0,   0,   0,   0, -65, -43,   4,   5,
	  0,   0,   0,   0, -25, -45,  25,  12,
)
