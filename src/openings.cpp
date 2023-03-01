# include "openings.hpp"
# include "parse_format.hpp"

# include <fstream>
# include <sstream>


BookPosition read_opening_from_line(std::string line){
	Board board = std::get<1>(parse_fen(STARTING_FEN));
	bool wtm = true;
	
	std::istringstream line_stream(line);
	std::string chunk;
	std::getline(line_stream, chunk, ' ');
	while (chunk != "|") {
		Move move = parse_move_xboard(chunk, board, wtm);
		board = (wtm ? make_move<true> : make_move<false>)(board, move);
		wtm = not wtm;
		std::getline(line_stream, chunk, ' ');
	}
	
	std::getline(line_stream, chunk);
	return std::make_tuple(wtm, board, chunk);
}


std::list<BookPosition> read_openings_from_file(const std::string book_name){
	std::string file_path = "openings/" + book_name + ".txt";
	std::ifstream file_stream(file_path);
	
	std::list<BookPosition> pos_list;
	std::string line;
	while (std::getline(file_stream, line)){
		pos_list.push_back(read_opening_from_line(line));
	}
	
	return pos_list;
}


# include "doctest.h"

TEST_CASE("Read starting position"){
	auto position = read_opening_from_line("| Starting Position");
	CHECK(std::get<0>(position));
	CHECK(std::get<2>(position) == "Starting Position");
	CHECK(std::get<1>(position).White.Pawn == RANK_2);
	CHECK(std::get<1>(position).Black.Pawn == RANK_7);
}

TEST_CASE("Read open sicilian"){
	auto position = read_opening_from_line("e2e4 c7c5 g1f3 b8c6 d2d4 c5d4 f3d4 g8f6 b1c3 | Open Sicilian with 2...Nc6");
	CHECK(not std::get<0>(position));
	CHECK(std::get<1>(position).White.Knight == (ToMask(C3) | ToMask(D4)));
	CHECK(std::get<1>(position).Black.Knight == (ToMask(C6) | ToMask(F6)));
}

TEST_CASE("Lengths meet expectations"){
	CHECK(read_openings_from_file("A").size() == 1);
	CHECK(read_openings_from_file("B").size() == 2);
	CHECK(read_openings_from_file("C").size() == 5);
	CHECK(read_openings_from_file("D").size() == 10);
	CHECK(read_openings_from_file("E").size() == 20);
	CHECK(read_openings_from_file("F").size() == 50);

}
