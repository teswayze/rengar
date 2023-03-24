# include "hashing.hpp"

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"
# include "movegen.hpp"
# include "parse_format.hpp"
# include <iostream>

const uint64_t mask = (1ull << 16) - 1;

void check_all_white_captures(Square to, uint64_t to_check){
	CHECK(to_check != (mask & black_pawn_hash[to]));
	CHECK(to_check != (mask & black_knight_hash[to]));
	CHECK(to_check != (mask & black_bishop_hash[to]));
	CHECK(to_check != (mask & black_rook_hash[to]));
	if (to == A8){ CHECK(to_check != (mask & (black_rook_hash[to] ^ black_cqs_hash))); }
	if (to == H8){ CHECK(to_check != (mask & (black_rook_hash[to] ^ black_cks_hash))); }
	CHECK(to_check != (mask & black_queen_hash[to]));
}

void check_all_black_captures(Square to, uint64_t to_check){
	CHECK(to_check != (mask & white_pawn_hash[to]));
	CHECK(to_check != (mask & white_knight_hash[to]));
	CHECK(to_check != (mask & white_bishop_hash[to]));
	CHECK(to_check != (mask & white_rook_hash[to]));
	if (to == A1){ CHECK(to_check != (mask & (white_rook_hash[to] ^ white_cqs_hash))); }
	if (to == H1){ CHECK(to_check != (mask & (white_rook_hash[to] ^ white_cks_hash))); }
	CHECK(to_check != (mask & white_queen_hash[to]));
}

TEST_CASE("White knight move doesn't lead to collision"){
	for (Square from = 0; from < 64; from++){
		INFO("From square ", format_square(from));
		Bitloop(knight_lookup[from], all_targets){
			Square to = SquareOf(all_targets);
			INFO("To square ", format_square(to));
			uint64_t to_check = (wtm_hash ^ white_knight_hash[from] ^ white_knight_hash[to]) & mask;
			CHECK(to_check);
			check_all_white_captures(to, to_check);
		}
	}
}

TEST_CASE("Black knight move doesn't lead to collision"){
	for (Square from = 0; from < 64; from++){
		INFO("From square ", format_square(from));
		Bitloop(knight_lookup[from], all_targets){
			Square to = SquareOf(all_targets);
			INFO("To square ", format_square(to));
			uint64_t to_check = (wtm_hash ^ black_knight_hash[from] ^ black_knight_hash[to]) & mask;
			CHECK(to_check);
			check_all_black_captures(to, to_check);
		}
	}
}

TEST_CASE("White king move doesn't lead to collision"){
	for (Square from = 0; from < 64; from++){
		INFO("From square ", format_square(from));
		Bitloop(king_lookup[from], all_targets){
			Square to = SquareOf(all_targets);
			INFO("To square ", format_square(to));
			uint64_t to_check = (wtm_hash ^ white_king_hash[from] ^ white_king_hash[to]) & mask;
			CHECK(to_check);
			check_all_white_captures(to, to_check);
			
			if (from == E1) {
				CHECK(to_check != (white_cqs_hash & mask));
				check_all_white_captures(to, to_check ^ (white_cqs_hash & mask));
				CHECK(to_check != (white_cks_hash & mask));
				check_all_white_captures(to, to_check ^ (white_cks_hash & mask));
				CHECK(to_check != ((white_cqs_hash ^ white_cks_hash) & mask));
				check_all_white_captures(to, to_check ^ ((white_cqs_hash ^ white_cks_hash) & mask));
			}
		}
	}
}

TEST_CASE("Black king move doesn't lead to collision"){
	for (Square from = 0; from < 64; from++){
		INFO("From square ", format_square(from));
		Bitloop(king_lookup[from], all_targets){
			Square to = SquareOf(all_targets);
			INFO("To square ", format_square(to));
			uint64_t to_check = (wtm_hash ^ black_king_hash[from] ^ black_king_hash[to]) & mask;
			CHECK(to_check);
			check_all_black_captures(to, to_check);
			
			if (from == E1) {
				CHECK(to_check != (black_cqs_hash & mask));
				check_all_black_captures(to, to_check ^ (black_cqs_hash & mask));
				CHECK(to_check != (black_cks_hash & mask));
				check_all_black_captures(to, to_check ^ (black_cks_hash & mask));
				CHECK(to_check != ((black_cqs_hash ^ black_cks_hash) & mask));
				check_all_black_captures(to, to_check ^ ((black_cqs_hash ^ black_cks_hash) & mask));
			}
		}
	}
}

# endif
