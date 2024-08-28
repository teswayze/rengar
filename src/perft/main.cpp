# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

# include "../movegen.hpp"
# include "../parse_format.hpp"
# include "../external/doctest.h"
# include "../unittest/consistency_check.hpp"
# include <iostream>

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
	std::cout << "PERFT: Starting position\n";
	
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

TEST_CASE("PERFT: Kiwipete") {
	Board board;
	parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -  0 1", board);
	std::cout << "PERFT: Kiwipete\n";
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(48, 8, 0, 2, 0, 0, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(2'039, 351, 1, 91, 0, 3, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(97'862, 17102, 45, 3'162, 0, 993, 0, 1);
	perft_helper<true>(board, 4);
	check_and_reset_counters(4'085'603, 757'163, 1'929, 128'013, 15'172, 25'523, 6, 43);

}

TEST_CASE("PERFT: En Passant Double Pin") {
	Board board;
	parse_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", board);
	std::cout << "PERFT: En Passant Double Pin\n";
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(14, 1, 0, 0, 0, 2, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(191, 14, 0, 0, 0, 10, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(2'812, 209, 2, 0, 0, 267, 0, 0);
	perft_helper<true>(board, 4);
	check_and_reset_counters(43'238, 3'348, 123, 0, 0, 1'680, 0, 17);
	perft_helper<true>(board, 5);
	check_and_reset_counters(674'624, 52'051, 1'165, 0, 0, 52'950, 3, 0);

}

TEST_CASE("PERFT: Check Evasion") {
	Board board;
	parse_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", board);
	std::cout << "PERFT: Check Evasion\n";
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 1, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(6, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(264, 87, 0, 6, 48, 10, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(9'467, 1'021, 4, 0, 120, 38, 0, 22);
	perft_helper<true>(board, 4);
	check_and_reset_counters(422'333, 131'393, 0, 7'795, 60'032, 15'492, 0, 5);

}

TEST_CASE("PERFT: Bug Catcher") {
	Board board;
	parse_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", board);
	std::cout << "PERFT: Bug Catcher\n";
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(44, 6, 0, 1, 4, 0, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(1'486, 222, 0, 0, 0, 117, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(62'379, 8'517, 0, 1'081, 5'068, 1'201, 0, 44);
	perft_helper<true>(board, 4);
	check_and_reset_counters(2'103'487, 296'153, 0, 0, 0, 158'486, 1'770, 240);

}

TEST_CASE("PERFT: Quiet Developed Middlegame") {
	Board board;
	parse_fen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", board);
	std::cout << "PERFT: Quiet Developed Middlegame\n";
	
	perft_helper<true>(board, 0);
	check_and_reset_counters(1, 0, 0, 0, 0, 0, 0, 0);
	perft_helper<true>(board, 1);
	check_and_reset_counters(46, 4, 0, 0, 0, 1, 0, 0);
	perft_helper<true>(board, 2);
	check_and_reset_counters(2'079, 203, 0, 0, 0, 40, 0, 0);
	perft_helper<true>(board, 3);
	check_and_reset_counters(89'890, 9470, 0, 0, 0, 1783, 0, 0);
	perft_helper<true>(board, 4);
	check_and_reset_counters(3'894'594, 440388, 0, 0, 0, 68'985, 20, 0);

}