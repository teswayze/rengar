# include "board.hpp"
# include "hashing.hpp"

# include <stdexcept>

PstEvalInfo recompute_from_sides(const HalfBoard &white, const HalfBoard &black){
	auto w_eval = static_eval_info<true>(white.Pawn, white.Knight, white.Bishop, white.Rook, white.Queen, white.King, white.Castle);
	auto b_eval = static_eval_info<false>(black.Pawn, black.Knight, black.Bishop, black.Rook, black.Queen, black.King, black.Castle);
	return half_to_full_eval_info(w_eval, b_eval);
}

BitMask rook_attacks(const BitMask rooks, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(rooks, loop_var){
		attacks |= rook_seen(TZCNT(loop_var), occ);
	}
	return attacks;
}

BitMask bishop_attacks(const BitMask bishops, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(bishops, loop_var){
		attacks |= bishop_seen(TZCNT(loop_var), occ);
	}
	return attacks;
}

BitMask queen_attacks(const BitMask queens, const BitMask occ){
	return rook_attacks(queens, occ) | bishop_attacks(queens, occ);
}

template <bool white>
Attacks calculate_attacks(const HalfBoard &side, const BitMask occ){
	return Attacks{
		pawn_attacks<white>(side.Pawn), knight_attacks(side.Knight), bishop_attacks(side.Bishop, occ),
				rook_attacks(side.Rook, occ), queen_attacks(side.Queen, occ), king_attacks(side.King)};
}

Board from_sides_without_eval(const HalfBoard &white, const HalfBoard &black){
	const BitMask occ = white.All | black.All;
	return Board{white.copy(), black.copy(), white.All | black.All, EMPTY_BOARD, recompute_from_sides(white, black),
		initialize_ue(white, black),
		calculate_attacks<true>(white, occ ^ ToMask(black.King)), calculate_attacks<false>(black, occ ^ ToMask(white.King))};
}

