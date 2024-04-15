# pragma once

# include "bitboard.hpp"
# include <cstdint>
# include <array>

int reset_rng();
uint64_t get_rand();

const int _unused = reset_rng();

const uint64_t wtm_hash = get_rand();

const std::array<uint64_t, 64> white_pawn_hash = {
		0, 0, 0, 0, 0, 0, 0, 0,
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		0, 0, 0, 0, 0, 0, 0, 0
};

const std::array<uint64_t, 64> black_pawn_hash = {
		0, 0, 0, 0, 0, 0, 0, 0,
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		0, 0, 0, 0, 0, 0, 0, 0
};

const std::array<uint64_t, 64> white_knight_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> black_knight_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> white_bishop_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> black_bishop_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> white_rook_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> black_rook_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> white_queen_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> black_queen_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> white_king_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const std::array<uint64_t, 64> black_king_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), 
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};

const uint64_t white_cqs_hash = get_rand();
const uint64_t white_cks_hash = get_rand();
const uint64_t black_cqs_hash = get_rand();
const uint64_t black_cks_hash = get_rand();


const std::array white_piece_hashes = {
	white_pawn_hash, white_knight_hash, white_bishop_hash, 
	white_rook_hash, white_queen_hash, white_king_hash,
};
const std::array black_piece_hashes = {
	black_pawn_hash, black_knight_hash, black_bishop_hash, 
	black_rook_hash, black_queen_hash, black_king_hash,
};
