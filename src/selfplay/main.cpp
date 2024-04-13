# include <iostream>
# include "../gamefile/rg_file.hpp"
# include "../parse_format.hpp"
# include "selfplay.hpp"
# include "../timer.hpp"


int main(int argc, char **argv){
    if (argc != 5) {
        std::cout << "Usage: ./selfplay <book_file> <target_file> <search_depth> <chess324_id>" << std::endl;
        return 1;
    }
    int search_depth = std::atoi(argv[3]);
    int chess324_id = std::atoi(argv[4]);

    Timer t;
    t.start();
    auto book_reader = RgFileReader(argv[1]);
    RgFileWriter game_writer;
    Board starting_board;
    parse_fen(get_chess324_starting_fen(chess324_id), starting_board);

    std::cout << "Playing " << book_reader.games_left << " games from opening book at " << argv[1] << std::endl;
    while (book_reader.games_left) game_writer.add_game(run_selfplay(starting_board, book_reader.next_game(), search_depth));
    std::cout << "Finished playing games in " << t.ms_elapsed() / 1000 << " seconds" << std::endl;

    game_writer.write_to_file(argv[2]);
}
