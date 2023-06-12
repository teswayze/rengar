# include <tuple>
# include <exception>
# include <iostream>

# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "eval.hpp"
# include "search.hpp"
# include "history.hpp"
# include "parse_format.hpp"
# include "hashing.hpp"
# include "hashtable.hpp"
# include "endgames.hpp"
# include "timer.hpp"

int positions_seen = 0;
int leaf_nodes = 0;
int qnodes = 0;
int null_move_prunes = 0;
int zz_checks_failed = 0;
int futility_prunes = 0;
int fail_low = 0;
int fail_high = 0;

void search_stats(){
	std::cout << leaf_nodes << " leaf_nodes" << std::endl;
	std::cout << qnodes << " qnodes" << std::endl;
	std::cout << null_move_prunes << " null_move_prunes" << std::endl;
	std::cout << zz_checks_failed << " zz_checks_failed" << std::endl;
	std::cout << futility_prunes << " futility_prunes" << std::endl;
	std::cout << fail_low << " fail_low" << std::endl;
	std::cout << fail_high << " fail_high" << std::endl;
}

bool non_terminal_node_found = false;
int log_level = 1;

void set_log_level(int level){ log_level = level; }

template <bool white>
int search_extension(const Board &board, const int alpha, const int beta){
	positions_seen++;

	auto cnp = checks_and_pins<white>(board);
	int best_eval = CHECKMATED;
	const bool not_check = (cnp.CheckMask == FULL_BOARD);
	if (not_check) {
		best_eval = eval<white>(board);
		if (best_eval >= beta) {
			leaf_nodes++;
			return best_eval;
		}
	}
	qnodes++;
	auto queue = not_check ? generate_forcing<white>(board, cnp) : generate_moves<white>(board, cnp, 0, 0, 0);

	while (not queue.empty() and best_eval < beta){
		const Move branch_move = queue.top();
		Board branch_board = board.copy();
		make_move<white>(branch_board, branch_move);
		const int branch_eval = -search_extension<not white>(branch_board, -beta, -std::max(alpha, best_eval));
		best_eval = std::max(branch_eval, best_eval);
		queue.pop();
	}
	return best_eval;
}


