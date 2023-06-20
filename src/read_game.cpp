# include <fstream>
# include "read_game.hpp"

std::vector<std::string> read_game(std::string game_path){
	std::ifstream game_file;
	game_file.open(game_path);

	std::string move;
	std::vector<std::string> v;
	while (std::getline(game_file, move, ' ')){
		v.push_back(move);
	}
	
	return v;
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

TEST_CASE("Opera game"){
	auto game = read_game("classics/opera_game.uci");
	
	CHECK(game[0] == "e2e4");
	CHECK(game.size() == 33);
	CHECK(game[32] == "d1d8");
}

# endif