# include "hashing.hpp"

const uint64_t prime = (1ull << 32) - 5;
const uint64_t mult = 376447995ull;
uint64_t state;

int reset_rng(){ state = 1ull; return 0; }

uint64_t get_rand(){
	state = (state * mult) % prime;
	uint64_t a = state;
	state = (state * mult) % prime;
	uint64_t b = state;
	return (a << 32) | b;
}
