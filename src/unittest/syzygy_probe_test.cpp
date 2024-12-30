# include "../external/doctest.h"
# include "../syzygy_probe.hpp"
# include "../parse_format.hpp"
# include "../tbvalidate/test_func.hpp"

const auto EXPECTED_TB3 = {"KPvK", "KNvK", "KBvK", "KRvK", "KQvK"};
const auto EXPECTED_TB4 = {
    "KPPvK", "KNPvK", "KNNvK", "KBPvK", "KBNvK", 
    "KBBvK", "KRPvK", "KRNvK", "KRBvK", "KRRvK", 
    "KQPvK", "KQNvK", "KQBvK", "KQRvK", "KQQvK", 
    "KPvKP", "KNvKP", "KNvKN", "KBvKP", "KBvKN",
    "KBvKB", "KRvKP", "KRvKN", "KRvKB", "KRvKR",
    "KQvKP", "KQvKN", "KQvKB", "KQvKR", "KQvKQ"
};

TEST_CASE("Rigorous checks for TB <=4"){
    auto all_tb_3_4 = all_tbs(4);
    
    for (const auto expected : EXPECTED_TB3) {
        const auto generated = all_tb_3_4.front();
        CHECK(generated.num() == 3);
        CHECK(generated.name() == expected); 
        CHECK(not generated.symmetric());
        CHECK(generated.has_pawns() == (expected == "KPvK"));

        all_tb_3_4.pop_front();
    }

    for (const auto expected : EXPECTED_TB4) {
        const auto generated = all_tb_3_4.front();
        CHECK(generated.num() == 4);
        CHECK(generated.name() == expected);

        const bool expected_symmetric = expected[2] == 'v' and expected[1] == expected[4];
        CHECK(generated.symmetric() == expected_symmetric);

        CHECK(generated.has_pawns() == (expected[2] == 'P' or expected[4] == 'P'));
        if (generated.has_pawns()){
            if (expected == "KPPvK") CHECK(generated.pawn_counts() == std::make_tuple(2, 0));
            else if (expected == "KPvKP") CHECK(generated.pawn_counts() == std::make_tuple(1, 1));
            else CHECK(generated.pawn_counts() == std::make_tuple(1, 0));
        } else CHECK(generated.enc_type_2() == (expected[1] == expected[2]));

        all_tb_3_4.pop_front();
    }

    CHECK(all_tb_3_4.empty());
}

TEST_CASE("Correct counts for TB <= 7"){
    const auto tb_up_to_7 = all_tbs(7);
    CHECK(tb_up_to_7.size() == 1511);

    int tb5_count = 0;
    int tb6_count = 0;
    int tb7_count = 0;

    int symmetric_count = 0;
    int has_pawns_count = 0;
    int both_have_pawns_count = 0;
    int enc_type_2_count = 0;

    std::array<int, 6> pc_0_count = {0, 0, 0, 0, 0, 0};
    std::array<int, 6> pc_1_count = {0, 0, 0, 0, 0, 0};

    for (const auto generated : tb_up_to_7) {
        if (generated.num() == 5) tb5_count++;
        if (generated.num() == 6) tb6_count++;
        if (generated.num() == 7) tb7_count++;
        if (generated.symmetric()) symmetric_count++;

        if (generated.has_pawns()) {
            has_pawns_count++;
            if (generated.both_have_pawns()) both_have_pawns_count++;

            int pc_0; int pc_1;
            std::tie(pc_0, pc_1) = generated.pawn_counts();
            pc_0_count[pc_0]++;
            pc_1_count[pc_1]++;
        } else if (generated.enc_type_2()) enc_type_2_count++;
    }

    CHECK(tb5_count == 110);
    CHECK(tb6_count == 365);
    CHECK(tb7_count == 1001);

    CHECK(symmetric_count == 20);
    CHECK(has_pawns_count == 861);
    CHECK(both_have_pawns_count == 146);
    CHECK(enc_type_2_count == 60);

    CHECK(pc_0_count[0] == 0);
    CHECK(pc_0_count[1] == 635);
    CHECK(pc_0_count[2] == 171);
    CHECK(pc_0_count[3] == 45);
    CHECK(pc_0_count[4] == 9);
    CHECK(pc_0_count[5] == 1);

    CHECK(pc_1_count[0] == 715);
    CHECK(pc_1_count[1] == 85);
    CHECK(pc_1_count[2] == 50);
    CHECK(pc_1_count[3] == 10);
    CHECK(pc_1_count[4] == 1);
    CHECK(pc_1_count[5] == 0);
}

TEST_CASE("Probe syzygy KPvK WDL"){
    auto tb3 = Tablebase(3, "syzygy3");
    CHECK(tb3.ready());
    test_syzygy_probing_vs_known_fens(tb3, "syzygy_test/KPvK.csv");
}

TEST_CASE("Probe syzygy KRvK WDL"){
    auto tb3 = Tablebase(3, "syzygy3");
    CHECK(tb3.ready());
    test_syzygy_probing_vs_known_fens(tb3, "syzygy_test/KRvK.csv");
}

TEST_CASE("Probe syzygy KQvK WDL"){
    auto tb3 = Tablebase(3, "syzygy3");
    CHECK(tb3.ready());
    test_syzygy_probing_vs_known_fens(tb3, "syzygy_test/KQvK.csv");
}
