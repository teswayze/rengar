# include <tuple>
# include <iostream>
# include <climits>

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
# include "variation.hpp"

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
	const bool is_check = (cnp.CheckMask != FULL_BOARD);
	if (not is_check) {
		best_eval = eval<white>(board);
		if (best_eval >= beta) {
			leaf_nodes++;
			return best_eval;
		}
	}
	qnodes++;
	auto queue = is_check ? generate_moves<white>(board, cnp, 0, 0, 0) : generate_forcing<white>(board, cnp);

	while (best_eval < beta and not queue.empty() and (is_check or queue.top_prio() > 0)){
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
std::tuple<int, VariationView> search_helper(const Board &board, const int depth, const int alpha, const int beta,
		History &history, const VariationView last_pv, const Move sibling_killer1, const Move sibling_killer2){
	if (is_insufficient_material(board)){ return std::make_tuple(0, last_pv.nullify()); }
	if (history.is_repetition(board.EvalInfo.hash)){ return std::make_tuple(0, last_pv.nullify()); }

	if (depth <= 0){
		non_terminal_node_found = true;
		return std::make_tuple(search_extension<white>(board, alpha, beta), last_pv.nullify());
	}

	const auto hash_key = white ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
	const auto hash_lookup_result = ht_lookup(hash_key);

	Move lookup_move = 0;
	if (hash_lookup_result.has_value()){
		int lookup_eval;
		uint8_t lookup_depth;
		std::tie(lookup_eval, lookup_move, lookup_depth) = hash_lookup_result.value();
		if ((lookup_depth >= depth) and (lookup_eval >= beta) and (move_flags(lookup_move) != EN_PASSANT_CAPTURE)){
			return std::make_tuple(lookup_eval, last_pv.singleton(lookup_move));
		}
	}

	const auto cnp = checks_and_pins<white>(board);
	const bool is_check = cnp.CheckMask != FULL_BOARD;
	Move child_killer1 = 0;
	Move child_killer2 = 0;
	if (allow_pruning and not is_check and last_pv.length == 0) {
		if (depth <= 2) {
			const int futility_eval = eval<white>(board) - depth * 128;
			if (futility_eval >= beta) {
				futility_prunes++;
				return std::make_tuple(futility_eval, last_pv.nullify());
			}
		} else {
			History fresh_history = history.make_irreversible();
			const auto nms_result = search_helper<not white>(board, depth - 3, -beta, -beta + 1, fresh_history, last_pv, 0, 0);
			const int nms_eval = -std::get<0>(nms_result);
			const VariationView nms_var = std::get<1>(nms_result);
			if (nms_eval >= beta) {
				if (depth <= 4) {
					null_move_prunes++;
					return std::make_tuple(nms_eval, last_pv.nullify());
				}
				const auto zz_check_result = search_helper<white, false>(board, depth - 4, beta - 1, beta, history, last_pv, sibling_killer1, sibling_killer2);
				if (std::get<0>(zz_check_result) >= beta) {
					null_move_prunes++;
					return zz_check_result;
				}
				zz_checks_failed++;
			}
			else if (nms_var.length) {
				child_killer1 = nms_var.head();
			}
		}
	}

	positions_seen++;

	auto queue = generate_moves<white>(board, cnp, last_pv.length ? last_pv.head() : lookup_move, sibling_killer1, sibling_killer2);
	if (queue.empty()){
		if (not is_check){
			return std::make_tuple(0, last_pv.nullify());
		}
		return std::make_tuple(CHECKMATED, last_pv.nullify());
	}

	int best_eval = INT_MIN;
	VariationView best_var = last_pv;
	int depth_reduction = 0;
	int move_index = 0;
	int reduction_index_cutoff = 8;
	const int next_depth = is_check ? depth : (depth - 1);
	while (best_eval < beta and not queue.empty() and depth_reduction <= next_depth){
		const int curr_alpha = std::max(alpha, best_eval);
		const Move branch_move = queue.top();
		Board branch_board = board.copy();
		make_move<white>(branch_board, branch_move);
		History branch_history = is_irreversible(board, branch_move) ? history.make_irreversible() : history.extend(board.EvalInfo.hash);
		const VariationView branch_hint = (last_pv.length and (last_pv.head() == branch_move)) ? last_pv.copy_branch() : last_pv.fresh_branch();

		auto search_res = search_helper<not white>(branch_board, next_depth - depth_reduction,
				-(depth_reduction ? (curr_alpha + 1) : beta), -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
		int branch_eval = -std::get<0>(search_res);

		if (depth_reduction and (branch_eval > curr_alpha)){
			search_res = search_helper<not white>(branch_board, next_depth, -beta, -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
			branch_eval = -std::get<0>(search_res);
		}

		const VariationView branch_var = std::get<1>(search_res);
		if (branch_eval > best_eval) {
			best_var = branch_var.prepend(branch_move);
			best_eval = branch_eval;
		} else if (branch_var.length) {
			const Move refutation = branch_var.head();
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
		ht_put(hash_key, std::make_tuple(best_eval, best_var.head(), depth));
	} else { fail_low++; }

	if (best_eval >= beta) fail_high++;

	return std::make_tuple(best_eval, best_var);
}

void log_info(Timer timer, int depth, VariationView var, int eval){
	std::cout << "info depth " << depth << " time " << timer.ms_elapsed() << " nodes " << positions_seen <<
					" pv" << show_variation(var) << " score cp " << eval <<  "\n";
}

template <bool white>
Move search_for_move(const Board &board, History &history, const int node_limit, const int depth_limit, const int time_limit_ms){
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
	VariationWorkspace workspace;
	VariationView var = VariationView(workspace);

	int depth = 0;
	int eval = 0;
	non_terminal_node_found = true;
	while ((CHECKMATED < eval) and (eval < -CHECKMATED) and non_terminal_node_found
			and (positions_seen < node_limit) and (depth < depth_limit) and (timer.ms_elapsed() < time_limit_ms)){
		depth++;
		non_terminal_node_found = false;
		std::tie(eval, var) = search_helper<white>(board, depth, CHECKMATED, -CHECKMATED, history, var, 0, 0);
		if (log_level >= 2) { log_info(timer, depth, var, eval); }
	}

	if (log_level == 1) { log_info(timer, depth, var, eval); }
	return var.head();
}

template Move search_for_move<true>(const Board&, History&, const int, const int, const int);
template Move search_for_move<false>(const Board&, History&, const int, const int, const int);