template <bool white, bool allow_pruning=true>
std::tuple<int, Variation> search_helper(const Board &board, const int depth, const int alpha, const int beta,
		const History history, const Variation last_pv, const Move sibling_killer1, const Move sibling_killer2){
	if (is_insufficient_material(board)){ return std::make_tuple(0, nullptr); }
	if (exists_in_history(board, history)){ return std::make_tuple(0, nullptr); }

	if (depth <= 0){
		non_terminal_node_found = true;
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

	const auto cnp = checks_and_pins<white>(board);
	const bool is_check = cnp.CheckMask != FULL_BOARD;
	Move child_killer1 = 0;
	Move child_killer2 = 0;
	if (allow_pruning and not is_check) {
		if (depth <= 2) {
			const int futility_eval = eval<white>(board) - depth * 128;
			if (futility_eval >= beta) {
				futility_prunes++;
				return std::make_tuple(futility_eval, nullptr);
			}
		} else {
			const auto nms_result = search_helper<not white>(board, depth - 3, -beta, -beta + 1, nullptr, nullptr, 0, 0);
			const int nms_eval = -std::get<0>(nms_result);
			const Variation nms_var = std::get<1>(nms_result);
			if (nms_eval >= beta) {
				if (depth <= 4) {
					null_move_prunes++;
					return std::make_tuple(nms_eval, nullptr);
				}
				const auto zz_check_result = search_helper<white, false>(board, depth - 4, beta - 1, beta, history, nullptr, sibling_killer1, sibling_killer2);
				if (std::get<0>(zz_check_result) >= beta) {
					null_move_prunes++;
					return zz_check_result;
				}
				zz_checks_failed++;
			}
			else if (nms_var) {
				child_killer1 = nms_var->head;
			}
		}
	}

	positions_seen++;

	auto queue = generate_moves<white>(board, cnp, last_pv ? last_pv->head : lookup_move, sibling_killer1, sibling_killer2);
	if (queue.empty()){
		if (not is_check){
			return std::make_tuple(0, nullptr);
		}
		return std::make_tuple(CHECKMATED, nullptr);
	}

	int best_eval = INT_MIN;
	Variation best_var = nullptr;
	int depth_reduction = 0;
	int move_index = 0;
	int reduction_index_cutoff = 8;
	const int next_depth = is_check ? depth : (depth - 1);
	while (best_eval < beta and not queue.empty() and depth_reduction <= next_depth){
		const int curr_alpha = std::max(alpha, best_eval);
		const Move branch_move = queue.top();
		Board branch_board = board.copy();
		make_move<white>(branch_board, branch_move);
		const History branch_history = is_irreversible(board, branch_move) ? nullptr : extend_history(board, history);
		const Variation branch_hint = (last_pv and (last_pv->head == branch_move)) ? last_pv->tail : nullptr;

		auto search_res = search_helper<not white>(branch_board, next_depth - depth_reduction,
				-(depth_reduction ? (curr_alpha + 1) : beta), -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
		int branch_eval = -std::get<0>(search_res);

		if (depth_reduction and (branch_eval > curr_alpha)){
			search_res = search_helper<not white>(branch_board, next_depth, -beta, -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
			branch_eval = -std::get<0>(search_res);
		}

		const Variation branch_var = std::get<1>(search_res);
		if (branch_eval > best_eval) {
			best_var = prepend_to_variation(branch_move, branch_var);
			best_eval = branch_eval;
		} else if (branch_var) {
			const Move refutation = branch_var->head;
			if ((refutation != child_killer1) and (move_destination(refutation) != move_destination(branch_move))) {
				child_killer2 = child_killer1;
				child_killer1 = refutation;
			}
		}

		queue.pop();
		move_index += 1;
		if (move_index == reduction_index_cutoff) {
			depth_reduction += 1;
			reduction_index_cutoff *= 2;
		}
	}

	if (best_eval > alpha) {
		ht_put(hash_key, std::make_tuple(best_eval, best_var->head, depth));
	} else { fail_low++; }

	if (best_eval >= beta) fail_high++;

	return std::make_tuple(best_eval, best_var);
}

void log_info(Timer timer, int depth, Variation var, int eval){
	std::cout << "info depth " << depth << " time " << timer.ms_elapsed() << " nodes " << positions_seen <<
					" pv" << show_variation(var) << " score cp " << eval <<  "\n";
}

template <bool white>
std::tuple<int, Variation> search_for_move(const Board &board, const History history, const int node_limit, const int depth_limit, const int time_limit_ms){
	Timer timer;
	timer.start();

	positions_seen = 0;
	leaf_nodes = 0;
	qnodes = 0;
	null_move_prunes = 0;
	zz_checks_failed = 0;
	futility_prunes = 0;
	fail_low = 0;
	fail_high = 0;
	History trimmed_history = remove_hash_from_history(remove_single_repetitions(history), board);

	int depth = 0;
	int eval = 0;
	Variation var = nullptr;
	non_terminal_node_found = true;
	while ((CHECKMATED < eval) and (eval < -CHECKMATED) and non_terminal_node_found
			and (positions_seen < node_limit) and (depth < depth_limit) and (timer.ms_elapsed() < time_limit_ms)){
		depth++;
		non_terminal_node_found = false;
		std::tie(eval, var) = search_helper<white>(board, depth, CHECKMATED, -CHECKMATED, trimmed_history, var, 0, 0);
		if (log_level >= 2) { log_info(timer, depth, var, eval); }
	}

	if (log_level >= 1) { log_info(timer, depth, var, eval); }
	return std::make_tuple(eval, var);
}

template std::tuple<int, Variation> search_for_move<true>(const Board&, const History, const int, const int, const int);
template std::tuple<int, Variation> search_for_move<false>(const Board&, const History, const int, const int, const int);
