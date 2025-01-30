# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

# include "../external/doctest.h"
# include "test_func.hpp"
# include "../timer.hpp"
# include "../parse_format.hpp"

TEST_CASE("Probe syzygy5 WDL"){
    Timer timer;
    timer.start();
    auto tb5 = Tablebase(5, "syzygy5");
    timer.stop();
    CHECK(tb5.ready());
    std::cout << "Initializing TBs took " << timer.ms_elapsed() << " ms" << std::endl;

    for (auto it = tb5.wdl_tables.begin(); it != tb5.wdl_tables.end(); it++){
        const std::string tb_name = it->first.name();
        INFO(tb_name);
        test_syzygy_probing_vs_known_fens(tb5, "syzygy_test/" + tb_name + ".csv");
    }
    std::cout << "Probing WDL took " << wdl_time() << " ms" << std::endl;
    std::cout << "Probing DTZ took " << dtz_time() << " ms" << std::endl;
}


TEST_CASE("Legal en-passant changes probe result"){
    auto tb4 = Tablebase(4, "syzygy5");
    CHECK(tb4.ready());
    Board board;

    // Draw without en-passant available
    bool wtm = parse_fen("8/8/1K5k/8/2Pp4/8/8/8 b - - 0 1", board);
    int wdl = tb4.probe_wdl(wtm, board);
    CHECK(wdl == 0);

    // Win with en-passant available
    wtm = parse_fen("8/8/1K5k/8/2Pp4/8/8/8 b - c3 0 1", board);
    wdl = tb4.probe_wdl(wtm, board);
    CHECK(wdl == 2);
}
