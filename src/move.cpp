# include <stdexcept>

# include "board.hpp"
# include "move.hpp"

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
	return move_flags(move) >= 6 or (ToMask(move_destination(move)) & board.Occ);
}

