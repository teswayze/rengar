# include "doctest.h"
# include <iostream>
# include "../parse_format.hpp"
# include "../gamefile/rg_file.hpp"

std::vector<std::string> fools_mate_uci = {"g2g4", "e7e5", "f2f3", "d8h4"};

std::vector<Move> make_move_vector(std::vector<std::string> str_vector){
    Board board;
    parse_fen(STARTING_FEN, board);
    bool wtm = true;
    std::vector<Move> move_vector;
    for (auto move_str : str_vector) {
        const Move move = parse_move_xboard(move_str, board, wtm);
        move_vector.push_back(move);
        (wtm ? make_move<true> : make_move<false>)(board, move);
        wtm = not wtm;
    }
    return move_vector;
}

TEST_CASE(".rg file round trip"){
    std::string filepath = ".unittest_tmp/round_trip_test.rg";

    RgFileWriter writer;
    writer.add_game(GameData{make_move_vector(fools_mate_uci), 'B'});
    writer.write_to_file(filepath);

    auto reader = RgFileReader(filepath);
    CHECK(reader.games_left == 1);

    auto game0 = reader.next_game();
    CHECK(game0.result == 'B');
    CHECK(game0.moves.size() == fools_mate_uci.size());
    for (size_t i = 0; i < game0.moves.size(); i++){
        CHECK(format_move_xboard(game0.moves[i]) == fools_mate_uci[i]);
    }
    CHECK(reader.games_left == 0);

}