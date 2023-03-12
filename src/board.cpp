# include "board.hpp"

# include <exception>

PstEvalInfo recompute_from_sides(const HalfBoard white, const HalfBoard black){
	auto w_eval = static_eval_info<true>(white.Pawn, white.Knight, white.Bishop, white.Rook, white.Queen, white.King);
	auto b_eval = static_eval_info<false>(black.Pawn, black.Knight, black.Bishop, black.Rook, black.Queen, black.King);
	return half_to_full_eval_info(w_eval, b_eval);
}

Board from_sides_without_eval(const HalfBoard white, const HalfBoard black){
	return Board{white, black, white.All | black.All, EMPTY_BOARD, recompute_from_sides(white, black)};
}

Board from_sides_without_eval_ep(const HalfBoard white, const HalfBoard black, const Square ep){
	return Board{white, black, white.All | black.All, ToMask(ep), recompute_from_sides(white, black)};
}

const PstEvalInfo NO_DIFFERENCE = PstEvalInfo{0, 0, 0};

template <bool white>
PstEvalInfo get_eval_contribution_at_square(const HalfBoard board, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return NO_DIFFERENCE; }
	if (mask & board.Pawn){ return PstEvalInfo{
		mg_pawn_table[FlipIf(white, square)] + MG_PAWN, eg_pawn_table[FlipIf(white, square)] + EG_PAWN, 0
	};}
	if (mask & board.Knight){ return PstEvalInfo{
		mg_knight_table[FlipIf(white, square)] + MG_KNIGHT, eg_knight_table[FlipIf(white, square)] + EG_KNIGHT, 1
	};}
	if (mask & board.Bishop){ return PstEvalInfo{
		mg_bishop_table[FlipIf(white, square)] + MG_BISHOP, eg_bishop_table[FlipIf(white, square)] + EG_BISHOP, 1
	};}
	if (mask & board.Rook){ return PstEvalInfo{
		mg_rook_table[FlipIf(white, square)] + MG_ROOK, eg_rook_table[FlipIf(white, square)] + EG_ROOK, 2
	};}
	if (mask & board.Queen){ return PstEvalInfo{
		mg_queen_table[FlipIf(white, square)] + MG_QUEEN, eg_queen_table[FlipIf(white, square)] + EG_QUEEN, 4
	};}
	
	throw std::logic_error("Something went wrong - trying to remove king? 'All' out of sync with other masks?");
}

template <bool white>
PstEvalInfo get_eval_contribution_of_pawn(const Square square){
	return PstEvalInfo{ mg_pawn_table[FlipIf(white, square)] + MG_PAWN, eg_pawn_table[FlipIf(white, square)] + EG_PAWN, 0 };
}


HalfBoard maybe_remove_piece(const HalfBoard board, const Square square){
	BitMask mask = ~ToMask(square);
	return HalfBoard{board.Pawn & mask, board.Knight & mask, board.Bishop & mask, board.Rook & mask, board.Queen & mask, board.King, board.All & mask, board.Castle & mask};
}

HalfBoard remove_pawn(const HalfBoard board, const Square square){
	return HalfBoard{board.Pawn ^ ToMask(square), board.Knight, board.Bishop, board.Rook, board.Queen, board.King, board.All ^ ToMask(square), board.Castle};
}


