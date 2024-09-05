# pragma once

# include <tuple>
# include "board.hpp"
# include "move_queue.hpp"
# include "history.hpp"

const int CHECKMATED = -10000;

void set_log_level(int level); // 0 is silent; 1 is standard; 2 is debug
void search_stats();

template <bool white>
std::tuple<Move, int> search_for_move_w_eval(const Board &board, History &history, 
    const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms);

template <bool white>
Move search_for_move(const Board &board, History &history, 
    const int node_limit, const int depth_limit, const int min_time_ms, const int max_time_ms){
        return std::get<0>(search_for_move_w_eval<white>(
            board, history, node_limit, depth_limit, min_time_ms, max_time_ms));
    }