Board from_sides_without_eval_ep(const HalfBoard &white, const HalfBoard &black, const Square ep_square){
	const BitMask occ = white.All | black.All;
	return Board{white.copy(), black.copy(), white.All | black.All, ToMask(ep_square), recompute_from_sides(white, black),
		initialize_ue(white, black),
		calculate_attacks<true>(white, occ ^ ToMask(black.King)), calculate_attacks<false>(black, occ ^ ToMask(white.King))};
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
int maybe_remove_piece(Board &board, const Square square){
	HalfBoard &side = get_side<white>(board);
	Attacks &attack = white ? board.WtAtk : board.BkAtk;
	const BitMask mask = ToMask(square);
	const int piece_at_square =
			((mask & (side.Rook | side.Queen)) ? 4 : 0) +
			((mask & (side.Knight | side.Bishop)) ? 2 : 0) +
			((mask & (side.Pawn | side.Bishop | side.Queen)) ? 1 : 0);
	switch (piece_at_square){
	case 0:
		return 0;
	case 1:
		side.Pawn ^= mask;
		side.All ^= mask;
		board.ue.remove_piece<white, PAWN>(square);
		board.EvalInfo.remove_pawn<white>(square);
		attack.Pawn = pawn_attacks<white>(side.Pawn);
		return 1;
	case 2:
		side.Knight ^= mask;
		side.All ^= mask;
		board.ue.remove_piece<white, KNIGHT>(square);
		board.EvalInfo.remove_knight<white>(square);
		attack.Knight = knight_attacks(side.Knight);
		return 2;
	case 3:
		side.Bishop ^= mask;
		side.All ^= mask;
		board.ue.remove_piece<white, BISHOP>(square);
		board.EvalInfo.remove_bishop<white>(square);
		if (__builtin_popcountll(side.Bishop) == 1) board.EvalInfo.remove_bishop_pair_bonus<white>();
		attack.Bishop = bishop_attacks(side.Bishop, board.Occ ^ ToMask(get_side<not white>(board).King));
		return 3;
	case 4:
		side.Rook ^= mask;
		side.All ^= mask;
		board.ue.remove_piece<white, ROOK>(square);
		board.EvalInfo.remove_rook<white>(square);
		void_castling_rights_at_square<white>(side.Castle, board.ue.hash, square);
		board.EvalInfo.hash = board.ue.hash;
		attack.Rook = rook_attacks(side.Rook, board.Occ ^ ToMask(get_side<not white>(board).King));
		return 4;
	case 5:
		side.Queen ^= mask;
		side.All ^= mask;
		board.ue.remove_piece<white, QUEEN>(square);
		board.EvalInfo.remove_queen<white>(square);
		attack.Queen = queen_attacks(side.Queen, board.Occ ^ ToMask(get_side<not white>(board).King));
		return 5;
	}
	
	throw std::logic_error("Something went wrong - Multiple pieces at square?");
}

template <bool white>
void remove_pawn(Board &board, const Square square){
	HalfBoard &side = get_side<white>(board);
	side.Pawn ^= ToMask(square);
	side.All ^= ToMask(square);
	board.ue.remove_piece<white, PAWN>(square);
	board.EvalInfo.remove_pawn<white>(square);
	(white ? board.WtAtk : board.BkAtk).Pawn = pawn_attacks<white>(side.Pawn);
}

template <bool white>
void pawn_move_common(Board &board, const Square from, const Square to){
	HalfBoard &side = get_side<white>(board);
	side.Pawn ^= ToMask(from) | ToMask(to);
	side.All ^= ToMask(from) | ToMask(to);
	board.ue.move_piece<white, PAWN>(from, to);
	board.EvalInfo.move_pawn<white>(from, to);
	(white ? board.WtAtk : board.BkAtk).Pawn = pawn_attacks<white>(side.Pawn);
}


void recalculate_slider_attacks_where_affected(Board &board, const BitMask affected){
	if (affected & board.WtAtk.Bishop & ~EDGES) board.WtAtk.Bishop = bishop_attacks(board.White.Bishop, board.Occ ^ ToMask(board.Black.King));
	if (affected & board.BkAtk.Bishop & ~EDGES) board.BkAtk.Bishop = bishop_attacks(board.Black.Bishop, board.Occ ^ ToMask(board.White.King));
	if (affected & board.WtAtk.Rook) board.WtAtk.Rook = rook_attacks(board.White.Rook, board.Occ ^ ToMask(board.Black.King));
	if (affected & board.BkAtk.Rook) board.BkAtk.Rook = rook_attacks(board.Black.Rook, board.Occ ^ ToMask(board.White.King));
	if (affected & board.WtAtk.Queen) board.WtAtk.Queen = queen_attacks(board.White.Queen, board.Occ ^ ToMask(board.Black.King));
	if (affected & board.BkAtk.Queen) board.BkAtk.Queen = queen_attacks(board.Black.Queen, board.Occ ^ ToMask(board.White.King));
}

template <bool white>
void recalculate_slider_attacks_where_affected_for_side(Board &board, const BitMask affected){
	if ((affected & board.WtAtk.Bishop & ~EDGES) and white) board.WtAtk.Bishop = bishop_attacks(board.White.Bishop, board.Occ ^ ToMask(board.Black.King));
	if ((affected & board.BkAtk.Bishop & ~EDGES) and not white) board.BkAtk.Bishop = bishop_attacks(board.Black.Bishop, board.Occ ^ ToMask(board.White.King));
	if ((affected & board.WtAtk.Rook) and white) board.WtAtk.Rook = rook_attacks(board.White.Rook, board.Occ ^ ToMask(board.Black.King));
	if ((affected & board.BkAtk.Rook) and not white) board.BkAtk.Rook = rook_attacks(board.Black.Rook, board.Occ ^ ToMask(board.White.King));
	if ((affected & board.WtAtk.Queen) and white) board.WtAtk.Queen = queen_attacks(board.White.Queen, board.Occ ^ ToMask(board.Black.King));
	if ((affected & board.BkAtk.Queen) and not white) board.BkAtk.Queen = queen_attacks(board.Black.Queen, board.Occ ^ ToMask(board.White.King));
}


template <bool white>
int make_move(Board &board, const Move move){
	HalfBoard &f = get_side<white>(board); // f for friendly
	HalfBoard &e = get_side<not white>(board); // e for enemy
	Attacks &f_atk = (white ? board.WtAtk : board.BkAtk);

	const Square from = move_source(move);
	const Square to = move_destination(move);
	const BitMask move_mask = ToMask(from) | ToMask(to);

	int capture = 0;
	board.EPMask = EMPTY_BOARD;
	
	switch (move_flags(move)){

	case KNIGHT_MOVE:
		f.Knight ^= move_mask;
		f.All ^= move_mask;
		board.ue.move_piece<white, KNIGHT>(from, to);
		board.EvalInfo.move_knight<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Knight = knight_attacks(f.Knight);
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case BISHOP_MOVE:
		f.Bishop ^= move_mask;
		f.All ^= move_mask;
		board.ue.move_piece<white, BISHOP>(from, to);
		board.EvalInfo.move_bishop<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Bishop = bishop_attacks(f.Bishop, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case ROOK_MOVE:
		f.Rook ^= move_mask;
		f.All ^= move_mask;
		void_castling_rights_at_square<white>(f.Castle, board.ue.hash, from);
		board.EvalInfo.hash = board.ue.hash;
		board.ue.move_piece<white, ROOK>(from, to);
		board.EvalInfo.move_rook<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case QUEEN_MOVE:
		f.Queen ^= move_mask;
		f.All ^= move_mask;
		board.ue.move_piece<white, QUEEN>(from, to);
		board.EvalInfo.move_queen<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case KING_MOVE:
		f.King = to;
		f.All ^= move_mask;
		void_all_castling_rights<white>(f.Castle, board.ue.hash);
		board.EvalInfo.hash = board.ue.hash;
		board.ue.move_piece<white, KING>(from, to);
		board.EvalInfo.move_king<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.King = king_attacks(to);
		recalculate_slider_attacks_where_affected_for_side<white>(board, capture ? ToMask(from) : move_mask);
		return capture;

	case CASTLE_QUEENSIDE:
		f.King = white ? C1 : C8;
		f.Rook ^= white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8));
		f.All ^= (white ? (ToMask(A1) | ToMask(D1)) : (ToMask(A8) | ToMask(D8))) ^ (white ? (ToMask(E1) | ToMask(C1)) : (ToMask(E8) | ToMask(C8)));
		void_all_castling_rights<white>(f.Castle, board.ue.hash);
		board.EvalInfo.hash = board.ue.hash;
		board.ue.castle_queenside<white>();
		board.EvalInfo.castle_queenside<white>();
		board.Occ = f.All | e.All;
		f_atk.King = king_attacks(white ? C1 : C8);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		if ((f_atk.Queen & ToMask(white ? E1 : E8)) and (f.Queen & (white ? RANK_1 : RANK_8))) f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		return 0;
	case CASTLE_KINGSIDE:
		f.King = white ? G1 : G8;
		f.Rook ^= white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8));
		f.All ^= (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))) ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8)));
		void_all_castling_rights<white>(f.Castle, board.ue.hash);
		board.EvalInfo.hash = board.ue.hash;
		board.ue.castle_kingside<white>();
		board.EvalInfo.castle_kingside<white>();
		board.Occ = f.All | e.All;
		f_atk.King = king_attacks(white ? G1 : G8);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		if ((f_atk.Queen & ToMask(white ? E1 : E8)) and (f.Queen & (white ? RANK_1 : RANK_8))) f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		return 0;

	case SINGLE_PAWN_PUSH:
		pawn_move_common<white>(board, from, to);
		board.Occ = f.All | e.All;
		recalculate_slider_attacks_where_affected(board, move_mask);
		return 0;
	case DOUBLE_PAWN_PUSH:
		pawn_move_common<white>(board, from, to);
		board.Occ = f.All | e.All;
		board.EPMask = ToMask(to);
		recalculate_slider_attacks_where_affected(board, move_mask);
		return 0;
	case PAWN_CAPTURE:
		pawn_move_common<white>(board, from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		recalculate_slider_attacks_where_affected(board, ToMask(from));
		return capture;
	case EN_PASSANT_CAPTURE:
		pawn_move_common<white>(board, from, to);
		remove_pawn<not white>(board, to + (white ? -8 : 8));
		board.Occ = f.All | e.All;
		recalculate_slider_attacks_where_affected(board, move_mask | ToMask(to + (white ? -8 : 8)));
		return 1;

	case PROMOTE_TO_KNIGHT:
		f.Pawn ^= ToMask(from);
		f.Knight ^= ToMask(to);
		f.All ^= move_mask;
		board.ue.promote_pawn<white, KNIGHT>(from, to);
		board.EvalInfo.promote_pawn_to_knight<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Pawn = pawn_attacks<white>(f.Pawn);
		f_atk.Knight = knight_attacks(f.Knight);
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case PROMOTE_TO_BISHOP:
		f.Pawn ^= ToMask(from);
		f.Bishop ^= ToMask(to);
		f.All ^= move_mask;
		board.ue.promote_pawn<white, BISHOP>(from, to);
		board.EvalInfo.promote_pawn_to_bishop<white>(from, to);
		if (__builtin_popcountll(f.Bishop) == 2) board.EvalInfo.remove_bishop_pair_bonus<not white>();
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Pawn = pawn_attacks<white>(f.Pawn);
		f_atk.Bishop = bishop_attacks(f.Bishop, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case PROMOTE_TO_ROOK:
		f.Pawn ^= ToMask(from);
		f.Rook ^= ToMask(to);
		f.All ^= move_mask;
		board.ue.promote_pawn<white, ROOK>(from, to);
		board.EvalInfo.promote_pawn_to_rook<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Pawn = pawn_attacks<white>(f.Pawn);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case PROMOTE_TO_QUEEN:
		f.Pawn ^= ToMask(from);
		f.Queen ^= ToMask(to);
		f.All ^= move_mask;
		board.ue.promote_pawn<white, QUEEN>(from, to);
		board.EvalInfo.promote_pawn_to_queen<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Pawn = pawn_attacks<white>(f.Pawn);
		f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	default:
		throw std::logic_error("Unexpected move flag");
	}
}

template int make_move<true>(Board&, const Move);
template int make_move<false>(Board&, const Move);

bool is_irreversible(const Board &board, const Move move){
	return move_flags(move) >= 8 or (ToMask(move_destination(move)) & board.Occ);
}
