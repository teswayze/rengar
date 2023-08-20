# include "doctest.h"
# include "score_move_order.hpp"

TEST_CASE("First move score"){
	int good_move_score = score_move_order({"e2e4"});
	int mediocre_move_score = score_move_order({"a2a3"});
	int bad_move_score = score_move_order({"g2g4"});

	CHECK(good_move_score < mediocre_move_score);
	CHECK(mediocre_move_score < bad_move_score);
}
