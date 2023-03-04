# include <tuple>
# include <exception>
# include <iostream>
# include <chrono>

# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "eval.hpp"
# include "move.hpp"
# include "search.hpp"
# include "history.hpp"
# include "parse_format.hpp"

int positions_seen;
int max_nodes;
bool verbose;

void set_debug(bool debug){ verbose = debug; }

struct NodeLimitReached{};


inline void new_position(){
	if (positions_seen >= max_nodes) { throw NodeLimitReached(); }
	positions_seen++;
}

template <bool white>
int search_extension(const Board board, const int alpha, const int beta, const History history){
	if (exists_in_history(board, history)){ return 0; }

	new_position();

	auto cnp = checks_and_pins<white>(board);
	int best_eval = CHECKMATED;
	const bool is_check = (cnp.CheckMask == FULL_BOARD);
	if (is_check) {
		best_eval = white ? eval(board) : -eval(board);
		if (best_eval >= beta) { return best_eval; }
	}
	auto queue = is_check ? generate_forcing<white>(board, cnp, 0) : generate_moves<white>(board, cnp, 0);

	while (not queue.empty() and best_eval < beta){
		const Move branch_move = queue.top();
		const Board branch_board = make_move<white>(board, branch_move);
		const History branch_history = is_check ? extend_history(board, history) : nullptr;
		const int branch_eval = -search_extension<not white>(
				branch_board, -beta, -std::max(alpha, best_eval), branch_history);
		best_eval = std::max(branch_eval, best_eval);
		queue.pop();
	}
	return best_eval;
}


template <bool white>
std::tuple<int, Variation> search_helper(const Board board, const int depth, const int alpha, const int beta,
		const History history, const Variation last_pv){
	if (depth == 0){
		return std::make_tuple(search_extension<white>(board, alpha, beta, history), nullptr);
	}

	if (exists_in_history(board, history)){ return std::make_tuple(0, nullptr); }

	new_position();

	const auto cnp = checks_and_pins<white>(board);
	const bool is_check = cnp.CheckMask != FULL_BOARD;
	if (not is_check) {
		const int futility_eval = (white ? eval(board) : -eval(board)) - (depth << 12);
		if (futility_eval >= beta) { return std::make_tuple(futility_eval, nullptr); }
	}

	auto queue = generate_moves<white>(board, cnp, last_pv ? last_pv->head : 0);
	if (queue.empty()){
		if (not is_check){
			return std::make_tuple(0, nullptr);
		}
		return std::make_tuple(CHECKMATED - depth, nullptr);
	}

	int best_eval = INT_MIN;
	Variation best_var = nullptr;
	while (not queue.empty() and best_eval < beta){
		const Move branch_move = queue.top();
		const Board branch_board = make_move<white>(board, branch_move);
		const History branch_history = is_irreversible(board, branch_move) ?
				nullptr : extend_history(board, history);
		const Variation branch_hint = (last_pv and (last_pv->head == branch_move)) ? last_pv->tail : nullptr;

		try {
			const auto search_res = search_helper<not white>(branch_board, is_check ? depth : (depth - 1),
					-beta, -std::max(alpha, best_eval), branch_history, branch_hint);
			const int branch_eval = -std::get<0>(search_res);
			if (branch_eval > best_eval) {
				const Variation branch_var = std::get<1>(search_res);
				best_var = prepend_to_variation(branch_move, branch_var);
				best_eval = branch_eval;
			}
		} catch (const NodeLimitReached &e) {
			if (last_pv and best_var) { return std::make_tuple(best_eval, best_var); }
			throw e;
		}

		queue.pop();
	}

	return std::make_tuple(best_eval, best_var);
}

void log_info(std::chrono::time_point<std::chrono::high_resolution_clock> start,
		int depth, Variation var, int eval){
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
	std::cout << "info depth " << depth << " time " << duration_ms << " nodes " << positions_seen <<
					" pv" << show_variation(var) << " score cp " << (eval / 24) <<  "\n";
}

template <bool white>
std::tuple<int, Variation> search_for_move(const Board board, const History history, const int node_limit){
	positions_seen = 0;
	max_nodes = node_limit;

	auto start = std::chrono::high_resolution_clock::now();
	int depth = 0;
	int eval = 0;
	Variation var = nullptr;
	try {while ((CHECKMATED < eval) and (eval < -CHECKMATED) and (positions_seen < max_nodes)){
		depth++;
		std::tie(eval, var) = search_helper<white>(board, depth, 2 * CHECKMATED, -2 * CHECKMATED, history, var);
		if (verbose) { log_info(start, depth, var, eval); }
	}} catch (const NodeLimitReached &e) { }

	log_info(start, depth, var, eval);
	return std::make_tuple(eval, var);
}

template std::tuple<int, Variation> search_for_move<true>(const Board, const History, const int);
template std::tuple<int, Variation> search_for_move<false>(const Board, const History, const int);
