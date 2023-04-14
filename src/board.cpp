# include "board.hpp"
# include "hashing.hpp"

# include <exception>

PstEvalInfo recompute_from_sides(const HalfBoard &white, const HalfBoard &black){
	auto w_eval = static_eval_info<true>(white.Pawn, white.Knight, white.Bishop, white.Rook, white.Queen, white.King, white.Castle);
	auto b_eval = static_eval_info<false>(black.Pawn, black.Knight, black.Bishop, black.Rook, black.Queen, black.King, black.Castle);
	return half_to_full_eval_info(w_eval, b_eval);
}

Board from_sides_without_eval(const HalfBoard &white, const HalfBoard &black){
	return Board{white.copy(), black.copy(), white.All | black.All, EMPTY_BOARD, recompute_from_sides(white, black)};
}

Board from_sides_without_eval_ep(const HalfBoard &white, const HalfBoard &black, const Square ep_square){
	return Board{white.copy(), black.copy(), white.All | black.All, ToMask(ep_square), recompute_from_sides(white, black)};
}

const PstEvalInfo NO_DIFFERENCE = PstEvalInfo{0, 0, 0, 0};

template <bool white>
constexpr uint64_t castle_hash_adj(Square square, BitMask castling_rights){
	const BitMask qs_square = white ? A1 : A8;
	if ((square == qs_square) and (castling_rights & ToMask(qs_square))) { return white ? white_cqs_hash : black_cqs_hash; }
	const BitMask ks_square = white ? H1 : H8;
	if ((square == ks_square) and (castling_rights & ToMask(ks_square))) { return white ? white_cks_hash : black_cks_hash; }
	return 0ull;
}

template <bool white>
PstEvalInfo get_eval_contribution_at_square(const HalfBoard &board, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return NO_DIFFERENCE; }
	if (mask & board.Pawn){ return PstEvalInfo{
		mg_pawn_table[FlipIf(white, square)] + MG_PAWN, eg_pawn_table[FlipIf(white, square)] + EG_PAWN, 0, (white ? white_pawn_hash : black_pawn_hash)[square]
	};}
	if (mask & board.Knight){ return PstEvalInfo{
		mg_knight_table[FlipIf(white, square)] + MG_KNIGHT, eg_knight_table[FlipIf(white, square)] + EG_KNIGHT, 1, (white ? white_knight_hash : black_knight_hash)[square]
	};}
	if (mask & board.Bishop){ return PstEvalInfo{
		mg_bishop_table[FlipIf(white, square)] + MG_BISHOP, eg_bishop_table[FlipIf(white, square)] + EG_BISHOP, 1, (white ? white_bishop_hash : black_bishop_hash)[square]
	};}
	if (mask & board.Rook){ return PstEvalInfo{
		mg_rook_table[FlipIf(white, square)] + MG_ROOK, eg_rook_table[FlipIf(white, square)] + EG_ROOK, 2,
		(white ? white_rook_hash : black_rook_hash)[square] ^ castle_hash_adj<white>(square, board.Castle)
	};}
	if (mask & board.Queen){ return PstEvalInfo{
		mg_queen_table[FlipIf(white, square)] + MG_QUEEN, eg_queen_table[FlipIf(white, square)] + EG_QUEEN, 4, (white ? white_queen_hash : black_queen_hash)[square]
	};}
	
	throw std::logic_error("Something went wrong - trying to remove king? 'All' out of sync with other masks?");
}

template <bool white>
PstEvalInfo get_eval_contribution_of_pawn(const Square square){
	return PstEvalInfo{ mg_pawn_table[FlipIf(white, square)] + MG_PAWN, eg_pawn_table[FlipIf(white, square)] + EG_PAWN, 0, (white ? white_pawn_hash : black_pawn_hash)[square] };
}


void maybe_remove_piece(HalfBoard &board, const Square square){
	BitMask mask = ~ToMask(square);
	board.Pawn &= mask;
	board.Knight &= mask;
	board.Bishop &= mask;
	board.Rook &= mask;
	board.Queen &= mask;
	// Can't capture the king!
	board.All &= mask;
	board.Castle &= mask;
}

