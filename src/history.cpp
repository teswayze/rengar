# include "board.hpp"
# include "history.hpp"
# include "parse_format.hpp"

History extend_history(const Board &board, const History history){
	return std::make_shared<ListNode<uint64_t>>(board.EvalInfo.hash, history);
}

template <bool check_second>
bool exists_in_history_helper(const uint64_t hash, const History history){
	// Only check every other to allow triangulation e.g.
	if (history == nullptr) { return false; }
	if (history->tail == nullptr) { return false; }
	if (check_second and (hash == history->tail->head)) { return true; }
	return exists_in_history_helper<true>(hash, history->tail->tail);
}

bool exists_in_history(const Board &board, const History history){
	return exists_in_history_helper<false>(board.EvalInfo.hash, history);
}

History remove_single_repetitions(const History history){
	if (history == nullptr) return nullptr;
	History trimmed_tail = remove_single_repetitions(history->tail);
	if (exists_in_history_helper<false>(history->head, history->tail)) {
		return std::make_shared<ListNode<uint64_t>>(history->head, trimmed_tail);
	}
	if (trimmed_tail == nullptr) return nullptr;
	return std::make_shared<ListNode<uint64_t>>(0ull, trimmed_tail);
}


Variation prepend_to_variation(const Move move, const Variation variation){
	return std::make_shared<ListNode<Move>>(move, variation);
}

std::string show_variation_helper(const Variation variation, std::string prefix){
	if (variation == nullptr) {return prefix;}
	return show_variation_helper(variation->tail, prefix + " " + format_move_xboard(variation->head));
}

std::string show_variation(const Variation variation){
	return show_variation_helper(variation, "");
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"


TEST_CASE("Repetition checking"){
	bool wtm; Board board0;
	std::tie(wtm, board0) = parse_fen("8/1p6/1P5p/3Rr2k/p3r1q1/3Q2P1/5B2/6K1 b - - 3 49");
	History history0 = nullptr;
	CHECK(not exists_in_history(board0, history0));

	Board board1 = make_move<false>(board0, move_from_squares(G4, F5, QUEEN_MOVE));
	History history1 = extend_history(board0, history0);
	CHECK(not exists_in_history(board1, history1));

	Board board2 = make_move<true>(board1, move_from_squares(D3, D1, QUEEN_MOVE));
	History history2 = extend_history(board1, history1);
	CHECK(not exists_in_history(board2, history2));

	Board board3 = make_move<false>(board2, move_from_squares(F5, G4, QUEEN_MOVE));
	History history3 = extend_history(board2, history2);
	CHECK(not exists_in_history(board3, history3));

	Board board4 = make_move<true>(board3, move_from_squares(D1, D3, QUEEN_MOVE));
	History history4 = extend_history(board3, history3);
	CHECK(exists_in_history(board4, history4));
	CHECK(not exists_in_history(board4, remove_single_repetitions(history4)));

	Board board5 = make_move<false>(board4, move_from_squares(E5, F5, ROOK_MOVE));
	History history5 = extend_history(board4, history4);
	CHECK(not exists_in_history(board5, history5));

	Board board6 = make_move<true>(board5, move_from_squares(G1, F1, KING_MOVE));
	History history6 = extend_history(board5, history5);
	CHECK(not exists_in_history(board6, history6));

	Board board7 = make_move<false>(board6, move_from_squares(F5, E5, ROOK_MOVE));
	History history7 = extend_history(board6, history6);
	CHECK(not exists_in_history(board7, history7));

	Board board8 = make_move<true>(board7, move_from_squares(F1, G1, KING_MOVE));
	History history8 = extend_history(board7, history7);
	CHECK(exists_in_history(board8, history8));
	CHECK(exists_in_history(board8, remove_single_repetitions(history8)));
}

# endif
