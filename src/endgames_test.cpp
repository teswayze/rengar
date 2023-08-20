# include "doctest.h"
# include "parse_format.hpp"
# include "endgames.hpp"

TEST_CASE("KvK"){
	Board board;
	parse_fen("8/8/8/4k3/8/3K4/8/8 w - - 0 1", board);
	CHECK(is_insufficient_material(board));
}

TEST_CASE("KPvK"){
	Board board;
	parse_fen("8/8/8/4k3/8/3K4/4P3/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(__builtin_popcountll(board.White.Pawn) == 1);
	CHECK(50 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 100);
}

TEST_CASE("KQvK"){
	Board board;
	parse_fen("K7/1Q6/8/8/8/4k3/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(board.EvalInfo.phase_count > -22);
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 50);
}

TEST_CASE("KRvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1R6/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 50);
}

TEST_CASE("KBvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1B6/8/8 w - - 0 1", board);
	CHECK(is_insufficient_material(board));
}

TEST_CASE("KBvKP"){
	Board board;
	parse_fen("7K/8/8/8/4pk2/1B6/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(make_endgame_adjustment(100, board) == 0);
}

TEST_CASE("KNvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1N6/8/8 w - - 0 1", board);
	CHECK(is_insufficient_material(board));
}

TEST_CASE("KNvKP"){
	Board board;
	parse_fen("7K/8/8/8/4pk2/1N6/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(make_endgame_adjustment(100, board) == 0);
}

TEST_CASE("KBBvK"){
	Board board;
	parse_fen("3B4/8/8/8/Bk6/8/8/7K w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 50);
}

TEST_CASE("KBNvK"){
	Board board;
	parse_fen("K7/2kB4/8/8/8/8/8/5N2 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 50);
}
