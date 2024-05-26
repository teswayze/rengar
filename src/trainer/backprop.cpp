# include "backprop.hpp"

# define PERMUTE_CONTROL 0b11'01'10'00


inline __m256i _add_8x32_with_saturation(__m256i x, __m256i y){
    const __m256i raw_sum = _mm256_add_epi32(x, y);
    const __m256i pos_overflow = _mm256_srai_epi32(_mm256_andnot_epi32(_mm256_or_epi32(x, y), raw_sum), 31);
    const __m256i neg_overflow = _mm256_srai_epi32(_mm256_andnot_epi32(raw_sum, _mm256_and_epi32(x, y)), 31);

    return _mm256_blendv_epi8(
        _mm256_blendv_epi8(raw_sum, _mm256_set1_epi32(INT32_MAX), pos_overflow),
        _mm256_set1_epi32(INT32_MIN),
        neg_overflow
    );
}

void SGDAdjuster::update(const Vector vector_adj, const int scalar_adj){
    const __m256i scalar_repeated = _mm256_set1_epi32(scalar_adj);
    const __m256i vector_permuted = _mm256_permute4x64_epi64(vector_adj, PERMUTE_CONTROL);
    
    const __m256i lower8_adjust = _mm256_mullo_epi32(
        _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vector_permuted)), scalar_repeated
    );
    lower8 = _mm256_add_epi32(lower8, lower8_adjust);

    const __m256i upper8_adjust = _mm256_mullo_epi32(
        _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vector_permuted, 1)), scalar_repeated
    );
    upper8 = _mm256_add_epi32(upper8, upper8_adjust);

    *params = _mm256_packs_epi32(_mm256_srai_epi32(lower8, 21), _mm256_srai_epi32(upper8, 21));
}


inline __m256i _scale_for_init(const __m256i unscaled){
    return _mm256_add_epi32(_mm256_set1_epi32(1 << 20), _mm256_slli_epi32(unscaled, 21));
}

SGDAdjuster init_sgd_adjuster(Vector *params){
    const __m256i permuted = _mm256_permute4x64_epi64(*params, PERMUTE_CONTROL);
    const __m256i lower8_unscaled = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(permuted));
    const __m256i upper8_unscaled = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(permuted, 1));
    return SGDAdjuster{_scale_for_init(lower8_unscaled), _scale_for_init(upper8_unscaled), params};
}
