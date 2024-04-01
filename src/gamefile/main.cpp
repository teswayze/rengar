# include <iostream>
# include <cstdlib>

# include "rg_file.hpp"
# include "../parse_format.hpp"

void send_error_message(){
    std::cout << "Usage: ./game_cat <file>" << std::endl;
}


int main(int argc, char **argv){
    if (argc != 2) {send_error_message(); return 1;}
    
    auto reader = RgFileReader(argv[1]);
    while (reader.games_left) {
        auto game = reader.next_game();
        switch (game.result){
            case 'B': std::cout << "Black win\n"; break;
            case 'W': std::cout << "White win\n"; break;
            case 'D': std::cout << "Draw\n"; break;
        }
        for (auto move : game.moves) std::cout << format_move_xboard(move) << " ";
        std::cout << std::endl;
    }
}
