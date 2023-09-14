# include <fstream>
# include <iostream>
# include "board.hpp"
# include "parse_format.hpp"
# include "read_game.hpp"

const std::string games_dir = "games/";
const std::string pgn_file_name = "/game.pgn";

move_vec read_game(std::string game_path){
	std::ifstream game_file;
	game_file.open(game_path);

	Board board;
	bool wtm = parse_fen(STARTING_FEN, board);

	std::string move_str;
	move_vec v;
	while (std::getline(game_file, move_str, ' ')){
		Move move = parse_move_san(move_str, board, wtm);
		v.push_back(move);
		(wtm ? make_move<true> : make_move<false>)(board, move);
		wtm = not wtm;
	}
	
	return v;
}

string_vec find_game_paths(int num_tournaments, char **tournament_names){
	std::cout << "Using " << num_tournaments << " tournaments" << std::endl;
	string_vec game_paths;
	for (int i = 0; i < num_tournaments; i++){
		for (const auto & entry : std::filesystem::directory_iterator(games_dir + tournament_names[i])){
			std::string path = entry.path();
			game_paths.push_back(path + pgn_file_name);
		}
	}
	return game_paths;
}

std::vector<move_vec> load_games(string_vec game_paths){
	std::cout << "Found " << game_paths.size() << " games to load..." << std::endl;
	std::vector<move_vec> games;
	for (const auto path : game_paths){
		games.push_back(read_game(path));
		if (games.size() % 1000 == 0) std::cout << "Loaded " << games.size() << " games..." << std::endl;
	}
	return games;
}
