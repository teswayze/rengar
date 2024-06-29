# include <iostream>
# include "csv.hpp"
# include "rg_file.hpp"
# include "../parse_format.hpp"
# include "../movegen.hpp"
# include "../eval.hpp"

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


void show_all_games_csv(const std::string file_path, const int chess324_id){
    std::cout << "GameNo,HalfMoveNo,GameResult,Eval" << std::endl;

    auto reader = RgFileReader(file_path);
    const int total_games = reader.games_left;
    while (reader.games_left) show_game_csv(reader.next_game(), total_games - reader.games_left, chess324_id);
}
