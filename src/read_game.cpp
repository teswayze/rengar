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
