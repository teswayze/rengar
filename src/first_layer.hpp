# include <immintrin.h>
# include <array>
# include "bitboard.hpp"

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

// Only stores the A-D files from black's perspective
extern std::array<FirstLayer, 184> first_layer_weights;

// 24 options for pawns, 32 options for each piece
// If you tried to access a pawn on your own backrank, that would look like a knight on the enemy backrank
// If you tried to access a pawn on the enemy backrank, that would access a negative index
const int pawn_idx_offset = -4;
const int knight_idx_offset = 24;
const int bishop_idx_offset = 56;
const int rook_idx_offset = 88;
const int queen_idx_offset = 120;
const int king_idx_offset = 152;

template <bool white, bool add>
inline void update_first_layer(FirstLayer layer, const Square square, const int piece_idx_offset){
    const bool flip_h = square & 4;
    const int idx = piece_idx_offset
        + 4 * (white ? (7 - square / 8) : (square / 8)) // Rank of square (0 is enemy backrank, 7 is own backrank)
        + flip_h ? (7 - square % 8) : (square % 8); // File of square (0 if A/H, 3 if D/E);
    const FirstLayer to_update = first_layer_weights[idx];

    layer.full_symm = (add ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.full_symm, to_update.full_symm);
    layer.vert_asym = ((add ^ white) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.vert_asym, to_update.vert_asym);
    layer.horz_asym = ((add ^ flip_h) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.horz_asym, to_update.horz_asym);
    layer.rotl_asym = ((add ^ white ^ flip_h) ? _mm256_add_epi16 : _mm256_sub_epi16)(layer.rotl_asym, to_update.rotl_asym);
}
