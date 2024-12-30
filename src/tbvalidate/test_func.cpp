# include "test_func.hpp"
# include "../external/doctest.h"
# include "../parse_format.hpp"

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
        const int probed_wdl = tb.probe_wdl(wtm, board);
        CHECK(expected_wdl == probed_wdl);

        std::getline(fen_csv, number_text);
        const int expected_dtz = std::atoi(number_text.begin().base());
        // TODO: check DTZ too
    }
}
