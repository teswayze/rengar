# include <iostream>
# include <sstream>
# include <tuple>

# include "history.hpp"
# include "parse_format.hpp"
# include "search.hpp"
# include "movegen.hpp"
# include "hashtable.hpp"


void print_legal_moves(bool wtm, Board &board, ChecksAndPins cnp){
	if (wtm) {
		auto queue = generate_moves<true>(board, cnp, 0, 0, 0);
		while (!queue.empty()){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	} else {
		auto queue = generate_moves<false>(board, cnp, 0, 0, 0);
		while (!queue.empty()){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	}
}


void print_forcing_moves(bool wtm, Board &board, ChecksAndPins cnp){
	if (wtm) {
		auto queue = generate_forcing<true>(board, cnp);
		while (!queue.empty()){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	} else {
		auto queue = generate_forcing<false>(board, cnp);
		while (!queue.empty()){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	}
}

uint8_t HASH_KEY_LENGTH = 24;


int main() {
	bool wtm = true;
	Board board;
	History history = nullptr;

	ht_init(HASH_KEY_LENGTH);

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		std::stringstream input_stream(input);

		std::string command;
		std::getline(input_stream, command, ' ');

		if (command == "uci"){
			std::cout << "id name Rengar\n";
			std::cout << "id author Thomas Swayze\n";
			std::cout << "uciok\n";
		}
		if (command == "debug") {
			std::string arg;
			std::getline(input_stream, arg, ' ');
			if (arg == "on"){
				set_log_level(2);
			} else if (arg == "off"){
				set_log_level(1);
			}
		}
		if (command == "isready"){
			std::cout << "readyok\n";
		}
		if (command == "ucinewgame"){
			ht_init(HASH_KEY_LENGTH);
		}
		if (command == "position"){
			std::string postype;
			std::getline(input_stream, postype, ' ');
			std::string fen;
			if (postype == "startpos"){
				fen = STARTING_FEN;
			} else if (postype == "fen"){
				fen = "";
				for (int i = 0; i < 6; i++){
					std::string fen_chunk;
					std::getline(input_stream, fen_chunk, ' ');
					fen = fen + (i == 0 ? "" : " ") + fen_chunk;
				}
			}
			wtm = parse_fen(fen, board);
			history = nullptr;

			std::getline(input_stream, command, ' ');
		}
		if (command == "moves"){
			for (std::string move_text; std::getline(input_stream, move_text, ' ');){
				Move move = parse_move_xboard(move_text, board, wtm);
				if (is_irreversible(board, move)){
					history = nullptr;
				} else {
					history = extend_history(board, history);
				}
				(wtm ? make_move<true> : make_move<false>) (board, move);
				wtm = !wtm;
			}
		}
		if (command == "show"){
			dump_board(board);
		}
		if (command == "legal"){
			auto cnp = wtm ? checks_and_pins<true>(board) : checks_and_pins<false>(board);
			print_legal_moves(wtm, board, cnp);
		}
		if (command == "forcing"){
			auto cnp = wtm ? checks_and_pins<true>(board) : checks_and_pins<false>(board);
			if (cnp.CheckMask != FULL_BOARD) { print_legal_moves(wtm, board, cnp); }
			else { print_forcing_moves(wtm, board, cnp); }
		}
		if (command == "go"){
			int nodes = 0;
			for (std::string arg; input_stream >> arg;) {
				if (arg == "nodes") {
					input_stream >> nodes;
				} else {
					throw std::invalid_argument("Unsupported go option " + arg);
				}
			}
			if (nodes == 0) {
				throw std::invalid_argument("Never got how many nodes to search");
			}
			auto eval_and_move = (wtm ? search_for_move<true> : search_for_move<false>)(board, history, nodes);
			Move move = std::get<1>(eval_and_move)->head;
			std::cout << "bestmove " << format_move_xboard(move) << "\n";
		}
		if (command == "ponderhit"){
			throw std::invalid_argument("Pondering not supported");
		}
		if (command == "hashstats"){
			ht_stats();
		}
		if (command == "quit"){
			return 0;
		}
	}

	return -1;
}
