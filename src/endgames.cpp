# include "endgames.hpp"

bool is_insufficient_material(const Board &board){
	return (board.EvalInfo.phase_count <= 1) and (not board.White.Pawn) and (not board.Black.Pawn);
}

inline bool only_has_minor(const HalfBoard &side){
	return (not side.Rook) and (not side.Queen) and (__builtin_popcountll(side.Bishop | side.Knight) <= 1);
}

int make_endgame_adjustment(int raw_eval, const Board &board){
	if ((not board.White.Pawn) and (raw_eval > 0)){
		if (only_has_minor(board.White)){
			// White only has a single minor - checkmate can't be forced
			return 0;
		}
		// White may be able to checkmate, but without pawns it's more difficult
		return raw_eval - raw_eval / 8;
	}
	if ((not board.Black.Pawn) and (raw_eval < 0)){
		if (only_has_minor(board.Black)){
			// Black only has a single minor - checkmate can't be forced
			return 0;
		}
		// Black may be able to checkmate, but without pawns it's more difficult
		return raw_eval - raw_eval / 8;
	}
	return raw_eval;
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("KPvK"){
	Board board;
	parse_fen("8/8/8/4k3/8/3K4/4P3/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(make_endgame_adjustment(100, board) == 100);
}

TEST_CASE("KQvK"){
	Board board;
	parse_fen("K7/1Q6/8/8/8/4k3/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 100);
}

TEST_CASE("KRvK"){
	Board board;
	parse_fen("7K/8/8/8/5k2/1R6/8/8 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 100);
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
	CHECK(make_endgame_adjustment(100, board) < 100);
}

TEST_CASE("KBNvK"){
	Board board;
	parse_fen("K7/2kB4/8/8/8/8/8/5N2 w - - 0 1", board);
	CHECK(not is_insufficient_material(board));
	CHECK(0 < make_endgame_adjustment(100, board));
	CHECK(make_endgame_adjustment(100, board) < 100);
}

# endif
