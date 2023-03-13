# include "board.hpp"
# include "eval.hpp"
# include "pst.hpp"


int eval(const Board board)
{
    return eval_from_info(board.EvalInfo);
}

int phase_of_game(const Board board){
	return std::min(board.EvalInfo.phase_count, 24);
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("Starting Position"){
	Board board = std::get<1>(parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
	CHECK(eval(board) == 0);
	CHECK(phase_of_game(board) == 24);
}

TEST_CASE("Albin Countergambit Missed Brilliancy"){
	Board board = std::get<1>(parse_fen("rnbqk1nr/ppp2ppp/8/4P3/1BP5/8/PP2K1PP/RN1Q1BqR w kq - 0 8"));
	CHECK(eval(board) == -25032);
	CHECK(phase_of_game(board) == 24);
}

TEST_CASE("Pieceless Endgame"){
	Board board = std::get<1>(parse_fen("8/8/8/4k3/8/4K3/4P3/8 w - - 0 1"));
	CHECK(eval(board) == 2472);
	CHECK(phase_of_game(board) == 0);
}

TEST_CASE("Berlin Endgame"){
	Board board = std::get<1>(parse_fen("r1bk1b1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNB2RK1 w - - 0 9"));
	CHECK(eval(board) == 656);
	CHECK(phase_of_game(board) == 14);
}

TEST_CASE("Fried Liver"){
	Board board = std::get<1>(parse_fen("r1bq1b1r/ppp3pp/2n1k3/3np3/2B5/2N2Q2/PPPP1PPP/R1B1K2R b KQ - 3 8"));
	CHECK(eval(board) == -5102);
	CHECK(phase_of_game(board) == 23);
}

TEST_CASE("Grunfeld Exchange Sacrifice"){
	Board board = std::get<1>(parse_fen("r2q1rk1/pp2p2p/4bpp1/n2P4/4P3/3BBP2/P3N1PP/Q4RK1 w - - 0 16"));
	CHECK(eval(board) == -1292);
	CHECK(phase_of_game(board) == 19);
}

TEST_CASE("Positional Sveshnikov"){
	Board board = std::get<1>(parse_fen("r1bqk2r/5ppp/p1np1b2/1p1Np3/4P3/N7/PPP2PPP/R2QKB1R w KQkq - 0 11"));
	CHECK(eval(board) == -1160);
	CHECK(phase_of_game(board) == 22);
}

TEST_CASE("Rook and Bishop Pawn vs Bishop Fortress"){
	Board board = std::get<1>(parse_fen("8/8/6B1/8/2k5/2p5/7r/2K5 w KQkq - 0 1"));
	CHECK(eval(board) == -9891);
	CHECK(phase_of_game(board) == 3);
}

# endif
