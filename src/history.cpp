# include "board.hpp"
# include "history.hpp"

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

History remove_hash_from_history(const History history, const Board &board){
	if (history == nullptr) return history;
	if (history->tail == nullptr) return history;
	auto second_head = (history->tail->head == board.EvalInfo.hash) ? 0ull : history->tail->head;
	return std::make_shared<ListNode<uint64_t>>(history->head, std::make_shared<ListNode<uint64_t>>(second_head,
			remove_hash_from_history(history->tail->tail, board)));
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"


TEST_CASE("Repetition checking"){
	bool wtm; Board board;
	wtm = parse_fen("8/1p6/1P5p/3Rr2k/p3r1q1/3Q2P1/5B2/6K1 b - - 3 49", board);
	History history0 = nullptr;
	CHECK(not exists_in_history(board, history0));

	History history1 = extend_history(board, history0);
	make_move<false>(board, move_from_squares(G4, F5, QUEEN_MOVE));
	CHECK(not exists_in_history(board, history1));

	History history2 = extend_history(board, history1);
	make_move<true>(board, move_from_squares(D3, D1, QUEEN_MOVE));
	CHECK(not exists_in_history(board, history2));

	History history3 = extend_history(board, history2);
	make_move<false>(board, move_from_squares(F5, G4, QUEEN_MOVE));
	CHECK(not exists_in_history(board, history3));

	History history4 = extend_history(board, history3);
	make_move<true>(board, move_from_squares(D1, D3, QUEEN_MOVE));
	CHECK(exists_in_history(board, history4));
	CHECK(not exists_in_history(board, remove_single_repetitions(history4)));

	History history5 = extend_history(board, history4);
	make_move<false>(board, move_from_squares(E5, F5, ROOK_MOVE));
	CHECK(not exists_in_history(board, history5));

	History history6 = extend_history(board, history5);
	make_move<true>(board, move_from_squares(G1, F1, KING_MOVE));
	CHECK(not exists_in_history(board, history6));

	History history7 = extend_history(board, history6);
	make_move<false>(board, move_from_squares(F5, E5, ROOK_MOVE));
	CHECK(not exists_in_history(board, history7));

	History history8 = extend_history(board, history7);
	make_move<true>(board, move_from_squares(F1, G1, KING_MOVE));
	CHECK(exists_in_history(board, history8));
	CHECK(exists_in_history(board, remove_single_repetitions(history8)));
}

# endif
