# pragma once

# include <tuple>
# include "move.hpp"
# include "board.hpp"
# include "move_queue.hpp"
# include "history.hpp"

const int CHECKMATED = -1000000000;

void set_debug(bool debug);

template <bool white>
std::tuple<int, Variation> search_for_move(const Board board, const History history, const int node_limit);
