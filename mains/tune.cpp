# include "register_params.hpp"
# include "read_game.hpp"
# include "math.hpp"
# include <iostream>
# include <filesystem>
# include <stdexcept>
# include <vector>

# ifdef TUNE_EVAL
# include "score_eval.hpp"
# define SCORE score_eval
# define START_VAL -21
# define NUM_ROUNDS 7
# else
# include "score_move_order.hpp"
# define SCORE score_move_order
# define START_VAL 55
# define NUM_ROUNDS 9
# endif

int_vec score_games(std::vector<move_vec> games){
	auto game_scores = std::vector(games.size(), 0);
	for (size_t i = 0; i < games.size(); i++){
		game_scores[i] = SCORE(games[i]);
	}
	return game_scores;
}

int sum_of_diffs(int_vec x, int_vec y){
	if (x.size() != y.size()) throw std::invalid_argument("Vectors have different sizes");
	int diff = 0;
	for (size_t i = 0; i < x.size(); i++) diff += x[i] - y[i];
	return diff;
}

int sum_of_abs_diffs(int_vec x, int_vec y){
	if (x.size() != y.size()) throw std::invalid_argument("Vectors have different sizes");
	int diff = 0;
	for (size_t i = 0; i < x.size(); i++) diff += (x[i] > y[i] ? x[i] - y[i] : y[i] - x[i]);
	return diff;
}

int next_fibbonacci_down(const int x){
	int a = 0;
	int b = 1;
	while (b < x or b < -x){
		const int temp = b;
		b = a + b;
		a = temp;
	}
	return x > 0 ? a : -a;
}


int main(int argc, char **argv){
	
	auto game_paths = find_game_paths(argc - 1, argv + 1);
	auto games = load_games(game_paths);
	
	std::cout << "Computing initial scores..." << std::endl;
	auto game_scores = score_games(games);
	auto tweak_queue = initialize_queue(START_VAL);
	int num_expected_rounds = NUM_ROUNDS * tweak_queue.size();
	std::cout << "Expecting to run " << num_expected_rounds << " rounds of tuning" << std::endl;

	while (not tweak_queue.empty()){
		if (num_expected_rounds % 100 == 0) std::cout << num_expected_rounds << " rounds left (priority = " << tweak_queue.top().priority << ")" << std::endl;
		auto tweak = tweak_queue.top().tweak;
		tweak_queue.pop();

		auto tweak_str = tweak_to_string(tweak);
		apply_tweak(tweak);
		auto new_scores = score_games(games);
		int next_prio = sum_of_abs_diffs(new_scores, game_scores);
		int next_mod = next_fibbonacci_down(tweak.proposed_mod);
		
		if (sum_of_diffs(new_scores, game_scores) < 0){
			std::cout << "Change accepted! " << tweak_str << std::endl;
			game_scores = new_scores;
		} else {
			unapply_tweak(tweak);
			next_mod *= -1;
		}
		if (next_mod) tweak_queue.push(TweakWithPriority{next_prio, ProposedTweak{tweak.param_id, tweak.index, next_mod}});
		
		num_expected_rounds--;
	}
	
	show_current_param_values();
	
	return 0;
}
