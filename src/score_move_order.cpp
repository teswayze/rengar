# include <stdexcept>

# include "score_move_order.hpp"
# include "movegen.hpp"
# include "parse_format.hpp"
# include "math.hpp"


template <bool wtm>
int score_move_choice_helper(Board &board, Move chosen_move){
	auto legal_moves = generate_moves<wtm>(board, checks_and_pins<wtm>(board), 0, 0, 0);
	
	int accum_prio = legal_moves.top_prio();
	bool match_found = (legal_moves.top() == chosen_move);
	int chosen_prio = legal_moves.top_prio();
	legal_moves.pop();
	
	while (!legal_moves.empty()) {
		accum_prio = log_add_smaller_exp(accum_prio, legal_moves.top_prio());
		if (legal_moves.top() == chosen_move) {
			chosen_prio = legal_moves.top_prio();
			match_found = true;
		}
		legal_moves.pop();
	}
	
	if (not match_found) std::invalid_argument("No matching legal move");
	
	return accum_prio - chosen_prio;
}


int score_move_order(std::vector<std::string> moves){
	Board board;
	bool wtm = parse_fen(STARTING_FEN, board);
	int score = 0;
	
	for (std::string move_str : moves){
		const Move chosen_move = parse_move_xboard(move_str, board, wtm);
		score += (wtm ? score_move_choice_helper<true> : score_move_choice_helper<false>)(board, chosen_move);
		(wtm ? make_move<true> : make_move<false>)(board, chosen_move);
		wtm = !wtm;
	}
	
	return score;
}
