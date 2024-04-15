# include "first_layer.hpp"

struct EfficientlyUpdatable{
    FirstLayer l1;
    uint64_t hash;

    template <bool white>
    void add_pawn(const Square square) {
        update_first_layer<white, true>(l1, square, pawn_idx_offset);
        hash ^= (white ? white_pawn_hash : black_pawn_hash)[square];
    }
    template <bool white>
    void remove_pawn(const Square square) {
        update_first_layer<white, false>(l1, square, pawn_idx_offset);
        hash ^= (white ? white_pawn_hash : black_pawn_hash)[square];
    }
    template <bool white>
    void move_pawn(const Square from, const Square to) {
        remove_pawn<white>(from);
        add_pawn<white>(to);
    }

    // TODO: other pieces
    // TODO: promotions
    // TODO: castling
};

// TODO: refresh declaration
