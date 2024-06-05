# include <array>
# include <tuple>
# include "../linalg.hpp"
# include "../eval.hpp"

struct SGDAdjuster{
    __m256i lower8;
    __m256i upper8;
    Vector *params;

    SGDAdjuster(Vector *params);
    void update(const Vector vector_adj, const int scalar_adj);
};


Vector vector_abs_back_prop(const Vector &input, const Vector &output_grad);
Vector vector_clamp_back_prop(const Vector &input, const Vector &output_grad);
std::tuple<Vector, Vector> vector_mul_back_prop(const Vector &input_x, const Vector &input_y, const Vector &output_grad);

Vector vector_dot_back_prop(const Vector &input, SGDAdjuster &weights, const int output_grad, const int learning_rate);
Vector matmul_back_prop(const Vector &input, std::array<SGDAdjuster, 16> &weights, const Vector &output_grad, 
    const int learning_rate);


struct L2Adjuster{
    SGDAdjuster final_va;
    SGDAdjuster final_fsxva;

    L2Adjuster();
    SecondLayer backprop(const SecondLayer &input, const int output_grad, const int learning_rate);
};
