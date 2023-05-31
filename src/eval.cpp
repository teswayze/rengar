# include "board.hpp"
# include "eval.hpp"
# include "pst.hpp"
# include "endgames.hpp"

const int mg_bishop_atk = 3;
const int mg_rook_atk = 3;
const int mg_queen_atk = -1;
const int mg_tempo = 5;

const int eg_bishop_atk = 4;
const int eg_rook_atk = 4;
const int eg_queen_atk = 5;
const int eg_tempo = -1;


template <bool wtm>
int eval(const Board &board)
{
	const int sign = wtm ? 1 : -1;

	const auto &info = board.EvalInfo;
	const int bishop_atk_cnt = __builtin_popcountll(board.WtAtk.Bishop) - __builtin_popcountll(board.BkAtk.Bishop);
	const int rook_atk_cnt = __builtin_popcountll(board.WtAtk.Rook) - __builtin_popcountll(board.BkAtk.Rook);
	const int queen_atk_cnt = __builtin_popcountll(board.WtAtk.Queen) - __builtin_popcountll(board.BkAtk.Queen);

	const int mg_eval = info.mg + mg_bishop_atk * bishop_atk_cnt + mg_rook_atk * rook_atk_cnt + mg_queen_atk * queen_atk_cnt + mg_tempo * sign;
	const int eg_eval = info.eg + eg_bishop_atk * bishop_atk_cnt + eg_rook_atk * rook_atk_cnt + eg_queen_atk * queen_atk_cnt + eg_tempo * sign;
    const int raw_eval = eg_eval * 256 + info.phase_count * (mg_eval - eg_eval);

    return sign * make_endgame_adjustment(raw_eval, board);
}

template int eval<true>(const Board&);
template int eval<false>(const Board&);


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("Starting Position"){
	bool wtm; Board board;
	wtm = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	CHECK(-2560 < eval<true>(board));
	CHECK(eval<true>(board) < 2560);
}

# endif
