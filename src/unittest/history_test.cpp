# include "../external/doctest.h"
# include "../board.hpp"
# include "../parse_format.hpp"
# include "../history.hpp"


TEST_CASE("Repetition checking from root"){
	Board board;
	parse_fen("8/1p6/1P5p/3Rr2k/p3r1q1/3Q2P1/5B2/6K1 b - - 3 49", board);
	History root_history;
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<false>(board, move_from_squares(G4, F5, QUEEN_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<true>(board, move_from_squares(D3, D1, QUEEN_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<false>(board, move_from_squares(F5, G4, QUEEN_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<true>(board, move_from_squares(D1, D3, QUEEN_MOVE));
	// Two-fold, but no game-ending repetition
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, false));
	CHECK(0 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<false>(board, move_from_squares(E5, F5, ROOK_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<true>(board, move_from_squares(G1, F1, KING_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<false>(board, move_from_squares(F5, E5, ROOK_MOVE));
	CHECK(-1 == take_view(root_history).index_of_repetition(board.ue.hash, true));

	root_history.extend_root(board.ue.hash);
	make_move<true>(board, move_from_squares(F1, G1, KING_MOVE));
	// Threefold repetition, ending the game
	CHECK(0 == take_view(root_history).index_of_repetition(board.ue.hash, false));
	CHECK(4 == take_view(root_history).index_of_repetition(board.ue.hash, true));
}

TEST_CASE("Repetition checking in search"){
	Board board;
	parse_fen("8/1p6/1P5p/3Rr2k/p3r1q1/3Q2P1/5B2/6K1 b - - 3 49", board);
	History root_history;
	HistoryView search_history = take_view(root_history);
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<false>(board, move_from_squares(G4, F5, QUEEN_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<true>(board, move_from_squares(D3, D1, QUEEN_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<false>(board, move_from_squares(F5, G4, QUEEN_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<true>(board, move_from_squares(D1, D3, QUEEN_MOVE));
	// Two-fold which we identify as a draw in search
	CHECK(0 == search_history.index_of_repetition(board.ue.hash, false));

	search_history = search_history.extend(board.ue.hash);
	make_move<false>(board, move_from_squares(E5, F5, ROOK_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<true>(board, move_from_squares(G1, F1, KING_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<false>(board, move_from_squares(F5, E5, ROOK_MOVE));
	CHECK(-1 == search_history.index_of_repetition(board.ue.hash, true));

	search_history = search_history.extend(board.ue.hash);
	make_move<true>(board, move_from_squares(F1, G1, KING_MOVE));
	// Threefold. When searching for repetition, we first find the one at ply 4
	CHECK(4 == search_history.index_of_repetition(board.ue.hash, false));
}
