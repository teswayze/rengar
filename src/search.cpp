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
# include "options.hpp"
# include "syzygy_probe.hpp"

const int CHECKMATED = -10000;
const std::array<int, 5> TB_EVAL = {-5000, -26, 0, 26, 5000};

int positions_seen = 0;
int leaf_nodes = 0;
int qnodes = 0;
int null_move_prunes = 0;
int zz_checks_failed = 0;
int futility_prunes = 0;
int fail_low = 0;
int fail_high = 0;
int repetitions = 0;
int tbhits = 0;

void search_stats(){
	std::cout << leaf_nodes << " leaf_nodes" << std::endl;
	std::cout << qnodes << " qnodes" << std::endl;
	std::cout << null_move_prunes << " null_move_prunes" << std::endl;
	std::cout << zz_checks_failed << " zz_checks_failed" << std::endl;
	std::cout << futility_prunes << " futility_prunes" << std::endl;
	std::cout << fail_low << " fail_low" << std::endl;
	std::cout << fail_high << " fail_high" << std::endl;
	std::cout << repetitions << " repetitions" << std::endl;
}

int log_level = 0;

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

	while (best_eval < beta and not queue.empty()){
		const Move branch_move = queue.top();
		Board branch_board = board.copy();
		make_move<white>(branch_board, branch_move);
		const int branch_eval = -search_extension<not white>(branch_board, -beta, -std::max(alpha, best_eval));
		best_eval = std::max(branch_eval, best_eval);
		queue.pop();
	}
	return best_eval;
}


int _global_node_limit = INT_MAX;
struct NodeLimitSafety{};

const int encourage_progress_after = 20;
const int force_progress_after = 90;
bool encourage_progress = false;
bool force_progress = false;

std::string syzygy_path_ = "";
int max_num_pieces_ = 5;
Tablebase search_tb;
bool tb_deactivate = false;
bool tb_filter_draw = false;

void set_tb_path(std::string syzygy_path){
	syzygy_path_ = syzygy_path;
	search_tb = Tablebase(max_num_pieces_, syzygy_path_);
}
void set_tb_max_num_pieces(int max_num_pieces){
	max_num_pieces_ = max_num_pieces;
	if (syzygy_path_.length() > 0) search_tb = Tablebase(max_num_pieces_, syzygy_path_);
}

