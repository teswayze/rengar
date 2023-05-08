# include "board.hpp"
# include "hashing.hpp"
# include "sliders.hpp"

# include <exception>

PstEvalInfo recompute_from_sides(const HalfBoard &white, const HalfBoard &black){
	auto w_eval = static_eval_info<true>(white.Pawn, white.Knight, white.Bishop, white.Rook, white.Queen, white.King, white.Castle);
	auto b_eval = static_eval_info<false>(black.Pawn, black.Knight, black.Bishop, black.Rook, black.Queen, black.King, black.Castle);
	return half_to_full_eval_info(w_eval, b_eval);
}

BitMask rook_attacks(const BitMask rooks, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(rooks, loop_var){
		attacks |= rook_seen(SquareOf(loop_var), occ);
	}
	return attacks;
}

BitMask bishop_attacks(const BitMask bishops, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(bishops, loop_var){
		attacks |= bishop_seen(SquareOf(loop_var), occ);
	}
	return attacks;
}

BitMask queen_attacks(const BitMask queens, const BitMask occ){
	return rook_attacks(queens, occ) | bishop_attacks(queens, occ);
}

Board from_sides_without_eval(const HalfBoard &white, const HalfBoard &black){
	const BitMask occ = white.All | black.All;
	const BitMask no_bk = occ &~black.King;
	const BitMask no_wk = occ &~white.King;
	return Board{white.copy(), black.copy(), white.All | black.All, EMPTY_BOARD, recompute_from_sides(white, black),
		bishop_attacks(white.Bishop, no_bk), bishop_attacks(black.Bishop, no_wk), rook_attacks(white.Rook, no_bk), rook_attacks(black.Rook, no_wk),
		queen_attacks(white.Queen, no_bk), queen_attacks(black.Queen, no_wk)};
}

Board from_sides_without_eval_ep(const HalfBoard &white, const HalfBoard &black, const Square ep_square){
	const BitMask occ = white.All | black.All;
	const BitMask no_bk = occ &~black.King;
	const BitMask no_wk = occ &~white.King;
	return Board{white.copy(), black.copy(), white.All | black.All, ToMask(ep_square), recompute_from_sides(white, black),
		bishop_attacks(white.Bishop, no_bk), bishop_attacks(black.Bishop, no_wk), rook_attacks(white.Rook, no_bk), rook_attacks(black.Rook, no_wk),
		bishop_attacks(white.Queen, no_bk) | rook_attacks(white.Queen, no_bk), bishop_attacks(black.Queen, no_wk) | rook_attacks(black.Queen, no_wk)};
}

template <bool white>
void void_castling_rights_at_square(BitMask &castling_rights, uint64_t &board_hash, const Square square){
	const Square qs_square = white ? A1 : A8;
	if ((square == qs_square) and (castling_rights & ToMask(qs_square))) {
		castling_rights ^= ToMask(qs_square);
		board_hash ^= white ? white_cqs_hash : black_cqs_hash;
	}
	const Square ks_square = white ? H1 : H8;
	if ((square == ks_square) and (castling_rights & ToMask(ks_square))) {
		castling_rights ^= ToMask(ks_square);
		board_hash ^= white ? white_cks_hash : black_cks_hash;
	}
}

template <bool white>
void void_all_castling_rights(BitMask &castling_rights, uint64_t &board_hash){
	const Square qs_square = white ? A1 : A8;
	if (castling_rights & ToMask(qs_square)) {
		board_hash ^= white ? white_cqs_hash : black_cqs_hash;
	}
	const Square ks_square = white ? H1 : H8;
	if (castling_rights & ToMask(ks_square)) {
		board_hash ^= white ? white_cks_hash : black_cks_hash;
	}
	castling_rights = EMPTY_BOARD;
}

