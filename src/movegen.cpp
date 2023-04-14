# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "parse_format.hpp"

# include <exception>

/* KNIGHTS */

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

BitMask knight_attacks(const BitMask knights){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(knights, loop_var){
		attacks |= knight_lookup[SquareOf(loop_var)];
	}
	return attacks;
}

template <bool white>
void generate_knight_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){
	Bitloop(get_side<white>(board).Knight & ~(cnp.HVPin | cnp.DiagPin), knights){
		const Square source = SquareOf(knights);
		const BitMask attacks = knight_lookup[source];
		Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
			queue.push_knight_move(source, SquareOf(target), board);
		}
	}
}

BitMask knight_checks(const BitMask knights, const Square king){
	const BitMask checking_knights = knights & knight_lookup[king];
	return checking_knights ? checking_knights : FULL_BOARD;
}

/* KINGS */

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

BitMask king_attacks(const Square king){ return king_lookup[king]; }

template <bool white>
void generate_king_moves(const Board &board, const BitMask enemy_control, MoveQueue<white> &queue){
	const HalfBoard &friendly = get_side<white>(board);
	const Square king = SquareOf(friendly.King);
	const BitMask attacks = king_lookup[king];
	Bitloop(attacks & ~enemy_control & ~friendly.All, target){
		queue.push_king_move(king, SquareOf(target), board);
	}

	if (friendly.Castle & ToMask(white ? A1 : A8)){
		if (not (board.Occ & (white ? (ToMask(B1) | ToMask(C1) | ToMask(D1)) :
				(ToMask(B8) | ToMask(C8) | ToMask(D8))))){
			if (not (enemy_control & (white ? (ToMask(C1) | ToMask(D1) | ToMask(E1)) :
					(ToMask(C8) | ToMask(D8) | ToMask(E8))))){
				queue.push_castle_qs(board);
			}
		}
	}

	if (friendly.Castle & ToMask(white ? H1 : H8)){
		if (not (board.Occ & (white ? (ToMask(F1) | ToMask(G1)) : (ToMask(F8) | ToMask(G8))))){
			if (not (enemy_control & (white ? (ToMask(E1) | ToMask(F1) | ToMask(G1)) :
					(ToMask(E8) | ToMask(F8) | ToMask(G8))))){
				queue.push_castle_ks(board);
			}
		}
	}
}

/* Slider helpers */

constexpr BitMask compute_slider_attack(const size_t piece_index, const size_t ext_block_mask){
	const BitMask lower_stoppers = (ext_block_mask << 1 & ~ToMask(piece_index)) | ToMask(0);
	const BitMask upper_stoppers = (ext_block_mask << 1 & ~ToMask(piece_index)) | ToMask(7);

	BitMask lower_mask = ToMask(piece_index);
	while (not (lower_mask & lower_stoppers)){
		lower_mask |= lower_mask >> 1;
	}
	BitMask upper_mask = ToMask(piece_index);
	while (not (upper_mask & upper_stoppers)){
		upper_mask |= upper_mask << 1;
	}
	return upper_mask ^ lower_mask;
}

const auto slider_attack_table = lookup_table_2d<BitMask, 8, 64>(compute_slider_attack);


constexpr std::tuple<BitMask, BitMask> compute_slider_cnp(
		const size_t piece_index, const size_t king_index, const size_t ext_block_mask){
	const size_t min = std::min(piece_index, king_index);
	const size_t max = std::max(piece_index, king_index);
	const BitMask between_mask = ToMask(max) - ToMask(min + 1);
	const BitMask between_or_attacking = between_mask | ToMask(piece_index);
	switch (__builtin_popcountll(between_mask & ext_block_mask << 1)){
	case 0: // Check but no pin
		return std::make_tuple(between_or_attacking, 0);
	case 1: // Pin but no check
		return std::make_tuple(0, between_or_attacking);
	default: // No check or pin
		return std::make_tuple(0, 0);
	}
}

const auto slider_cnp_table =
		lookup_table_3d<std::tuple<BitMask, BitMask>, 8, 8, 64>(compute_slider_cnp);


/* ROOKS */

constexpr BitMask rank_of_square(const size_t square){
	return RANK_1 << (8 * (square / 8));
}
const auto rank_lookup = lookup_table<BitMask, 64>(rank_of_square);
constexpr BitMask file_of_square(const size_t square){
	return A_FILE << (square % 8);
}
const auto file_lookup = lookup_table<BitMask, 64>(file_of_square);

constexpr BitMask compute_rank_blocker(const size_t square){
	return rank_of_square(square) & ~(A_FILE | H_FILE);
}
const auto rank_blocker_lookup = lookup_table<BitMask, 64>(compute_rank_blocker);
constexpr BitMask compute_file_blocker(const size_t square){
	return file_of_square(square) & ~(RANK_1 | RANK_8);
}
const auto file_blocker_lookup = lookup_table<BitMask, 64>(compute_file_blocker);

