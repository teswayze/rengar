# pragma once

# include <immintrin.h>
# include <array>

using L1Vector = __m256i; // 16 int16s
using L2Vector = __m256i; // 8 int32s

inline L1Vector l1_vector_set(
    const int16_t x0, const int16_t x1, const int16_t x2, const int16_t x3,
    const int16_t x4, const int16_t x5, const int16_t x6, const int16_t x7,
    const int16_t x8, const int16_t x9, const int16_t x10, const int16_t x11,
    const int16_t x12, const int16_t x13, const int16_t x14, const int16_t x15
){ 
    return _mm256_setr_epi16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
}
inline L1Vector vector_add(const L1Vector &x, const L1Vector &y) { return _mm256_add_epi16(x, y); }
inline L1Vector vector_sub(const L1Vector &x, const L1Vector &y) { return _mm256_sub_epi16(x, y); }