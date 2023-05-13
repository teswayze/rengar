# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "movegen.hpp"
# include "parse_format.hpp"
# include "attacks.hpp"

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
			queue.push_knight_move(source, SquareOf(target), get_side<not white>(board));
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
		queue.push_king_move(king, SquareOf(target), get_side<not white>(board));
	}

	if (friendly.Castle & ToMask(white ? A1 : A8)){
		if (not (board.Occ & (white ? (ToMask(B1) | ToMask(C1) | ToMask(D1)) :
				(ToMask(B8) | ToMask(C8) | ToMask(D8))))){
			if (not (enemy_control & (white ? (ToMask(C1) | ToMask(D1) | ToMask(E1)) :
					(ToMask(C8) | ToMask(D8) | ToMask(E8))))){
				queue.push_castle_qs();
			}
		}
	}

	if (friendly.Castle & ToMask(white ? H1 : H8)){
		if (not (board.Occ & (white ? (ToMask(F1) | ToMask(G1)) : (ToMask(F8) | ToMask(G8))))){
			if (not (enemy_control & (white ? (ToMask(E1) | ToMask(F1) | ToMask(G1)) :
					(ToMask(E8) | ToMask(F8) | ToMask(G8))))){
				queue.push_castle_ks();
			}
		}
	}
}

/* ROOKS */

