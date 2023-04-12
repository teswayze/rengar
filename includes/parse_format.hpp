# pragma once

# include <string>

# include "board.hpp"

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

bool parse_fen(std::string fen, Board &out_board); // Return value is true if white to move
std::string format_move(Move move);
std::string format_move_san(Move move);
std::string format_move_xboard(Move move);
Move parse_move_xboard(std::string move_str, Board &board, bool wtm);
Move parse_move_san(std::string move_str, Board &board, bool wtm);
std::string format_square(Square square);
std::string format_mask(BitMask mask);
void dump_board(Board &board);
