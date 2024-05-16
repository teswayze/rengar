# include "selfplay.hpp"
# include "adjudication.hpp"
# include "../search.hpp"
# include "../history.hpp"
# include "../move_queue.hpp"
# include "../hashtable.hpp"

GameData run_selfplay(const Board &starting_board, const GameData book_line, int search_depth){
    Board game_board = starting_board.copy();
    bool wtm = true;
    std::vector<Move> moves;
    History history;
    ht_init(16);
    initialize_move_order_arrays();

    for (Move book_move : book_line.moves) {
        (wtm ? make_move<true> : make_move<false>)(game_board, book_move);
        wtm = not wtm;
        moves.push_back(book_move);
    }

    char result = adjuicate_game(game_board, wtm, history);
    while (result == 'U') {
        Move search_move = (wtm ? search_for_move<true> : search_for_move<false>)
            (game_board, history, INT_MAX, search_depth, INT_MAX, INT_MAX);
        if (is_irreversible(game_board, search_move)){
            history = history.wipe();
        } else {
            history = history.extend_root(game_board.ue.hash);
        }
        (wtm ? make_move<true> : make_move<false>)(game_board, search_move);
        wtm = not wtm;
        moves.push_back(search_move);
        result = adjuicate_game(game_board, wtm, history);
    }

    return GameData{moves, result};
}