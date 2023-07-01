# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "history.hpp"

const int CHECKMATED = -10000;

void set_log_level(int level); // 0 is silent; 1 is standard; 2 is debug
void search_stats();

template <bool white>
Move search_for_move(const Board &board, const History history, const int node_limit, const int depth_limit, const int time_limit_ms);
