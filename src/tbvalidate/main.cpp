# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

# include "../external/doctest.h"
# include "test_func.hpp"
# include "../timer.hpp"
# include "../parse_format.hpp"
# include "../search.hpp"
# include "../hashtable.hpp"

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


TEST_CASE("Converts DTZ 91 win (KRNvKQ)"){
    Board board;
    bool wtm = parse_fen("1N1q4/1k6/8/8/8/8/1R6/1K6 b - - 0 1", board);
    set_tb_path("syzygy5");

    test_plays_expected_continuation(wtm, board, 1, {
        "b7c8", "b1c2", "d8c7", "c2d1", "c7d6", "d1c2", "d6c5", "c2d1", "c5d4", "d1c1",
        "d4f4", "c1c2", "f4c4", "c2d1", "c4a4", "d1d2", "c8d8", "d2c3", "a4a3", "b2b3",
        "a3c5", "c3d2", "c5g5", "d2e2", "g5g2"
        // DTZ 66 from here, and black has multiple options with similar DTZ
    });
}


TEST_CASE("Stalls DTZ 132 blessed loss (KBBvKN)"){
    Board board;
    bool wtm = parse_fen("8/8/8/1B6/8/8/8/1KBk2n1 b - - 0 1", board);
    set_tb_path("syzygy5");

    test_plays_expected_continuation(wtm, board, 1, {
        "g1e2", "b5a4", "d1e1", "c1b2", "e1d2", "a4b3", "e2c3", "b1a1", "d2d3"
        // DTZ 122 from here, and black has multiple options with similar DTZ
    });
}


TEST_CASE("Finds only drawing move: white (KPPvKR)"){
    Board board;
    bool wtm = parse_fen("8/8/8/7K/2kp4/1R2p3/8/8 w - - 0 1", board);
    set_tb_path("syzygy5");
    ht_init(12);

    test_plays_expected_continuation(wtm, board, 2, {
        "b3b1", "d4d3", "h5g4", "d3d2", "g4f3", "c4d3", "b1a1", "e3e2", "a1a3", "d3c2", "a3a2"
        // DTZ 122 from here, and black has multiple options with similar DTZ
    });
}


TEST_CASE("Finds only drawing move: black (KPPvKR)"){
    Board board;
    bool wtm = parse_fen("8/8/8/7K/2kp4/4p3/8/1R6 b - - 0 1", board);
    set_tb_path("syzygy5");
    ht_init(12);

    test_plays_expected_continuation(wtm, board, 2, {
        "d4d3", "h5g4", "d3d2", "g4f3", "c4d3", "b1a1", "e3e2", "a1a3", "d3c2"
        // DTZ 122 from here, and black has multiple options with similar DTZ
    });
}


TEST_CASE("Don't play ridiculous DTZ move in lost position"){
    Board board;
    bool wtm = parse_fen("8/8/8/8/3k4/7r/2RQ4/3K4 b - - 0 1", board);
    set_tb_path("syzygy5");
    ht_init(12);

    test_plays_expected_continuation(wtm, board, 2, {
        "h3d3", "d2d3", "d4d3"
        // Rd3 has DTZ=1 because the queen can capture. Black gets a DTZ of 3 by playing Ke5/Ke4. But that's dumb.
    });
}
