# include <fstream>
# include <iostream>
# include "../board.hpp"
# include "../parse_format.hpp"
# include "read_game.hpp"

const std::string pgn_file_name = "/game.pgn";
const std::string results_file_name = "/result.txt";

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

std::vector<move_vec> load_games(string_vec game_paths){
	std::cout << "Found " << game_paths.size() << " games to load..." << std::endl;
	std::vector<move_vec> games;
	for (const auto &path : game_paths){
		games.push_back(read_game(path + pgn_file_name));
		if (games.size() % 1000 == 0) std::cout << "Loaded " << games.size() << " games..." << std::endl;
	}
	return games;
}

int_vec read_results(string_vec game_paths){
	std::cout << "Reading results from " << game_paths.size() << " games..." << std::endl;
	int_vec results;
	for (const auto &path : game_paths){
		std::ifstream game_file;
		game_file.open(path + results_file_name);
		std::string res_str;
		std::getline(game_file, res_str, ' ');

		if (res_str == "White") results.push_back(1);
		else if (res_str == "Black") results.push_back(-1);
		else if (res_str == "Draw") results.push_back(0);
		else throw std::invalid_argument("Expected 'White', 'Black' or 'Draw' but saw " + res_str);

		if (results.size() % 1000 == 0) std::cout << "Read " << results.size() << " results..." << std::endl;
	}
	return results;
}