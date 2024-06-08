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
Vector matmul_back_prop(const Vector &input, SGDAdjusterArr<16> &weights, const Vector &output_grad, 
const int learning_rate){
    const Vector clamped_output_grad = vector_clamp(output_grad);
    return _matmul_back_prop_helper<16>(input, weights.data.data(), (uint16_t*) &clamped_output_grad, learning_rate);
}


L2Adjuster::L2Adjuster() : va(&w_l2_va), fsxva(&w_l2_fsxva) {}
SecondLayer L2Adjuster::backprop(const SecondLayer &input, const int output_grad, const int learning_rate){
    Vector grad_fs;
    Vector grad_va;

    std::tie(grad_fs, grad_va) = vector_mul_back_prop(input.full_symm, input.vert_asym,
        vector_dot_back_prop(vector_mul(input.full_symm, input.vert_asym), fsxva, output_grad, learning_rate)
    );
    grad_va = vector_add(grad_va, vector_dot_back_prop(input.vert_asym, va, output_grad, learning_rate));

    return SecondLayer{vector_clamp_back_prop(input.full_symm, grad_fs), vector_clamp_back_prop(input.vert_asym, grad_va)};
}

L1Adjuster::L1Adjuster() : bias_fs(&w_l1_bias_fs), fs_fs(w_l1_fs_fs.data()), 
    absva_fs(w_l1_absva_fs.data()), absha_fs(w_l1_absha_fs.data()), absra_fs(w_l1_absra_fs.data()), 
    va_va(w_l1_va_va.data()), fsxva_va(w_l1_fsxva_va.data()), haxra_va(w_l1_haxra_va.data()) {}
FirstLayer L1Adjuster::backprop(const FirstLayer &input, const SecondLayer &output_grad, const int learning_rate){
    bias_fs.update(output_grad.full_symm, learning_rate);

    Vector grad_fs;
    Vector grad_va;
    Vector grad_ha;
    Vector grad_ra;

    std::tie(grad_fs, grad_va) = vector_mul_back_prop(input.full_symm, input.vert_asym,
        matmul_back_prop(vector_mul(input.full_symm, input.vert_asym), fsxva_va, output_grad.vert_asym, learning_rate)
    );
    std::tie(grad_ha, grad_ra) = vector_mul_back_prop(input.horz_asym, input.rotl_asym,
        matmul_back_prop(vector_mul(input.horz_asym, input.rotl_asym), haxra_va, output_grad.vert_asym, learning_rate)
    );
    grad_va = vector_add(grad_va, matmul_back_prop(input.vert_asym, va_va, output_grad.vert_asym, learning_rate));
    grad_fs = vector_add(grad_fs, matmul_back_prop(input.full_symm, fs_fs, output_grad.full_symm, learning_rate));
    grad_va = vector_add(grad_va, vector_abs_back_prop(input.vert_asym, 
        matmul_back_prop(vector_abs(input.vert_asym), absva_fs, output_grad.full_symm, learning_rate)
    ));
    grad_ha = vector_add(grad_ha, vector_abs_back_prop(input.horz_asym, 
        matmul_back_prop(vector_abs(input.horz_asym), absha_fs, output_grad.full_symm, learning_rate)
    ));
    grad_ra = vector_add(grad_ra, vector_abs_back_prop(input.rotl_asym, 
        matmul_back_prop(vector_abs(input.rotl_asym), absra_fs, output_grad.full_symm, learning_rate)
    ));

    return FirstLayer{
        vector_clamp_back_prop(input.full_symm, grad_fs), vector_clamp_back_prop(input.vert_asym, grad_va),
        vector_clamp_back_prop(input.horz_asym, grad_ha), vector_clamp_back_prop(input.rotl_asym, grad_ra),
    };
}

L0Adjuster::L0Adjuster() : pst_fs(w_l0_pst_fs.data()), pst_va(w_l0_pst_va.data()), pst_ha(w_l0_pst_ha.data()),
    pst_ra(w_l0_pst_ra.data()), tempo_va(&w_l0_tempo_va) {}

template <bool white, Piece piece>
inline void L0Adjuster::backprop_piece_mask(const BitMask mask, const FirstLayer &output_grad, const int learning_rate){
    Bitloop(mask, x){
        const Square square = TZCNT(x);
        const bool flip_h = square & 4;
        const size_t idx = calculate_pst_idx<white, piece>(square);

        pst_fs.data[idx].update(output_grad.full_symm, learning_rate);
        pst_va.data[idx].update(output_grad.vert_asym, white ? -learning_rate : learning_rate);
        pst_ha.data[idx].update(output_grad.horz_asym, flip_h ? -learning_rate : learning_rate);
        pst_ra.data[idx].update(output_grad.rotl_asym, (white ^ flip_h) ? -learning_rate : learning_rate);
    }
}
template <bool white>
inline void L0Adjuster::backprop_half_board(const HalfBoard &hb, const FirstLayer &output_grad, const int learning_rate){
    backprop_piece_mask<white, PAWN>(hb.Pawn, output_grad, learning_rate);
    backprop_piece_mask<white, KNIGHT>(hb.Knight, output_grad, learning_rate);
    backprop_piece_mask<white, BISHOP>(hb.Bishop, output_grad, learning_rate);
    backprop_piece_mask<white, ROOK>(hb.Rook, output_grad, learning_rate);
    backprop_piece_mask<white, QUEEN>(hb.Queen, output_grad, learning_rate);
    backprop_piece_mask<white, KING>(ToMask(hb.King), output_grad, learning_rate);
}
void L0Adjuster::backprop(const bool wtm, const Board &board, const FirstLayer &output_grad, const int learning_rate){
    tempo_va.update(output_grad.vert_asym, wtm ? learning_rate : -learning_rate);
    backprop_half_board<true>(board.White, output_grad, learning_rate);
    backprop_half_board<false>(board.Black, output_grad, learning_rate);
}