inline BitMask rook_seen_rank(const Square square, const BitMask occ){
	const BitMask ext_block_mask = _pext_u64(occ, rank_blocker_lookup[square]);
	const BitMask ext_seen_mask = slider_attack_table[square % 8][ext_block_mask];
	return _pdep_u64(ext_seen_mask, rank_lookup[square]);
}
inline BitMask rook_seen_file(const Square square, const BitMask occ){
	const BitMask ext_block_mask = _pext_u64(occ, file_blocker_lookup[square]);
	const BitMask ext_seen_mask = slider_attack_table[square / 8][ext_block_mask];
	return _pdep_u64(ext_seen_mask, file_lookup[square]);
}
inline BitMask rook_seen(const Square square, const BitMask occ){
	return rook_seen_rank(square, occ) | rook_seen_file(square, occ);
}

BitMask rook_attacks(const BitMask rooks, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(rooks, loop_var){
		attacks |= rook_seen(SquareOf(loop_var), occ);
	}
	return attacks;
}

template <bool white, bool queen>
void generate_rook_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){
	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Rook;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = SquareOf(unpinned);
			const BitMask attacks = rook_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move(source, SquareOf(target), board);}
				else {queue.push_rook_move(source, SquareOf(target), board);}
			}
		}
	Bitloop(pieces & cnp.HVPin, pinned){
		const Square source = SquareOf(pinned);
		const BitMask attacks = rook_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.HVPin, target){
			if (queen){queue.push_queen_move(source, SquareOf(target), board);}
			else {queue.push_rook_move(source, SquareOf(target), board);}
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
		const Square rook = SquareOf(loop_var);
		const BitMask ext_block_mask = _pext_u64(occ, rank_blocker_lookup[king]);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[rook % 8][king % 8][ext_block_mask];
		h_check_mask |= _pdep_u64(check, rank);
		hv_pin |= _pdep_u64(pin, rank);
	}

	const BitMask file = file_lookup[king];
	Bitloop(file & rooks, loop_var){
		const Square rook = SquareOf(loop_var);
		const BitMask ext_block_mask = _pext_u64(occ, file_blocker_lookup[king]);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[rook / 8][king / 8][ext_block_mask];
		v_check_mask |= _pdep_u64(check, file);
		hv_pin |= _pdep_u64(pin, file);
	}

	BitMask check_mask = (h_check_mask ? h_check_mask : FULL_BOARD) &
			(v_check_mask ? v_check_mask : FULL_BOARD);

	return std::make_tuple(check_mask, hv_pin);
}

/* BISHOPS */

template <bool upper>
constexpr BitMask compute_diagonal_of_square(const size_t square){
	auto shift = upper ? 9 : 7;
	BitMask mask = ToMask(square);
	while (not (mask & ((upper ? H_FILE : A_FILE) | RANK_8))){
		mask |= mask << shift;
	}
	while (not (mask & ((upper ? A_FILE : H_FILE) | RANK_1))){
		mask |= mask >> shift;
	}
	return mask;
}

const auto upper_diag_lookup = lookup_table<BitMask, 64>(compute_diagonal_of_square<true>);
const auto lower_diag_lookup = lookup_table<BitMask, 64>(compute_diagonal_of_square<false>);

template <bool upper>
constexpr BitMask compute_blocker_mask(const size_t square){
	return compute_diagonal_of_square<upper>(square) & ~EDGES;
}

const auto upper_blocker_lookup = lookup_table<BitMask, 64>(compute_blocker_mask<true>);
const auto lower_blocker_lookup = lookup_table<BitMask, 64>(compute_blocker_mask<false>);


constexpr auto index_on_upper(Square square){
	return std::min(square / 8, square % 8);
}
constexpr auto index_on_lower(Square square){
	return std::min(square / 8, 7 - square % 8);
}

inline BitMask bishop_seen_upper(const Square square, const BitMask occ){
	const BitMask ext_block_mask = _pext_u64(occ, upper_blocker_lookup[square]);
	const BitMask ext_seen_mask = slider_attack_table[index_on_upper(square)][ext_block_mask];
	return _pdep_u64(ext_seen_mask, upper_diag_lookup[square]);
}

inline BitMask bishop_seen_lower(const Square square, const BitMask occ){
	const BitMask ext_block_mask = _pext_u64(occ, lower_blocker_lookup[square]);
	const BitMask ext_seen_mask = slider_attack_table[index_on_lower(square)][ext_block_mask];
	return _pdep_u64(ext_seen_mask, lower_diag_lookup[square]);
}

