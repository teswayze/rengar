# include "board.hpp"

bool is_insufficient_material(const Board &board);
int make_endgame_adjustment(int raw_eval, const Board &board);
int mop_up_evaluation(const Board &board);
