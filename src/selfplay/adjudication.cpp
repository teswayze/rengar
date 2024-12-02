# include "adjudication.hpp"
# include "../endgames.hpp"
# include "../movegen.hpp"

char adjuicate_game(const Board &board, bool wtm, const HistoryView &history){
    // Insufficient material -> draw
    if (is_insufficient_material(board)) return 'D';
    // Fifty move rule -> draw
    if (history.curr_idx - history.irreversible_idx >= 100) return 'D';
    // Threefold repetition -> draw
    if (history.index_of_repetition(board.ue.hash, false) != -1) return 'D';

    auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    auto queue = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);
    if (queue.empty()){
        // Stalemate -> draw
        if (cnp.CheckMask == FULL_BOARD) return 'D';
        // Checkmate -> loss for side to move
        return wtm ? 'B' : 'W';
    }
    
    return 'U';
}
