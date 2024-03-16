# include "doctest.h"
# include "../hashing.hpp"
# include <vector>
# include <iostream>

size_t ht_size = 1 << 16;
auto seen = std::vector(ht_size, false);

void visit(uint64_t hash){
	size_t key = hash % ht_size;
	CHECK(key);
	CHECK(not seen[key]);
	seen[key] = true;
}

TEST_CASE("No duplicates or zeros in hash units"){
	seen[0] = true;
	
	visit(wtm_hash);
	
	for (auto i = 0; i < 64; i++){
		visit(white_knight_hash[i]);
		visit(white_bishop_hash[i]);
		visit(white_rook_hash[i]);
		visit(white_queen_hash[i]);
		visit(white_king_hash[i]);
		
		visit(black_knight_hash[i]);
		visit(black_bishop_hash[i]);
		visit(black_rook_hash[i]);
		visit(black_queen_hash[i]);
		visit(black_king_hash[i]);
	}
	
	for (auto j = 8; j < 56; j++){
		visit(white_pawn_hash[j]);
		visit(black_pawn_hash[j]);
	}

	visit(white_cqs_hash);
	visit(white_cks_hash);
	visit(black_cqs_hash);
	visit(black_cks_hash);
}
