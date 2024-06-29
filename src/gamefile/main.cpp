# include <iostream>
# include <cstdlib>

# include "rg_file.hpp"
# include "csv.hpp"
# include "torch_prep.hpp"
# include "../parse_format.hpp"

void send_error_message(){
    std::cout << "Usage: ./game_cat <file> {--csv} {--torch-prep} {--324-id <0-323>}" << std::endl;
}


void show_game_uci(const GameData game, const int game_no){
    std::cout << "Game " << game_no << ": ";
    switch (game.result){
        case 'B': std::cout << "Black win\n"; break;
        case 'W': std::cout << "White win\n"; break;
        case 'D': std::cout << "Draw\n"; break;
    }
    for (auto move : game.moves) std::cout << format_move_xboard(move) << " ";
    std::cout << std::endl;
}


void show_all_games_uci(const std::string file_path){
    auto reader = RgFileReader(file_path);
    const int total_games = reader.games_left;
    while (reader.games_left) show_game_uci(reader.next_game(), total_games - reader.games_left);
}


int main(int argc, char **argv){
    if (argc < 2) {send_error_message(); return 1;}

    bool csv_format = false;
    bool torch_prep = false;
    auto arg_idx = 2;
    int chess324_id = 171;
    while (arg_idx < argc){
        if (std::strncmp(argv[arg_idx], "--csv", 5) == 0){
            csv_format = true;
            arg_idx += 1;
        } else if (std::strncmp(argv[arg_idx], "--torch-prep", 12) == 0) {
            torch_prep = true;
            arg_idx += 1;
        } else if (std::strncmp(argv[arg_idx], "--324-id", 8) == 0){
            chess324_id = std::atoi(argv[arg_idx + 1]);
            arg_idx += 2;
        } else { send_error_message(); return 1;}
    }

    if (csv_format) show_all_games_csv(argv[1], chess324_id);
    else if (torch_prep) prep_games_for_pytorch(argv[1], chess324_id);
    else show_all_games_uci(argv[1]);
}
