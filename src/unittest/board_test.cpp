# include "../external/doctest.h"
# include "consistency_check.hpp"
# include "../parse_format.hpp"
# include "../board.hpp"
# include "../hashing.hpp"
# include <vector>
# include <string>

constexpr bool operator==(const HalfBoard &x, const HalfBoard &y){
	return x.Pawn == y.Pawn and x.Knight == y.Knight and x.Bishop == y.Bishop and x.Rook == y.Rook
			and x.Queen == y.Queen and x.King == y.King;
}

void check_consistent_for_game(const std::vector<std::string> move_strings){
	Board board;
	parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	bool wtm = true;

	for (size_t i = 0; i < move_strings.size(); i++){
		INFO(move_strings[i]);
		Move move = parse_move_xboard(move_strings[i], board, wtm);
		(wtm ? make_move<true> : make_move<false>)(board, move);
		check_consistent_fb(board);
		wtm = not wtm;
	}
}


TEST_CASE("Check consistent for fool's mate"){
	check_consistent_for_game({"g2g4", "e7e5", "f2f3", "d8h4"});
}

TEST_CASE("Check consistent for opera game"){
	check_consistent_for_game({
		"e2e4", "e7e5", "g1f3", "d7d6", "d2d4", "c8g4", "d4e5", "g4f3", "d1f3", "d6e5",
		"f1c4", "g8f6", "f3b3", "d8e7", "b1c3", "c7c6", "c1g5", "b7b5", "c3b5", "c6b5",
		"c4b5", "b8d7", "e1c1", "a8d8", "d1d7", "d8d7", "h1d1", "e7e6", "b5d7", "f6d7",
		"b3b8", "d7b8", "d1d8",
	});
}

TEST_CASE("Check consistent for Harmon v Borgov"){
	check_consistent_for_game({
		"d2d4", "d7d5", "c2c4", "e7e5", "e2e4", "d5c4", "c1e3", "g8f6", "b1c3", "b8c6",
		"d4d5", "c6e7", "f1c4", "e7g6", "f2f3", "f8d6", "d1d2", "c8d7", "g1e2", "a7a6",
		"c4b3", "b7b5", "a2a4", "e8g8", "e1g1", "d8e7", "a1c1", "f6h5", "g2g3", "h7h6",
		"b3c2", "a8b8", "a4b5", "a6b5", "c1a1", "b8a8", "c2d3", "d6b4", "a1a8", "f8a8",
		"d2c2", "b4c5", "c3d1", "c5d6", "d1f2", "h5f4", "f1c1", "e7g5", "g1h1", "g5h5",
		"e2g1", "f4d3", "f2d3", "f7f5", "d3c5", "d7c8", "c1f1", "g6e7", "c2d3", "f5e4",
		"f3e4", "h5g6", "h1g2", "g8h7", "g1f3", "e7g8", "f3h4", "g6g4", "h4f5", "g8f6",
		"h2h3", "g4g6", "c5e6", "a8a4", "b2b3", "a4e4", "f5d6", "c8e6", "d5e6", "c7d6",
		"e6e7", "d6d5", "e3c5", "g6e8", "d3f3", "e8c6", "b3b4", "c6e8", "f3f5", "h7h8",
		"f5f6", "g7f6", "f1f6", "e8h5", "f6f8", "h8g7", "e7e8q", "e4e2", "g2f1", "h5h3",
		"f1e2", "h3g2", "f8f2", "g2e4", "e2d2",
	});
}

TEST_CASE("Check consistent for Kasparov's Immortal"){
	check_consistent_for_game({
		"e2e4", "d7d6", "d2d4", "g8f6", "b1c3", "g7g6", "c1e3", "f8g7", "d1d2", "c7c6",
		"f2f3", "b7b5", "g1e2", "b8d7", "e3h6", "g7h6", "d2h6", "c8b7", "a2a3", "e7e5",
		"e1c1", "d8e7", "c1b1", "a7a6", "e2c1", "e8c8", "c1b3", "e5d4", "d1d4", "c6c5",
		"d4d1", "d7b6", "g2g3", "c8b8", "b3a5", "b7a8", "f1h3", "d6d5", "h6f4", "b8a7",
		"h1e1", "d5d4", "c3d5", "b6d5", "e4d5", "e7d6", "d1d4", "c5d4", "e1e7", "a7b6",
		"f4d4", "b6a5", "b2b4", "a5a4", "d4c3", "d6d5", "e7a7", "a8b7", "a7b7", "d5c4",
		"c3f6", "a4a3", "f6a6", "a3b4", "c2c3", "b4c3", "a6a1", "c3d2", "a1b2", "d2d1",
		"h3f1", "d8d2", "b7d7", "d2d7", "f1c4", "b5c4", "b2h8", "d7d3", "h8a8", "c4c3",
		"a8a4", "d1e1", "f3f4", "f7f5", "b1c1", "d3d2", "a4a7",
	});
}


