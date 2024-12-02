# pragma once

# include <array>
# include "board.hpp"
# include "external/Eigen/Core"

struct SecondLayer{
    Eigen::Vector4f full_symm;
    Eigen::Vector4f vert_asym;
};

struct ForwardPassOutput{
    FirstLayer l1;
    SecondLayer l2;
    int eval;
};

template <bool wtm>
ForwardPassOutput forward_pass(const Board &board);

template <bool wtm>
int eval(const Board &board);

void set_mop_up_mode(const bool activate_mop_up_mode);
bool is_mop_up_mode();