void remove_pawn(HalfBoard &board, const Square square){
	board.Pawn ^= ToMask(square);
	board.All ^= ToMask(square);
}


constexpr uint64_t hash_diff(std::array<uint64_t, 64> table, Square from, Square to){
	return table[from] ^ table[to];
}


template <bool white>
PstEvalInfo compute_eval_diff_for_move(const Board &board, const Move move){
	const HalfBoard &enemy = get_side<not white>(board);
	const BitMask castle_rights = get_side<white>(board).Castle;
	const uint64_t all_castle_hash = castle_hash_adj<white>(white ? A1 : A8, castle_rights) ^ castle_hash_adj<white>(white ? H1 : H8, castle_rights);
	const Square from = move_source(move);
	const Square to = move_destination(move);
	const MoveFlags flags = move_flags(move);
	const PstEvalInfo removal_info = (flags == EN_PASSANT_CAPTURE) ? get_eval_contribution_of_pawn<not white>(to + (white ? -8 : 8))
			: get_eval_contribution_at_square<not white>(enemy, to);

	switch (flags){
		case NULL_MOVE:
			return NO_DIFFERENCE;

		case KNIGHT_MOVE:
			return PstEvalInfo{
				mg_knight_table[FlipIf(white, to)] - mg_knight_table[FlipIf(white, from)] + removal_info.mg, 
				eg_knight_table[FlipIf(white, to)] - eg_knight_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_knight_hash : black_knight_hash, from, to) ^ removal_info.hash
			};
		case BISHOP_MOVE:
			return PstEvalInfo{
				mg_bishop_table[FlipIf(white, to)] - mg_bishop_table[FlipIf(white, from)] + removal_info.mg, 
				eg_bishop_table[FlipIf(white, to)] - eg_bishop_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_bishop_hash : black_bishop_hash, from, to) ^ removal_info.hash
			};
		case ROOK_MOVE:
			return PstEvalInfo{
				mg_rook_table[FlipIf(white, to)] - mg_rook_table[FlipIf(white, from)] + removal_info.mg, 
				eg_rook_table[FlipIf(white, to)] - eg_rook_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_rook_hash : black_rook_hash, from, to) ^ removal_info.hash ^ castle_hash_adj<white>(from, castle_rights)
			};
		case QUEEN_MOVE:
			return PstEvalInfo{
				mg_queen_table[FlipIf(white, to)] - mg_queen_table[FlipIf(white, from)] + removal_info.mg, 
				eg_queen_table[FlipIf(white, to)] - eg_queen_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_queen_hash : black_queen_hash, from, to) ^ removal_info.hash
			};
		case KING_MOVE:
			return PstEvalInfo{
				mg_king_table[FlipIf(white, to)] - mg_king_table[FlipIf(white, from)] + removal_info.mg, 
				eg_king_table[FlipIf(white, to)] - eg_king_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_king_hash : black_king_hash, from, to) ^ removal_info.hash ^ all_castle_hash
			};

		case CASTLE_QUEENSIDE:
			return PstEvalInfo{
				mg_king_table[C8] + mg_rook_table[D8] - mg_king_table[E8] - mg_rook_table[A8],
				eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8],
				0,
				all_castle_hash ^ (white ? (hash_diff(white_king_hash, E1, C1) ^ hash_diff(white_rook_hash, A1, D1)) :
						(hash_diff(black_king_hash, E8, C8) ^ hash_diff(black_rook_hash, A8, D8)))
			};
		case CASTLE_KINGSIDE:
			return PstEvalInfo{
				mg_king_table[G8] + mg_rook_table[F8] - mg_king_table[E8] - mg_rook_table[H8],
				eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8],
				0,
				all_castle_hash ^ (white ? (hash_diff(white_king_hash, E1, G1) ^ hash_diff(white_rook_hash, H1, F1)) :
										(hash_diff(black_king_hash, E8, G8) ^ hash_diff(black_rook_hash, H8, F8)))
			};

		case SINGLE_PAWN_PUSH:
		case DOUBLE_PAWN_PUSH:
			return PstEvalInfo{
				mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
				eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
				0,
				hash_diff(white ? white_pawn_hash : black_pawn_hash, from, to)
			};
		case PAWN_CAPTURE:
		case EN_PASSANT_CAPTURE:
			return PstEvalInfo{
				mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)] + removal_info.mg, 
				eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)] + removal_info.eg,
				-removal_info.phase_count,
				hash_diff(white ? white_pawn_hash : black_pawn_hash, from, to) ^ removal_info.hash
			};

		case PROMOTE_TO_KNIGHT:
			return PstEvalInfo{
				mg_knight_table[FlipIf(white, to)] + MG_KNIGHT - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_knight_table[FlipIf(white, to)] + EG_KNIGHT - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				1 - removal_info.phase_count,
				(white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_knight_hash : black_knight_hash)[to] ^ removal_info.hash
			};
		case PROMOTE_TO_BISHOP:
			return PstEvalInfo{
				mg_bishop_table[FlipIf(white, to)] + MG_BISHOP - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_bishop_table[FlipIf(white, to)] + EG_BISHOP - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				1 - removal_info.phase_count,
				(white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_bishop_hash : black_bishop_hash)[to] ^ removal_info.hash
			};
		case PROMOTE_TO_ROOK:
			return PstEvalInfo{
				mg_rook_table[FlipIf(white, to)] + MG_ROOK - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_rook_table[FlipIf(white, to)] + EG_ROOK - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				2 - removal_info.phase_count,
				(white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_rook_hash : black_rook_hash)[to] ^ removal_info.hash
			};
		case PROMOTE_TO_QUEEN:
			return PstEvalInfo{
				mg_queen_table[FlipIf(white, to)] + MG_QUEEN - mg_pawn_table[FlipIf(white, from)] - MG_PAWN + removal_info.mg, 
				eg_queen_table[FlipIf(white, to)] + EG_QUEEN - eg_pawn_table[FlipIf(white, from)] - EG_PAWN + removal_info.eg,
				4 - removal_info.phase_count,
				(white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_queen_hash : black_queen_hash)[to] ^ removal_info.hash
			};
		}
		throw std::logic_error("Unexpected move flag");
}