TEST_CASE("White pawn promotion"){
	Board b = from_sides_without_eval(from_masks(ToMask(C7), ToMask(C1), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, C2, EMPTY_BOARD),
			from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B8), EMPTY_BOARD, A1, EMPTY_BOARD));
	//Fun fact: the only winning move is to capture the rook and promote to bishop!
	
	Board push_n = b.copy();
	make_move<true>(push_n, move_from_squares(C7, C8, PROMOTE_TO_KNIGHT));
	CHECK(push_n.White.Pawn == EMPTY_BOARD);
	CHECK(push_n.White.Knight == (ToMask(C1) | ToMask(C8)));
	CHECK(push_n.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_b = b.copy();
	make_move<true>(push_b, move_from_squares(C7, C8, PROMOTE_TO_BISHOP));
	CHECK(push_b.White.Pawn == EMPTY_BOARD);
	CHECK(push_b.White.Bishop == ToMask(C8));
	CHECK(push_b.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_r = b.copy();
	make_move<true>(push_r, move_from_squares(C7, C8, PROMOTE_TO_ROOK));
	CHECK(push_r.White.Pawn == EMPTY_BOARD);
	CHECK(push_r.White.Rook == ToMask(C8));
	CHECK(push_r.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_q = b.copy();
	make_move<true>(push_q, move_from_squares(C7, C8, PROMOTE_TO_QUEEN));
	CHECK(push_q.White.Pawn == EMPTY_BOARD);
	CHECK(push_q.White.Queen == ToMask(C8));
	CHECK(push_q.Black == b.Black);
	check_consistent_fb(push_q);
	
	Board take_n = b.copy();
	make_move<true>(take_n, move_from_squares(C7, B8, PROMOTE_TO_KNIGHT));
	CHECK(take_n.White.Pawn == EMPTY_BOARD);
	CHECK(take_n.White.Knight == (ToMask(C1) | ToMask(B8)));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_b = b.copy();
	make_move<true>(take_b, move_from_squares(C7, B8, PROMOTE_TO_BISHOP));
	CHECK(take_b.White.Pawn == EMPTY_BOARD);
	CHECK(take_b.White.Bishop == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_r = b.copy();
	make_move<true>(take_r, move_from_squares(C7, B8, PROMOTE_TO_ROOK));
	CHECK(take_r.White.Pawn == EMPTY_BOARD);
	CHECK(take_r.White.Rook == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_q = b.copy();
	make_move<true>(take_q, move_from_squares(C7, B8, PROMOTE_TO_QUEEN));
	CHECK(take_q.White.Pawn == EMPTY_BOARD);
	CHECK(take_q.White.Queen == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_q);
}

TEST_CASE("Black pawn promotion"){
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(G1), EMPTY_BOARD, H8, EMPTY_BOARD),
			from_masks(ToMask(F2), ToMask(F8), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, F7, EMPTY_BOARD));
	//Fun fact: the only winning move is to capture the rook and promote to bishop!
	
	Board push_n = b.copy();
	make_move<false>(push_n, move_from_squares(F2, F1, PROMOTE_TO_KNIGHT));
	CHECK(push_n.Black.Pawn == EMPTY_BOARD);
	CHECK(push_n.Black.Knight == (ToMask(F1) | ToMask(F8)));
	CHECK(push_n.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_b = b.copy();
	make_move<false>(push_b, move_from_squares(F2, F1, PROMOTE_TO_BISHOP));
	CHECK(push_b.Black.Pawn == EMPTY_BOARD);
	CHECK(push_b.Black.Bishop == ToMask(F1));
	CHECK(push_b.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_r = b.copy();
	make_move<false>(push_r, move_from_squares(F2, F1, PROMOTE_TO_ROOK));
	CHECK(push_r.Black.Pawn == EMPTY_BOARD);
	CHECK(push_r.Black.Rook == ToMask(F1));
	CHECK(push_r.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_q = b.copy();
	make_move<false>(push_q, move_from_squares(F2, F1, PROMOTE_TO_QUEEN));
	CHECK(push_q.Black.Pawn == EMPTY_BOARD);
	CHECK(push_q.Black.Queen == ToMask(F1));
	CHECK(push_q.White == b.White);
	check_consistent_fb(push_q);
	
	Board take_n = b.copy();
	make_move<false>(take_n, move_from_squares(F2, G1, PROMOTE_TO_KNIGHT));
	CHECK(take_n.Black.Pawn == EMPTY_BOARD);
	CHECK(take_n.Black.Knight == (ToMask(G1) | ToMask(F8)));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_b = b.copy();
	make_move<false>(take_b, move_from_squares(F2, G1, PROMOTE_TO_BISHOP));
	CHECK(take_b.Black.Pawn == EMPTY_BOARD);
	CHECK(take_b.Black.Bishop == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_r = b.copy();
	make_move<false>(take_r, move_from_squares(F2, G1, PROMOTE_TO_ROOK));
	CHECK(take_r.Black.Pawn == EMPTY_BOARD);
	CHECK(take_r.Black.Rook == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_q = b.copy();
	make_move<false>(take_q, move_from_squares(F2, G1, PROMOTE_TO_QUEEN));
	CHECK(take_q.Black.Pawn == EMPTY_BOARD);
	CHECK(take_q.Black.Queen == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_q);
}

TEST_CASE("Quiet moves"){
	Board b = from_sides_without_eval(from_masks(ToMask(H2), ToMask(F3), ToMask(E2), ToMask(A1) | ToMask(H1), ToMask(D2), E1, ToMask(A1) | ToMask(H1)),
			from_masks(ToMask(H7), ToMask(F6), ToMask(E7), ToMask(A8) | ToMask(H8), ToMask(D7), E8, ToMask(A8) | ToMask(H8)));
	
	SUBCASE("White moves"){
		Board w_n = b.copy();
		make_move<true>(w_n, move_from_squares(F3, D4, KNIGHT_MOVE));
		CHECK(w_n.White.Knight == ToMask(D4));
		CHECK(w_n.Black == b.Black);
		check_consistent_fb(w_n);
	
		Board w_b = b.copy();
		make_move<true>(w_b, move_from_squares(E2, B5, BISHOP_MOVE));
		CHECK(w_b.White.Bishop == ToMask(B5));
		CHECK(w_b.Black == b.Black);
		check_consistent_fb(w_b);
	
		Board w_r = b.copy();
		make_move<true>(w_r, move_from_squares(H1, G1, ROOK_MOVE));
		CHECK(w_r.White.Rook == (ToMask(A1) | ToMask(G1)));
		CHECK(w_r.White.Castle == ToMask(A1));
		CHECK(w_r.Black == b.Black);
		check_consistent_fb(w_r);

		Board w_q = b.copy();
		make_move<true>(w_q, move_from_squares(D2, B2, QUEEN_MOVE));
		CHECK(w_q.White.Queen == ToMask(B2));
		CHECK(w_q.Black == b.Black);
		check_consistent_fb(w_q);
	
		Board w_k = b.copy();
		make_move<true>(w_k, move_from_squares(E1, F2, KING_MOVE));
		CHECK(w_k.White.King == F2);
		CHECK(w_k.White.Castle == EMPTY_BOARD);
		CHECK(w_k.Black == b.Black);
		check_consistent_fb(w_k);
	
		Board w_cq = b.copy();
		make_move<true>(w_cq, move_from_squares(E1, C1, CASTLE_QUEENSIDE));
		CHECK(w_cq.White.King == C1);
		CHECK(w_cq.White.Rook == (ToMask(D1) | ToMask(H1)));
		CHECK(w_cq.White.Castle == EMPTY_BOARD);
		CHECK(w_cq.Black == b.Black);
		check_consistent_fb(w_cq);

		Board w_ck = b.copy();
		make_move<true>(w_ck, move_from_squares(E1, G1, CASTLE_KINGSIDE));
		CHECK(w_ck.White.King == G1);
		CHECK(w_ck.White.Rook == (ToMask(A1) | ToMask(F1)));
		CHECK(w_ck.White.Castle == EMPTY_BOARD);
		CHECK(w_ck.Black == b.Black);
		check_consistent_fb(w_ck);

		Board w_p1 = b.copy();
		make_move<true>(w_p1, move_from_squares(H2, H3, SINGLE_PAWN_PUSH));
		CHECK(w_p1.White.Pawn == ToMask(H3));
		CHECK(w_p1.EPMask == EMPTY_BOARD);
		CHECK(w_p1.Black == b.Black);
		check_consistent_fb(w_p1);

		Board w_p2 = b.copy();
		make_move<true>(w_p2, move_from_squares(H2, H4, DOUBLE_PAWN_PUSH));
		CHECK(w_p2.White.Pawn == ToMask(H4));
		CHECK(w_p2.EPMask == ToMask(H4));
		CHECK(w_p2.Black == b.Black);
		check_consistent_fb(w_p2);
	}

	SUBCASE("Black moves"){
		Board b_n = b.copy();
		make_move<false>(b_n, move_from_squares(F6, D5, KNIGHT_MOVE));
		CHECK(b_n.Black.Knight == ToMask(D5));
		CHECK(b_n.White == b.White);
		check_consistent_fb(b_n);

		Board b_b = b.copy();
		make_move<false>(b_b, move_from_squares(E7, B4, BISHOP_MOVE));
		CHECK(b_b.Black.Bishop == ToMask(B4));
		CHECK(b_b.White == b.White);
		check_consistent_fb(b_b);

		Board b_r = b.copy();
		make_move<false>(b_r, move_from_squares(H8, G8, ROOK_MOVE));
		CHECK(b_r.Black.Rook == (ToMask(A8) | ToMask(G8)));
		CHECK(b_r.Black.Castle == ToMask(A8));
		CHECK(b_r.White == b.White);
		check_consistent_fb(b_r);

		Board b_q = b.copy();
		make_move<false>(b_q, move_from_squares(D7, B7, QUEEN_MOVE));
		CHECK(b_q.Black.Queen == ToMask(B7));
		CHECK(b_q.White == b.White);
		check_consistent_fb(b_q);

		Board b_k = b.copy();
		make_move<false>(b_k, move_from_squares(E8, F7, KING_MOVE));
		CHECK(b_k.Black.King == F7);
		CHECK(b_k.Black.Castle == EMPTY_BOARD);
		CHECK(b_k.White == b.White);
		check_consistent_fb(b_k);

		Board b_cq = b.copy();
		make_move<false>(b_cq, move_from_squares(E8, C8, CASTLE_QUEENSIDE));
		CHECK(b_cq.Black.King == C8);
		CHECK(b_cq.Black.Rook == (ToMask(D8) | ToMask(H8)));
		CHECK(b_cq.Black.Castle == EMPTY_BOARD);
		CHECK(b_cq.White == b.White);
		check_consistent_fb(b_cq);

		Board b_ck = b.copy();
		make_move<false>(b_ck, move_from_squares(E8, G8, CASTLE_KINGSIDE));
		CHECK(b_ck.Black.King == G8);
		CHECK(b_ck.Black.Rook == (ToMask(A8) | ToMask(F8)));
		CHECK(b_ck.Black.Castle == EMPTY_BOARD);
		CHECK(b_ck.White == b.White);
		check_consistent_fb(b_ck);

		Board b_p1 = b.copy();
		make_move<false>(b_p1, move_from_squares(H7, H6, SINGLE_PAWN_PUSH));
		CHECK(b_p1.Black.Pawn == ToMask(H6));
		CHECK(b_p1.EPMask == EMPTY_BOARD);
		CHECK(b_p1.White == b.White);
		check_consistent_fb(b_p1);

		Board b_p2 = b.copy();
		make_move<false>(b_p2, move_from_squares(H7, H5, DOUBLE_PAWN_PUSH));
		CHECK(b_p2.Black.Pawn == ToMask(H5));
		CHECK(b_p2.EPMask == ToMask(H5));
		CHECK(b_p2.White == b.White);
		check_consistent_fb(b_p2);
	}

}

TEST_CASE("White captures"){
	Board b = from_sides_without_eval_ep(from_masks(ToMask(F5), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, B4, EMPTY_BOARD),
				from_masks(ToMask(E5) | ToMask(B3) | ToMask(A3), ToMask(A5), ToMask(A4), ToMask(C3), ToMask(G6), E8, EMPTY_BOARD),
				D5);
	
	Board x_p = b.copy();
	make_move<true>(x_p, move_from_squares(B4, A3, KING_MOVE));
	CHECK(x_p.Black.Pawn == (ToMask(E5) | ToMask(B3)));
	check_consistent_fb(x_p);
	
	Board x_n = b.copy();
	make_move<true>(x_n, move_from_squares(B4, A5, KING_MOVE));
	CHECK(x_n.Black.Knight == EMPTY_BOARD);
	check_consistent_fb(x_n);
	
	Board x_b = b.copy();
	make_move<true>(x_b, move_from_squares(B4, A4, KING_MOVE));
	CHECK(x_b.Black.Bishop == EMPTY_BOARD);
	check_consistent_fb(x_b);
	
	Board x_r = b.copy();
	make_move<true>(x_r, move_from_squares(B4, C3, KING_MOVE));
	CHECK(x_r.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(x_r);
	
	Board x_q = b.copy();
	make_move<true>(x_q, move_from_squares(F5, G6, PAWN_CAPTURE));
	CHECK(x_q.White.Pawn == ToMask(G6));
	CHECK(x_q.Black.Queen == EMPTY_BOARD);
	check_consistent_fb(x_q);
	
	Board x_ep = b.copy();
	make_move<true>(x_ep, move_from_squares(F5, E6, EN_PASSANT_CAPTURE));
	CHECK(x_ep.White.Pawn == ToMask(E6));
	CHECK(x_ep.Black.Pawn == (ToMask(B3) | ToMask(A3)));
	check_consistent_fb(x_ep);
}

TEST_CASE("Black captures"){
	Board b = from_sides_without_eval_ep(from_masks(ToMask(E4) | ToMask(B6) | ToMask(A6), ToMask(A4), ToMask(A5), ToMask(C6), ToMask(G3), E1, EMPTY_BOARD),
				from_masks(ToMask(F4), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, B5, EMPTY_BOARD),
				D4);
	
	Board x_p = b.copy();
	make_move<false>(x_p, move_from_squares(B5, A6, KING_MOVE));
	CHECK(x_p.White.Pawn == (ToMask(E4) | ToMask(B6)));
	check_consistent_fb(x_p);
	
	Board x_n = b.copy();
	make_move<false>(x_n, move_from_squares(B5, A4, KING_MOVE));
	CHECK(x_n.White.Knight == EMPTY_BOARD);
	check_consistent_fb(x_n);
	
	Board x_b = b.copy();
	make_move<false>(x_b, move_from_squares(B5, A5, KING_MOVE));
	CHECK(x_b.White.Bishop == EMPTY_BOARD);
	check_consistent_fb(x_b);
	
	Board x_r = b.copy();
	make_move<false>(x_r, move_from_squares(B5, C6, KING_MOVE));
	CHECK(x_r.White.Rook == EMPTY_BOARD);
	check_consistent_fb(x_r);
	
	Board x_q = b.copy();
	make_move<false>(x_q, move_from_squares(F4, G3, PAWN_CAPTURE));
	CHECK(x_q.Black.Pawn == ToMask(G3));
	CHECK(x_q.White.Queen == EMPTY_BOARD);
	check_consistent_fb(x_q);
	
	Board x_ep = b.copy();
	make_move<false>(x_ep, move_from_squares(F4, E3, EN_PASSANT_CAPTURE));
	CHECK(x_ep.Black.Pawn == ToMask(E3));
	CHECK(x_ep.White.Pawn == (ToMask(B6) | ToMask(A6)));
	check_consistent_fb(x_ep);
}

TEST_CASE("Capturing rook strips castling rights"){
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A1) | ToMask(H1), EMPTY_BOARD, E1, ToMask(A1) | ToMask(H1)),
				from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A8) | ToMask(H8), EMPTY_BOARD, E8, ToMask(A8) | ToMask(H8)));
	
	auto expected_w = white_king_hash[E1] ^ white_rook_hash[A1] ^ white_rook_hash[H1] ^ white_cqs_hash ^ white_cks_hash;
	auto expected_b = black_king_hash[E8] ^ black_rook_hash[A8] ^ black_rook_hash[H8] ^ black_cqs_hash ^ black_cks_hash;

	CHECK(b.ue.hash == (expected_w ^ expected_b));

	Board w_q = b.copy();
	make_move<true>(w_q, move_from_squares(A1, A8, ROOK_MOVE));
	CHECK(w_q.Black.Castle == ToMask(H8));
	check_consistent_fb(w_q);
	
	Board w_k = b.copy();
	make_move<true>(w_k, move_from_squares(H1, H8, ROOK_MOVE));
	CHECK(w_k.Black.Castle == ToMask(A8));
	check_consistent_fb(w_k);
	
	Board b_q = b.copy();
	make_move<false>(b_q, move_from_squares(A8, A1, ROOK_MOVE));
	CHECK(b_q.White.Castle == ToMask(H1));
	check_consistent_fb(b_q);
	
	Board b_k = b.copy();
	make_move<false>(b_k, move_from_squares(H8, H1, ROOK_MOVE));
	CHECK(b_k.White.Castle == ToMask(A1));
	check_consistent_fb(b_k);
}
