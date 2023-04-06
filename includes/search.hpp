# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "history.hpp"

const int CHECKMATED = -1000000000;

void set_log_level(int level); // 0 is silent; 1 is standard; 2 is debug

template <bool white>
std::tuple<int, Variation> search_for_move(const Board board, const History history, const int node_limit);
