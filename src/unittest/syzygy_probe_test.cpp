# include "../external/doctest.h"
# include "../syzygy_probe.hpp"

const auto EXPECTED_TB3 = {"KPvK", "KNvK", "KBvK", "KRvK", "KQvK"};

TEST_CASE("Enumerate all TB 3"){
    auto all_tb_3 = all_tbs(3);
    
    for (const auto expected : EXPECTED_TB3) {
        const std::string generated = all_tb_3.front().name();
        CHECK(generated == expected); 
        all_tb_3.pop_front();
    }

    CHECK(all_tb_3.empty());
}

const auto EXPECTED_TB4 = {
    "KPPvK", "KNPvK", "KNNvK", "KBPvK", "KBNvK", 
    "KBBvK", "KRPvK", "KRNvK", "KRBvK", "KRRvK", 
    "KQPvK", "KQNvK", "KQBvK", "KQRvK", "KQQvK", 
    "KPvKP", "KNvKP", "KNvKN", "KBvKP", "KBvKN",
    "KBvKB", "KRvKP", "KRvKN", "KRvKB", "KRvKR",
    "KQvKP", "KQvKN", "KQvKB", "KQvKR", "KQvKQ"
};

TEST_CASE("Enumerate all TB 4"){
    auto all_tb_3_4 = all_tbs(4);
    
    for (const auto expected : EXPECTED_TB3) {
        const std::string generated = all_tb_3_4.front().name();
        CHECK(generated == expected); 
        all_tb_3_4.pop_front();
    }

    for (const auto expected : EXPECTED_TB4) {
        const std::string generated = all_tb_3_4.front().name();
        CHECK(generated == expected); 
        all_tb_3_4.pop_front();
    }

    CHECK(all_tb_3_4.empty());
}

TEST_CASE("Correct number of TB 5"){
    CHECK(all_tbs(5).size() == 145);
}

TEST_CASE("Correct number of TB 6"){
    CHECK(all_tbs(6).size() == 510);
}
