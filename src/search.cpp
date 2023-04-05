# include <tuple>
# include <exception>
# include <iostream>
# include <chrono>

# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "eval.hpp"
# include "search.hpp"
# include "history.hpp"
# include "parse_format.hpp"
# include "hashing.hpp"
# include "hashtable.hpp"

int positions_seen;
int max_nodes;
int log_level = 1;

void set_log_level(int level){ log_level = level; }

struct NodeLimitReached{};


inline void new_position(){
	if (positions_seen >= max_nodes) { throw NodeLimitReached(); }
	positions_seen++;
}

template <bool white>
int search_extension(const Board board, const int alpha, const int beta){
	new_position();

	auto cnp = checks_and_pins<white>(board);
	int best_eval = CHECKMATED;
	const bool not_check = (cnp.CheckMask == FULL_BOARD);
	if (not_check) {
		best_eval = white ? eval(board) : -eval(board);
		if (best_eval >= beta) { return best_eval; }
	}
	auto queue = not_check ? generate_forcing<white>(board, cnp) : generate_moves<white>(board, cnp, 0, 0, 0);

	while (not queue.empty() and best_eval < beta){
		const Move branch_move = queue.top();
		const Board branch_board = make_move_with_new_eval<white>(board, branch_move, queue.top_eval_info());
		const int branch_eval = -search_extension<not white>(branch_board, -beta, -std::max(alpha, best_eval));
		best_eval = std::max(branch_eval, best_eval);
		queue.pop();
	}
	return best_eval;
}


template <bool white>
std::tuple<int, Variation> search_helper(const Board board, const int depth, const int alpha, const int beta,
		const History history, const Variation last_pv, const Move sibling_killer1, const Move sibling_killer2){
	if (exists_in_history(board, history)){ return std::make_tuple(0, nullptr); }

	if (depth == 0){
		return std::make_tuple(search_extension<white>(board, alpha, beta), nullptr);
	}

	const auto hash_key = white ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
	const auto hash_lookup_result = ht_lookup(hash_key);

	Move lookup_move = 0;
	if (hash_lookup_result.has_value()){
		int lookup_eval;
		uint8_t lookup_depth;
		std::tie(lookup_eval, lookup_move, lookup_depth) = hash_lookup_result.value();
		if ((lookup_depth >= depth) and (lookup_eval >= beta) and (move_flags(lookup_move) != EN_PASSANT_CAPTURE)){
			return std::make_tuple(lookup_eval, prepend_to_variation(lookup_move, nullptr));
		}
	}

	new_position();

	const auto cnp = checks_and_pins<white>(board);
	const bool is_check = cnp.CheckMask != FULL_BOARD;
	if (not is_check) {
		const int futility_eval = (white ? eval(board) : -eval(board)) - (depth << 12);
		if (futility_eval >= beta) { return std::make_tuple(futility_eval, nullptr); }
	}

	auto queue = generate_moves<white>(board, cnp, last_pv ? last_pv->head : lookup_move, sibling_killer1, sibling_killer2);
	if (queue.empty()){
		if (not is_check){
			return std::make_tuple(0, nullptr);
		}
		return std::make_tuple(CHECKMATED - depth, nullptr);
	}

	int best_eval = INT_MIN;
	Variation best_var = nullptr;
	Move child_killer1 = 0;
	Move child_killer2 = 0;
	while (not queue.empty() and best_eval < beta){
		const Move branch_move = queue.top();
		const Board branch_board = make_move_with_new_eval<white>(board, branch_move, queue.top_eval_info());
		const History branch_history = is_irreversible(board, branch_move) ? nullptr : extend_history(board, history);
		const Variation branch_hint = (last_pv and (last_pv->head == branch_move)) ? last_pv->tail : nullptr;

		try {
			const auto search_res = search_helper<not white>(branch_board, is_check ? depth : (depth - 1),
					-beta, -std::max(alpha, best_eval), branch_history, branch_hint, child_killer1, child_killer2);
			const int branch_eval = -std::get<0>(search_res);
			const Variation branch_var = std::get<1>(search_res);
			if (branch_eval > best_eval) {
				best_var = prepend_to_variation(branch_move, branch_var);
				best_eval = branch_eval;
			} else if (branch_var) {
				const Move refutation = branch_var->head;
				if ((refutation != child_killer1) and (move_destination(refutation) != move_destination(branch_move))
						and not (ToMask(move_destination(refutation)) & get_side<white>(board).All)) {
					child_killer2 = child_killer1;
					child_killer1 = refutation;
				}
			}
		} catch (const NodeLimitReached &e) {
			if (last_pv and best_var) { return std::make_tuple(best_eval, best_var); }
			throw e;
		}

		queue.pop();
	}

	if (best_eval > alpha) {
		ht_put(hash_key, std::make_tuple(best_eval, best_var->head, depth));
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
std::tuple<int, Variation> search_for_move(const Board board, const int node_limit){
	positions_seen = 0;
	max_nodes = node_limit;

	auto start = std::chrono::high_resolution_clock::now();
	int depth = 0;
	int eval = 0;
	Variation var = nullptr;
	try {while ((CHECKMATED < eval) and (eval < -CHECKMATED) and (positions_seen < max_nodes)){
		depth++;
		std::tie(eval, var) = search_helper<white>(board, depth, 2 * CHECKMATED, -2 * CHECKMATED, nullptr, var, 0, 0);
		if (log_level >= 2) { log_info(start, depth, var, eval); }
	}} catch (const NodeLimitReached &e) { }

	if (log_level >= 1) { log_info(start, depth, var, eval); }
	return std::make_tuple(eval, var);
}

template std::tuple<int, Variation> search_for_move<true>(const Board, const int);
template std::tuple<int, Variation> search_for_move<false>(const Board, const int);
