# include "openings.hpp"
# include "parse_format.hpp"

# include <fstream>
# include <sstream>
# include <stdexcept>


BookPosition read_opening_from_line(std::string line){
	Board board = std::get<1>(parse_fen(STARTING_FEN));
	bool wtm = true;
	
	std::istringstream line_stream(line);
	std::string chunk;
	std::string pgn;
	std::getline(line_stream, chunk, ' ');
	while (chunk != "|") {
		Move move = parse_move_xboard(chunk, board, wtm);
		board = (wtm ? make_move<true> : make_move<false>)(board, move);
		wtm = not wtm;
		pgn = pgn + format_move_san(move) + " ";
		std::getline(line_stream, chunk, ' ');
	}
	
	return std::make_tuple(wtm, board, pgn);
}


std::ifstream open_book(const std::string book_name){
	return std::ifstream("openings/" + book_name + ".book");
}


std::list<BookPosition> read_openings_from_file(const std::string book_name){
	std::ifstream file_stream = open_book(book_name);
	
	std::list<BookPosition> pos_list;
	std::string line;
	while (std::getline(file_stream, line)){
		pos_list.push_back(read_opening_from_line(line));
	}
	
	return pos_list;
}


bool has_suffix(const std::string full_string, const std::string suffix) {
    if (full_string.length() >= suffix.length()) {
        return (0 == full_string.compare(full_string.length() - suffix.length(), suffix.length(), suffix));
    } else {
        return false;
    }
}


BookPosition read_specific_opening(const std::string book_name, const std::string opening_name){
	std::ifstream file_stream = open_book(book_name);
	std::string line;
	
	while (std::getline(file_stream, line)){
		if (has_suffix(line, "| " + opening_name)){
			return read_opening_from_line(line);
		}
	}
	throw std::invalid_argument(book_name + ", " + opening_name);
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

TEST_CASE("Read starting position"){
	auto position = read_opening_from_line("| Starting-Position");
	CHECK(std::get<0>(position));
	CHECK(std::get<2>(position) == "");
	CHECK(std::get<1>(position).White.Pawn == RANK_2);
	CHECK(std::get<1>(position).Black.Pawn == RANK_7);
}

TEST_CASE("Read open sicilian"){
	auto position = read_opening_from_line("e2e4 c7c5 g1f3 b8c6 d2d4 c5d4 f3d4 g8f6 b1c3 | Sicilian-Old");
	CHECK(not std::get<0>(position));
	CHECK(std::get<1>(position).White.Knight == (ToMask(C3) | ToMask(D4)));
	CHECK(std::get<1>(position).Black.Knight == (ToMask(C6) | ToMask(F6)));
	CHECK(std::get<2>(position) == "e4 c5 Ng1f3 Nb8c6 d4 cxd4 Nf3d4 Ng8f6 Nb1c3 ");
}

TEST_CASE("Lengths meet expectations"){
	CHECK(read_openings_from_file("A").size() == 1);
	CHECK(read_openings_from_file("B").size() == 2);
	CHECK(read_openings_from_file("C").size() == 5);
	CHECK(read_openings_from_file("D").size() == 10);
	CHECK(read_openings_from_file("E").size() == 20);
	CHECK(read_openings_from_file("F").size() == 50);
}

TEST_CASE("String suffix check helper"){
	CHECK(has_suffix("unary", "nary"));
	CHECK(has_suffix("binary", "nary"));
	CHECK(not has_suffix("tertiary", "nary"));
	CHECK(not has_suffix("ry", "nary"));
}

TEST_CASE("Read specific opeing from file"){
	auto position = read_specific_opening("F", "Caro-Kann-Advance");
	BitMask expected_white_pawn = RANK_2 ^ (ToMask(D2) | ToMask(E2) | ToMask(D4) | ToMask(E5));
	CHECK(std::get<1>(position).White.Pawn == expected_white_pawn);
	BitMask expected_black_pawn = RANK_7 ^ (ToMask(C7) | ToMask(D7) | ToMask(C6) | ToMask(D5));
	CHECK(std::get<1>(position).Black.Pawn == expected_black_pawn);
	CHECK(std::get<2>(position) == "e4 c6 d4 d5 e5 Bc8f5 ");
}

# endif
