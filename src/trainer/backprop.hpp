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
struct SGDAdjuster16{
    std::array<SGDAdjuster, 16> data;

    SGDAdjuster16(Vector *params);
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
    SGDAdjuster16 fs_fs;
    SGDAdjuster16 absva_fs;
    SGDAdjuster16 absha_fs;
    SGDAdjuster16 absra_fs;
    SGDAdjuster16 va_va;
    SGDAdjuster16 fsxva_va;
    SGDAdjuster16 haxra_va;

    L1Adjuster();
    FirstLayer backprop(const FirstLayer &input, const SecondLayer &output_grad, const int learning_rate);
};
