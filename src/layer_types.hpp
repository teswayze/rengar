# pragma once

# include <immintrin.h>

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
