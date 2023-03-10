# include "board.hpp"

# include <tuple>
# include <exception>

template <bool white>
HalfBoard from_masks(const BitMask p, const BitMask n, const BitMask b, const BitMask r, 
		const BitMask q, const BitMask k, const BitMask castle){
	return HalfBoard{p, n, b, r, q, k, p | n | b | r | q | k, castle, static_eval_info<white>(p, n, b, r, q, k)};
}

template HalfBoard from_masks<true>(const BitMask, const BitMask, const BitMask, const BitMask, 
		const BitMask, const BitMask, const BitMask);
template HalfBoard from_masks<false>(const BitMask, const BitMask, const BitMask, const BitMask, 
		const BitMask, const BitMask, const BitMask);

const uint8_t NOOP = 0;
const uint8_t REMOVE_PAWN = 1;
const uint8_t REMOVE_KNIGHT = 2;
const uint8_t REMOVE_BISHOP = 3;
const uint8_t REMOVE_ROOK = 4;
const uint8_t REMOVE_QUEEN = 5;
// No king needed because we never remove the king

using RemovalInfo = std::tuple<uint8_t, int, int>;
const RemovalInfo NOTHING_TO_REMOVE = std::make_tuple(NOOP, 0, 0);

template <bool white>
RemovalInfo get_removal_info(const HalfBoard board, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return NOTHING_TO_REMOVE; }
	if (mask & board.Pawn){ return std::make_tuple(REMOVE_PAWN,
				mg_pawn_table[FlipIf(white, square)] + MG_PAWN, eg_pawn_table[FlipIf(white, square)] + EG_PAWN);}
	if (mask & board.Knight){ return std::make_tuple(REMOVE_KNIGHT,
				mg_knight_table[FlipIf(white, square)] + MG_KNIGHT, eg_knight_table[FlipIf(white, square)] + EG_KNIGHT);}
	if (mask & board.Bishop){ return std::make_tuple(REMOVE_BISHOP,
				mg_bishop_table[FlipIf(white, square)] + MG_BISHOP, eg_bishop_table[FlipIf(white, square)] + EG_BISHOP);}
	if (mask & board.Rook){ return std::make_tuple(REMOVE_ROOK,
				mg_rook_table[FlipIf(white, square)] + MG_ROOK, eg_rook_table[FlipIf(white, square)] + EG_ROOK);}
	if (mask & board.Queen){ return std::make_tuple(REMOVE_QUEEN,
				mg_queen_table[FlipIf(white, square)] + MG_QUEEN, eg_queen_table[FlipIf(white, square)] + EG_QUEEN);}
	
	throw std::logic_error("Something went wrong - trying to remove king? 'All' out of sync with other masks?");
}

HalfBoard remove_piece_with_info(const HalfBoard board, const Square square, const RemovalInfo info){
	const BitMask mask = ToMask(square);
	
	switch (std::get<0>(info)){
	case NOOP: return board;
	case REMOVE_PAWN: return HalfBoard{board.Pawn ^ mask, board.Knight, board.Bishop, board.Rook, board.Queen, board.King, board.All ^ mask, board.Castle, 
			PstEvalInfo{board.EvalInfo.mg - std::get<1>(info), board.EvalInfo.eg - std::get<2>(info), board.EvalInfo.phase_count}};
	case REMOVE_KNIGHT: return HalfBoard{board.Pawn, board.Knight ^ mask, board.Bishop, board.Rook, board.Queen, board.King, board.All ^ mask, board.Castle, 
			PstEvalInfo{board.EvalInfo.mg - std::get<1>(info), board.EvalInfo.eg - std::get<2>(info), static_cast<uint8_t>(board.EvalInfo.phase_count - 1)}};
	case REMOVE_BISHOP: return HalfBoard{board.Pawn, board.Knight, board.Bishop ^ mask, board.Rook, board.Queen, board.King, board.All ^ mask, board.Castle, 
			PstEvalInfo{board.EvalInfo.mg - std::get<1>(info), board.EvalInfo.eg - std::get<2>(info), static_cast<uint8_t>(board.EvalInfo.phase_count - 1)}};
	case REMOVE_ROOK: return HalfBoard{board.Pawn, board.Knight, board.Bishop, board.Rook ^ mask, board.Queen, board.King, board.All ^ mask, board.Castle & (~mask), 
			PstEvalInfo{board.EvalInfo.mg - std::get<1>(info), board.EvalInfo.eg - std::get<2>(info), static_cast<uint8_t>(board.EvalInfo.phase_count - 2)}};
	case REMOVE_QUEEN: return HalfBoard{board.Pawn, board.Knight, board.Bishop, board.Rook, board.Queen ^ mask, board.King, board.All ^ mask, board.Castle, 
			PstEvalInfo{board.EvalInfo.mg - std::get<1>(info), board.EvalInfo.eg - std::get<2>(info), static_cast<uint8_t>(board.EvalInfo.phase_count - 4)}};
	}
	
	throw std::logic_error("Unrecognized first element of remval info tuple");
}


