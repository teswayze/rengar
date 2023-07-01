# include "variation.hpp"
# include <algorithm>

VariationView VariationView::prepend(const Move move) const{
	auto prev = location - maxsize - 1;
	*prev = move;
	std::copy(location, location + length, prev + 1);
	return VariationView{prev, length + 1, maxsize + 1};
}
VariationView VariationView::fresh_branch() const{
	return VariationView{location + maxsize, 0, maxsize - 1};
}
VariationView VariationView::copy_branch() const{
	if (length) std::copy(location + 1, location + length, location + maxsize);
	return VariationView{location + maxsize, length - 1, maxsize - 1};
}
VariationView VariationView::nullify() const{
	return VariationView{location, 0, maxsize};
}
VariationView VariationView::singleton(const Move move) const{
	*location = move;
	return VariationView{location, 1, maxsize};
}
Move VariationView::head() const{
	return *location;
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

const Move c4 = move_from_squares(C2, C4, DOUBLE_PAWN_PUSH);
const Move d4 = move_from_squares(D2, D4, DOUBLE_PAWN_PUSH);
const Move e4 = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);
const Move Bf4 = move_from_squares(C1, C4, BISHOP_MOVE);

const Move c5 = move_from_squares(C7, C5, DOUBLE_PAWN_PUSH);
const Move d5 = move_from_squares(D7, D5, DOUBLE_PAWN_PUSH);
const Move e6 = move_from_squares(E7, E6, SINGLE_PAWN_PUSH);
const Move c6 = move_from_squares(C7, C6, SINGLE_PAWN_PUSH);

TEST_CASE("Variation building"){
	VariationWorkspace vw;
	
	auto start = VariationView(vw);
	CHECK(start.location == vw.data());
	CHECK(start.length == 0);
	auto queens_pawn = start.fresh_branch();
	CHECK(queens_pawn.length == 0);
	auto dqp = queens_pawn.fresh_branch();
	CHECK(dqp.length == 0);
	auto queens_gambit = dqp.fresh_branch();
	CHECK(queens_gambit.length == 0);
	auto qgd = queens_gambit.fresh_branch();
	CHECK(qgd.length == 0);
	
	queens_gambit = qgd.prepend(e6);
	CHECK(queens_gambit.length == 1);
	CHECK(queens_gambit.head() == e6);
	
	auto slav = queens_gambit.fresh_branch();
	CHECK(slav.length == 0);
	queens_gambit = slav.prepend(c6);
	CHECK(queens_gambit.length == 1);
	CHECK(queens_gambit.head() == c6);
	
	dqp = queens_gambit.prepend(c4);
	CHECK(dqp.length == 2);
	CHECK(dqp.head() == c4);
	CHECK(dqp.location[1] == c6);
	
	queens_pawn = dqp.prepend(d5);
	CHECK(queens_pawn.length == 3);
	CHECK(queens_pawn.head() == d5);
	CHECK(queens_pawn.location[1] == c4);
	CHECK(queens_pawn.location[2] == c6);
	
	auto french_invi = queens_pawn.fresh_branch();
	CHECK(french_invi.length == 0);
	french_invi = french_invi.singleton(e4);
	CHECK(french_invi.length == 1);
	CHECK(french_invi.head() == e4);
	
	CHECK(queens_pawn.length == 3);
	CHECK(queens_pawn.head() == d5);
	CHECK(queens_pawn.location[1] == c4);
	CHECK(queens_pawn.location[2] == c6);
	
	auto indian_game = queens_pawn.fresh_branch();
	auto london = indian_game.fresh_branch();
	london = london.fresh_branch().prepend(c5);
	CHECK(london.length == 1);
	CHECK(london.head() == c5);
	indian_game = london.prepend(Bf4);
	CHECK(indian_game.length == 2);
	CHECK(indian_game.head() == Bf4);
	CHECK(indian_game.location[1] == c5);
	
	indian_game = indian_game.singleton(c4);
	CHECK(indian_game.length == 1);
	CHECK(indian_game.head() == c4);
	
	CHECK(queens_pawn.length == 3);
	CHECK(queens_pawn.head() == d5);
	CHECK(queens_pawn.location[1] == c4);
	CHECK(queens_pawn.location[2] == c6);
	
	start = queens_pawn.prepend(d4);

	CHECK(start.length == 4);
	CHECK(start.location == vw.data());
	CHECK(start.head() == d4);
	CHECK(start.location[1] == d5);
	CHECK(start.location[2] == c4);
	CHECK(start.location[3] == c6);
}

# endif
