# include "register_params.hpp"
# include "read_game.hpp"
# include "math.hpp"
# include <iostream>
# include <stdexcept>
# include <vector>
# include <filesystem>

# ifdef TUNE_EVAL
# include "score_eval.hpp"
# define START_VAL 21
# define NUM_ROUNDS 7
# define PARAM_KIND "EVAL"
# else
# include "score_move_order.hpp"
# define START_VAL 55
# define NUM_ROUNDS 9
# define PARAM_KIND "MOVE_ORDER"
# endif

const std::string games_dir = "games/";

int64_t sum_of_diffs(int_vec x, int_vec y){
	if (x.size() != y.size()) throw std::invalid_argument("Vectors have different sizes");
	int64_t diff = 0;
	for (size_t i = 0; i < x.size(); i++) diff += x[i] - y[i];
	return diff;
}

uint64_t sum_of_abs_diffs(int_vec x, int_vec y){
	if (x.size() != y.size()) throw std::invalid_argument("Vectors have different sizes");
	uint64_t diff = 0;
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

string_vec find_game_paths(int num_tournaments, char **tournament_names){
	std::cout << "Using " << num_tournaments << " tournaments" << std::endl;
	string_vec game_paths;
	for (int i = 0; i < num_tournaments; i++){
		for (const auto & entry : std::filesystem::directory_iterator(games_dir + tournament_names[i])){
			std::string path = entry.path();
			game_paths.push_back(path);
		}
	}
	return game_paths;
}

int main(int argc, char **argv){
	
	auto game_paths = find_game_paths(argc - 1, argv + 1);
	auto games = load_games(game_paths);
	# ifdef TUNE_EVAL
	auto results = read_results(game_paths);
	if (games.size() != results.size()) throw std::invalid_argument("Vectors have different sizes");
	# endif
	
	std::cout << "Computing initial scores..." << std::endl;
	int_vec game_scores = std::vector(games.size(), 0);
	for (size_t i = 0; i < games.size(); i++){
		# ifdef TUNE_EVAL
		game_scores[i] = score_eval(games[i], results[i]);
		# else
		game_scores[i] = score_move_order(games[i]);
		# endif
	}

	auto tweak_queue = initialize_queue(START_VAL);
	int num_expected_rounds = NUM_ROUNDS * tweak_queue.size();
	std::cout << "Expecting to run " << num_expected_rounds << " rounds of tuning" << std::endl;

	while (not tweak_queue.empty()){
		if (num_expected_rounds % 100 == 0) std::cout << num_expected_rounds << " rounds left (priority = " << 
			tweak_queue.top().priority << ")" << std::endl;
		auto tweak = tweak_queue.top().tweak;
		tweak_queue.pop();

		auto tweak_str = tweak_to_string(tweak);
		apply_tweak(tweak);

		int_vec new_scores = std::vector(games.size(), 0);
		for (size_t i = 0; i < games.size(); i++){
			# ifdef TUNE_EVAL
			new_scores[i] = score_eval(games[i], results[i]);
			# else
			new_scores[i] = score_move_order(games[i]);
			# endif
		}

		uint64_t next_prio = sum_of_abs_diffs(new_scores, game_scores);
		int next_mod = next_fibbonacci_down(tweak.proposed_mod);
		
		if (sum_of_diffs(new_scores, game_scores) < 0) {
			if (tweak_queue.top().priority < 0xFFFF'FFFF'FFFF'FFFF) {
				std::cout << "Change accepted! " << tweak_str << std::endl;
				game_scores = new_scores;
			} else {
				unapply_tweak(tweak);
				next_mod = tweak.proposed_mod;
				num_expected_rounds++;
			}
		} else {
			unapply_tweak(tweak);
			next_mod *= -1;
		}
		if (next_mod) tweak_queue.push(TweakWithPriority{next_prio, ProposedTweak{tweak.param_id, tweak.index, next_mod}});
		
		num_expected_rounds--;
	}
	
	show_current_param_values(PARAM_KIND);
	
	return 0;
}