template <bool white, bool queen>
void generate_rook_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){
	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Rook;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = SquareOf(unpinned);
			const BitMask attacks = rook_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move(source, SquareOf(target), get_side<not white>(board));}
				else {queue.push_rook_move(source, SquareOf(target), get_side<not white>(board));}
			}
		}
	Bitloop(pieces & cnp.HVPin, pinned){
		const Square source = SquareOf(pinned);
		const BitMask attacks = rook_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.HVPin, target){
			if (queen){queue.push_queen_move(source, SquareOf(target), get_side<not white>(board));}
			else {queue.push_rook_move(source, SquareOf(target), get_side<not white>(board));}
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


template <bool white, bool queen>
void generate_bishop_moves(const Board &board, const ChecksAndPins cnp, MoveQueue<white> &queue){

	const BitMask pieces = queen ? get_side<white>(board).Queen : get_side<white>(board).Bishop;
	Bitloop(pieces & ~(cnp.HVPin | cnp.DiagPin), unpinned){
			const Square source = SquareOf(unpinned);
			const BitMask attacks = bishop_seen(source, board.Occ);
			Bitloop(attacks & cnp.CheckMask & ~get_side<white>(board).All, target){
				if (queen){queue.push_queen_move(source, SquareOf(target), get_side<not white>(board));}
				else {queue.push_bishop_move(source, SquareOf(target), get_side<not white>(board));}
			}
		}
	Bitloop(pieces & cnp.DiagPin, pinned){
		const Square source = SquareOf(pinned);
		const BitMask attacks = bishop_seen(source, board.Occ);
		Bitloop(attacks & cnp.CheckMask & cnp.DiagPin, target){
			if (queen){queue.push_queen_move(source, SquareOf(target), get_side<not white>(board));}
			else {queue.push_bishop_move(source, SquareOf(target), get_side<not white>(board));}
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
const BitMask EP_RANK = white ? RANK_5 : RANK_4;


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
	if (not (EP_RANK<white> & king)){
		return false;
	}
	if (not (EP_RANK<white> & enemy_rooks)){
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
		queue.push_single_pawn_move(source);
	}
	Bitloop(can_push & (white ? RANK_2 : RANK_7) &
			shift_back<white>(cnp.CheckMask & ~board.Occ, 16), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_double_pawn_move(source);
	}
	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 7) & ~LEFTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_pawn_capture_left(source, enemy);
	}
	Bitloop(friendly.Pawn & shift_back<white>(cnp.CheckMask & enemy.All, 9) & ~RIGHTMOST_FILE<white> &
			~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin), loop_var){
		const Square source = SquareOf(loop_var);
		queue.push_pawn_capture_right(source, enemy);
	}
	const BitMask ep_left = friendly.Pawn & shift_forward<white>(cnp.CheckMask & board.EPMask, 1)
			& ~LEFTMOST_FILE<white> & EP_RANK<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 7) | ~cnp.DiagPin);
	if (ep_left){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_left | board.EPMask))){
			const Square source = SquareOf(ep_left);
			queue.push_ep_capture_left(source);
		}
	}
	const BitMask ep_right = friendly.Pawn & shift_back<white>(cnp.CheckMask & board.EPMask, 1)
			& ~RIGHTMOST_FILE<white> & EP_RANK<white> & ~cnp.HVPin & (shift_back<white>(cnp.DiagPin, 9) | ~cnp.DiagPin);
	if (ep_right){
		if (not is_ep_pin_edge_case<white>(friendly.King, enemy.Rook | enemy.Queen,
				board.Occ & ~(ep_right | board.EPMask))){
			const Square source = SquareOf(ep_right);
			queue.push_ep_capture_right(source);
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

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "parse_format.hpp"
# include <vector>
# include <algorithm>
# include <iostream>

template <bool white>
void check_board_has_expected_legal_moves(const Board &board, const std::vector<std::string> move_strings){
	auto cnp = checks_and_pins<white>(board);
	auto queue = generate_moves<white>(board, cnp, 0, 0, 0);

	std::vector<Move> moves;
	for (size_t i = 0; i < move_strings.size(); i++){
		moves.insert(moves.end(), parse_move_xboard(move_strings[i], board, white));
	}

	Move illegal_move = 0;
	for (; !queue.empty(); queue.pop()){
		Move move = queue.top();
		auto move_loc = std::find(moves.begin(), moves.end(), move);
		if (move_loc == moves.end()){ illegal_move = move; }
		else { moves.erase(move_loc); }
	}
	CHECK(illegal_move == 0);
	CHECK(moves.size() == 0);
	if (moves.size()) std::cout << moves[0] << std::endl;
}


TEST_CASE("Starting position"){
	Board board;
	parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	check_board_has_expected_legal_moves<true>(board, {
			"a2a3", "a2a4", "b2b3", "b2b4", "c2c3", "c2c4", "d2d3", "d2d4", "e2e3", "e2e4",
			"f2f3", "f2f4", "g2g3", "g2g4", "h2h3", "h2h4", "b1a3", "b1c3", "g1f3", "g1h3",
	});
}

TEST_CASE("Diagonal check from opera game"){
	Board board;
	parse_fen("rn2kb1r/p3qppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/R3K2R b KQkq - 0 11", board);
	check_board_has_expected_legal_moves<false>(board, {"b8c6", "b8d7", "f6d7", "e7d7", "e8d8"});
}

TEST_CASE("Lateral check from opera game"){
	Board board;
	parse_fen("1Q2kb1r/p2n1ppp/4q3/4p1B1/4P3/8/PPP2PPP/2KR4 b k - 1 16", board);
	check_board_has_expected_legal_moves<false>(board, {"d7b8"});
}

TEST_CASE("Pinned knight from opera game"){
	Board board;
	parse_fen("r3kb1r/p2nqppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/2KR3R b kq - 2 12", board);
	check_board_has_expected_legal_moves<false>(board, {
			"a7a6", "a7a5", "g7g6", "h7h6", "h7h5", "f6d5", "f6e4", "f6g4", "f6g8", "f6h5",
			"a8b8", "a8c8", "a8d8", "h8g8", "e7a3", "e7b4", "e7c5", "e7d6", "e7d8", "e7e6",
			"e8d8", "e8c8",
	});
}

TEST_CASE("Pinned rook from opera game"){
	Board board;
	parse_fen("4kb1r/p2rqppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/2KR4 b k - 1 14", board);
	check_board_has_expected_legal_moves<false>(board, {
			"a7a6", "a7a5", "g7g6", "h7h6", "h7h5", "f6d5", "f6e4", "f6g4", "f6g8", "f6h5",
			"h8g8", "e7a3", "e7b4", "e7c5", "e7d6", "e7d8", "e7e6", "e8d8",
	});
}

TEST_CASE("Pinned pawn from Harmon v Borgov"){
	Board board;
	parse_fen("r5k1/2pb1pp1/3b2np/1p1Pp1q1/4Pn2/3BBPP1/1PQ1NN1P/2R3K1 w - - 8 25", board);
	check_board_has_expected_legal_moves<true>(board, {
			"b2b3", "b2b4", "g3g4", "h2h3", "h2h4", "e2c3", "e2d4", "e2f4", "f2d1", "f2g4",
			"f2h3", "f2h1", "d3b5", "d3c4", "e3a7", "e3b6", "e3c5", "e3d4", "e3d2", "e3f4",
			"c1a1", "c1b1", "c1d1", "c1e1", "c1f1", "c2a4", "c2b3", "c2b1", "c2c3", "c2c4",
			"c2c5", "c2c6", "c2c7", "c2d2", "c2d1", "g1f1", "g1h1",
	});
}

TEST_CASE("Pawn promotion from Harmon v Borgov"){
	Board board;
	parse_fen("5R2/4P1k1/7p/1pBpp2q/1P2r3/6PP/6K1/8 w - - 3 49", board);
	check_board_has_expected_legal_moves<true>(board, {
			"e7e8q", "e7e8n", "e7e8r", "e7e8b", "g3g4", "h3h4", "c5b6", "c5a7", "c5d6", "c5d4",
			"c5e3", "c5f2", "c5g1", "f8f1", "f8f2", "f8f3", "f8f4", "f8f5", "f8f6", "f8f7",
			"f8a8", "f8b8", "f8c8", "f8d8", "f8e8", "f8g8", "f8h8", "g2f1", "g2f2", "g2g1",
			"g2h1", "g2h2",
	});
}

TEST_CASE("Pinned queen from Berlin Airlift"){
	Board board;
	parse_fen("4k3/pp5p/2p5/2pP2Pn/8/N1Pr1pp1/PP2q3/R1Q1R2K b - - 2 27", board);
	check_board_has_expected_legal_moves<false>(board, {
			"a7a6", "a7a5", "b7b6", "b7b5", "c6d5", "c5c4", "f3f2", "g3g2", "h7h6", "h5f4",
			"h5f6", "h5g7", "d3c3", "d3d4", "d3d5", "d3d2", "d3d1", "d3e3", "e2e1", "e2e3",
			"e2e4", "e2e5", "e2e6", "e2e7", "e8d8", "e8d7", "e8e7", "e8f7", "e8f8",
	});
}

TEST_CASE("Double check from Evergreen Game"){
	Board board;
	parse_fen("1r4r1/pbpknp1p/1b3P2/5B2/8/B1P2q2/P4PPP/3R2K1 b - - 1 22", board);
	check_board_has_expected_legal_moves<false>(board, { "d7c6", "d7e8" });
}

TEST_CASE("En passant legal from Italian Game"){
	Board board;
	parse_fen("r1bqk2r/ppp2ppp/2n2n2/2bpP3/2Bp4/2P2N2/PP3PPP/RNBQK2R w KQkq d6 0 7", board);
	check_board_has_expected_legal_moves<true>(board, {
			"a2a3", "a2a4", "b2b3", "b2b4", "c3d4", "e5d6", "e5e6", "e5f6", "g2g3", "g2g4",
			"h2h3", "h2h4", "b1a3", "b1d2", "f3d2", "f3d4", "f3g5", "f3h4", "f3g1", "c4a6",
			"c4b5", "c4b3", "c4d5", "c4d3", "c4e2", "c4f1", "c1d2", "c1e3", "c1f4", "c1g5",
			"c1h6", "h1f1", "h1g1", "d1a4", "d1b3", "d1c2", "d1d2", "d1d3", "d1d4", "d1e2",
			"e1d2", "e1e2", "e1f1", "e1g1",
	});
}

TEST_CASE("En passant illegal from Italian Game"){
	Board board;
	parse_fen("r1bqk2r/ppp2ppp/2n5/1BbpP3/3pn3/2P2N2/PP3PPP/RNBQK2R w KQkq - 2 8", board);
	check_board_has_expected_legal_moves<true>(board, {
			"a2a3", "a2a4", "b2b3", "b2b4", "c3c4", "c3d4", "e5e6", "g2g3", "g2g4", "h2h3",
			"h2h4", "b1a3", "b1d2", "f3d2", "f3d4", "f3g5", "f3h4", "f3g1", "b5a6", "b5a4",
			"b5c6", "b5c4", "b5d3", "b5e2", "b5f1", "c1d2", "c1e3", "c1f4", "c1g5", "c1h6",
			"h1f1", "h1g1", "d1a4", "d1b3", "d1c2", "d1d2", "d1d3", "d1d4", "d1e2", "e1e2",
			"e1f1", "e1g1",
	});
}

TEST_CASE("Castling illegal from Kasparov's Immortal"){
	Board board;
	parse_fen("r1bqk2r/p2npp1p/2pp1npQ/1p6/3PP3/2N2P2/PPP1N1PP/R3KB1R b KQkq - 0 9", board);
	check_board_has_expected_legal_moves<false>(board, {
			"a7a6", "a7a5", "b5b4", "c6c5", "d6d5", "e7e6", "e7e5", "g6g5", "d7b8", "d7b6",
			"d7c5", "d7e5", "d7f8", "f6d5", "f6e4", "f6g4", "f6h5", "f6g8", "c8b7", "c8a6",
			"a8b8", "h8g8", "h8f8", "d8c7", "d8b6", "d8a5",
	});
}

TEST_CASE("Pawn check from Kasparov's Immortal"){
	Board board;
	parse_fen("b2r3r/4Rp1p/p2q1np1/kp1P4/1P1Q4/P4PPB/2P4P/1K6 b - - 0 27", board);
	check_board_has_expected_legal_moves<false>(board, { "a5a4", "d6b4" });
}

TEST_CASE("Pawn check brilliancy from Kasparov's Immortal"){
	Board board;
	parse_fen("3r3r/1R3p1p/Q5p1/1p6/1kq5/2P2PPB/7P/1K6 b - - 0 33", board);
	check_board_has_expected_legal_moves<false>(board, { "b4b3", "b4c3", "b4c5", "c4c3" });
}

# endif