template <bool white>
HalfBoard remove_piece(const HalfBoard board, const Square square){
	return remove_piece_with_info(board, square, get_removal_info<white>(board, square));
}

template HalfBoard remove_piece<true>(const HalfBoard, const Square);
template HalfBoard remove_piece<false>(const HalfBoard, const Square);

constexpr BitMask move_piece(BitMask initial, Square from, Square to){
	return initial ^ (ToMask(from) | ToMask(to));
}

using EvalDiffInfo = std::tuple<int, int, RemovalInfo>;

template <bool white>
EvalDiffInfo get_movement_info(const HalfBoard enemy, const Move move){
	const Square from = move_source(move);
	const Square to = move_destination(move);

	switch (move_flags(move)){
		case NULL_MOVE:
			return std::make_tuple(0, 0, NOTHING_TO_REMOVE);

		case KNIGHT_MOVE:
			return std::make_tuple(
					mg_knight_table[FlipIf(white, to)] - mg_knight_table[FlipIf(white, from)], 
					eg_knight_table[FlipIf(white, to)] - eg_knight_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case BISHOP_MOVE:
			return std::make_tuple(
					mg_bishop_table[FlipIf(white, to)] - mg_bishop_table[FlipIf(white, from)], 
					eg_bishop_table[FlipIf(white, to)] - eg_bishop_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case ROOK_MOVE:
			return std::make_tuple(
					mg_rook_table[FlipIf(white, to)] - mg_rook_table[FlipIf(white, from)], 
					eg_rook_table[FlipIf(white, to)] - eg_rook_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case QUEEN_MOVE:
			return std::make_tuple(
					mg_queen_table[FlipIf(white, to)] - mg_queen_table[FlipIf(white, from)], 
					eg_queen_table[FlipIf(white, to)] - eg_queen_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case KING_MOVE:
			return std::make_tuple(
					mg_king_table[FlipIf(white, to)] - mg_king_table[FlipIf(white, from)], 
					eg_king_table[FlipIf(white, to)] - eg_king_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case CASTLE_QUEENSIDE:
			return std::make_tuple(
					mg_king_table[C8] + mg_rook_table[D8] - mg_king_table[E8] - mg_rook_table[A8],
					eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8],
					NOTHING_TO_REMOVE);
		case CASTLE_KINGSIDE:
			return std::make_tuple(
					mg_king_table[G8] + mg_rook_table[F8] - mg_king_table[E8] - mg_rook_table[H8],
					eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8],
					NOTHING_TO_REMOVE);

		case SINGLE_PAWN_PUSH:
		case DOUBLE_PAWN_PUSH:
			return std::make_tuple(
					mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
					eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
					NOTHING_TO_REMOVE);
		case PAWN_CAPTURE:
			return std::make_tuple(
					mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
					eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to));
		case EN_PASSANT_CAPTURE:
			return std::make_tuple(
					mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)], 
					eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)],
					get_removal_info<not white>(enemy, to + (white ? -8 : 8)));

		case PROMOTE_TO_KNIGHT:
			return std::make_tuple(
					mg_knight_table[FlipIf(white, to)] + MG_KNIGHT - mg_pawn_table[FlipIf(white, from)] - MG_PAWN, 
					eg_knight_table[FlipIf(white, to)] + EG_KNIGHT - eg_pawn_table[FlipIf(white, from)] - EG_PAWN,
					get_removal_info<not white>(enemy, to));
		case PROMOTE_TO_BISHOP:
			return std::make_tuple(
					mg_bishop_table[FlipIf(white, to)] + MG_BISHOP - mg_pawn_table[FlipIf(white, from)] - MG_PAWN, 
					eg_bishop_table[FlipIf(white, to)] + EG_BISHOP - eg_pawn_table[FlipIf(white, from)] - EG_PAWN,
					get_removal_info<not white>(enemy, to));
		case PROMOTE_TO_ROOK:
			return std::make_tuple(
					mg_rook_table[FlipIf(white, to)] + MG_ROOK - mg_pawn_table[FlipIf(white, from)] - MG_PAWN, 
					eg_rook_table[FlipIf(white, to)] + EG_ROOK - eg_pawn_table[FlipIf(white, from)] - EG_PAWN,
					get_removal_info<not white>(enemy, to));
		case PROMOTE_TO_QUEEN:
			return std::make_tuple(
					mg_queen_table[FlipIf(white, to)] + MG_QUEEN - mg_pawn_table[FlipIf(white, from)] - MG_PAWN, 
					eg_queen_table[FlipIf(white, to)] + EG_QUEEN - eg_pawn_table[FlipIf(white, from)] - EG_PAWN,
					get_removal_info<not white>(enemy, to));
		}
		throw std::logic_error("Unexpected move flag");
}