inline BitMask bishop_seen(const Square square, const BitMask occ){
	return bishop_seen_upper(square, occ) | bishop_seen_lower(square, occ);
}


BitMask bishop_attacks(const BitMask bishops, const BitMask occ){
	BitMask attacks = EMPTY_BOARD;
	Bitloop(bishops, loop_var){
		attacks |= bishop_seen(SquareOf(loop_var), occ);
	}
	return attacks;
}


template <bool white, bool queen>
void generate_bishop_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){

	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Bishop;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = SquareOf(unpinned);
			const BitMask attacks = bishop_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move(source, SquareOf(target), board);}
				else {queue.push_bishop_move(source, SquareOf(target), board);}
			}
		}
	Bitloop(pieces & cnp.DiagPin, pinned){
		const Square source = SquareOf(pinned);
		const BitMask attacks = bishop_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.DiagPin, target){
			if (queen){queue.push_queen_move(source, SquareOf(target), board);}
			else {queue.push_bishop_move(source, SquareOf(target), board);}
		}
	}

}

std::tuple<BitMask, BitMask> bishop_checks_and_pins(
		const BitMask bishops, const BitMask occ, const Square king){
	BitMask check_mask = EMPTY_BOARD;
	BitMask diag_pin = EMPTY_BOARD;

	const BitMask upper_diag = upper_diag_lookup[king];
	Bitloop(upper_diag & bishops, loop_var){
		const Square bishop = SquareOf(loop_var);
		const BitMask ext_block_mask = _pext_u64(occ, upper_blocker_lookup[king]);
		const auto k_index = index_on_upper(king);
		const auto b_index = index_on_upper(bishop);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[b_index][k_index][ext_block_mask];
		check_mask |= _pdep_u64(check, upper_diag);
		diag_pin |= _pdep_u64(pin, upper_diag);
	}

	const BitMask lower_diag = lower_diag_lookup[king];
	Bitloop(lower_diag & bishops, loop_var){
		const Square bishop = SquareOf(loop_var);
		const BitMask ext_block_mask = _pext_u64(occ, lower_blocker_lookup[king]);
		const auto k_index = index_on_lower(king);
		const auto b_index = index_on_lower(bishop);
		BitMask check, pin;
		std::tie(check, pin) = slider_cnp_table[b_index][k_index][ext_block_mask];
		check_mask |= _pdep_u64(check, lower_diag);
		diag_pin |= _pdep_u64(pin, lower_diag);
	}

	return std::make_tuple(check_mask ? check_mask : FULL_BOARD, diag_pin);
}

/* PAWNS */

template <bool white>
const BitMask LEFTMOST_FILE = white ? A_FILE : H_FILE;

template <bool white>
const BitMask RIGHTMOST_FILE = white ? H_FILE : A_FILE;


template <bool white>
constexpr BitMask shift_forward(BitMask mask, uint8_t shift){
	return white ? mask << shift : mask >> shift;
}

template <bool white>
constexpr BitMask shift_back(BitMask mask, uint8_t shift){
	return white ? mask >> shift : mask << shift;
}

template <bool white>
constexpr Square shift_sq_forward(Square square, uint8_t shift){
	return white ? square + shift : square - shift;
}

template <bool white>
constexpr BitMask pawn_attacks(const BitMask pawns){
	return shift_forward<white>(pawns & ~LEFTMOST_FILE<white>, 7) |
			shift_forward<white>(pawns & ~RIGHTMOST_FILE<white>, 9);
}

template <bool white>
constexpr bool is_ep_pin_edge_case(const BitMask king, const BitMask enemy_rooks, const BitMask occ){
	const BitMask EP_RANK = white ? RANK_5 : RANK_4;
	if (not (EP_RANK & king)){
		return false;
	}
	if (not (EP_RANK & enemy_rooks)){
		return false;
	}
	return rook_seen_rank(SquareOf(king), occ) & enemy_rooks;
}

template <bool white>
void generate_pawn_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){
	const HalfBoard &friendly = get_side<white>(board);
	const HalfBoard &enemy = get_side<not white>(board);

	const BitMask can_push = friendly.Pawn & ~shift_back<white>(board.Occ, 8) & ~cnp.DiagPin &
			(~cnp.HVPin | shift_back<white>(cnp.HVPin, 8));
	Bitloop(can_push & shift_back<white>(cnp.CheckMask, 8), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_single_pawn_move(source, board);
	}
	Bitloop(can_push & (white ? RANK_2 : RANK_7) &
			shift_back<white>(cnp.CheckMask & ~board.Occ, 16), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_double_pawn_move(source, board);
	}
	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 7) & ~LEFTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_pawn_capture_left(source, board);
	}
	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 9) & ~RIGHTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_pawn_capture_right(source, board);
	}
	const BitMask ep_left = friendly.Pawn & shift_forward<white>(cnp.CheckMask & board.EPMask, 1)
			& ~LEFTMOST_FILE<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin);
	if (ep_left){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_left | board.EPMask))){
			const Square source = SquareOf(ep_left);
			queue.push_ep_capture_left(source, board);
		}
	}
	const BitMask ep_right = friendly.Pawn & shift_back<white>(cnp.CheckMask & board.EPMask, 1)
			& ~RIGHTMOST_FILE<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin);
	if (ep_right){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_right | board.EPMask))){
			const Square source = SquareOf(ep_right);
			queue.push_ep_capture_right(source, board);
		}
	}
}

