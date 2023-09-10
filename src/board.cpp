# include "board.hpp"
# include "hashing.hpp"
# include "weights/endgame.hpp"
# include "weights/osc_middlegame.hpp"
# include "weights/ssc_middlegame.hpp"
# include "weights/phase_count.hpp"

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
		calculate_attacks<true>(white, occ ^ ToMask(black.King)), calculate_attacks<false>(black, occ ^ ToMask(white.King))};
}

Board from_sides_without_eval_ep(const HalfBoard &white, const HalfBoard &black, const Square ep_square){
	const BitMask occ = white.All | black.All;
	return Board{white.copy(), black.copy(), white.All | black.All, ToMask(ep_square), recompute_from_sides(white, black),
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
		remove_pawn<white>(board, square);
		return 1;
	case 2:
		side.Knight ^= mask;
		side.All ^= mask;
		board.EvalInfo.remove_knight<white>(square);
		attack.Knight = knight_attacks(side.Knight);
		return 2;
	case 3:
		side.Bishop ^= mask;
		side.All ^= mask;
		board.EvalInfo.remove_bishop<white>(square);
		attack.Bishop = bishop_attacks(side.Bishop, board.Occ ^ ToMask(get_side<not white>(board).King));
		return 3;
	case 4:
		side.Rook ^= mask;
		side.All ^= mask;
		board.EvalInfo.remove_rook<white>(square);
		void_castling_rights_at_square<white>(side.Castle, board.EvalInfo.hash, square);
		attack.Rook = rook_attacks(side.Rook, board.Occ ^ ToMask(get_side<not white>(board).King));
		return 4;
	case 5:
		side.Queen ^= mask;
		side.All ^= mask;
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
	board.EvalInfo.remove_pawn<white>(square);
	(white ? board.WtAtk : board.BkAtk).Pawn = pawn_attacks<white>(side.Pawn);
}

constexpr uint64_t hash_diff(std::array<uint64_t, 64> table, Square from, Square to){
	return table[from] ^ table[to];
}

template <bool white>
void pawn_move_common(Board &board, const Square from, const Square to){
	HalfBoard &side = get_side<white>(board);
	side.Pawn ^= ToMask(from) | ToMask(to);
	side.All ^= ToMask(from) | ToMask(to);
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
	const int sign = white ? 1 : -1;

	int capture = 0;
	board.EPMask = EMPTY_BOARD;
	
	switch (move_flags(move)){

	case KNIGHT_MOVE:
		f.Knight ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.move_knight<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Knight = knight_attacks(f.Knight);
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case BISHOP_MOVE:
		f.Bishop ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.move_bishop<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Bishop = bishop_attacks(f.Bishop, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case ROOK_MOVE:
		f.Rook ^= move_mask;
		f.All ^= move_mask;
		void_castling_rights_at_square<white>(f.Castle, board.EvalInfo.hash, from);
		board.EvalInfo.move_rook<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case QUEEN_MOVE:
		f.Queen ^= move_mask;
		f.All ^= move_mask;
		board.EvalInfo.move_queen<white>(from, to);
		board.Occ = f.All | e.All;
		capture = maybe_remove_piece<not white>(board, to);
		f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		recalculate_slider_attacks_where_affected(board, capture ? ToMask(from) : move_mask);
		return capture;
	case KING_MOVE:
		f.King = to;
		f.All ^= move_mask;
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
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
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
		board.EvalInfo.mg_kk += sign * (
			ssc_mg_king_table[C8] + ssc_mg_rook_table[D8] - ssc_mg_king_table[E8] - ssc_mg_rook_table[A8]);
		(white ? board.EvalInfo.mg_qk : board.EvalInfo.mg_kq) += sign * (
			osc_mg_king_table[C8] + osc_mg_rook_table[D8] - osc_mg_king_table[E8] - osc_mg_rook_table[A8]);
		(white ? board.EvalInfo.mg_kq : board.EvalInfo.mg_qk) += sign * (
			osc_mg_king_table[F8] + osc_mg_rook_table[E8] - osc_mg_king_table[D8] - osc_mg_rook_table[H8]);
		board.EvalInfo.mg_qq += sign * (
			ssc_mg_king_table[F8] + ssc_mg_rook_table[E8] - ssc_mg_king_table[D8] - ssc_mg_rook_table[H8]);
		board.EvalInfo.eg += sign * (eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8]);
		board.EvalInfo.hash ^= white ? (hash_diff(white_king_hash, E1, C1) ^ hash_diff(white_rook_hash, A1, D1)) :
				(hash_diff(black_king_hash, E8, C8) ^ hash_diff(black_rook_hash, A8, D8));
		board.Occ = f.All | e.All;
		f_atk.King = king_attacks(white ? C1 : C8);
		f_atk.Rook = rook_attacks(f.Rook, board.Occ ^ ToMask(e.King));
		if ((f_atk.Queen & ToMask(white ? E1 : E8)) and (f.Queen & (white ? RANK_1 : RANK_8))) f_atk.Queen = queen_attacks(f.Queen, board.Occ ^ ToMask(e.King));
		return 0;
	case CASTLE_KINGSIDE:
		f.King = white ? G1 : G8;
		f.Rook ^= white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8));
		f.All ^= (white ? (ToMask(H1) | ToMask(F1)) : (ToMask(H8) | ToMask(F8))) ^ (white ? (ToMask(E1) | ToMask(G1)) : (ToMask(E8) | ToMask(G8)));
		void_all_castling_rights<white>(f.Castle, board.EvalInfo.hash);
		board.EvalInfo.mg_kk += sign * (
			ssc_mg_king_table[G8] + ssc_mg_rook_table[F8] - ssc_mg_king_table[E8] - ssc_mg_rook_table[H8]);
		(white ? board.EvalInfo.mg_qk : board.EvalInfo.mg_kq) += sign * (
			osc_mg_king_table[G8] + osc_mg_rook_table[F8] - osc_mg_king_table[E8] - osc_mg_rook_table[H8]);
		(white ? board.EvalInfo.mg_kq : board.EvalInfo.mg_qk) += sign * (
			osc_mg_king_table[B8] + osc_mg_rook_table[C8] - osc_mg_king_table[D8] - osc_mg_rook_table[A8]);
		board.EvalInfo.mg_qq += sign * (
			ssc_mg_king_table[B8] + ssc_mg_rook_table[C8] - ssc_mg_king_table[D8] - ssc_mg_rook_table[A8]);
		board.EvalInfo.eg += sign * (eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8]);
		board.EvalInfo.hash ^= white ? (hash_diff(white_king_hash, E1, G1) ^ hash_diff(white_rook_hash, H1, F1)) :
				(hash_diff(black_king_hash, E8, G8) ^ hash_diff(black_rook_hash, H8, F8));
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
		board.EvalInfo.mg_kk += sign * (ssc_mg_knight_table[FlipIf(white, to)] + ssc_mg_knight - 
			ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn);
		board.EvalInfo.mg_qk += sign * (osc_mg_knight_table[RotIf(white, to ^ 7)] + osc_mg_knight - 
			osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn);
		board.EvalInfo.mg_kq += sign * (osc_mg_knight_table[RotIf(white, to)] + osc_mg_knight - 
			osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn);
		board.EvalInfo.mg_qq += sign * (ssc_mg_knight_table[FlipIf(white, to ^ 7)] + ssc_mg_knight - 
			ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn);
		board.EvalInfo.eg += sign * (eg_knight_table[FlipIf(white, to)] + eg_knight - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_knight - pc_pawn;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_knight_hash : black_knight_hash)[to];
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
		board.EvalInfo.mg_kk += sign * (ssc_mg_bishop_table[FlipIf(white, to)] + ssc_mg_bishop - 
			ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn);
		board.EvalInfo.mg_qk += sign * (osc_mg_bishop_table[RotIf(white, to ^ 7)] + osc_mg_bishop - 
			osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn);
		board.EvalInfo.mg_kq += sign * (osc_mg_bishop_table[RotIf(white, to)] + osc_mg_bishop - 
			osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn);
		board.EvalInfo.mg_qq += sign * (ssc_mg_bishop_table[FlipIf(white, to ^ 7)] + ssc_mg_bishop - 
			ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn);
		board.EvalInfo.eg += sign * (eg_bishop_table[FlipIf(white, to)] + eg_bishop - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_bishop - pc_pawn;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_bishop_hash : black_bishop_hash)[to];
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
		board.EvalInfo.mg_kk += sign * (ssc_mg_rook_table[FlipIf(white, to)] + ssc_mg_rook - 
			ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn);
		board.EvalInfo.mg_qk += sign * (osc_mg_rook_table[RotIf(white, to ^ 7)] + osc_mg_rook - 
			osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn);
		board.EvalInfo.mg_kq += sign * (osc_mg_rook_table[RotIf(white, to)] + osc_mg_rook - 
			osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn);
		board.EvalInfo.mg_qq += sign * (ssc_mg_rook_table[FlipIf(white, to ^ 7)] + ssc_mg_rook - 
			ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn);
		board.EvalInfo.eg += sign * (eg_rook_table[FlipIf(white, to)] + eg_rook - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_rook - pc_pawn;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_rook_hash : black_rook_hash)[to];
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
		board.EvalInfo.mg_kk += sign * (ssc_mg_queen_table[FlipIf(white, to)] + ssc_mg_queen - 
			ssc_mg_pawn_table[FlipIf(white, from)] - ssc_mg_pawn);
		board.EvalInfo.mg_qk += sign * (osc_mg_queen_table[RotIf(white, to ^ 7)] + osc_mg_queen - 
			osc_mg_pawn_table[RotIf(white, from ^ 7)] - osc_mg_pawn);
		board.EvalInfo.mg_kq += sign * (osc_mg_queen_table[RotIf(white, to)] + osc_mg_queen - 
			osc_mg_pawn_table[RotIf(white, from)] - osc_mg_pawn);
		board.EvalInfo.mg_qq += sign * (ssc_mg_queen_table[FlipIf(white, to ^ 7)] + ssc_mg_queen - 
			ssc_mg_pawn_table[FlipIf(white, from ^ 7)] - ssc_mg_pawn);
		board.EvalInfo.eg += sign * (eg_queen_table[FlipIf(white, to)] + eg_queen - eg_pawn_table[FlipIf(white, from)] - eg_pawn);
		board.EvalInfo.phase_count += pc_queen - pc_pawn;
		board.EvalInfo.hash ^= (white ? white_pawn_hash : black_pawn_hash)[from] ^ (white ? white_queen_hash : black_queen_hash)[to];
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
