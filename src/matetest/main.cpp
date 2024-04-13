# include <iostream>
# include <fstream>
# include <vector>
# include <optional>
# include "../parse_format.hpp"
# include "../selfplay/selfplay.hpp"
# include "../timer.hpp"


int main(int argc, char **argv){
    if (argc != 3) {
        std::cout << "Usage: ./matetest <endgame> <search_depth>" << std::endl;
        return 1;
    }

    Timer t;
    t.start();

    const std::string endgame = argv[1];

	std::ifstream fen_file;
    fen_file.open("mate_test_fens/" + endgame + ".txt");
    std::vector<std::string> fens;
    std::string fen;
	while (std::getline(fen_file, fen)){
        fens.push_back(fen);
    }
    assert(fens.size() == 100);

    auto failure_count = 0;
    std::optional<GameData> failure_example;
    std::optional<std::string> failure_fen;
    const int search_depth = atoi(argv[2]);
    const std::vector<Move> no_moves;
    const GameData bookless = GameData{no_moves, 'U'};
    for (auto fen : fens) {
        Board board;
        parse_fen(fen, board);
        GameData result = run_selfplay(board, bookless, search_depth);
        if (result.result != 'W') {
            failure_count++;
            failure_example = result;
            failure_fen = fen;
        }
    }

    std::cout << endgame << ": checkmated in " << 100 - failure_count << " of 100 positions in " 
        << t.ms_elapsed() / 1000 << " seconds" << std::endl;
    if (failure_fen.has_value()) {
        std::cout << "Failure example:" << std::endl;
        std::cout << failure_fen.value() << std::endl;
        for (auto move : failure_example.value().moves) std::cout << format_move_xboard(move) << " ";
        std::cout << std::endl;
    }
}
