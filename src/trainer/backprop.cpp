# include "backprop.hpp"

# define PERMUTE_CONTROL 0b11'01'10'00

inline __m256i _add_8x32_with_saturation(__m256i x, __m256i y){
    const __m256i x_sign = _mm256_srai_epi32(x, 31);
    const __m256i y_sign = _mm256_srai_epi32(y, 31);
    const __m256i raw_sum = _mm256_add_epi32(x, y);
    const __m256i sum_sign = _mm256_srai_epi32(raw_sum, 31);

    const __m256i no_pos_overflow = _mm256_blendv_epi8(
        _mm256_set1_epi32(-1),
        _mm256_blendv_epi8(y_sign, _mm256_set1_epi32(-1), x_sign),
        sum_sign
    );
    const __m256i neg_overflow = _mm256_blendv_epi8(
        _mm256_blendv_epi8(_mm256_setzero_si256(), y_sign, x_sign),
        _mm256_setzero_si256(),
        sum_sign
    );

    return _mm256_blendv_epi8(
        _mm256_blendv_epi8(_mm256_set1_epi32(INT32_MAX), raw_sum, no_pos_overflow),
        _mm256_set1_epi32(-(1023 << 21)),
        neg_overflow
    );
}

void SGDAdjuster::update(const Vector vector_adj, const int scalar_adj){
    const __m256i scalar_repeated = _mm256_set1_epi32(scalar_adj);
    const __m256i vector_permuted = _mm256_permute4x64_epi64(vector_adj, PERMUTE_CONTROL);
    
    const __m256i lower8_adjust = _mm256_mullo_epi32(
        _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vector_permuted)), scalar_repeated
    );
    lower8 = _add_8x32_with_saturation(lower8, lower8_adjust);

    const __m256i upper8_adjust = _mm256_mullo_epi32(
        _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vector_permuted, 1)), scalar_repeated
    );
    upper8 = _add_8x32_with_saturation(upper8, upper8_adjust);

    *params = _mm256_packs_epi32(_mm256_srai_epi32(lower8, 21), _mm256_srai_epi32(upper8, 21));
}


inline __m256i _scale_for_init(const __m256i unscaled){
    return _mm256_add_epi32(_mm256_set1_epi32(1 << 20), _mm256_slli_epi32(unscaled, 21));
}

SGDAdjuster::SGDAdjuster(Vector *params) : params(params) {
    const __m256i permuted = _mm256_permute4x64_epi64(*params, PERMUTE_CONTROL);
    const __m256i lower8_unscaled = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(permuted));
    const __m256i upper8_unscaled = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(permuted, 1));
    lower8 = _scale_for_init(lower8_unscaled);
    upper8 = _scale_for_init(upper8_unscaled);
}

template <size_t n>
inline std::array<SGDAdjuster, n> init_sgd_adjuster_array(std::array<Vector, n> params){
    std::array<SGDAdjuster, n> adjuster_arr;
    for (size_t i = 0; i < n; i++) adjuster_arr[i] = SGDAdjuster(params.data() + i);
    return adjuster_arr;
}


Vector vector_abs_back_prop(const Vector &input, const Vector &output_grad){
    return _mm256_sign_epi16(output_grad, input);
}
Vector vector_clamp_back_prop(const Vector &input, const Vector &output_grad){
    const Vector neg_1_if_clamped = vector_sub(_mm256_set1_epi16(1022), vector_abs(input));
    return _mm256_blendv_epi8(output_grad, vector_zero, _mm256_srai_epi16(neg_1_if_clamped, 15));
}
std::tuple<Vector, Vector> vector_mul_back_prop(const Vector &input_x, const Vector &input_y, const Vector &output_grad){
    const Vector clamped_output_grad = vector_clamp(output_grad);
    return std::make_tuple(vector_mul(input_y, clamped_output_grad), vector_mul(input_x, clamped_output_grad));
}

Vector vector_dot_back_prop(const Vector &input, SGDAdjuster &weights, const int output_grad, const int learning_rate){
    weights.update(input, output_grad * learning_rate);
    return vector_mul(*weights.params, _mm256_set1_epi16(output_grad));
}
template <int i>
inline Vector _matmul_back_prop_helper(const Vector &input, SGDAdjuster *weights, const uint16_t *output_grad, 
const int learning_rate){
    if (i == 1) return vector_dot_back_prop(input, *weights, *output_grad, learning_rate);
    return vector_add(
        _matmul_back_prop_helper<i / 2>(input, weights, output_grad, learning_rate),
        _matmul_back_prop_helper<i / 2>(input, weights + i / 2, output_grad + i / 2, learning_rate)
        );
}
Vector matmul_back_prop(const Vector &input, std::array<SGDAdjuster, 16> &weights, const Vector &output_grad, 
const int learning_rate){
    const Vector clamped_output_grad = vector_clamp(output_grad);
    return _matmul_back_prop_helper<16>(input, weights.data(), (uint16_t*) &clamped_output_grad, learning_rate);
}


L2Adjuster::L2Adjuster() : final_va(&w_final_va), final_fsxva(&w_final_fsxva) {}
SecondLayer L2Adjuster::backprop(const SecondLayer &input, const int output_grad, const int learning_rate){
    Vector grad_fs;
    Vector grad_va;

    std::tie(grad_fs, grad_va) = vector_mul_back_prop(input.full_symm, input.vert_asym,
        vector_dot_back_prop(vector_mul(input.full_symm, input.vert_asym), final_fsxva, output_grad, learning_rate)
    );
    grad_va = vector_add(grad_va, vector_dot_back_prop(input.vert_asym, final_va, output_grad, learning_rate));

    return SecondLayer{vector_clamp_back_prop(input.full_symm, grad_fs), vector_clamp_back_prop(input.vert_asym, grad_va)};
}
