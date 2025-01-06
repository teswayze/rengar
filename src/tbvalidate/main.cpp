# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

# include "../external/doctest.h"
# include "test_func.hpp"
# include "../timer.hpp"

TEST_CASE("Probe syzygy5 WDL"){
    Timer timer;
    timer.start();
    auto tb5 = Tablebase(5, "syzygy5");
    CHECK(tb5.ready());
    std::cout << "Initializing TBs took " << timer.ms_elapsed() << " ms" << std::endl;

    timer.reset();
    timer.start();
    for (auto it = tb5.wdl_tables.begin(); it != tb5.wdl_tables.end(); it++){
        const std::string tb_name = it->first.name();
        INFO(tb_name);
        test_syzygy_probing_vs_known_fens(tb5, "syzygy_test/" + tb_name + ".csv");
    }
    std::cout << "Probing test positions took " << timer.ms_elapsed() << " ms" << std::endl;
}
