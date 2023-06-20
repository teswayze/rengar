# include "register_params.hpp"
# include "score_move_order.hpp"
# include "read_game.hpp"
# include <iostream>
# include <filesystem>
# include <exception>

using string_vec = std::vector<std::string>;
using int_vec = std::vector<int>;
const std::string games_dir = "games/";
const std::string uci_file_name = "/game.uci";


string_vec find_game_paths(int num_tournaments, char **tournament_names){
	std::cout << "Using " << num_tournaments << " tournaments" << std::endl;
	string_vec game_paths;
	for (int i = 0; i < num_tournaments; i++){
		for (const auto & entry : std::filesystem::directory_iterator(games_dir + tournament_names[i])){
			std::string path = entry.path();
			game_paths.push_back(path + uci_file_name);
		}
	}
	return game_paths;
}

std::vector<string_vec> load_games(string_vec game_paths){
	std::cout << "Found " << game_paths.size() << " games to load..." << std::endl;
	std::vector<string_vec> games;
	for (const auto path : game_paths){
		games.push_back(read_game(path));
		if (games.size() % 1000 == 0) std::cout << "Loaded " << games.size() << " games..." << std::endl;
	}
	return games;
}

int_vec score_games(std::vector<string_vec> games){
	auto game_scores = std::vector(games.size(), 0);
	for (size_t i = 0; i < games.size(); i++){
		game_scores[i] = score_move_order(games[i]);
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


int main(int argc, char **argv){
	
	auto game_paths = find_game_paths(argc - 1, argv + 1);
	auto games = load_games(game_paths);
	
	std::cout << "Computing initial scores..." << std::endl;
	auto game_scores = score_games(games);
	auto tweak_queue = initialize_queue(-64);

	while (not tweak_queue.empty()){
		auto tweak = tweak_queue.top().tweak;
		std::cout << "Trying " << tweak_to_string(tweak) << " (priority = " << tweak_queue.top().priority << ")" << std::endl;
		tweak_queue.pop();

		apply_tweak(tweak);
		auto new_scores = score_games(games);
		int next_prio = sum_of_abs_diffs(new_scores, game_scores);
		
		if (sum_of_diffs(new_scores, game_scores) < 0){
			std::cout << "Change accepted!" << std::endl;
			game_scores = new_scores;
			tweak_queue.push(TweakWithPriority{next_prio, tweak});
		} else {
			unapply_tweak(tweak);
			int next_mod = -tweak.proposed_mod / 2;
			if (next_mod) tweak_queue.push(TweakWithPriority{next_prio, ProposedTweak{tweak.param_id, tweak.index, next_mod}}); 
		}
	}
	
	show_current_param_values();
	
	return 0;
}
