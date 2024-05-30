# include "doctest.h"
# include "../trainer/loss.hpp"

TEST_CASE("Loss gradient"){
    SUBCASE("Draw"){
        CHECK(loss_gradient<'D'>(-600) == 511);
        CHECK(loss_gradient<'D'>(-500) == 500);
        CHECK(loss_gradient<'D'>(-400) == 400);
        CHECK(loss_gradient<'D'>(-300) == 300);
        CHECK(loss_gradient<'D'>(-200) == 200);
        CHECK(loss_gradient<'D'>(-100) == 100);
        CHECK(loss_gradient<'D'>(0) == 0);
        CHECK(loss_gradient<'D'>(100) == -100);
        CHECK(loss_gradient<'D'>(200) == -200);
        CHECK(loss_gradient<'D'>(300) == -300);
        CHECK(loss_gradient<'D'>(400) == -400);
        CHECK(loss_gradient<'D'>(500) == -500);
        CHECK(loss_gradient<'D'>(600) == -511);
    }
    SUBCASE("White win"){
        CHECK(loss_gradient<'W'>(-600) == 1022);
        CHECK(loss_gradient<'W'>(-500) == 1000);
        CHECK(loss_gradient<'W'>(-400) == 812);
        CHECK(loss_gradient<'W'>(-300) == 643);
        CHECK(loss_gradient<'W'>(-200) == 495);
        CHECK(loss_gradient<'W'>(-100) == 365);
        CHECK(loss_gradient<'W'>(0) == 256);
        CHECK(loss_gradient<'W'>(100) == 165);
        CHECK(loss_gradient<'W'>(200) == 95);
        CHECK(loss_gradient<'W'>(300) == 43);
        CHECK(loss_gradient<'W'>(400) == 12);
        CHECK(loss_gradient<'W'>(500) == 0);
        CHECK(loss_gradient<'W'>(600) == 0);
    }
    SUBCASE("Black win"){
        CHECK(loss_gradient<'B'>(-600) == 0);
        CHECK(loss_gradient<'B'>(-500) == 0);
        CHECK(loss_gradient<'B'>(-400) == -12);
        CHECK(loss_gradient<'B'>(-300) == -43);
        CHECK(loss_gradient<'B'>(-200) == -95);
        CHECK(loss_gradient<'B'>(-100) == -165);
        CHECK(loss_gradient<'B'>(0) == -256);
        CHECK(loss_gradient<'B'>(100) == -365);
        CHECK(loss_gradient<'B'>(200) == -495);
        CHECK(loss_gradient<'B'>(300) == -643);
        CHECK(loss_gradient<'B'>(400) == -812);
        CHECK(loss_gradient<'B'>(500) == -1000);
        CHECK(loss_gradient<'B'>(600) == -1022);
    }
}
