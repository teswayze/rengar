# include "doctest.h"
# include "parse_format.hpp"
# include "eval.hpp"

TEST_CASE("Starting Position"){
	bool wtm; Board board;
	wtm = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	CHECK(-10 < eval<true>(board));
	CHECK(eval<true>(board) < 10);
}