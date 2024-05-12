# pragma once

# include <array>
# include "board.hpp"
# include "linalg.hpp"

struct SecondLayer{
    Vector full_symm;
    Vector vert_asym;
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
