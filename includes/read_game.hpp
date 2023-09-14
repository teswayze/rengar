# include <vector>
# include <string>
# include "board.hpp"

using move_vec = std::vector<Move>;
using string_vec = std::vector<std::string>;
using int_vec = std::vector<int>;

move_vec read_game(std::string game_path);
string_vec find_game_paths(int num_tournaments, char **tournament_names);
std::vector<move_vec> load_games(string_vec game_paths);
int_vec read_results(string_vec game_paths);