template <bool white>
void make_move_with_new_eval(Board &board, const Move move, const PstEvalInfo new_eval){
	HalfBoard &f = get_side<white>(board); // f for friendly
	HalfBoard &e = get_side<not white>(board); // e for enemy

	const Square from = move_source(move);
	const Square to = move_destination(move);
	const BitMask move_mask = ToMask(from) | ToMask(to);
	board.EPMask = EMPTY_BOARD;
	board.EvalInfo = new_eval;
	
	switch (move_flags(move)){

	case KNIGHT_MOVE:
		f.Knight ^= move_mask;
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case BISHOP_MOVE:
		f.Bishop ^= move_mask;
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case ROOK_MOVE:
		f.Rook ^= move_mask;
		f.All ^= move_mask;
		f.Castle &= ~ToMask(from);
		maybe_remove_piece(e, to);
		break;
	case QUEEN_MOVE:
		f.Queen ^= move_mask;
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case KING_MOVE:
		f.King = ToMask(to);
		f.All ^= move_mask;
		f.Castle = EMPTY_BOARD;
		maybe_remove_piece(e, to);
		break;

	case CASTLE_QUEENSIDE:
		f.King = white ? ToMask(C1) : ToMask(C8);
		f.Rook ^= white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8));
		f.All ^= (white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8))) ^ (white ? (ToMask(E1) | ToMask(C1)) : (ToMask(E8) | ToMask(C8)));
		f.Castle = EMPTY_BOARD;
		break;
	case CASTLE_KINGSIDE:
		f.King = white ? ToMask(G1) : ToMask(G8);
		f.Rook ^= white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8));
		f.All ^= (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))) ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8)));
		f.Castle = EMPTY_BOARD;
		break;

	case SINGLE_PAWN_PUSH:
		f.Pawn ^= move_mask;
		f.All ^= move_mask;
		break;
	case DOUBLE_PAWN_PUSH:
		f.Pawn ^= move_mask;
		f.All ^= move_mask;
		board.EPMask = ToMask(to);
		break;
	case PAWN_CAPTURE:
		f.Pawn ^= move_mask;
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case EN_PASSANT_CAPTURE:
		f.Pawn ^= move_mask;
		f.All ^= move_mask;
		remove_pawn(e, to + (white ? -8 : 8));
		break;

	case PROMOTE_TO_KNIGHT:
		f.Pawn ^= ToMask(from);
		f.Knight ^= ToMask(to);
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case PROMOTE_TO_BISHOP:
		f.Pawn ^= ToMask(from);
		f.Bishop ^= ToMask(to);
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case PROMOTE_TO_ROOK:
		f.Pawn ^= ToMask(from);
		f.Rook ^= ToMask(to);
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	case PROMOTE_TO_QUEEN:
		f.Pawn ^= ToMask(from);
		f.Queen ^= ToMask(to);
		f.All ^= move_mask;
		maybe_remove_piece(e, to);
		break;
	default:
		throw std::logic_error("Unexpected move flag");
	}

	board.Occ = f.All | e.All;
}

