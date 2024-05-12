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
inline Vector matmul(const Matrix &M, const Vector x);
inline int vector_dot(const Vector &x, const Vector &y);
