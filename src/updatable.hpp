# include "first_layer.hpp"
# include "hashing.hpp"

struct EfficientlyUpdatable{
    FirstLayer l1;
    uint64_t hash;

    template <bool white, Piece piece>
    void add_piece(const Square square) {
        update_first_layer<white, true, piece>(l1, square);
        hash ^= (white ? white_piece_hashes : black_piece_hashes)[piece][square];
    }
    template <bool white, Piece piece>
    void remove_piece(const Square square) {
        update_first_layer<white, false, piece>(l1, square);
        hash ^= (white ? white_piece_hashes : black_piece_hashes)[piece][square];
    }
    template <bool white, Piece piece>
    void move_piece(const Square from, const Square to) {
        remove_piece<white, piece>(from);
        add_piece<white, piece>(to);
    }
    template <bool white, Piece piece>
	void promote_pawn(const Square from, const Square to){
        remove_piece<white, PAWN>(from);
        add_piece<white, piece>(to);
    }
    template <bool white>
    void castle_queenside(){
        move_piece<white, KING>(white ? E1 : E8, white ? C1 : C8);
        move_piece<white, ROOK>(white ? A1 : A8, white ? D1 : D8);
    }
    template <bool white>
    void castle_kingside(){
        move_piece<white, KING>(white ? E1 : E8, white ? G1 : G8);
        move_piece<white, ROOK>(white ? H1 : H8, white ? F1 : F8);
    }
};

EfficientlyUpdatable initialize_ue(const HalfBoard &white, const HalfBoard &black);
