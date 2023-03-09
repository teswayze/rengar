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

template <bool white>
RemovalInfo get_removal_info(const HalfBoard board, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & board.All)){ return std::make_tuple(NOOP, 0, 0); }
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


# include "doctest.h"


template <bool white>
inline void check_consistent(HalfBoard h){
	auto recomputed = static_eval_info<white>(h.Pawn, h.Knight, h.Bishop, h.Rook, h.Queen, h.King);
	CHECK(h.EvalInfo.mg == recomputed.mg);
	CHECK(h.EvalInfo.eg == recomputed.eg);
	CHECK(h.EvalInfo.phase_count == recomputed.phase_count);
}


TEST_CASE("Removing pieces from white starting position"){
	HalfBoard h = from_masks<true>(RANK_2, ToMask(B1) | ToMask(G1), ToMask(C1) | ToMask(F1),
			ToMask(A1) | ToMask(H1), ToMask(D1), ToMask(E1), ToMask(A1) | ToMask(H1));
	
	HalfBoard no_g2 = remove_piece<true>(h, G2);
	CHECK(no_g2.Pawn == (RANK_2 & ~ToMask(G2)));
	CHECK(no_g2.All == (RANK_1 | RANK_2 & ~ToMask(G2)));
	check_consistent<true>(no_g2);
	
	HalfBoard no_b1 = remove_piece<true>(h, B1);
	CHECK(no_b1.Knight == ToMask(G1));
	CHECK(no_b1.All == (RANK_2 | RANK_1 & ~ToMask(B1)));
	check_consistent<true>(no_b1);
	
	HalfBoard no_c1 = remove_piece<true>(h, C1);
	CHECK(no_c1.Bishop == ToMask(F1));
	CHECK(no_c1.All == (RANK_2 | RANK_1 & ~ToMask(C1)));
	check_consistent<true>(no_c1);
		
	HalfBoard no_a1 = remove_piece<true>(h, A1);
	CHECK(no_a1.Rook == ToMask(H1));
	CHECK(no_a1.All == (RANK_2 | RANK_1 & ~ToMask(A1)));
	CHECK(no_a1.Castle == ToMask(H1));
	check_consistent<true>(no_a1);
	
	HalfBoard no_d1 = remove_piece<true>(h, D1);
	CHECK(no_d1.Queen == EMPTY_BOARD);
	CHECK(no_d1.All == (RANK_2| RANK_1 & ~ToMask(D1)));
	check_consistent<true>(no_d1);
	
	HalfBoard same = remove_piece<true>(h, B8);
	CHECK(h == same);
	CHECK(h.All == same.All);
	check_consistent<true>(same);
}

TEST_CASE("Removing pieces from black starting position"){
	HalfBoard h = from_masks<false>(RANK_7, ToMask(B8) | ToMask(G8), ToMask(C8) | ToMask(F8),
			ToMask(A8) | ToMask(H8), ToMask(D8), ToMask(E8), ToMask(A8) | ToMask(H8));
	
	HalfBoard no_g7 = remove_piece<false>(h, G7);
	CHECK(no_g7.Pawn == (RANK_7 & ~ToMask(G7)));
	CHECK(no_g7.All == (RANK_8 | RANK_7 & ~ToMask(G7)));
	check_consistent<false>(no_g7);
	
	HalfBoard no_g8 = remove_piece<false>(h, G8);
	CHECK(no_g8.Knight == ToMask(B8));
	CHECK(no_g8.All == (RANK_7 | RANK_8 & ~ToMask(G8)));
	check_consistent<false>(no_g8);
	
	HalfBoard no_c8 = remove_piece<false>(h, C8);
	CHECK(no_c8.Bishop == ToMask(F8));
	CHECK(no_c8.All == (RANK_7 | RANK_8 & ~ToMask(C8)));
	check_consistent<false>(no_c8);
		
	HalfBoard no_h8 = remove_piece<false>(h, H8);
	CHECK(no_h8.Rook == ToMask(A8));
	CHECK(no_h8.All == (RANK_7 | RANK_8 & ~ToMask(H8)));
	CHECK(no_h8.Castle == ToMask(A8));
	check_consistent<false>(no_h8);
	
	HalfBoard no_d8 = remove_piece<false>(h, D8);
	CHECK(no_d8.Queen == EMPTY_BOARD);
	CHECK(no_d8.All == (RANK_7 | RANK_8 & ~ToMask(D8)));
	check_consistent<false>(no_d8);
	
	HalfBoard same = remove_piece<true>(h, F1);
	CHECK(h == same);
	CHECK(h.All == same.All);
	check_consistent<false>(same);
}

