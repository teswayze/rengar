# include "doctest.h"
# include "read_game.hpp"

TEST_CASE("Opera game"){
	auto game = read_game("classics/opera_game.uci");
	
	CHECK(game[0] == "e2e4");
	CHECK(game.size() == 33);
	CHECK(game[32] == "d1d8");
}