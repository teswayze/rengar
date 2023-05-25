# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

# include "movegen.hpp"
# include "parse_format.hpp"
# include "doctest.h"
# include "consistency_check.hpp"

int nodes = 0;
int captures = 0;
int en_passants = 0;
int castles = 0;
int promotions = 0;
int checks = 0;
int double_checks = 0;
int checkmates = 0;


void check_and_reset_counters(
		const int e_nodes, const int e_captures, const int e_en_passants, const int e_castles, 
		const int e_promotions, const int e_checks, const int e_double_checks, const int e_checkmates) {
	CHECK(e_nodes == nodes);
	CHECK(e_captures == captures);
	CHECK(e_en_passants == en_passants);
	CHECK(e_castles == castles);
	CHECK(e_promotions == promotions);
	CHECK(e_checks == checks);
	CHECK(e_double_checks == double_checks);
	CHECK(e_checkmates == checkmates);
	
	nodes = 0;
	captures = 0;
	en_passants = 0;
	castles = 0;
	promotions = 0;
	checks = 0;
	double_checks = 0;
	checkmates = 0;
}

template <bool wtm>
void perft_helper(const Board &board, const int depth){	
	const auto cnp = checks_and_pins<wtm>(board);
	
	if (depth == 0){
		check_consistent_fb(board);

		nodes += 1;
		if (cnp.CheckMask != FULL_BOARD) {
			checks += 1;
			if (cnp.CheckMask == EMPTY_BOARD) double_checks += 1;
			if (generate_moves<wtm>(board, cnp, 0, 0, 0).empty()) checkmates += 1;
		}
		return;
	}
	
	auto queue = generate_moves<wtm>(board, cnp, 0, 0, 0);
	
	while (not queue.empty()) {
		const auto move = queue.top();
		auto next_board = board.copy();
		const auto maybe_capture = make_move<wtm>(next_board, move);
		
		INFO(format_move_xboard(move));
		
		if (depth == 1){
			if (maybe_capture) captures += 1;
			if (move_flags(move) == EN_PASSANT_CAPTURE) en_passants += 1;
			if ((move_flags(move) == CASTLE_QUEENSIDE) or (move_flags(move) == CASTLE_KINGSIDE)) castles += 1;
			if (move_flags(move) >= PROMOTE_TO_KNIGHT) promotions += 1;
		}
		
		perft_helper<not wtm>(next_board, depth - 1);
		
		queue.pop();
	}
	
}

TEST_CASE("PERFT: Starting position") {
	Board board;
	parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(20, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(400, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(8'902, 34, 0, 0, 0, 12, 0, 0);
	perft_helper<true>(board, 4);
	check_and_reset_counters(197'281, 1'576, 0, 0, 0, 469, 0, 8);
	perft_helper<true>(board, 5);
	check_and_reset_counters(4'865'609, 82'719, 258, 0, 0, 27'351, 0, 347);

}
