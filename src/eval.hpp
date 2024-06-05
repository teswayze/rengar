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


extern Vector w_l0_tempo_va;

extern Vector w_l1_bias_fs;
extern Matrix w_l1_fs_fs;
extern Matrix w_l1_absva_fs;
extern Matrix w_l1_absha_fs;
extern Matrix w_l1_absra_fs;

extern Matrix w_l1_va_va;
extern Matrix w_l1_fsxva_va;
extern Matrix w_l1_haxra_va;

extern Vector w_l2_va;
extern Vector w_l2_fsxva;
