# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "parse_format.hpp"
# include "attacks.hpp"
# include "bmi2_fallback.hpp"

/* KNIGHTS */

template <bool white>
void generate_knight_moves(const Board &board, const ChecksAndPins cnp, MoveQueue &queue){
	Bitloop(get_side<white>(board).Knight & ~(cnp.HVPin | cnp.DiagPin), knights){
		const Square source = TZCNT(knights);
		const BitMask attacks = knight_lookup[source];
		Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
			queue.push_knight_move<white>(source, TZCNT(target));
		}
	}
}

constexpr BitMask knight_checks(const BitMask knights, const Square king){
	const BitMask checking_knights = knights & knight_lookup[king];
	return checking_knights ? checking_knights : FULL_BOARD;
}

/* KINGS */

template <bool white>
void generate_king_moves(const Board &board, const BitMask enemy_control, MoveQueue &queue){
	const HalfBoard &friendly = get_side<white>(board);
	const BitMask attacks = king_lookup[friendly.King];
	Bitloop(attacks & ~enemy_control & ~friendly.All, target){
		queue.push_king_move<white>(friendly.King, TZCNT(target));
	}

	if (friendly.Castle & ToMask(white ? A1 : A8)){
		if (not (board.Occ & (white ? (ToMask(B1) | ToMask(C1) | ToMask(D1)) :
				(ToMask(B8) | ToMask(C8) | ToMask(D8))))){
			if (not (enemy_control & (white ? (ToMask(C1) | ToMask(D1) | ToMask(E1)) :
					(ToMask(C8) | ToMask(D8) | ToMask(E8))))){
				queue.push_castle_qs<white>();
			}
		}
	}

	if (friendly.Castle & ToMask(white ? H1 : H8)){
		if (not (board.Occ & (white ? (ToMask(F1) | ToMask(G1)) : (ToMask(F8) | ToMask(G8))))){
			if (not (enemy_control & (white ? (ToMask(E1) | ToMask(F1) | ToMask(G1)) :
					(ToMask(E8) | ToMask(F8) | ToMask(G8))))){
				queue.push_castle_ks<white>();
			}
		}
	}
}

/* ROOKS */

template <bool white, bool queen>
void generate_rook_moves(const Board &board, const ChecksAndPins cnp, MoveQueue &queue){
	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Rook;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = TZCNT(unpinned);
			const BitMask attacks = rook_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move<white>(source, TZCNT(target));}
				else {queue.push_rook_move<white>(source, TZCNT(target));}
			}
		}
	Bitloop(pieces & cnp.HVPin, pinned){
		const Square source = TZCNT(pinned);
		const BitMask attacks = rook_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.HVPin, target){
			if (queen){queue.push_queen_move<white>(source, TZCNT(target));}
			else {queue.push_rook_move<white>(source, TZCNT(target));}
		}
	}
}

std::tuple<BitMask, BitMask> rook_checks_and_pins(
		const BitMask rooks, const BitMask occ, const Square king){
	BitMask h_check_mask = EMPTY_BOARD;
	BitMask v_check_mask = EMPTY_BOARD;
	BitMask hv_pin = EMPTY_BOARD;

	const BitMask rank = rank_lookup[king];
	Bitloop(rank & rooks, loop_var){
		const Square rook = TZCNT(loop_var);
		const BitMask ext_block_mask = PEXT(occ, rank_blocker_lookup[king]);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[rook % 8][king % 8][ext_block_mask];
		h_check_mask |= PDEP(check, rank);
		hv_pin |= PDEP(pin, rank);
	}

	const BitMask file = file_lookup[king];
	Bitloop(file & rooks, loop_var){
		const Square rook = TZCNT(loop_var);
		const BitMask ext_block_mask = PEXT(occ, file_blocker_lookup[king]);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[rook / 8][king / 8][ext_block_mask];
		v_check_mask |= PDEP(check, file);
		hv_pin |= PDEP(pin, file);
	}

	BitMask check_mask = (h_check_mask ? h_check_mask : FULL_BOARD) &
			(v_check_mask ? v_check_mask : FULL_BOARD);

	return std::make_tuple(check_mask, hv_pin);
}

/* BISHOPS */