template <bool white>
Board make_move_with_info(const Board board, const Move move, const EvalDiffInfo movement_info){
	const HalfBoard f = get_side<white>(board); // f for friendly
	const HalfBoard e = get_side<not white>(board); // e for enemy
	const Square from = move_source(move);
	const Square to = move_destination(move);
	const int mg_diff = std::get<0>(movement_info);
	const int eg_diff = std::get<1>(movement_info);
	const RemovalInfo removal_info = std::get<2>(movement_info);

	switch (move_flags(move)){
	case NULL_MOVE:
		return board;

	case KNIGHT_MOVE:
		return from_sides<white>(
			HalfBoard{f.Pawn, move_piece(f.Knight, from, to), f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case BISHOP_MOVE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, move_piece(f.Bishop, from, to), f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case ROOK_MOVE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, f.Bishop, move_piece(f.Rook, from, to), f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle & ~ToMask(from), PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case QUEEN_MOVE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook, move_piece(f.Queen, from, to), f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case KING_MOVE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, f.Bishop, f.Rook, f.Queen, move_piece(f.King, from, to), 
				move_piece(f.All, from, to), EMPTY_BOARD, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case CASTLE_QUEENSIDE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, f.Bishop, move_piece(f.Rook, white ? A1 : A8, white ? D1 : D8), f.Queen, move_piece(f.King, white ? E1 : E8, white ? C1 : C8),
				move_piece(move_piece(f.All, white ? E1 : E8, white ? C1 : C8), white ? A1 : A8, white ? D1 : D8), EMPTY_BOARD,
				PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			e
		);
	case CASTLE_KINGSIDE:
		return from_sides<white>(
			HalfBoard{f.Pawn, f.Knight, f.Bishop, move_piece(f.Rook, white ? H1 : H8, white ? F1 : F8), f.Queen, move_piece(f.King, white ? E1 : E8, white ? G1 : G8),
			move_piece(move_piece(f.All, white ? E1 : E8, white ? G1 : G8), white ? H1 : H8, white ? F1 : F8), EMPTY_BOARD,
			PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			e
		);

	case SINGLE_PAWN_PUSH:
		return from_sides<white>(
			HalfBoard{move_piece(f.Pawn, from, to), f.Knight, f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			e
		);
	case DOUBLE_PAWN_PUSH:
		return from_sides_ep<white>(
			HalfBoard{move_piece(f.Pawn, from, to), f.Knight, f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			e, to
		);
	case PAWN_CAPTURE:
		return from_sides<white>(
			HalfBoard{move_piece(f.Pawn, from, to), f.Knight, f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to, removal_info)
		);
	case EN_PASSANT_CAPTURE:
		return from_sides<white>(
			HalfBoard{move_piece(f.Pawn, from, to), f.Knight, f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, f.EvalInfo.phase_count}},
			remove_piece_with_info(e, to + (white ? -8 : 8), removal_info)
		);
	case PROMOTE_TO_KNIGHT:
		return from_sides<white>(
			HalfBoard{f.Pawn ^ ToMask(from), f.Knight | ToMask(to), f.Bishop, f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, static_cast<uint8_t>(f.EvalInfo.phase_count + 1)}},
			remove_piece_with_info(e, to, removal_info)
		);
	case PROMOTE_TO_BISHOP:
		return from_sides<white>(
			HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop | ToMask(to), f.Rook, f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, static_cast<uint8_t>(f.EvalInfo.phase_count + 1)}},
			remove_piece_with_info(e, to, removal_info)
		);
	case PROMOTE_TO_ROOK:
		return from_sides<white>(
			HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop, f.Rook | ToMask(to), f.Queen, f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, static_cast<uint8_t>(f.EvalInfo.phase_count + 2)}},
			remove_piece_with_info(e, to, removal_info)
		);
	case PROMOTE_TO_QUEEN:
		return from_sides<white>(
			HalfBoard{f.Pawn ^ ToMask(from), f.Knight, f.Bishop, f.Rook, f.Queen | ToMask(to), f.King, 
				move_piece(f.All, from, to), f.Castle, PstEvalInfo{f.EvalInfo.mg + mg_diff, f.EvalInfo.eg + eg_diff, static_cast<uint8_t>(f.EvalInfo.phase_count + 4)}},
			remove_piece_with_info(e, to, removal_info)
		);
	}
	throw std::logic_error("Unexpected move flag");
}