template <bool white>
BitMask pawn_checks(const BitMask pawns, const BitMask king){
	const BitMask checking_pawns = pawns & pawn_attacks<not white>(king);
	return checking_pawns ? checking_pawns : FULL_BOARD;
}

/* Putting it all together */

template <bool white>
constexpr BitMask enemy_control(const Board &board)
{
	const HalfBoard &enemy = get_side<not white>(board);
	const BitMask friendly_king = get_side<white>(board).King;
	return pawn_attacks<not white>(enemy.Pawn) |
			knight_attacks(enemy.Knight) |
			king_attacks(SquareOf(enemy.King)) |
			bishop_attacks(enemy.Bishop | enemy.Queen, board.Occ & ~friendly_king) |
			rook_attacks(enemy.Rook | enemy.Queen, board.Occ & ~friendly_king);
}

template <bool white>
ChecksAndPins checks_and_pins(const Board &board){
	const HalfBoard &enemy = get_side<not white>(board);
	const BitMask friendly_king = get_side<white>(board).King;
	BitMask bishop_checks, bishop_pins, rook_checks, rook_pins;
	std::tie(bishop_checks, bishop_pins) = bishop_checks_and_pins(
			enemy.Bishop | enemy.Queen, board.Occ, SquareOf(friendly_king));
	std::tie(rook_checks, rook_pins) = rook_checks_and_pins(
			enemy.Rook | enemy.Queen, board.Occ, SquareOf(friendly_king));
	const BitMask pawn_checks_ = pawn_checks<not white>(enemy.Pawn, friendly_king);
	const BitMask knight_checks_ = knight_checks(enemy.Knight, SquareOf(friendly_king));
	return ChecksAndPins(pawn_checks_ & knight_checks_ & bishop_checks & rook_checks, rook_pins, bishop_pins);
}

template <bool white>
MoveQueue<white> generate_moves(const Board &board, const ChecksAndPins cnp, const Move hint, const Move killer1, const Move killer2){
	auto queue = MoveQueue<white>(hint, killer1, killer2);

	generate_pawn_moves<white>(board, cnp, queue);
	generate_knight_moves<white>(board, cnp, queue);
	generate_bishop_moves<white, false>(board, cnp, queue);
	generate_rook_moves<white, false>(board, cnp, queue);
	generate_bishop_moves<white, true>(board, cnp, queue);
	generate_rook_moves<white, true>(board, cnp, queue);
	generate_king_moves<white>(board, enemy_control<white>(board), queue);

	return queue;
}

template <bool white>
MoveQueue<white> generate_forcing(const Board &board, const ChecksAndPins cnp){
	BitMask enemy_occ = get_side<not white>(board).All;
	auto queue = MoveQueue<white>();

	BitMask pawn_target = enemy_occ | (white ? RANK_8 : RANK_1);
	generate_pawn_moves<white>(board, ChecksAndPins(pawn_target, cnp.HVPin, cnp.DiagPin), queue);

	auto piece_cnp = ChecksAndPins(enemy_occ, cnp.HVPin, cnp.DiagPin);
	generate_knight_moves<white>(board, piece_cnp, queue);
	generate_bishop_moves<white, false>(board, piece_cnp, queue);
	generate_rook_moves<white, false>(board, piece_cnp, queue);
	generate_bishop_moves<white, true>(board, piece_cnp, queue);
	generate_rook_moves<white, true>(board, piece_cnp, queue);
	generate_king_moves<white>(board, enemy_control<white>(board) | ~enemy_occ, queue);

	return queue;
}

template ChecksAndPins checks_and_pins<true>(const Board&);
template ChecksAndPins checks_and_pins<false>(const Board&);
template MoveQueue<true> generate_moves<true>(const Board&, const ChecksAndPins, const Move, const Move, const Move);
template MoveQueue<false> generate_moves<false>(const Board&, const ChecksAndPins, const Move, const Move, const Move);
template MoveQueue<true> generate_forcing<true>(const Board&, const ChecksAndPins);
template MoveQueue<false> generate_forcing<false>(const Board&, const ChecksAndPins);
