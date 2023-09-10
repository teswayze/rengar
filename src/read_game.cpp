# include <fstream>
# include "board.hpp"
# include "parse_format.hpp"
# include "read_game.hpp"

std::vector<Move> read_game(std::string game_path){
	std::ifstream game_file;
	game_file.open(game_path);

	Board board;
	bool wtm = parse_fen(STARTING_FEN, board);

	std::string move_str;
	std::vector<Move> v;
	while (std::getline(game_file, move_str, ' ')){
		Move move = parse_move_san(move_str, board, wtm);
		v.push_back(move);
		(wtm ? make_move<true> : make_move<false>)(board, move);
		wtm = not wtm;
	}
	
	return v;
}
