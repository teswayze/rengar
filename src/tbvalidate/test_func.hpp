# include <string>
# include "../syzygy_probe.hpp"

void test_syzygy_probing_vs_known_fens(Tablebase &tb, const std::string fen_csv_file);
int wdl_time();
int dtz_time();

void test_plays_expected_continuation(const bool w_hero, Board &board, const int depth, const std::vector<std::string> &uci_moves);