constexpr BitMask move_piece(BitMask initial, Square from, Square to){
	return (initial & ~ToMask(from)) | ToMask(to);
}

template <bool white>
Board make_move(const Board board, const Move move){
	const HalfBoard f = get_side<white>(board); // f for friendly
	const HalfBoard e = get_side<not white>(board); // e for enemy

	switch (move_flags(move)){
	case NULL_MOVE:
		return board;

	case KNIGHT_MOVE:
		return from_sides<white>(
			from_masks<white>(f.Pawn,
					move_piece(f.Knight, move_source(move), move_destination(move)),
					f.Bishop, f.Rook, f.Queen, f.King, f.Castle),
			remove_piece<not white>(e, move_destination(move))
		);
	case BISHOP_MOVE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight,
					move_piece(f.Bishop, move_source(move), move_destination(move)),
					f.Rook, f.Queen, f.King, f.Castle),
			remove_piece<not white>(e, move_destination(move))
		);
	case ROOK_MOVE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight, f.Bishop,
					move_piece(f.Rook, move_source(move), move_destination(move)),
					f.Queen, f.King, f.Castle & ~ToMask(move_source(move))),
			remove_piece<not white>(e, move_destination(move))
		);
	case QUEEN_MOVE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight, f.Bishop, f.Rook,
					move_piece(f.Queen, move_source(move), move_destination(move)),
					f.King, f.Castle),
			remove_piece<not white>(e, move_destination(move))
		);
	case KING_MOVE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight, f.Bishop, f.Rook, f.Queen,
					move_piece(f.King, move_source(move), move_destination(move)),
					EMPTY_BOARD),
			remove_piece<not white>(e, move_destination(move))
		);
	case CASTLE_QUEENSIDE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight, f.Bishop,
					move_piece(f.Rook, white ? A1 : A8, white ? D1 : D8),
					f.Queen,
					move_piece(f.King, white ? E1 : E8, white ? C1 : C8),
					EMPTY_BOARD), e
		);
	case CASTLE_KINGSIDE:
		return from_sides<white>(
			from_masks<white>(f.Pawn, f.Knight, f.Bishop,
					move_piece(f.Rook, white ? H1 : H8, white ? F1 : F8),
					f.Queen,
					move_piece(f.King, white ? E1 : E8, white ? G1 : G8),
					EMPTY_BOARD), e
		);

	case SINGLE_PAWN_PUSH:
		return from_sides<white>(
			from_masks<white>(move_piece(f.Pawn, move_source(move), move_destination(move)),
					f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.Castle), e
		);
	case DOUBLE_PAWN_PUSH:
		return from_sides_ep<white>(
			from_masks<white>(move_piece(f.Pawn, move_source(move), move_destination(move)),
					f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.Castle),
					e, move_destination(move)
		);
	case PAWN_CAPTURE:
		return from_sides<white>(
			from_masks<white>(move_piece(f.Pawn, move_source(move), move_destination(move)),
					f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move))
		);
	case EN_PASSANT_CAPTURE:
		return from_sides<white>(
			from_masks<white>(move_piece(f.Pawn, move_source(move), move_destination(move)),
					f.Knight, f.Bishop, f.Rook, f.Queen, f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move) + (white ? -8 : 8))
		);
	case PROMOTE_TO_KNIGHT:
		return from_sides<white>(
			from_masks<white>(f.Pawn & ~ToMask(move_source(move)),
					f.Knight | ToMask(move_destination(move)), f.Bishop, f.Rook, f.Queen, f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move))
		);
	case PROMOTE_TO_BISHOP:
		return from_sides<white>(
			from_masks<white>(f.Pawn & ~ToMask(move_source(move)), f.Knight,
					f.Bishop | ToMask(move_destination(move)), f.Rook, f.Queen, f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move))
		);
	case PROMOTE_TO_ROOK:
		return from_sides<white>(
			from_masks<white>(f.Pawn & ~ToMask(move_source(move)), f.Knight, f.Bishop,
					f.Rook | ToMask(move_destination(move)), f.Queen, f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move))
		);
	case PROMOTE_TO_QUEEN:
		return from_sides<white>(
			from_masks<white>(f.Pawn & ~ToMask(move_source(move)), f.Knight, f.Bishop, f.Rook,
					f.Queen | ToMask(move_destination(move)), f.King, f.Castle),
					remove_piece<not white>(e, move_destination(move))
		);
	}
	throw std::logic_error("Unexpected move flag");
}

template Board make_move<true>(Board, Move);
template Board make_move<false>(Board, Move);

bool is_irreversible(const Board board, const Move move){
	return move_flags(move) >= 8 or (ToMask(move_destination(move)) & board.Occ);
}
