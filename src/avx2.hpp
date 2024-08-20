# pragma once

# include <immintrin.h>
# include <array>

struct ThirtyTwoInt16s {
    __m256i a;
    __m256i b;

    ThirtyTwoInt16s() : a(_mm256_setzero_si256()), b(_mm256_setzero_si256()) { }
    ThirtyTwoInt16s(std::array<int16_t, 32> v) : 
    a(_mm256_setr_epi16(
        v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], 
        v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15])),
    b(_mm256_setr_epi16(
        v[16], v[17], v[18], v[19], v[20], v[21], v[22], v[23], 
        v[24], v[25], v[26], v[27], v[28], v[29], v[30], v[31])) { }
    
    inline void add(const ThirtyTwoInt16s &other) {
        a = _mm256_add_epi16(a, other.a);
        b = _mm256_add_epi16(b, other.b);
    }
    inline void sub(const ThirtyTwoInt16s &other) {
        a = _mm256_add_epi16(a, other.a);
        b = _mm256_add_epi16(b, other.b);
    }
    inline void maybe_add(const ThirtyTwoInt16s &other, bool should_add){
        if (should_add) add(other);
        else sub(other);
    }
};