template <bool white>
PstEvalInfo compute_eval_diff_for_move(const HalfBoard enemy, const Move move){
	const Square from = move_source(move);
	const Square to = move_destination(move);
	const PstEvalInfo removal_info = get_eval_contribution_at_square<not white>(enemy, to);

	switch (move_flags(move)){
		case NULL_MOVE:
			return NO_DIFFERENCE;

		case KNIGHT_MOVE:
			return PstEvalInfo{
				mg_knight_table[FlipIf(white, to)] - mg_knight_table[FlipIf(white, from)] + removal_info.mg, 
				eg_knight_table[FlipIf(white, to)] - eg_knight_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};
		case BISHOP_MOVE:
			return PstEvalInfo{
				mg_bishop_table[FlipIf(white, to)] - mg_bishop_table[FlipIf(white, from)] + removal_info.mg, 
				eg_bishop_table[FlipIf(white, to)] - eg_bishop_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};
		case ROOK_MOVE:
			return PstEvalInfo{
				mg_rook_table[FlipIf(white, to)] - mg_rook_table[FlipIf(white, from)] + removal_info.mg, 
				eg_rook_table[FlipIf(white, to)] - eg_rook_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};
		case QUEEN_MOVE:
			return PstEvalInfo{
				mg_queen_table[FlipIf(white, to)] - mg_queen_table[FlipIf(white, from)] + removal_info.mg, 
				eg_queen_table[FlipIf(white, to)] - eg_queen_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};
		case KING_MOVE:
			return PstEvalInfo{
				mg_king_table[FlipIf(white, to)] - mg_king_table[FlipIf(white, from)] + removal_info.mg, 
				eg_king_table[FlipIf(white, to)] - eg_king_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};

		case CASTLE_QUEENSIDE:
			return PstEvalInfo{
				mg_king_table[C8] + mg_rook_table[D8] - mg_king_table[E8] - mg_rook_table[A8],
				eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8],
				0};
		case CASTLE_KINGSIDE:
			return PstEvalInfo{
				mg_king_table[G8] + mg_rook_table[F8] - mg_king_table[E8] - mg_rook_table[H8],
				eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8],
				0};

		case SINGLE_PAWN_PUSH:
		case DOUBLE_PAWN_PUSH:
			return PstEvalInfo{
				mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
				eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
				0};
		case PAWN_CAPTURE:
			return PstEvalInfo{
				mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)] + removal_info.mg, 
				eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count};
		case EN_PASSANT_CAPTURE:
			return get_eval_contribution_of_pawn<not white>(to + (white ? -8 : 8)) + PstEvalInfo{
				mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
				eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
				0};

		case PROMOTE_TO_KNIGHT:
			return PstEvalInfo{
				mg_knight_table[FlipIf(white, to)] + MG_KNIGHT - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_knight_table[FlipIf(white, to)] + EG_KNIGHT - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				1 - removal_info.phase_count};
		case PROMOTE_TO_BISHOP:
			return PstEvalInfo{
				mg_bishop_table[FlipIf(white, to)] + MG_BISHOP - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_bishop_table[FlipIf(white, to)] + EG_BISHOP - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				1 - removal_info.phase_count};
		case PROMOTE_TO_ROOK:
			return PstEvalInfo{
				mg_rook_table[FlipIf(white, to)] + MG_ROOK - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_rook_table[FlipIf(white, to)] + EG_ROOK - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				2 - removal_info.phase_count};
		case PROMOTE_TO_QUEEN:
			return PstEvalInfo{
				mg_queen_table[FlipIf(white, to)] + MG_QUEEN - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_queen_table[FlipIf(white, to)] + EG_QUEEN - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				4 - removal_info.phase_count};
		}
		throw std::logic_error("Unexpected move flag");
}

