# include "hashing.hpp"

template <bool white>
uint64_t half_board_hash(
		const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const BitMask king, const BitMask castle
		){
    uint64_t hash = 0ull;

    Bitloop(pawn, x){ hash ^= (white ? white_pawn_hash : black_pawn_hash)[SquareOf(x)]; }
    Bitloop(knight, x){ hash ^= (white ? white_knight_hash : black_knight_hash)[SquareOf(x)]; }
    Bitloop(bishop, x){ hash ^= (white ? white_bishop_hash : black_bishop_hash)[SquareOf(x)]; }
    Bitloop(rook, x){ hash ^= (white ? white_rook_hash : black_rook_hash)[SquareOf(x)]; }
    Bitloop(queen, x){ hash ^= (white ? white_queen_hash : black_queen_hash)[SquareOf(x)]; }
    hash ^= (white ? white_king_hash : black_king_hash)[SquareOf(king)];

    if (white and (castle & ToMask(A1))){ hash ^= white_cqs_hash; }
    if (white and (castle & ToMask(H1))){ hash ^= white_cks_hash; }
    if ((not white) and (castle & ToMask(A8))){ hash ^= black_cqs_hash; }
    if ((not white) and (castle & ToMask(H8))){ hash ^= black_cks_hash; }

    return hash;
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include <vector>

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
	
	for (auto k = 0; k < 8; k++){
		visit(ep_file_hash[k]);
	}
}

# endif