template <bool white>
void make_move(Board &board, const Move move){
	make_move_with_new_eval<white>(board, move, adjust_eval<white>(board.EvalInfo, compute_eval_diff_for_move<white>(board, move)));
}

template void make_move<true>(Board&, const Move);
template void make_move<false>(Board&, const Move);

bool is_irreversible(const Board &board, const Move move){
	return move_flags(move) >= 8 or (ToMask(move_destination(move)) & board.Occ);
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"


template <bool white>
inline void check_consistent_hb(HalfBoard &h){
	CHECK(h.All == (h.Pawn | h.Knight | h.Bishop | h.Rook | h.Queen | h.King));
}

inline void check_consistent_fb(Board &b){
	check_consistent_hb<true>(b.White);
	check_consistent_hb<false>(b.Black);
	CHECK(b.Occ == (b.White.All | b.Black.All));
	PstEvalInfo recomputed = recompute_from_sides(b.White, b.Black);
	CHECK(b.EvalInfo.mg == recomputed.mg);
	CHECK(b.EvalInfo.eg == recomputed.eg);
	CHECK(b.EvalInfo.phase_count == recomputed.phase_count);
	CHECK(b.EvalInfo.hash == recomputed.hash);
}

constexpr bool operator==(const HalfBoard &x, const HalfBoard &y){
	return x.Pawn == y.Pawn and x.Knight == y.Knight and x.Bishop == y.Bishop and x.Rook == y.Rook
			and x.Queen == y.Queen and x.King == y.King;
}

TEST_CASE("White pawn promotion"){
	Board b = from_sides_without_eval(from_masks(ToMask(C7), ToMask(C1), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(C2), EMPTY_BOARD),
			from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B8), EMPTY_BOARD, ToMask(A1), EMPTY_BOARD));
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
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(G1), EMPTY_BOARD, ToMask(H8), EMPTY_BOARD),
			from_masks(ToMask(F2), ToMask(F8), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(F7), EMPTY_BOARD));
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
	Board b = from_sides_without_eval(from_masks(ToMask(H2), ToMask(F3), ToMask(E2), ToMask(A1) | ToMask(H1), ToMask(D2), ToMask(E1), ToMask(A1) | ToMask(H1)),
			from_masks(ToMask(H7), ToMask(F6), ToMask(E7), ToMask(A8) | ToMask(H8), ToMask(D7), ToMask(E8), ToMask(A8) | ToMask(H8)));
	
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
		CHECK(w_k.White.King == ToMask(F2));
		CHECK(w_k.White.Castle == EMPTY_BOARD);
		CHECK(w_k.Black == b.Black);
		check_consistent_fb(w_k);
	
		Board w_cq = b.copy();
		make_move<true>(w_cq, move_from_squares(E1, C1, CASTLE_QUEENSIDE));
		CHECK(w_cq.White.King == ToMask(C1));
		CHECK(w_cq.White.Rook == (ToMask(D1) | ToMask(H1)));
		CHECK(w_cq.White.Castle == EMPTY_BOARD);
		CHECK(w_cq.Black == b.Black);
		check_consistent_fb(w_cq);

		Board w_ck = b.copy();
		make_move<true>(w_ck, move_from_squares(E1, G1, CASTLE_KINGSIDE));
		CHECK(w_ck.White.King == ToMask(G1));
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
		CHECK(b_k.Black.King == ToMask(F7));
		CHECK(b_k.Black.Castle == EMPTY_BOARD);
		CHECK(b_k.White == b.White);
		check_consistent_fb(b_k);

		Board b_cq = b.copy();
		make_move<false>(b_cq, move_from_squares(E8, C8, CASTLE_QUEENSIDE));
		CHECK(b_cq.Black.King == ToMask(C8));
		CHECK(b_cq.Black.Rook == (ToMask(D8) | ToMask(H8)));
		CHECK(b_cq.Black.Castle == EMPTY_BOARD);
		CHECK(b_cq.White == b.White);
		check_consistent_fb(b_cq);

		Board b_ck = b.copy();
		make_move<false>(b_ck, move_from_squares(E8, G8, CASTLE_KINGSIDE));
		CHECK(b_ck.Black.King == ToMask(G8));
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
	Board b = from_sides_without_eval_ep(from_masks(ToMask(F5), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B4), EMPTY_BOARD),
				from_masks(ToMask(E5) | ToMask(B3) | ToMask(A3), ToMask(A5), ToMask(A4), ToMask(C3), ToMask(G6), ToMask(E8), EMPTY_BOARD),
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
	Board b = from_sides_without_eval_ep(from_masks(ToMask(E4) | ToMask(B6) | ToMask(A6), ToMask(A4), ToMask(A5), ToMask(C6), ToMask(G3), ToMask(E1), EMPTY_BOARD),
				from_masks(ToMask(F4), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B5), EMPTY_BOARD),	
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
	Board b = from_sides_without_eval(from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A1) | ToMask(H1), EMPTY_BOARD, ToMask(E1), ToMask(A1) | ToMask(H1)),
				from_masks(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(A8) | ToMask(H8), EMPTY_BOARD, ToMask(E8), ToMask(A8) | ToMask(H8)));
	
	auto expected_w = white_king_hash[E1] ^ white_rook_hash[A1] ^ white_rook_hash[H1] ^ white_cqs_hash ^ white_cks_hash;
	auto expected_b = black_king_hash[E8] ^ black_rook_hash[A8] ^ black_rook_hash[H8] ^ black_cqs_hash ^ black_cks_hash;
	auto actual_w = static_eval_info<true>(b.White.Pawn, b.White.Knight, b.White.Bishop, b.White.Rook, b.White.Queen, b.White.King, b.White.Castle).hash;
	CHECK(expected_w == actual_w);
	auto actual_b = static_eval_info<false>(b.Black.Pawn, b.Black.Knight, b.Black.Bishop, b.Black.Rook, b.Black.Queen, b.Black.King, b.Black.Castle).hash;
	CHECK(expected_b == actual_b);

	CHECK(b.EvalInfo.hash == (expected_w ^ expected_b));

	Board w_q = b.copy();
	make_move<true>(w_q, move_from_squares(A1, A8, ROOK_MOVE));
	CHECK(w_q.Black.Castle == ToMask(H8));
	
	Board w_k = b.copy();
	make_move<true>(w_k, move_from_squares(H1, H8, ROOK_MOVE));
	CHECK(w_k.Black.Castle == ToMask(A8));
	
	Board b_q = b.copy();
	make_move<false>(b_q, move_from_squares(A8, A1, ROOK_MOVE));
	CHECK(b_q.White.Castle == ToMask(H1));
	
	Board b_k = b.copy();
	make_move<false>(b_k, move_from_squares(H8, H1, ROOK_MOVE));
	CHECK(b_k.White.Castle == ToMask(A1));
}

# endif
