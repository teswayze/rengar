# include <array>
# include "../linalg.hpp"

struct SGDAdjuster{
    __m256i lower8;
    __m256i upper8;
    Vector *params;

    void update(const Vector vector_adj, const int scalar_adj);
};

SGDAdjuster init_sgd_adjuster(Vector *params);
template <size_t n>
inline std::array<SGDAdjuster, n> init_sgd_adjuster_array(std::array<Vector, n> params){
    std::array<SGDAdjuster, n> adjuster_arr;
    for (size_t i = 0; i < n; i++) adjuster_arr[i] = init_sgd_adjuster(params.data() + i);
    return adjuster_arr;
}


Vector vector_dot_back_prop(const Vector input, SGDAdjuster weights, const int output_grad, const int learning_rate);