template <bool white>
Board make_move(Board board, Move move){
	return make_move_with_info<white>(board, move, get_movement_info<white>(get_side<not white>(board), move));
}

template Board make_move<true>(Board, Move);
template Board make_move<false>(Board, Move);

bool is_irreversible(const Board board, const Move move){
	return move_flags(move) >= 8 or (ToMask(move_destination(move)) & board.Occ);
}


# include "doctest.h"


template <bool white>
inline void check_consistent_hb(HalfBoard h){
	auto recomputed = static_eval_info<white>(h.Pawn, h.Knight, h.Bishop, h.Rook, h.Queen, h.King);
	CHECK(h.All == (h.Pawn | h.Knight | h.Bishop | h.Rook | h.Queen | h.King));
	CHECK(h.EvalInfo.mg == recomputed.mg);
	CHECK(h.EvalInfo.eg == recomputed.eg);
	CHECK(h.EvalInfo.phase_count == recomputed.phase_count);
}


TEST_CASE("Removing pieces from white starting position"){
	HalfBoard h = from_masks<true>(RANK_2, ToMask(B1) | ToMask(G1), ToMask(C1) | ToMask(F1),
			ToMask(A1) | ToMask(H1), ToMask(D1), ToMask(E1), ToMask(A1) | ToMask(H1));
	
	HalfBoard no_g2 = remove_piece<true>(h, G2);
	CHECK(no_g2.Pawn == (RANK_2 & ~ToMask(G2)));
	check_consistent_hb<true>(no_g2);
	
	HalfBoard no_b1 = remove_piece<true>(h, B1);
	CHECK(no_b1.Knight == ToMask(G1));
	check_consistent_hb<true>(no_b1);
	
	HalfBoard no_c1 = remove_piece<true>(h, C1);
	CHECK(no_c1.Bishop == ToMask(F1));
	check_consistent_hb<true>(no_c1);
		
	HalfBoard no_a1 = remove_piece<true>(h, A1);
	CHECK(no_a1.Rook == ToMask(H1));
	CHECK(no_a1.Castle == ToMask(H1));
	check_consistent_hb<true>(no_a1);
	
	HalfBoard no_d1 = remove_piece<true>(h, D1);
	CHECK(no_d1.Queen == EMPTY_BOARD);
	check_consistent_hb<true>(no_d1);
	
	HalfBoard same = remove_piece<true>(h, B8);
	CHECK(h == same);
	check_consistent_hb<true>(same);
}

