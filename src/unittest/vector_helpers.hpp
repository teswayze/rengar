# pragma once

# include "doctest.h"
# include <immintrin.h>

inline void check_equal(const __m256i left, const __m256i right){
    int16_t *left_ptr = (int16_t*) &left;
    int16_t *right_ptr = (int16_t*) &right;
    for (auto i = 0; i < 16; i ++) CHECK(left_ptr[i] == right_ptr[i]);
}

inline void check_opposite(const __m256i left, const __m256i right){
    int16_t *left_ptr = (int16_t*) &left;
    int16_t *right_ptr = (int16_t*) &right;
    for (auto i = 0; i < 16; i ++) CHECK(left_ptr[i] == -right_ptr[i]);
}
