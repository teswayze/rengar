# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "lookup.hpp"

struct ChecksAndPins{
	const BitMask CheckMask;
	const BitMask HVPin;
	const BitMask DiagPin;

	ChecksAndPins(BitMask check_mask, BitMask hv_pin, BitMask diag_pin) :
		CheckMask(check_mask), HVPin(hv_pin), DiagPin(diag_pin) { }
};

template <bool white>
ChecksAndPins checks_and_pins(const Board board);

template <bool white>
MoveQueue<white> generate_moves(const Board board, const ChecksAndPins cnp, const Move hint);

template <bool white>
MoveQueue<white> generate_forcing(const Board board, const ChecksAndPins cnp, const Move hint);


constexpr BitMask compute_knight_moves(const size_t square){
	return (ToMask(square) & ~A_FILE) >> 17 |
			(ToMask(square) & ~H_FILE) >> 15 |
			(ToMask(square) & ~(A_FILE | B_FILE)) >> 10 |
			(ToMask(square) & ~(G_FILE | H_FILE)) >> 6 |
			(ToMask(square) & ~(A_FILE | B_FILE)) << 6 |
			(ToMask(square) & ~(G_FILE | H_FILE)) << 10 |
			(ToMask(square) & ~A_FILE) << 15 |
			(ToMask(square) & ~H_FILE) << 17;
}
const auto knight_lookup = lookup_table<BitMask, 64>(compute_knight_moves);

constexpr BitMask compute_king_moves(const size_t square){
	return (ToMask(square) & ~A_FILE) >> 9 |
			(ToMask(square) >> 8) |
			(ToMask(square) & ~H_FILE) >> 7 |
			(ToMask(square) & ~A_FILE) >> 1 |
			(ToMask(square) & ~H_FILE) << 1 |
			(ToMask(square) & ~A_FILE) << 7 |
			(ToMask(square) << 8) |
			(ToMask(square) & ~H_FILE) << 9;
}

const auto king_lookup = lookup_table<BitMask, 64>(compute_king_moves);
