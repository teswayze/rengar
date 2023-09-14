# include <vector>
# include "eval.hpp"
# include "math.hpp"
# include "parse_format.hpp"

template <bool wtm>
int score_move_choice_helper(Board &board, int game_result){
    int evaluation = eval<wtm>(board) * (wtm ? 1 : -1);
    int quantmoid_game_prediction = quantmoid(evaluation);
    int error = quantmoid_game_prediction - game_result * 127;
    return error * error;
}

int score_eval(std::vector<Move> moves, int result){
    Board board;
	bool wtm = parse_fen(STARTING_FEN, board);
	int score = 0;
	
	for (Move chosen_move : moves){
		score += (wtm ? score_move_choice_helper<true> : score_move_choice_helper<false>)(board, result);
		(wtm ? make_move<true> : make_move<false>)(board, chosen_move);
		wtm = !wtm;
	}
	
	return score;
}
