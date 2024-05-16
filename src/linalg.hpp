# pragma once

# include <immintrin.h>
# include <array>

// Everything here is represented by 16-bit integers
// The common pattern is that a value of 256 encodes a value of 1
// This means if we multiply by two things we must divide 256 at the end
using Vector = __m256i;
using Matrix = std::array<Vector, 16>;

const Vector vector_zero = _mm256_setzero_si256();

// Initialization
inline Vector vector_set(
    const int16_t x0, const int16_t x1, const int16_t x2, const int16_t x3,
    const int16_t x4, const int16_t x5, const int16_t x6, const int16_t x7,
    const int16_t x8, const int16_t x9, const int16_t x10, const int16_t x11,
    const int16_t x12, const int16_t x13, const int16_t x14, const int16_t x15
){ 
    return _mm256_setr_epi16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
}

// Basic arithmetic
inline Vector vector_add(const Vector &x, const Vector &y) { return _mm256_adds_epi16(x, y); }
inline Vector vector_sub(const Vector &x, const Vector &y) { return _mm256_subs_epi16(x, y); }

// Activation
inline Vector vector_abs(const Vector &x) { return _mm256_abs_epi16(x); }

inline Vector _vector_clamp_helper(const Vector &x) {
    return _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(x, 2), _mm256_set1_epi16(-181)), _mm256_set1_epi16(181));
}
inline Vector vector_clamp_mul(const Vector &x, const Vector &y) { // Clamp x and y to +/- 2*sqrt(2), then multiply
    return _mm256_srai_epi16(_mm256_mullo_epi16(_vector_clamp_helper(x), _vector_clamp_helper(y)), 4);
}

// Linear algebra
inline int vector_dot(const Vector &x, const Vector &y){
    const auto to_sum8 = _mm256_madd_epi16(x, y);
    const auto to_sum4 = _mm_add_epi32(_mm256_castsi256_si128(to_sum8), _mm256_extracti128_si256(to_sum8, 1));
    return (_mm_extract_epi32(to_sum4, 0) + _mm_extract_epi32(to_sum4, 1) 
            + _mm_extract_epi32(to_sum4, 2) + _mm_extract_epi32(to_sum4, 3)) / 256;
}

inline __m128i _matmul_helper(const Vector *M, const Vector &x) {
    const auto M0x = _mm256_madd_epi16(M[0], x);
    const auto M1x = _mm256_madd_epi16(M[1], x);
    const auto M2x = _mm256_madd_epi16(M[2], x);
    const auto M3x = _mm256_madd_epi16(M[3], x);

    const auto sum = _mm256_hadd_epi32(_mm256_hadd_epi32(M0x, M1x), _mm256_hadd_epi32(M2x, M3x));
    return _mm_add_epi32(_mm256_castsi256_si128(sum), _mm256_extracti128_si256(sum, 1));
}
inline __m256i _matmul_helper2(const Vector *M, const Vector &x) {
    return _mm256_srai_epi32(_mm256_set_m128i(_matmul_helper(M, x), _matmul_helper(M, x + 4)), 8);
}
inline Vector matmul(const Matrix &M, const Vector &x){
    return _mm256_packs_epi32(_matmul_helper2(M.data(), x), _matmul_helper2(M.data() + 8, x));
}

inline int16_t* begin(Vector &x) { return (int16_t*)&x; }
inline int16_t* end(Vector &x) { return begin(x) + 16; }

// For debugging/testing
struct VectorBoundaries{
    const int16_t *begin_;
    const int16_t *end_;

    const int16_t* begin() const { return begin_; }
    const int16_t* end() const { return end_; }
};
inline VectorBoundaries vector_iterator(const Vector &x){
    const int16_t *begin = (int16_t *) &x;
    return VectorBoundaries{begin, begin + 16};
}