template <bool white, bool allow_pruning=true>
std::tuple<int, VariationView, int> search_helper(const Board &board, const int depth, const int alpha, const int beta,
		HistoryView &history, const VariationView last_pv, const Move sibling_killer1, const Move sibling_killer2){
	// Draw by insufficent material
	if (is_insufficient_material(board)){ return std::make_tuple(0, last_pv.nullify(), history.curr_idx); }
	// Draw by threefold repetition 
	// Also return 0 for no progress if we repeat a position - unless we've failed to make progress for a while
	// In that case we allow backtracking through a twofold repetition to find the best way to make progress
	if (history.curr_idx > history.history.root_idx) {
		int index_of_repetition = history.index_of_repetition(board.ue.hash, not encourage_progress);
		if (index_of_repetition != -1){ 
			repetitions++;
			if (index_of_repetition < history.history.root_idx) index_of_repetition = history.curr_idx;
			return std::make_tuple(0, last_pv.nullify(), index_of_repetition); 
		}
	}
	// Draw by fifty move rule
	if (force_progress and (history.irreversible_idx == 0) and (history.curr_idx == 100)) {
		// Just need to check for checkmate
		const auto cnp = checks_and_pins<white>(board);
		if (cnp.CheckMask == FULL_BOARD) return std::make_tuple(0, last_pv.nullify(), history.curr_idx);
		auto queue = generate_moves<white>(board, cnp, 0, 0, 0);
		if (not queue.empty()) return std::make_tuple(0, last_pv.nullify(), history.curr_idx);
		// Checkmate delivered on the 100th move!
		return std::make_tuple(CHECKMATED - depth, last_pv.nullify(), history.curr_idx);
	}

	if (depth <= 0){
		int qscore = search_extension<white>(board, alpha, beta);
		// If we haven't made progress for a while, halve the eval to encourage us to do something
		if (encourage_progress and history.irreversible_idx == 0) qscore /= 2;
		return std::make_tuple(qscore, last_pv.nullify(), history.curr_idx);
	}

	const auto hash_key = board.ue.hash ^ (white ? wtm_hash : 0) ^ 
		// Clear the hash when we enter mop-up mode to avoid scores from non-mop-up eval which has a different level
		(is_mop_up_mode() ? mop_up_hash : 0) ^
		// Clear the hash if we've started halving the eval or we'll get a lot of misleading scores from the hash table
		((encourage_progress and history.irreversible_idx == 0) ? halfmove_clock_hash[encourage_progress_after] : 0) ^
		// If nearing the 50-move-rule limit, a position's score may change based on how many moves we have to make progress
		((force_progress and history.irreversible_idx == 0) ? halfmove_clock_hash[history.curr_idx] : 0);
	const auto hash_lookup_result = ht_lookup(hash_key);

	Move lookup_move = 0;
	if (hash_lookup_result.has_value()){
		int lookup_eval;
		uint8_t lookup_depth;
		std::tie(lookup_eval, lookup_move, lookup_depth) = hash_lookup_result.value();
		if ((lookup_depth >= depth) and (lookup_eval >= beta) and (move_flags(lookup_move) != EN_PASSANT_CAPTURE)){
			return std::make_tuple(lookup_eval, last_pv.singleton(lookup_move), history.curr_idx);
		}
	}

	if (positions_seen >= _global_node_limit) throw NodeLimitSafety();

	if ((not tb_deactivate) and __builtin_popcountll(board.Occ) <= search_tb.max_num_pieces_) {
		tbhits++;
		int wdl = search_tb.probe_wdl(white, board);
		if ((not tb_filter_draw) or (wdl != 0)) return std::make_tuple(TB_EVAL[wdl + 2], last_pv.nullify(), history.curr_idx);
	}

	const auto cnp = checks_and_pins<white>(board);
	const bool is_check = cnp.CheckMask != FULL_BOARD;
	Move child_killer1 = 0;
	Move child_killer2 = 0;
	if (allow_pruning and not is_check and last_pv.length == 0) {
		int board_eval = eval<white>(board);
		if (encourage_progress and history.irreversible_idx == 0) board_eval /= 2;
		const int futility_eval =board_eval - depth * rfp_margin;
		if (futility_eval >= beta) {
			futility_prunes++;
			return std::make_tuple(futility_eval, last_pv.nullify(), history.curr_idx);
		}
		if (depth > 2) {
			HistoryView fresh_history = history.make_irreversible();
			const auto nms_result = search_helper<not white>(board, depth - nmp_depth, -beta, -beta + 1, fresh_history, last_pv, 0, 0);
			const int nms_eval = -std::get<0>(nms_result);
			const VariationView nms_var = std::get<1>(nms_result);
			if (nms_eval >= beta) {
				if (depth <= 4) {
					null_move_prunes++;
					return std::make_tuple(nms_eval, last_pv.nullify(), history.curr_idx);
				}
				const auto zz_check_result = search_helper<white, false>(board, depth - nmp_depth - 1, beta - 1, beta, history, last_pv, sibling_killer1, sibling_killer2);
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
			return std::make_tuple(0, last_pv.nullify(), history.curr_idx);
		}
		return std::make_tuple(CHECKMATED - depth, last_pv.nullify(), history.curr_idx);
	}

	int best_eval = INT_MIN;
	VariationView best_var = last_pv;
	int depth_reduction = 0;
	int move_index = 0;
	const std::array reduction_index_arr = {lmr0, lmr1, lmr2, lmr3, 128};
	const int next_depth = is_check ? depth : (depth - 1);
	int min_repetition_idx = history.curr_idx;
	while (best_eval < beta and not queue.empty() and depth_reduction <= next_depth){
		const int curr_alpha = std::max(alpha, best_eval);
		const Move branch_move = queue.top();
		Board branch_board = board.copy();
		make_move<white>(branch_board, branch_move);
		HistoryView branch_history = is_irreversible(board, branch_move) ? history.make_irreversible() : history.extend(board.ue.hash);
		const VariationView branch_hint = (last_pv.length and (last_pv.head() == branch_move)) ? last_pv.copy_branch() : last_pv.fresh_branch();

		auto search_res = search_helper<not white>(branch_board, next_depth - depth_reduction,
				-(depth_reduction ? (curr_alpha + 1) : beta), -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
		int branch_eval = -std::get<0>(search_res);
		min_repetition_idx = std::min(min_repetition_idx, std::get<2>(search_res));

		if (depth_reduction and (branch_eval > curr_alpha)){
			search_res = search_helper<not white>(branch_board, next_depth, -beta, -curr_alpha, branch_history, branch_hint, child_killer1, child_killer2);
			branch_eval = -std::get<0>(search_res);
			min_repetition_idx = std::min(min_repetition_idx, std::get<2>(search_res));
		}

		const VariationView branch_var = std::get<1>(search_res);
		if (branch_eval > best_eval) {
			if (branch_eval > alpha) best_var = branch_var.prepend(branch_move);
			best_eval = branch_eval;
			if (branch_eval > alpha) queue.template update_frequency_for_beta_cutoff<white>();
		} else if (branch_var.length) {
			const Move refutation = branch_var.head();
			if ((refutation != child_killer1) and (move_destination(refutation) != move_destination(branch_move))) {
				child_killer2 = child_killer1;
				child_killer1 = refutation;
			}
		}

		queue.pop();
		if (branch_eval >= TB_EVAL[0]) move_index += 1;
		if (move_index == reduction_index_arr[depth_reduction]) {
			depth_reduction += 1;
			move_index = 0;
		}
	}

	if ((best_eval > alpha) and ((min_repetition_idx >= history.curr_idx) or (best_eval > 0))){
		ht_put(hash_key, std::make_tuple(best_eval, best_var.head(), depth));
	} else { fail_low++; }

	if (best_eval >= beta) fail_high++;

	return std::make_tuple(best_eval, best_var, min_repetition_idx);
}

void log_info(int ms_elapsed, int depth, VariationView var, int eval){
	std::cout << "info depth " << depth << " time " << ms_elapsed << " nodes " << positions_seen << " tbhits " << tbhits << 
					" pv" << show_variation(var) << " score cp " << eval << std::endl;
}

template <bool white>
std::tuple<Move, int> search_for_move_w_eval(const Board &board, History &history, 
	const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms){
	Timer timer;
	timer.start();
	_global_node_limit = node_limit;

	positions_seen = 0;
	leaf_nodes = 0;
	qnodes = 0;
	null_move_prunes = 0;
	zz_checks_failed = 0;
	futility_prunes = 0;
	fail_low = 0;
	fail_high = 0;
	repetitions = 0;
	tbhits = 0;
	VariationWorkspace workspace;
	VariationView var = VariationView(workspace);
	HistoryView hv = take_view(history);
	const bool is_mop_up_mode = (not board.White.Pawn) and (not board.Black.Pawn);
	set_mop_up_mode(is_mop_up_mode);
	force_progress = history.root_idx >= force_progress_after;
	encourage_progress = force_progress or ((history.root_idx >= encourage_progress_after) and not is_mop_up_mode);

	int depth = 1;
	int eval = 0;
	int ms_elapsed = 0;
	try {
		bool should_increment_depth = false;
		int aspiration_window_radius = 200;
		while ((ms_elapsed < min_time_ms or depth < 4) and 
			(not should_increment_depth or ((eval > CHECKMATED) and (eval < TB_EVAL[4]) and (depth < depth_limit)))){

			// We've proven a TB loss, so we deactivate the tables and hope we can swindle an opponent without TBs
			if (eval <= TB_EVAL[0]) tb_deactivate = true;

			else if (should_increment_depth) depth++;
			int new_eval = eval;
			std::tie(new_eval, var, std::ignore) = search_helper<white>(board, depth, 
				eval - aspiration_window_radius, eval + aspiration_window_radius, hv, var, 0, 0);

			should_increment_depth = (std::abs(new_eval - eval) < aspiration_window_radius);
			aspiration_window_radius = should_increment_depth ? aw_start : (aspiration_window_radius * aw_increase) / 8;
			eval = new_eval;

			ms_elapsed = timer.ms_elapsed();
			if ((ms_elapsed > 0) and (max_time_ms < INT_MAX)) {
				auto npms = positions_seen / ms_elapsed;
				_global_node_limit = npms * max_time_ms;
			}
            if (log_level >= 2) { log_info(ms_elapsed, depth, var, eval); }
		}

		if (log_level == 1) { log_info(ms_elapsed, depth, var, eval); }
	} catch (NodeLimitSafety e) { 
		if (log_level >= 1) log_info(timer.ms_elapsed(), depth, var.singleton(var.head()), eval);
	}
	return std::make_tuple(var.head(), eval);
}

template <bool white>
Move search_for_move(const Board &board, History &history, 
    const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms){
	if (__builtin_popcountll(board.Occ) <= search_tb.max_num_pieces_) {
		int dtz; Move move;
		std::tie(dtz, move) = search_tb.probe_dtz(white, board, true);
		if (dtz > 0 or (dtz < 0 and -10 * dtz + history.root_idx >= 100)) {
			// Either a win or a loss with a reaonably high DTZ -> play DTZ optimal move
			return move;
		}
		if (dtz < 0) {
			// If we have a loss with a low DTZ, we turn TBs off and run a normal search (TODO)
			tb_deactivate = true;
		} else {
			// If we have a draw, we filter to moves at the root that keep the draw (TODO)
			tb_deactivate = false;
			tb_filter_draw = true;
		}
	} else {
		tb_deactivate = false;
		tb_filter_draw = false;
	}

	return std::get<0>(search_for_move_w_eval<white>(
		board, history, node_limit, depth_limit, min_time_ms, max_time_ms));
}

template Move search_for_move<true>(const Board&, History&, const int, const int, const int, const int);
template Move search_for_move<false>(const Board&, History&, const int, const int, const int, const int);
