# pragma once

# include "bitboard.hpp"
# include "lookup.hpp"

# include <tuple>

struct Attacks {
	BitMask Pawn;
	BitMask Knight;
	BitMask Bishop;
	BitMask Rook;
	BitMask Queen;
	BitMask King;

	BitMask all() const { return Pawn | Knight | Bishop | Rook | Queen | King; }

	Attacks() = default;
	Attacks(const Attacks&) = delete;

	Attacks copy() const {
		return Attacks{ Pawn, Knight, Bishop, Rook, Queen, King };
	}
};

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


/* KNIGHTS */

constexpr BitMask knight_attacks(const BitMask knights){
	return ((knights >> 17 | knights << 15) & ~H_FILE) |
			((knights >> 15 | knights << 17) & ~A_FILE) |
			((knights >> 10 | knights << 6) & ~(H_FILE | G_FILE)) |
			((knights >> 6 | knights << 10) & ~(A_FILE | B_FILE));
}

constexpr BitMask compute_knight_moves(const size_t square){
	return knight_attacks(ToMask(square));
}
const auto knight_lookup = lookup_table<BitMask, 64>(compute_knight_moves);


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

constexpr BitMask king_attacks(const Square king){ return king_lookup[king]; }


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
