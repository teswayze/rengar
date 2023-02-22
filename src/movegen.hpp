# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"

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
