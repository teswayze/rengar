# include "endgames.hpp"

bool is_insufficient_material(const Board &board){
	return (board.EvalInfo.phase_count <= 1) and (not board.White.Pawn) and (not board.Black.Pawn);
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("KPvK"){
	Board board;
	parse_fen("8/8/8/4k3/8/3K4/4P3/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
}

TEST_CASE("KQvK"){
	Board board;
	parse_fen("K7/1Q6/8/8/8/4k3/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
}

TEST_CASE("KRvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1R6/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
}

TEST_CASE("KBvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1B6/8/8 w - - 0 1", board);
	CHECK(is_insufficient_material(board));
}

TEST_CASE("KNvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1N6/8/8 w - - 0 1", board);
	CHECK(is_insufficient_material(board));
}

TEST_CASE("KBBvK"){
	Board board;
	parse_fen("3B4/8/8/8/Bk6/8/8/7K w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
}

TEST_CASE("KBBvK"){
	Board board;
	parse_fen("K7/2kB4/8/8/8/8/8/5N2 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
}

# endif
