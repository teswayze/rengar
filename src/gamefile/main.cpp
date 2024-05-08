# include <iostream>
# include <cstdlib>

# include "rg_file.hpp"
# include "../parse_format.hpp"
# include "../movegen.hpp"
# include "../eval.hpp"

void send_error_message(){
    std::cout << "Usage: ./game_cat <file> {--csv} {--324-id <0-323>}" << std::endl;
}


void show_game_uci(const GameData game, const int game_no, const int chess324_id){
    std::cout << "Game " << game_no << ": ";
    switch (game.result){
        case 'B': std::cout << "Black win\n"; break;
        case 'W': std::cout << "White win\n"; break;
        case 'D': std::cout << "Draw\n"; break;
    }
    for (auto move : game.moves) std::cout << format_move_xboard(move) << " ";
    std::cout << std::endl;
}


void show_game_csv(const GameData game, const int game_no, const int chess324_id){
    Board board;
    parse_fen(get_chess324_starting_fen(chess324_id), board);
    bool wtm = true;
    size_t half_move_no = 0;
    for (auto move : game.moves) {
        if (move_flags(move) < PAWN_CAPTURE and not (ToMask(move_destination(move)) & board.Occ)) {
            int sign = wtm ? 1 : -1;
            const auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
            if (cnp.CheckMask == FULL_BOARD) {
                std::cout << game_no << "," << half_move_no << "," << game.result << "," << 
                    sign *(wtm ? eval<true> : eval<false>)(board) << std::endl;
            }
        }

        (wtm ? make_move<true> : make_move<false>)(board, move);
        wtm = not wtm;
        half_move_no += 1;
    }
}


int main(int argc, char **argv){
    if (argc < 2) {send_error_message(); return 1;}

    bool csv_format = false;
    auto arg_idx = 2;
    int chess324_id = 171;
    while (arg_idx < argc){
        if (std::strncmp(argv[arg_idx], "--csv", 5) == 0){
            csv_format = true;
            arg_idx += 1;
        } else if (std::strncmp(argv[arg_idx], "--324-id", 8) == 0){
            chess324_id = std::atoi(argv[arg_idx + 1]);
            arg_idx += 2;
        } else { send_error_message(); return 1;}
    }

    if (csv_format) std::cout << "GameNo,HalfMoveNo,GameResult,Eval" << std::endl;
    const auto show_game_func = csv_format ? show_game_csv : show_game_uci;
    
    auto reader = RgFileReader(argv[1]);
    const int total_games = reader.games_left;
    while (reader.games_left) show_game_func(reader.next_game(), total_games - reader.games_left, chess324_id);
}
