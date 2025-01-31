# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "history.hpp"

void set_log_level(int level); // 0 is silent; 1 is standard; 2 is debug
void search_stats();
void set_tb_path(std::string syzygy_path);
void set_tb_max_num_pieces(int max_num_pieces);

template <bool white>
std::tuple<Move, int> search_for_move_w_eval(const Board &board, History &history, 
    const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms);

template <bool white>
Move search_for_move(const Board &board, History &history, 
    const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms);
