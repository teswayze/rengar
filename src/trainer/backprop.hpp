# include <array>
# include <tuple>
# include "../linalg.hpp"
# include "../eval.hpp"

struct SGDAdjuster{
    __m256i lower8;
    __m256i upper8;
    Vector *params;

    SGDAdjuster() = default;
    SGDAdjuster(Vector *params);
    void update(const Vector vector_adj, const int scalar_adj);
};
template <size_t n>
struct SGDAdjusterArr{
    std::array<SGDAdjuster, n> data;

    SGDAdjusterArr(Vector *params){ for (size_t i = 0; i < n; i++) data[i] = SGDAdjuster(params + i); }
};

Vector vector_abs_back_prop(const Vector &input, const Vector &output_grad);
Vector vector_clamp_back_prop(const Vector &input, const Vector &output_grad);
std::tuple<Vector, Vector> vector_mul_back_prop(const Vector &input_x, const Vector &input_y, const Vector &output_grad);

Vector vector_dot_back_prop(const Vector &input, SGDAdjuster &weights, const int output_grad, const int learning_rate);
Vector matmul_back_prop(const Vector &input, std::array<SGDAdjuster, 16> &weights, const Vector &output_grad, 
    const int learning_rate);


struct L2Adjuster{
    SGDAdjuster va;
    SGDAdjuster fsxva;

    L2Adjuster();
    SecondLayer backprop(const SecondLayer &input, const int output_grad, const int learning_rate);
};

struct L1Adjuster{
    SGDAdjuster bias_fs;
    SGDAdjusterArr<16> fs_fs;
    SGDAdjusterArr<16> absva_fs;
    SGDAdjusterArr<16> absha_fs;
    SGDAdjusterArr<16> absra_fs;
    SGDAdjusterArr<16> va_va;
    SGDAdjusterArr<16> fsxva_va;
    SGDAdjusterArr<16> haxra_va;

    L1Adjuster();
    FirstLayer backprop(const FirstLayer &input, const SecondLayer &output_grad, const int learning_rate);
};

struct L0Adjuster{
    SGDAdjusterArr<184> pst_fs;    
    SGDAdjusterArr<184> pst_va;
    SGDAdjusterArr<184> pst_ha;
    SGDAdjusterArr<184> pst_ra;
    SGDAdjuster tempo_va;

    L0Adjuster();
    template <bool white, Piece piece>
    inline void backprop_piece_mask(const BitMask mask, const FirstLayer &output_grad, const int learning_rate);
    template <bool white>
    inline void backprop_half_board(const HalfBoard &hb, const FirstLayer &output_grad, const int learning_rate);
    void backprop(const bool wtm, const Board &board, const FirstLayer &output_grad, const int learning_rate);
};
