# include <iostream>
# include <sstream>
# include <tuple>
# include <climits>

# include "history.hpp"
# include "parse_format.hpp"
# include "search.hpp"
# include "movegen.hpp"
# include "hashtable.hpp"
# include "eval.hpp"
# include "hashing.hpp"

# ifndef RENGAR_VERSION
# define RENGAR_VERSION unversioned
# endif
// https://stackoverflow.com/questions/2410976/how-can-i-define-a-string-literal-on-the-gcc-command-line
# define STRINGIZE(x) #x
# define STRINGIZE_VALUE_OF(x) STRINGIZE(x)
const auto rengar_version = STRINGIZE_VALUE_OF(RENGAR_VERSION);

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
		while (!queue.empty() and queue.top_prio() > qsearch_prio_cutoff){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	} else {
		auto queue = generate_forcing<false>(board, cnp);
		while (!queue.empty() and queue.top_prio() > 0){
			std::cout << format_move_xboard(queue.top()) << "\n";
			queue.pop();
		}
	}
}

int HASH_KEY_LENGTH = 24;


int main() {
	Board board;
	bool wtm = parse_fen(STARTING_FEN, board);
	History history;

	set_log_level(2);
	ht_init(HASH_KEY_LENGTH);

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		std::stringstream input_stream(input);

		std::string command;
		std::getline(input_stream, command, ' ');

		if (command == "uci"){
			std::cout << "id name Rengar " << rengar_version << "\n";
			std::cout << "id author Thomas Swayze\n";
			std::cout << "option name hashbits type spin default " << HASH_KEY_LENGTH << " min 0 max 32\n";
			std::cout << "option name hash type spin default " << (sizeof(StorageValue) << (HASH_KEY_LENGTH - 20)) 
				<< " min 1 max 65536\n";
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
			initialize_move_order_arrays();
		}
		if (command == "setoption"){
			std::string arg;
			std::getline(input_stream, arg, ' ');
			if (arg == "name") {
				std::getline(input_stream, arg, ' ');
				if (arg == "hashbits") {
					std::getline(input_stream, arg, ' ');
					if (arg == "value") {
						input_stream >> HASH_KEY_LENGTH;
						ht_init(HASH_KEY_LENGTH);
					}
				} else if (arg == "hash") {
					std::getline(input_stream, arg, ' ');
					if (arg == "value") {
						int hash_size_mb;
						input_stream >> hash_size_mb;
						int num_hash_entries = (hash_size_mb << 20) / sizeof(StorageValue);
						HASH_KEY_LENGTH = 31 - __builtin_clz(num_hash_entries);
						ht_init(HASH_KEY_LENGTH);
					}
				}
			}
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
			history = history.wipe();

			std::getline(input_stream, command, ' ');
		}
		if (command == "moves"){
			for (std::string move_text; std::getline(input_stream, move_text, ' ');){
				Move move = parse_move_xboard(move_text, board, wtm);
				if (is_irreversible(board, move)){
					history = history.wipe();
				} else {
					history = history.extend_root(board.EvalInfo.hash);
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
			int nodes = INT_MAX;
			int depth = max_var_length;
			int min_time_ms = INT_MAX;
			int max_time_ms = INT_MAX;
			for (std::string arg; input_stream >> arg;) {
				if (arg == "nodes") {
					input_stream >> nodes;
				} else if (arg == "depth") {
					input_stream >> depth;
				} else if (arg == "movetime") {
					int movetime;
					input_stream >> movetime;
					min_time_ms = movetime / 2;
					max_time_ms = movetime;
				} else if (arg == "wtime") {
					int wtime;
					input_stream >> wtime;
					if (wtm) {
						min_time_ms = wtime / 40;
						max_time_ms = wtime / 4;
					}
				} else if (arg == "btime") {
					int btime;
					input_stream >> btime;
					if (not wtm) {
						min_time_ms = btime / 40;
						max_time_ms = btime / 4;
					}
				} else if (arg == "winc") {
					int winc;
					input_stream >> winc;
				} else if (arg == "binc") {
					int winc;
					input_stream >> winc;
				} else if (arg == "movestogo") {
					int movestogo;
					input_stream >> movestogo;
				} else {
					throw std::invalid_argument("Unsupported go option " + arg);
				}
			}
			if ((nodes == INT_MAX) and (depth == INT_MAX) and (max_time_ms == INT_MAX)) {
				throw std::invalid_argument("No limit on search");
			}
			Move move = (wtm ? search_for_move<true> : search_for_move<false>)(
				board, history, nodes, depth, min_time_ms, max_time_ms);
			std::cout << "bestmove " << format_move_xboard(move) << "\n";
		}
		if (command == "ponderhit"){
			throw std::invalid_argument("Pondering not supported");
		}
		if (command == "hashstats"){
			ht_stats();
		}
		if (command == "searchstats"){
			search_stats();
		}
		if (command == "lookup"){
			const auto hash_key = wtm ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
			const auto hash_lookup_result = ht_lookup(hash_key);
			if (hash_lookup_result.has_value()) {
				std::cout << "Score: " << std::get<0>(hash_lookup_result.value()) << std::endl;
				std::cout << "Move: " << format_move_xboard(std::get<1>(hash_lookup_result.value())) << std::endl;
				std::cout << "Depth: " << (int) std::get<2>(hash_lookup_result.value()) << std::endl;
			} else { std::cout << "Miss" << std::endl; }
		}
		if (command == "eval"){
			const auto mg_pst_eval = (board.White.King % 8 >= 4) ? 
				((board.Black.King % 8 >= 4) ? board.EvalInfo.mg_kk : board.EvalInfo.mg_kq) :
				((board.Black.King % 8 >= 4) ? board.EvalInfo.mg_qk : board.EvalInfo.mg_qq);
			std::cout << "mg = " << mg_pst_eval << std::endl;
			std::cout << "eg = " << board.EvalInfo.eg << std::endl;
			std::cout << "phase_count = " << board.EvalInfo.phase_count << std::endl;
			std::cout << "eval = " << (wtm ? eval<true>(board) : -eval<false>(board)) << std::endl;
		}
		if (command == "quit"){
			return 0;
		}
	}

	return -1;
}
