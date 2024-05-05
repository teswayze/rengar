# pragma once

# include <immintrin.h>
# include "hashing.hpp"

struct FirstLayer{
    // Invariant under rotation and reflection
    // Examples: game phase, position openness, position sharpness
    __m256i full_symm;

    // Invariant under horizontal reflection; negates under vertical reflection
    // Examples: material difference, piece activity, pawn structure quality
    // The final evaluation will look like this
    __m256i vert_asym;

    // Invariant under vertical reflection; negates under horizontal reflection
    // Examples: castling side if SSC, pawn storm/shield if OSC, relative color complex strength
    __m256i horz_asym;

    // Invariant under 180 degree rotation; negates on either reflection
    // Examples: castling side if OSC, pawn chain direction, pawn race orientation
    __m256i rotl_asym;
};

extern std::array<FirstLayer, 184> first_layer_weights;

// 24 options for pawns, 32 options for each piece
// If you tried to access a pawn on your own backrank, that would look like a knight on the enemy backrank
// If you tried to access a pawn on the enemy backrank, that would access a negative index
const std::array<int, 6> piece_idx_offsets = {-4, 24, 56, 88, 120, 152};

template <bool white, bool add, Piece piece>
inline void update_first_layer(FirstLayer &layer, const Square square){
    const bool flip_h = square & 4;
    const int idx = piece_idx_offsets[piece]
        + 4 * (white ? (7 - square / 8) : (square / 8)) // Rank of square (0 is enemy backrank, 7 is own backrank)
        + (flip_h ? (7 - square % 8) : (square % 8)); // File of square (0 if A/H, 3 if D/E);
    const FirstLayer to_update = first_layer_weights[idx];

    layer.full_symm = (add ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.full_symm, to_update.full_symm);
    layer.vert_asym = ((add ^ white) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.vert_asym, to_update.vert_asym);
    layer.horz_asym = ((add ^ flip_h) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.horz_asym, to_update.horz_asym);
    layer.rotl_asym = ((add ^ white ^ flip_h) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.rotl_asym, to_update.rotl_asym);
}

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
