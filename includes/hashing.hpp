# pragma once

# include <cstdint>
# include <array>

const uint64_t prime = (1ull << 32) - 5;
const uint64_t mult = 2654435764ull;
uint64_t state = 1ull;

uint64_t get_rand(){
	state = (state * mult) % prime;
	uint64_t a = state;
	state = (state * mult) % prime;
	uint64_t b = state;
	return (a << 32) | b;
}

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

const std::array<uint64_t, 64> ep_file_hash = {
		get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand(), get_rand()
};