template <bool white>
Board make_move_with_new_eval(const Board board, const Move move, const PstEvalInfo new_eval){
	const HalfBoard f = get_side<white>(board); // f for friendly
	const HalfBoard e = get_side<not white>(board); // e for enemy

	const Square from = move_source(move);
	const Square to = move_destination(move);
	const BitMask move_mask = ToMask(from) | ToMask(to);
	
	switch (move_flags(move)){
	case NULL_MOVE:
		return board;

	case KNIGHT_MOVE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight ^ move_mask, f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case BISHOP_MOVE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop ^ move_mask, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case ROOK_MOVE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook ^ move_mask, f.Queen, f.King, f.All ^ move_mask, f.Castle & ~ToMask(from)},
					maybe_remove_piece(e, to), new_eval
				);
	case QUEEN_MOVE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook, f.Queen ^ move_mask, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case KING_MOVE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook, f.Queen, f.King ^ move_mask, f.All ^ move_mask, EMPTY_BOARD},
					maybe_remove_piece(e, to), new_eval
				);

	case CASTLE_QUEENSIDE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook ^ (white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8))), f.Queen, 
							f.King ^ (white ? (ToMask(E1) | ToMask(C1)) : (ToMask(E8) | ToMask(C8))),
							f.All ^ (white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8))) ^ (white ? (ToMask(E1) | ToMask(C1)) : (ToMask(E8) | ToMask(C8))), EMPTY_BOARD},
					e, new_eval
				);
	case CASTLE_KINGSIDE:
		return from_sides<white>(
					HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook ^ (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))), f.Queen,
			f.King ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8))), 
			f.All ^ (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))) ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8))), EMPTY_BOARD},
					e, new_eval
				);

	case SINGLE_PAWN_PUSH:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ move_mask, f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					e, new_eval
				);
	case DOUBLE_PAWN_PUSH:
		return from_sides_ep<white>(
					HalfBoard{f.Pawn ^ move_mask, f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					e, to, new_eval
				);
	case PAWN_CAPTURE:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ move_mask, f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case EN_PASSANT_CAPTURE:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ move_mask, f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					remove_pawn(e, to + (white ? -8 : 8)), new_eval
				);

	case PROMOTE_TO_KNIGHT:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ ToMask(from), f.Knight ^ ToMask(to), f.Bishop, f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case PROMOTE_TO_BISHOP:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop ^ ToMask(to), f.Rook, f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case PROMOTE_TO_ROOK:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop, f.Rook ^ ToMask(to), f.Queen, f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	case PROMOTE_TO_QUEEN:
		return from_sides<white>(
					HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop, f.Rook, f.Queen ^ ToMask(to), f.King, f.All ^ move_mask, f.Castle},
					maybe_remove_piece(e, to), new_eval
				);
	}
	throw std::logic_error("Unexpected move flag");
}

template <bool white>
Board make_move(Board board, Move move){
	return make_move_with_new_eval<white>(board, move, adjust_eval<white>(board.EvalInfo, compute_eval_diff_for_move<white>(get_side<not white>(board), move)));
}

template Board make_move<true>(Board, Move);
template Board make_move<false>(Board, Move);

bool is_irreversible(const Board board, const Move move){
	return move_flags(move) >= 8 or (ToMask(move_destination(move)) & board.Occ);
}


# include "doctest.h"


template <bool white>
inline void check_consistent_hb(HalfBoard h){
	CHECK(h.All == (h.Pawn | h.Knight | h.Bishop | h.Rook | h.Queen | h.King));
}

inline void check_consistent_fb(Board b){
	check_consistent_hb<true>(b.White);
	check_consistent_hb<false>(b.Black);
	CHECK(b.Occ == (b.White.All | b.Black.All));
	auto recomputed = recompute_from_sides(b.White, b.Black);
	CHECK(b.EvalInfo.mg == recomputed.mg);
	CHECK(b.EvalInfo.eg == recomputed.eg);
	CHECK(b.EvalInfo.phase_count == recomputed.phase_count);
}

