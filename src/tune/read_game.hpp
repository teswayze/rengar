# include <vector>
# include <string>
# include "board.hpp"

using move_vec = std::vector<Move>;
using string_vec = std::vector<std::string>;
using int_vec = std::vector<int>;

move_vec read_game(std::string game_path);
std::vector<move_vec> load_games(string_vec game_paths);
int_vec read_results(string_vec game_paths);
