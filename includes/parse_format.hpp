# pragma once

# include <string>
# include <tuple>

# include "board.hpp"
# include "move.hpp"

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::tuple<bool, Board> parse_fen(std::string fen);
std::string format_move(Move move);
std::string format_move_san(Move move);
std::string format_move_xboard(Move move);
Move parse_move_xboard(std::string move_str, Board board, bool wtm);
std::string format_square(Square square);
std::string format_mask(BitMask mask);
void dump_board(Board board);