TEST_CASE("Removing pieces from black starting position"){
	HalfBoard h = from_masks<false>(RANK_7, ToMask(B8) | ToMask(G8), ToMask(C8) | ToMask(F8),
			ToMask(A8) | ToMask(H8), ToMask(D8), ToMask(E8), ToMask(A8) | ToMask(H8));
	
	HalfBoard no_g7 = remove_piece<false>(h, G7);
	CHECK(no_g7.Pawn == (RANK_7 & ~ToMask(G7)));
	CHECK(no_g7.All == (RANK_8 | RANK_7 & ~ToMask(G7)));
	check_consistent_hb<false>(no_g7);
	
	HalfBoard no_g8 = remove_piece<false>(h, G8);
	CHECK(no_g8.Knight == ToMask(B8));
	CHECK(no_g8.All == (RANK_7 | RANK_8 & ~ToMask(G8)));
	check_consistent_hb<false>(no_g8);
	
	HalfBoard no_c8 = remove_piece<false>(h, C8);
	CHECK(no_c8.Bishop == ToMask(F8));
	CHECK(no_c8.All == (RANK_7 | RANK_8 & ~ToMask(C8)));
	check_consistent_hb<false>(no_c8);
		
	HalfBoard no_h8 = remove_piece<false>(h, H8);
	CHECK(no_h8.Rook == ToMask(A8));
	CHECK(no_h8.All == (RANK_7 | RANK_8 & ~ToMask(H8)));
	CHECK(no_h8.Castle == ToMask(A8));
	check_consistent_hb<false>(no_h8);
	
	HalfBoard no_d8 = remove_piece<false>(h, D8);
	CHECK(no_d8.Queen == EMPTY_BOARD);
	CHECK(no_d8.All == (RANK_7 | RANK_8 & ~ToMask(D8)));
	check_consistent_hb<false>(no_d8);
	
	HalfBoard same = remove_piece<true>(h, F1);
	CHECK(h == same);
	CHECK(h.All == same.All);
	check_consistent_hb<false>(same);
}

inline void check_consistent_fb(Board b){
	check_consistent_hb<true>(b.White);
	check_consistent_hb<false>(b.Black);
	CHECK(b.Occ == (b.White.All | b.Black.All));
}

TEST_CASE("White pawn promotion"){
	Board b = Board(from_masks<true>(ToMask(C7), ToMask(C1), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(C2), EMPTY_BOARD),
			from_masks<false>(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B8), EMPTY_BOARD, ToMask(A1), EMPTY_BOARD));
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
	Board b = Board(from_masks<true>(EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(G1), EMPTY_BOARD, ToMask(H8), EMPTY_BOARD),
			from_masks<false>(ToMask(F2), ToMask(F8), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(F7), EMPTY_BOARD));
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
	Board b = Board(from_masks<true>(ToMask(H2), ToMask(F3), ToMask(E2), ToMask(A1) | ToMask(H1), ToMask(D2), ToMask(E1), ToMask(A1) | ToMask(H1)),
			from_masks<false>(ToMask(H7), ToMask(F6), ToMask(E7), ToMask(A8) | ToMask(H8), ToMask(D7), ToMask(E8), ToMask(A8) | ToMask(H8)));
	
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
	Board b = Board(from_masks<true>(ToMask(F5), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B4), EMPTY_BOARD),
				from_masks<false>(ToMask(E5) | ToMask(B3) | ToMask(A3), ToMask(A5), ToMask(A4), ToMask(C3), ToMask(G6), ToMask(E8), EMPTY_BOARD),
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
	Board b = Board(from_masks<true>(ToMask(E4) | ToMask(B6) | ToMask(A6), ToMask(A4), ToMask(A5), ToMask(C6), ToMask(G3), ToMask(E1), EMPTY_BOARD),
				from_masks<false>(ToMask(F4), EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, EMPTY_BOARD, ToMask(B5), EMPTY_BOARD),	
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
