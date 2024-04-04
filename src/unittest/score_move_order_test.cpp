# include "doctest.h"
# include "../tune/score_move_order.hpp"
# include "../move_queue.hpp"

TEST_CASE("First move score"){
	initialize_move_order_arrays();

	int good_move_score = score_move_order({move_from_squares(E2, E4, DOUBLE_PAWN_PUSH)});
	int mediocre_move_score = score_move_order({move_from_squares(A2, A3, SINGLE_PAWN_PUSH)});
	int bad_move_score = score_move_order({move_from_squares(G2, G4, DOUBLE_PAWN_PUSH)});

	CHECK(good_move_score < mediocre_move_score);
	CHECK(mediocre_move_score < bad_move_score);
}
