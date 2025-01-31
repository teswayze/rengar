# include "test_func.hpp"
# include "../external/doctest.h"
# include "../parse_format.hpp"
# include "../timer.hpp"
# include "../search.hpp"

Timer wdl_timer;
Timer dtz_timer;

void test_syzygy_probing_vs_known_fens(Tablebase &tb, const std::string fen_csv_file) {
    std::ifstream fen_csv;
    fen_csv.open(fen_csv_file);
    CHECK(fen_csv);

    std::string header;
    std::getline(fen_csv, header);  // Skip the first row with the column names
    
    for (int i = 0; i < 100; i++) {
        INFO(i);

        std::string fen;
        std::getline(fen_csv, fen, ',');
        INFO(fen);

        Board board;
        const bool wtm = parse_fen(fen, board);

        std::string(number_text);
        std::getline(fen_csv, number_text, ',');
        const int expected_wdl = std::atoi(number_text.begin().base());
        wdl_timer.start();
        const int probed_wdl = tb.probe_wdl(wtm, board);
        wdl_timer.stop();
        CHECK(expected_wdl == probed_wdl);

        std::getline(fen_csv, number_text);
        const int expected_dtz = std::atoi(number_text.begin().base());
        dtz_timer.start();
        const int probed_dtz = std::get<0>(tb.probe_dtz(wtm, board, false));
        dtz_timer.stop();
        CHECK(expected_dtz == probed_dtz);
    }
}

int wdl_time() { return wdl_timer.ms_elapsed(); }
int dtz_time() { return dtz_timer.ms_elapsed(); }


void test_plays_expected_continuation(const bool w_hero, Board &board, const std::vector<std::string> &uci_moves){
    bool wtm = w_hero;
    History history;

    for (const std::string uci_move : uci_moves) {
        Move expected_move = parse_move_xboard(uci_move, board, wtm);
        INFO(format_move_xboard(expected_move));
        if (w_hero == wtm) {
            Move chosen_move = (wtm ? search_for_move<true> : search_for_move<false>)(
                board, history, INT_MAX, 1, INT_MAX, INT_MAX
            );
            INFO(format_move_xboard(chosen_move));
            CHECK(chosen_move == expected_move);
        }
        (wtm ? make_move<true> : make_move<false>)(board, expected_move);
        wtm = not wtm;
    }
}