template <bool white, bool queen>
void generate_bishop_moves(const Board &board, const ChecksAndPins cnp, MoveQueue &queue){

	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Bishop;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = TZCNT(unpinned);
			const BitMask attacks = bishop_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move<white>(source, TZCNT(target));}
				else {queue.push_bishop_move<white>(source, TZCNT(target));}
			}
		}
	Bitloop(pieces & cnp.DiagPin, pinned){
		const Square source = TZCNT(pinned);
		const BitMask attacks = bishop_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.DiagPin, target){
			if (queen){queue.push_queen_move<white>(source, TZCNT(target));}
			else {queue.push_bishop_move<white>(source, TZCNT(target));}
		}
	}

}

std::tuple<BitMask, BitMask> bishop_checks_and_pins(
		const BitMask bishops, const BitMask occ, const Square king){
	BitMask check_mask = EMPTY_BOARD;
	BitMask diag_pin = EMPTY_BOARD;

	const BitMask upper_diag = upper_diag_lookup[king];
	Bitloop(upper_diag & bishops, loop_var){
		const Square bishop = TZCNT(loop_var);
		const BitMask ext_block_mask = PEXT(occ, upper_blocker_lookup[king]);
		const auto k_index = index_on_upper(king);
		const auto b_index = index_on_upper(bishop);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[b_index][k_index][ext_block_mask];
		check_mask |= PDEP(check, upper_diag);
		diag_pin |= PDEP(pin, upper_diag);
	}

	const BitMask lower_diag = lower_diag_lookup[king];
	Bitloop(lower_diag & bishops, loop_var){
		const Square bishop = TZCNT(loop_var);
		const BitMask ext_block_mask = PEXT(occ, lower_blocker_lookup[king]);
		const auto k_index = index_on_lower(king);
		const auto b_index = index_on_lower(bishop);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[b_index][k_index][ext_block_mask];
		check_mask |= PDEP(check, lower_diag);
		diag_pin |= PDEP(pin, lower_diag);
	}

	return std::make_tuple(check_mask ? check_mask : FULL_BOARD, diag_pin);
}

/* PAWNS */

template <bool white>
constexpr bool is_ep_pin_edge_case(const Square king, const BitMask enemy_rooks, const BitMask occ){
	if (not (EP_RANK<white> & ToMask(king))){
		return false;
	}
	if (not (EP_RANK<white> & enemy_rooks)){
		return false;
	}
	return rook_seen_rank(king, occ) & enemy_rooks;
}

template <bool white, bool include_non_forcing>
void generate_pawn_moves(const Board &board, const ChecksAndPins cnp, MoveQueue &queue){
	const HalfBoard &friendly = get_side<white>(board);
	const HalfBoard &enemy = get_side<not white>(board);

	const BitMask can_push = friendly.Pawn & ~shift_back<white>(board.Occ, 8) & ~cnp.DiagPin &
			(~cnp.HVPin | shift_back<white>(cnp.HVPin, 8));
	Bitloop(can_push & shift_back<white>(cnp.CheckMask, 8), loop_var){
		const Square source = TZCNT(loop_var);
		queue.push_single_pawn_move<white, include_non_forcing>(source);
	}

	if (include_non_forcing) {
		Bitloop(can_push & (white ? RANK_2 : RANK_7) &
				shift_back<white>(cnp.CheckMask & ~board.Occ, 16), loop_var){
			const Square source = TZCNT(loop_var);
			queue.push_double_pawn_move<white>(source);
		}
	}

	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 7) & ~LEFTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin), loop_var){
		const Square source = TZCNT(loop_var);
		queue.push_pawn_capture_left<white, include_non_forcing>(source);
	}
	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 9) & ~RIGHTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin), loop_var){
		const Square source = TZCNT(loop_var);
		queue.push_pawn_capture_right<white, include_non_forcing>(source);
	}

	const BitMask ep_left = friendly.Pawn & shift_forward<white>(cnp.CheckMask & board.EPMask, 1)
			& ~LEFTMOST_FILE<white> & EP_RANK<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin);
	if (ep_left){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_left | board.EPMask))){
			const Square source = TZCNT(ep_left);
			queue.push_ep_capture_left<white>(source);
		}
	}

	const BitMask ep_right = friendly.Pawn & shift_back<white>(cnp.CheckMask & board.EPMask, 1)
			& ~RIGHTMOST_FILE<white> & EP_RANK<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin);
	if (ep_right){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_right | board.EPMask))){
			const Square source = TZCNT(ep_right);
			queue.push_ep_capture_right<white>(source);
		}
	}
}

