# pragma once

# include "linalg.hpp"
# include "hashing.hpp"
# include "avx2.hpp"
# include "weights/pst.hpp"

struct FirstLayer{
    // Invariant under rotation and reflection
    // Examples: game phase, position openness, position sharpness
    Vector full_symm;

    // Invariant under horizontal reflection; negates under vertical reflection
    // Examples: material difference, piece activity, pawn structure quality
    // The final evaluation will look like this
    Vector vert_asym;

    // Invariant under vertical reflection; negates under horizontal reflection
    // Examples: castling side if SSC, pawn storm/shield if OSC, relative color complex strength
    Vector horz_asym;

    // Invariant under 180 degree rotation; negates on either reflection
    // Examples: castling side if OSC, pawn chain direction, pawn race orientation
    Vector rotl_asym;
};

struct FirstLayerV2{
    // Invariant under rotation and reflection
    // Examples: game phase, position openness, position sharpness
    ThirtyTwoInt16s full_symm;

    // Invariant under horizontal reflection; negates under vertical reflection
    // Examples: material difference, piece activity, pawn structure quality
    // The final evaluation will look like this
    ThirtyTwoInt16s vert_asym;

    // Invariant under vertical reflection; negates under horizontal reflection
    // Examples: castling side if SSC, pawn storm/shield if OSC, relative color complex strength
    ThirtyTwoInt16s horz_asym;

    // Invariant under 180 degree rotation; negates on either reflection
    // Examples: castling side if OSC, pawn chain direction, pawn race orientation
    ThirtyTwoInt16s rotl_asym;
};

extern std::array<Vector, 184> w_l0_pst_fs;
extern std::array<Vector, 184> w_l0_pst_va;
extern std::array<Vector, 184> w_l0_pst_ha;
extern std::array<Vector, 184> w_l0_pst_ra;

// 24 options for pawns, 32 options for each piece
// If you tried to access a pawn on your own backrank, that would look like a knight on the enemy backrank
// If you tried to access a pawn on the enemy backrank, that would access a negative index
const std::array<int, 6> piece_idx_offsets = {-4, 24, 56, 88, 120, 152};

template <bool white, Piece piece>
inline size_t calculate_pst_idx(const Square square){
    const bool flip_h = square & 4;
    return piece_idx_offsets[piece]
        + 4 * (white ? (7 - square / 8) : (square / 8)) // Rank of square (0 is enemy backrank, 7 is own backrank)
        + (flip_h ? (7 - square % 8) : (square % 8)); // File of square (0 if A/H, 3 if D/E);
}

template <bool white, bool add, Piece piece>
inline void update_first_layer(FirstLayer &layer, const Square square){
    const bool flip_h = square & 4;
    const size_t idx = calculate_pst_idx<white, piece>(square);

    layer.full_symm = (add ? vector_add : vector_sub)(layer.full_symm, w_l0_pst_fs[idx]);
    layer.vert_asym = ((add ^ white) ? vector_sub : vector_add)(layer.vert_asym, w_l0_pst_va[idx]);
    layer.horz_asym = ((add ^ flip_h) ? vector_sub : vector_add)(layer.horz_asym, w_l0_pst_ha[idx]);
    layer.rotl_asym = ((add ^ white ^ flip_h) ? vector_add : vector_sub)(layer.rotl_asym, w_l0_pst_ra[idx]);
}

template <bool white, bool add, Piece piece>
inline void update_first_layer_v2(FirstLayerV2 &layer, const Square square){
    const bool flip_h = square & 4;
    const size_t idx = calculate_pst_idx<white, piece>(square);

    layer.full_symm.maybe_add(pst_fs[idx], add);
    layer.vert_asym.maybe_add(pst_fs[idx], ~add ^ white);
    layer.horz_asym.maybe_add(pst_fs[idx], ~add ^ flip_h);
    layer.rotl_asym.maybe_add(pst_fs[idx], add ^ white ^ flip_h);
}

struct EfficientlyUpdatable{
    FirstLayer l1;
    FirstLayerV2 l1v2;
    uint64_t hash;

    template <bool white, Piece piece>
    void add_piece(const Square square) {
        update_first_layer<white, true, piece>(l1, square);
        update_first_layer_v2<white, true, piece>(l1v2, square);
        hash ^= (white ? white_piece_hashes : black_piece_hashes)[piece][square];
    }
    template <bool white, Piece piece>
    void remove_piece(const Square square) {
        update_first_layer<white, false, piece>(l1, square);
        update_first_layer_v2<white, true, piece>(l1v2, square);
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
