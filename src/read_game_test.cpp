# include "doctest.h"
# include "read_game.hpp"

TEST_CASE("Opera game"){
	auto game = read_game("classics/opera_game.pgn");
	
	CHECK(game[0] == move_from_squares(E2, E4, DOUBLE_PAWN_PUSH));
	CHECK(game.size() == 33);
	CHECK(game[32] == move_from_squares(D1, D8, ROOK_MOVE));
}