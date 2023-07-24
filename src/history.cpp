# include "history.hpp"

bool History::is_repetition(const uint64_t hash) const{
	bool twofold = false;
	for (int idx = curr_idx - 4; idx >= irreversible_idx; idx -= 2){
		if (hash_array[idx] == hash){
			if (twofold or (idx >= root_idx)) return true;
			twofold = true;
		}
	}
	return false;
}
History History::extend(const uint64_t hash){
	hash_array[curr_idx] = hash;
	return History{hash_array, curr_idx + 1, root_idx, irreversible_idx};
}
History History::make_irreversible() const{
	return History{hash_array, curr_idx + 1, root_idx, curr_idx + 1};
}

History History::wipe(){
	return History{hash_array, 0, 0, 0};
}
History History::extend_root(const uint64_t hash){
	hash_array[curr_idx] = hash;
	return History{hash_array, curr_idx + 1, curr_idx + 1, irreversible_idx};
}



# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "board.hpp"
# include "parse_format.hpp"


TEST_CASE("Repetition checking"){
	bool wtm; Board board;
	wtm = parse_fen("8/1p6/1P5p/3Rr2k/p3r1q1/3Q2P1/5B2/6K1 b - - 3 49", board);
	History root_history;
	History search_history;
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<false>(board, move_from_squares(G4, F5, QUEEN_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<true>(board, move_from_squares(D3, D1, QUEEN_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<false>(board, move_from_squares(F5, G4, QUEEN_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<true>(board, move_from_squares(D1, D3, QUEEN_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<false>(board, move_from_squares(E5, F5, ROOK_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<true>(board, move_from_squares(G1, F1, KING_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<false>(board, move_from_squares(F5, E5, ROOK_MOVE));
	CHECK(not root_history.is_repetition(board.EvalInfo.hash));
	CHECK(not search_history.is_repetition(board.EvalInfo.hash));

	root_history = root_history.extend_root(board.EvalInfo.hash);
	search_history = search_history.extend(board.EvalInfo.hash);
	make_move<true>(board, move_from_squares(F1, G1, KING_MOVE));
	CHECK(root_history.is_repetition(board.EvalInfo.hash));
	CHECK(search_history.is_repetition(board.EvalInfo.hash));
}

# endif