TEST_CASE("White pawn promotion"){
	Board b = from_sides_without_eval(from_masks(ToMask(C7), ToMask(C1), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(C2), EMPTY_BOARD),
			from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B8), EMPTY_BOARD, ToMask(A1), EMPTY_BOARD));
	//Fun fact: the only winning move is to capture the rook and promote to bishop!
	
	Board push_n = make_move<true>(b, move_from_squares(C7, C8, PROMOTE_TO_KNIGHT));
	CHECK(push_n.White.Pawn == EMPTY_BOARD);
	CHECK(push_n.White.Knight == (ToMask(C1) | ToMask(C8)));
	CHECK(push_n.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_b = make_move<true>(b, move_from_squares(C7, C8, PROMOTE_TO_BISHOP));
	CHECK(push_b.White.Pawn == EMPTY_BOARD);
	CHECK(push_b.White.Bishop == ToMask(C8));
	CHECK(push_b.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_r = make_move<true>(b, move_from_squares(C7, C8, PROMOTE_TO_ROOK));
	CHECK(push_r.White.Pawn == EMPTY_BOARD);
	CHECK(push_r.White.Rook == ToMask(C8));
	CHECK(push_r.Black == b.Black);
	check_consistent_fb(push_n);
	
	Board push_q = make_move<true>(b, move_from_squares(C7, C8, PROMOTE_TO_QUEEN));
	CHECK(push_q.White.Pawn == EMPTY_BOARD);
	CHECK(push_q.White.Queen == ToMask(C8));
	CHECK(push_q.Black == b.Black);
	check_consistent_fb(push_q);
	
	Board take_n = make_move<true>(b, move_from_squares(C7, B8, PROMOTE_TO_KNIGHT));
	CHECK(take_n.White.Pawn == EMPTY_BOARD);
	CHECK(take_n.White.Knight == (ToMask(C1) | ToMask(B8)));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_b = make_move<true>(b, move_from_squares(C7, B8, PROMOTE_TO_BISHOP));
	CHECK(take_b.White.Pawn == EMPTY_BOARD);
	CHECK(take_b.White.Bishop == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_r = make_move<true>(b, move_from_squares(C7, B8, PROMOTE_TO_ROOK));
	CHECK(take_r.White.Pawn == EMPTY_BOARD);
	CHECK(take_r.White.Rook == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_q = make_move<true>(b, move_from_squares(C7, B8, PROMOTE_TO_QUEEN));
	CHECK(take_q.White.Pawn == EMPTY_BOARD);
	CHECK(take_q.White.Queen == ToMask(B8));
	CHECK(take_n.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(take_q);
}

TEST_CASE("Black pawn promotion"){
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(G1), EMPTY_BOARD, ToMask(H8), EMPTY_BOARD),
			from_masks(ToMask(F2), ToMask(F8), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(F7), EMPTY_BOARD));
	//Fun fact: the only winning move is to capture the rook and promote to bishop!
	
	Board push_n = make_move<false>(b, move_from_squares(F2, F1, PROMOTE_TO_KNIGHT));
	CHECK(push_n.Black.Pawn == EMPTY_BOARD);
	CHECK(push_n.Black.Knight == (ToMask(F1) | ToMask(F8)));
	CHECK(push_n.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_b = make_move<false>(b, move_from_squares(F2, F1, PROMOTE_TO_BISHOP));
	CHECK(push_b.Black.Pawn == EMPTY_BOARD);
	CHECK(push_b.Black.Bishop == ToMask(F1));
	CHECK(push_b.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_r = make_move<false>(b, move_from_squares(F2, F1, PROMOTE_TO_ROOK));
	CHECK(push_r.Black.Pawn == EMPTY_BOARD);
	CHECK(push_r.Black.Rook == ToMask(F1));
	CHECK(push_r.White == b.White);
	check_consistent_fb(push_n);
	
	Board push_q = make_move<false>(b, move_from_squares(F2, F1, PROMOTE_TO_QUEEN));
	CHECK(push_q.Black.Pawn == EMPTY_BOARD);
	CHECK(push_q.Black.Queen == ToMask(F1));
	CHECK(push_q.White == b.White);
	check_consistent_fb(push_q);
	
	Board take_n = make_move<false>(b, move_from_squares(F2, G1, PROMOTE_TO_KNIGHT));
	CHECK(take_n.Black.Pawn == EMPTY_BOARD);
	CHECK(take_n.Black.Knight == (ToMask(G1) | ToMask(F8)));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_b = make_move<false>(b, move_from_squares(F2, G1, PROMOTE_TO_BISHOP));
	CHECK(take_b.Black.Pawn == EMPTY_BOARD);
	CHECK(take_b.Black.Bishop == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_r = make_move<false>(b, move_from_squares(F2, G1, PROMOTE_TO_ROOK));
	CHECK(take_r.Black.Pawn == EMPTY_BOARD);
	CHECK(take_r.Black.Rook == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_n);
	
	Board take_q = make_move<false>(b, move_from_squares(F2, G1, PROMOTE_TO_QUEEN));
	CHECK(take_q.Black.Pawn == EMPTY_BOARD);
	CHECK(take_q.Black.Queen == ToMask(G1));
	CHECK(take_n.White.Rook == EMPTY_BOARD);
	check_consistent_fb(take_q);
}

TEST_CASE("Quiet moves"){
	Board b = from_sides_without_eval(from_masks(ToMask(H2), ToMask(F3), ToMask(E2), ToMask(A1) | ToMask(H1), ToMask(D2), ToMask(E1), ToMask(A1) | ToMask(H1)),
			from_masks(ToMask(H7), ToMask(F6), ToMask(E7), ToMask(A8) | ToMask(H8), ToMask(D7), ToMask(E8), ToMask(A8) | ToMask(H8)));
	
	Board w_n = make_move<true>(b, move_from_squares(F3, D4, KNIGHT_MOVE));
	CHECK(w_n.White.Knight == ToMask(D4));
	CHECK(w_n.Black == b.Black);
	check_consistent_fb(w_n);
	
	Board w_b = make_move<true>(b, move_from_squares(E2, B5, BISHOP_MOVE));
	CHECK(w_b.White.Bishop == ToMask(B5));
	CHECK(w_b.Black == b.Black);
	check_consistent_fb(w_b);
	
	Board w_r = make_move<true>(b, move_from_squares(H1, G1, ROOK_MOVE));
	CHECK(w_r.White.Rook == (ToMask(A1) | ToMask(G1)));
	CHECK(w_r.White.Castle == ToMask(A1));
	CHECK(w_r.Black == b.Black);
	check_consistent_fb(w_r);
	
	Board w_q = make_move<true>(b, move_from_squares(D2, B2, QUEEN_MOVE));
	CHECK(w_q.White.Queen == ToMask(B2));
	CHECK(w_q.Black == b.Black);
	check_consistent_fb(w_q);
	
	Board w_k = make_move<true>(b, move_from_squares(E1, F2, KING_MOVE));
	CHECK(w_k.White.King == ToMask(F2));
	CHECK(w_k.White.Castle == EMPTY_BOARD);
	CHECK(w_k.Black == b.Black);
	check_consistent_fb(w_k);
	
	Board w_cq = make_move<true>(b, move_from_squares(E1, C1, CASTLE_QUEENSIDE));
	CHECK(w_cq.White.King == ToMask(C1));
	CHECK(w_cq.White.Rook == (ToMask(D1) | ToMask(H1)));
	CHECK(w_cq.White.Castle == EMPTY_BOARD);
	CHECK(w_cq.Black == b.Black);
	check_consistent_fb(w_cq);
	
	Board w_ck = make_move<true>(b, move_from_squares(E1, G1, CASTLE_KINGSIDE));
	CHECK(w_ck.White.King == ToMask(G1));
	CHECK(w_ck.White.Rook == (ToMask(A1) | ToMask(F1)));
	CHECK(w_ck.White.Castle == EMPTY_BOARD);
	CHECK(w_ck.Black == b.Black);
	check_consistent_fb(w_ck);
	
	Board w_p1 = make_move<true>(b, move_from_squares(H2, H3, SINGLE_PAWN_PUSH));
	CHECK(w_p1.White.Pawn == ToMask(H3));
	CHECK(w_p1.EPMask == EMPTY_BOARD);
	CHECK(w_p1.Black == b.Black);
	check_consistent_fb(w_p1);
	
	Board w_p2 = make_move<true>(b, move_from_squares(H2, H4, DOUBLE_PAWN_PUSH));
	CHECK(w_p2.White.Pawn == ToMask(H4));
	CHECK(w_p2.EPMask == ToMask(H4));
	CHECK(w_p2.Black == b.Black);
	check_consistent_fb(w_p2);

	
	Board b_n = make_move<false>(b, move_from_squares(F6, D5, KNIGHT_MOVE));
	CHECK(b_n.Black.Knight == ToMask(D5));
	CHECK(b_n.White == b.White);
	check_consistent_fb(b_n);
	
	Board b_b = make_move<false>(b, move_from_squares(E7, B4, BISHOP_MOVE));
	CHECK(b_b.Black.Bishop == ToMask(B4));
	CHECK(b_b.White == b.White);
	check_consistent_fb(b_b);
	
	Board b_r = make_move<false>(b, move_from_squares(H8, G8, ROOK_MOVE));
	CHECK(b_r.Black.Rook == (ToMask(A8) | ToMask(G8)));
	CHECK(b_r.Black.Castle == ToMask(A8));
	CHECK(b_r.White == b.White);
	check_consistent_fb(b_r);
	
	Board b_q = make_move<false>(b, move_from_squares(D7, B7, QUEEN_MOVE));
	CHECK(b_q.Black.Queen == ToMask(B7));
	CHECK(b_q.White == b.White);
	check_consistent_fb(b_q);
	
	Board b_k = make_move<false>(b, move_from_squares(E8, F7, KING_MOVE));
	CHECK(b_k.Black.King == ToMask(F7));
	CHECK(b_k.Black.Castle == EMPTY_BOARD);
	CHECK(b_k.White == b.White);
	check_consistent_fb(b_k);
	
	Board b_cq = make_move<false>(b, move_from_squares(E8, C8, CASTLE_QUEENSIDE));
	CHECK(b_cq.Black.King == ToMask(C8));
	CHECK(b_cq.Black.Rook == (ToMask(D8) | ToMask(H8)));
	CHECK(b_cq.Black.Castle == EMPTY_BOARD);
	CHECK(b_cq.White == b.White);
	check_consistent_fb(b_cq);
	
	Board b_ck = make_move<false>(b, move_from_squares(E8, G8, CASTLE_KINGSIDE));
	CHECK(b_ck.Black.King == ToMask(G8));
	CHECK(b_ck.Black.Rook == (ToMask(A8) | ToMask(F8)));
	CHECK(b_ck.Black.Castle == EMPTY_BOARD);
	CHECK(b_ck.White == b.White);
	check_consistent_fb(b_ck);
	
	Board b_p1 = make_move<false>(b, move_from_squares(H7, H6, SINGLE_PAWN_PUSH));
	CHECK(b_p1.Black.Pawn == ToMask(H6));
	CHECK(b_p1.EPMask == EMPTY_BOARD);
	CHECK(b_p1.White == b.White);
	check_consistent_fb(b_p1);
	
	Board b_p2 = make_move<false>(b, move_from_squares(H7, H5, DOUBLE_PAWN_PUSH));
	CHECK(b_p2.Black.Pawn == ToMask(H5));
	CHECK(b_p2.EPMask == ToMask(H5));
	CHECK(b_p2.White == b.White);
	check_consistent_fb(b_p2);
}

TEST_CASE("White captures"){
	Board b = from_sides_without_eval_ep(from_masks(ToMask(F5), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B4), EMPTY_BOARD),
				from_masks(ToMask(E5) | ToMask(B3) | ToMask(A3), ToMask(A5), ToMask(A4), ToMask(C3), ToMask(G6), ToMask(E8), EMPTY_BOARD),
				D5);
	
	Board x_p = make_move<true>(b, move_from_squares(B4, A3, KING_MOVE));
	CHECK(x_p.Black.Pawn == (ToMask(E5) | ToMask(B3)));
	check_consistent_fb(x_p);
	
	Board x_n = make_move<true>(b, move_from_squares(B4, A5, KING_MOVE));
	CHECK(x_n.Black.Knight == EMPTY_BOARD);
	check_consistent_fb(x_n);
	
	Board x_b = make_move<true>(b, move_from_squares(B4, A4, KING_MOVE));
	CHECK(x_b.Black.Bishop == EMPTY_BOARD);
	check_consistent_fb(x_b);
	
	Board x_r = make_move<true>(b, move_from_squares(B4, C3, KING_MOVE));
	CHECK(x_r.Black.Rook == EMPTY_BOARD);
	check_consistent_fb(x_r);
	
	Board x_q = make_move<true>(b, move_from_squares(F5, G6, PAWN_CAPTURE));
	CHECK(x_q.White.Pawn == ToMask(G6));
	CHECK(x_q.Black.Queen == EMPTY_BOARD);
	check_consistent_fb(x_q);
	
	Board x_ep = make_move<true>(b, move_from_squares(F5, E6, EN_PASSANT_CAPTURE));
	CHECK(x_ep.White.Pawn == ToMask(E6));
	CHECK(x_ep.Black.Pawn == (ToMask(B3) | ToMask(A3)));
	check_consistent_fb(x_ep);
}

TEST_CASE("Black captures"){
	Board b = from_sides_without_eval_ep(from_masks(ToMask(E4) | ToMask(B6) | ToMask(A6), ToMask(A4), ToMask(A5), ToMask(C6), ToMask(G3), ToMask(E1), EMPTY_BOARD),
				from_masks(ToMask(F4), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B5), EMPTY_BOARD),	
				D4);
	
	Board x_p = make_move<false>(b, move_from_squares(B5, A6, KING_MOVE));
	CHECK(x_p.White.Pawn == (ToMask(E4) | ToMask(B6)));
	check_consistent_fb(x_p);
	
	Board x_n = make_move<false>(b, move_from_squares(B5, A4, KING_MOVE));
	CHECK(x_n.White.Knight == EMPTY_BOARD);
	check_consistent_fb(x_n);
	
	Board x_b = make_move<false>(b, move_from_squares(B5, A5, KING_MOVE));
	CHECK(x_b.White.Bishop == EMPTY_BOARD);
	check_consistent_fb(x_b);
	
	Board x_r = make_move<false>(b, move_from_squares(B5, C6, KING_MOVE));
	CHECK(x_r.White.Rook == EMPTY_BOARD);
	check_consistent_fb(x_r);
	
	Board x_q = make_move<false>(b, move_from_squares(F4, G3, PAWN_CAPTURE));
	CHECK(x_q.Black.Pawn == ToMask(G3));
	CHECK(x_q.White.Queen == EMPTY_BOARD);
	check_consistent_fb(x_q);
	
	Board x_ep = make_move<false>(b, move_from_squares(F4, E3, EN_PASSANT_CAPTURE));
	CHECK(x_ep.Black.Pawn == ToMask(E3));
	CHECK(x_ep.White.Pawn == (ToMask(B6) | ToMask(A6)));
	check_consistent_fb(x_ep);
}

TEST_CASE("Capturing rook strips castling rights"){
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A1) | ToMask(H1), EMPTY_BOARD, ToMask(E1), ToMask(A1) | ToMask(H1)),
				from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A8) | ToMask(H8), EMPTY_BOARD, ToMask(E8), ToMask(A8) | ToMask(H8)));
	
	Board w_q = make_move<true>(b, move_from_squares(A1, A8, ROOK_MOVE));
	CHECK(w_q.Black.Castle == ToMask(H8));
	
	Board w_k = make_move<true>(b, move_from_squares(H1, H8, ROOK_MOVE));
	CHECK(w_k.Black.Castle == ToMask(A8));
	
	Board b_q = make_move<false>(b, move_from_squares(A8, A1, ROOK_MOVE));
	CHECK(b_q.White.Castle == ToMask(H1));
	
	Board b_k = make_move<false>(b, move_from_squares(H8, H1, ROOK_MOVE));
	CHECK(b_k.White.Castle == ToMask(A1));
}