template <bool white>
int maybe_remove_piece(HalfBoard &board, PstEvalInfo &info, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return 0; }
	board.All ^= mask;
	const int sign = white ? -1 : 1;
	if (mask & board.Pawn){
		board.Pawn ^= mask;
		info.mg += sign * (mg_pawn_table[FlipIf(white, square)] + mg_pawn);
		info.eg += sign * (eg_pawn_table[FlipIf(white, square)] + eg_pawn);
		info.hash ^= (white ? white_pawn_hash : black_pawn_hash)[square];
		return 1;
	}
	if (mask & board.Knight){
		board.Knight ^= mask;
		info.mg += sign * (mg_knight_table[FlipIf(white, square)] + mg_knight);
		info.eg += sign * (eg_knight_table[FlipIf(white, square)] + eg_knight);
		info.phase_count -= pc_knight;
		info.hash ^= (white ? white_knight_hash : black_knight_hash)[square];
		return 2;
	}
	if (mask & board.Bishop){
		board.Bishop ^= mask;
		info.mg += sign * (mg_bishop_table[FlipIf(white, square)] + mg_bishop);
		info.eg += sign * (eg_bishop_table[FlipIf(white, square)] + eg_bishop);
		info.phase_count -= pc_bishop;
		info.hash ^= (white ? white_bishop_hash : black_bishop_hash)[square];
		return 3;
	}
	if (mask & board.Rook){
		board.Rook ^= mask;
		board.Castle &= ~mask;
		info.mg += sign * (mg_rook_table[FlipIf(white, square)] + mg_rook);
		info.eg += sign * (eg_rook_table[FlipIf(white, square)] + eg_rook);
		info.phase_count -= pc_rook;
		info.hash ^= (white ? white_rook_hash : black_rook_hash)[square];
		void_castling_rights_at_square<white>(board.Castle, info.hash, square);
		return 4;
	}
	if (mask & board.Queen){
		board.Queen ^= mask;
		info.mg += sign * (mg_queen_table[FlipIf(white, square)] + mg_queen);
		info.eg += sign * (eg_queen_table[FlipIf(white, square)] + eg_queen);
		info.phase_count -= pc_queen;
		info.hash ^= (white ? white_queen_hash : black_queen_hash)[square];
		return 5;
	}
	
	throw std::logic_error("Something went wrong - trying to remove king? 'All' out of sync with other masks?");
}

template <bool white>
void remove_pawn(HalfBoard &board, PstEvalInfo &info, const Square square){
	const int sign = white ? -1 : 1;
	board.Pawn ^= ToMask(square);
	board.All ^= ToMask(square);
	info.mg += sign * (mg_pawn_table[FlipIf(white, square)] + mg_pawn);
	info.eg += sign * (eg_pawn_table[FlipIf(white, square)] + eg_pawn);
	info.hash ^= (white ? white_pawn_hash : black_pawn_hash)[square];
}

constexpr uint64_t hash_diff(std::array<uint64_t, 64> table, Square from, Square to){
	return table[from] ^ table[to];
}

template <bool white>
void pawn_move_common(HalfBoard &side, PstEvalInfo &info, const Square from, const Square to){
	const int sign = white ? 1 : -1;
	side.Pawn ^= ToMask(from) | ToMask(to);
	side.All ^= ToMask(from) | ToMask(to);
	info.mg += sign * (mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)]);
	info.eg += sign * (eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)]);
	info.hash ^= hash_diff(white ? white_pawn_hash : black_pawn_hash, from, to);
}


void recalculate_attack_masks_where_affected(Board &board, const BitMask affected){
	if (affected & board.WtBishopAtk) board.WtBishopAtk = bishop_attacks(board.White.Bishop, board.Occ & ~board.Black.King);
	if (affected & board.BkBishopAtk) board.BkBishopAtk = bishop_attacks(board.Black.Bishop, board.Occ & ~board.White.King);
	if (affected & board.WtRookAtk) board.WtRookAtk = rook_attacks(board.White.Rook, board.Occ & ~board.Black.King);
	if (affected & board.BkRookAtk) board.BkRookAtk = rook_attacks(board.Black.Rook, board.Occ & ~board.White.King);
	if (affected & board.WtQueenAtk) board.WtQueenAtk = queen_attacks(board.White.Queen, board.Occ & ~board.Black.King);
	if (affected & board.BkQueenAtk) board.BkQueenAtk = queen_attacks(board.Black.Queen, board.Occ & ~board.White.King);
}

template <bool white>
void recalculate_bishop_attack_mask(Board &board){
	if (white) board.WtBishopAtk = bishop_attacks(board.White.Bishop, (board.White.All | board.Black.All) & ~board.Black.King);
	else board.BkBishopAtk = bishop_attacks(board.Black.Bishop, (board.White.All | board.Black.All) & ~board.White.King);
}

template <bool white>
void recalculate_rook_attack_mask(Board &board){
	if (white) board.WtRookAtk = rook_attacks(board.White.Rook, (board.White.All | board.Black.All) & ~board.Black.King);
	else board.BkRookAtk = rook_attacks(board.Black.Rook, (board.White.All | board.Black.All) & ~board.White.King);
}

