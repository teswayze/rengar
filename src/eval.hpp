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


extern Vector w_l1_tempo_va;

extern Vector w_l2_bias_fs;
extern Matrix w_l2_fs_fs;
extern Matrix w_l2_absva_fs;
extern Matrix w_l2_absha_fs;
extern Matrix w_l2_absra_fs;

extern Matrix w_l2_va_va;
extern Matrix w_l2_fsxva_va;
extern Matrix w_l2_haxra_va;

extern Vector w_final_va;
extern Vector w_final_fsxva;