template <bool white>
BitMask pawn_checks(const BitMask pawns, const Square king){
	const BitMask checking_pawns = pawns & pawn_attacks<not white>(ToMask(king));
	return checking_pawns ? checking_pawns : FULL_BOARD;
}

/* Putting it all together */

template <bool white>
ChecksAndPins checks_and_pins(const Board &board){
	const HalfBoard &enemy = get_side<not white>(board);
	const BitMask friendly_king = get_side<white>(board).King;
	BitMask bishop_checks, bishop_pins, rook_checks, rook_pins;
	std::tie(bishop_checks, bishop_pins) = bishop_checks_and_pins(
			enemy.Bishop | enemy.Queen, board.Occ, friendly_king);
	std::tie(rook_checks, rook_pins) = rook_checks_and_pins(
			enemy.Rook | enemy.Queen, board.Occ, friendly_king);
	const BitMask pawn_checks_ = pawn_checks<not white>(enemy.Pawn, friendly_king);
	const BitMask knight_checks_ = knight_checks(enemy.Knight, friendly_king);
	return ChecksAndPins(pawn_checks_ & knight_checks_ & bishop_checks & rook_checks, rook_pins, bishop_pins);
}

template <bool white>
MoveQueue generate_moves(const Board &board, const ChecksAndPins cnp, const Move hint, const Move killer1, const Move killer2){
	auto queue = MoveQueue(white, board, hint, killer1, killer2);

	generate_pawn_moves<white, true>(board, cnp, queue);
	generate_knight_moves<white>(board, cnp, queue);
	generate_bishop_moves<white, false>(board, cnp, queue);
	generate_rook_moves<white, false>(board, cnp, queue);
	generate_bishop_moves<white, true>(board, cnp, queue);
	generate_rook_moves<white, true>(board, cnp, queue);
	generate_king_moves<white>(board, white ? board.BkAtk.all() : board.WtAtk.all(), queue);

	queue.heapify();
	return queue;
}

template <bool white>
MoveQueue generate_forcing(const Board &board, const ChecksAndPins cnp){
	BitMask enemy_occ = get_side<not white>(board).All;
	auto queue = MoveQueue(white, board);

	BitMask pawn_target = enemy_occ | (white ? RANK_8 : RANK_1);
	generate_pawn_moves<white, false>(board, ChecksAndPins(pawn_target, cnp.HVPin, cnp.DiagPin), queue);

	const auto pawn_defend_pawn = get_side<not white>(board).Pawn & (white ? board.BkAtk.Pawn : board.WtAtk.Pawn);
	auto piece_cnp = ChecksAndPins(enemy_occ & ~pawn_defend_pawn, cnp.HVPin, cnp.DiagPin);
	generate_knight_moves<white>(board, piece_cnp, queue);
	generate_bishop_moves<white, false>(board, piece_cnp, queue);
	generate_rook_moves<white, false>(board, piece_cnp, queue);
	generate_bishop_moves<white, true>(board, piece_cnp, queue);
	generate_rook_moves<white, true>(board, piece_cnp, queue);
	generate_king_moves<white>(board, (white ? board.BkAtk.all() : board.WtAtk.all()) | ~enemy_occ, queue);

	queue.heapify();
	return queue;
}

template ChecksAndPins checks_and_pins<true>(const Board&);
template ChecksAndPins checks_and_pins<false>(const Board&);
template MoveQueue generate_moves<true>(const Board&, const ChecksAndPins, const Move, const Move, const Move);
template MoveQueue generate_moves<false>(const Board&, const ChecksAndPins, const Move, const Move, const Move);
template MoveQueue generate_forcing<true>(const Board&, const ChecksAndPins);
template MoveQueue generate_forcing<false>(const Board&, const ChecksAndPins);