template <bool white>
void recalculate_queen_attack_mask(Board &board){
	if (white) board.WtQueenAtk = queen_attacks(board.White.Queen, (board.White.All | board.Black.All) & ~board.Black.King);
	else board.BkQueenAtk = queen_attacks(board.Black.Queen, (board.White.All | board.Black.All) & ~board.White.King);
}


template <bool white>
int make_move(Board &board, const Move move){
	HalfBoard &f = get_side<white>(board); // f for friendly
	HalfBoard &e = get_side<not white>(board); // e for enemy

	const Square from = move_source(move);
	const Square to = move_destination(move);
	const BitMask move_mask = ToMask(from) | ToMask(to);
	const int sign = white ? 1 : -1;

	int capture = 0;
	board.EPMask = EMPTY_BOARD;
	
	switch (move_flags(move)){

	case KNIGHT_MOVE:
		f.Knight ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_knight_table[FlipIf(white, to)] - mg_knight_table[FlipIf(white, from)]);
		board.EvalInfo.eg += sign * (eg_knight_table[FlipIf(white, to)] - eg_knight_table[FlipIf(white, from)]);
		board.EvalInfo.hash ^= hash_diff(white ? white_knight_hash : black_knight_hash, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case BISHOP_MOVE:
		f.Bishop ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_bishop_table[FlipIf(white, to)] - mg_bishop_table[FlipIf(white, from)]);
		board.EvalInfo.eg += sign * (eg_bishop_table[FlipIf(white, to)] - eg_bishop_table[FlipIf(white, from)]);
		board.EvalInfo.hash ^= hash_diff(white ? white_bishop_hash : black_bishop_hash, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case ROOK_MOVE:
		f.Rook ^= move_mask;
		f.All ^= move_mask;
		void_castling_rights_at_square<white>(f.Castle, board.EvalInfo.hash, from);
		board.EvalInfo.mg += sign * (mg_rook_table[FlipIf(white, to)] - mg_rook_table[FlipIf(white, from)]);
		board.EvalInfo.eg += sign * (eg_rook_table[FlipIf(white, to)] - eg_rook_table[FlipIf(white, from)]);
		board.EvalInfo.hash ^= hash_diff(white ? white_rook_hash : black_rook_hash, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case QUEEN_MOVE:
		f.Queen ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_queen_table[FlipIf(white, to)] - mg_queen_table[FlipIf(white, from)]);
		board.EvalInfo.eg += sign * (eg_queen_table[FlipIf(white, to)] - eg_queen_table[FlipIf(white, from)]);
		board.EvalInfo.hash ^= hash_diff(white ? white_queen_hash : black_queen_hash, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case KING_MOVE:
		f.King = ToMask(to);
		f.All ^= move_mask;
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
		board.EvalInfo.mg += sign * (mg_king_table[FlipIf(white, to)] - mg_king_table[FlipIf(white, from)]);
		board.EvalInfo.eg += sign * (eg_king_table[FlipIf(white, to)] - eg_king_table[FlipIf(white, from)]);
		board.EvalInfo.hash ^= hash_diff(white ? white_king_hash : black_king_hash, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;

	case CASTLE_QUEENSIDE:
		f.King = white ? ToMask(C1) : ToMask(C8);
		f.Rook ^= white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8));
		f.All ^= (white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8))) ^ (white ? (ToMask(E1) | ToMask(C1)) : (ToMask(E8) | ToMask(C8)));
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
		board.EvalInfo.mg += sign * (mg_king_table[C8] + mg_rook_table[D8] - mg_king_table[E8] - mg_rook_table[A8]);
		board.EvalInfo.eg += sign * (eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8]);
		board.EvalInfo.hash ^= white ? (hash_diff(white_king_hash, E1, C1) ^ hash_diff(white_rook_hash, A1, D1)) :
				(hash_diff(black_king_hash, E8, C8) ^ hash_diff(black_rook_hash, A8, D8));
		break;
	case CASTLE_KINGSIDE:
		f.King = white ? ToMask(G1) : ToMask(G8);
		f.Rook ^= white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8));
		f.All ^= (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))) ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8)));
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
		board.EvalInfo.mg += sign * (mg_king_table[G8] + mg_rook_table[F8] - mg_king_table[E8] - mg_rook_table[H8]);
		board.EvalInfo.eg += sign * (eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8]);
		board.EvalInfo.hash ^= white ? (hash_diff(white_king_hash, E1, G1) ^ hash_diff(white_rook_hash, H1, F1)) :
				(hash_diff(black_king_hash, E8, G8) ^ hash_diff(black_rook_hash, H8, F8));
		break;

	case SINGLE_PAWN_PUSH:
		pawn_move_common<white>(f, board.EvalInfo, from, to);
		break;
	case DOUBLE_PAWN_PUSH:
		pawn_move_common<white>(f, board.EvalInfo, from, to);
		board.EPMask = ToMask(to);
		break;
	case PAWN_CAPTURE:
		pawn_move_common<white>(f, board.EvalInfo, from, to);
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case EN_PASSANT_CAPTURE:
		pawn_move_common<white>(f, board.EvalInfo, from, to);
		remove_pawn<not white>(e, board.EvalInfo, to + (white ? -8 : 8));
		capture = 1;
		break;

	case PROMOTE_TO_KNIGHT:
		f.Pawn ^= ToMask(from);
		f.Knight ^= ToMask(to);
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_knight_table[FlipIf(white, to)] + mg_knight - mg_pawn_table[FlipIf(white, from)] - mg_pawn);
		board.EvalInfo.eg += sign * (eg_knight_table[FlipIf(white, to)] + eg_knight - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_knight;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_knight_hash : black_knight_hash)[to];
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		break;
	case PROMOTE_TO_BISHOP:
		f.Pawn ^= ToMask(from);
		f.Bishop ^= ToMask(to);
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_bishop_table[FlipIf(white, to)] + mg_bishop - mg_pawn_table[FlipIf(white, from)] - mg_pawn);
		board.EvalInfo.eg += sign * (eg_bishop_table[FlipIf(white, to)] + eg_bishop - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_bishop;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_bishop_hash : black_bishop_hash)[to];
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		recalculate_bishop_attack_mask<white>(board);
		break;
	case PROMOTE_TO_ROOK:
		f.Pawn ^= ToMask(from);
		f.Rook ^= ToMask(to);
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_rook_table[FlipIf(white, to)] + mg_rook - mg_pawn_table[FlipIf(white, from)] - mg_pawn);
		board.EvalInfo.eg += sign * (eg_rook_table[FlipIf(white, to)] + eg_rook - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_rook;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_rook_hash : black_rook_hash)[to];
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		recalculate_rook_attack_mask<white>(board);
		break;
	case PROMOTE_TO_QUEEN:
		f.Pawn ^= ToMask(from);
		f.Queen ^= ToMask(to);
		f.All ^= move_mask;
		board.EvalInfo.mg += sign * (mg_queen_table[FlipIf(white, to)] + mg_queen - mg_pawn_table[FlipIf(white, from)] - mg_pawn);
		board.EvalInfo.eg += sign * (eg_queen_table[FlipIf(white, to)] + eg_queen - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_queen;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_queen_hash : black_queen_hash)[to];
		capture = maybe_remove_piece<not white>(e, board.EvalInfo, to);
		recalculate_queen_attack_mask<white>(board);
		break;
	default:
		throw std::logic_error("Unexpected move flag");
	}

	board.Occ = f.All | e.All;

	recalculate_attack_masks_where_affected(board, capture ? ((move_flags(move) == EN_PASSANT_CAPTURE) ? (move_mask ^ to + (white ? -8 : 8)) : ToMask(from)) : move_mask);
	if (capture == 3) recalculate_bishop_attack_mask<not white>(board);
	if (capture == 4) recalculate_rook_attack_mask<not white>(board);
	if (capture == 5) recalculate_queen_attack_mask<not white>(board);

	return capture;
}

template int make_move<true>(Board&, const Move);
template int make_move<false>(Board&, const Move);

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

	Board copy = b.EPMask ? from_sides_without_eval_ep(b.White, b.Black, SquareOf(b.EPMask)) : from_sides_without_eval(b.White, b.Black);
	CHECK(b.Occ == copy.Occ);
	CHECK(b.EvalInfo.mg == copy.EvalInfo.mg);
	CHECK(b.EvalInfo.eg == copy.EvalInfo.eg);
	CHECK(b.EvalInfo.phase_count == copy.EvalInfo.phase_count);
	CHECK(b.EvalInfo.hash == copy.EvalInfo.hash);
	CHECK(b.WtBishopAtk == copy.WtBishopAtk);
	CHECK(b.BkBishopAtk == copy.BkBishopAtk);
	CHECK(b.WtRookAtk == copy.WtRookAtk);
	CHECK(b.BkRookAtk == copy.BkRookAtk);
	CHECK(b.WtQueenAtk == copy.WtQueenAtk);
	CHECK(b.BkQueenAtk == copy.BkQueenAtk);
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
