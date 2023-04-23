# include "board.hpp"
# include "eval.hpp"
# include "pst.hpp"
# include "endgames.hpp"


int eval(const Board &board)
{
    int raw_eval = eval_from_info(board.EvalInfo);
    return make_endgame_adjustment(raw_eval, board);
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("Starting Position"){
	bool wtm; Board board;
	wtm = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	CHECK(eval(board) == 0);
}

# endif
