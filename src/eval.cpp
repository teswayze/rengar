# include "eval.hpp"

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


inline SecondLayer l1_l2_transition(const FirstLayer l1){
	auto l2_fs = w_l2_bias_fs;
	l2_fs = vector_add(l2_fs, matmul(w_l2_fs_fs, l1.full_symm));
	l2_fs = vector_add(l2_fs, matmul(w_l2_absva_fs, vector_abs(l1.vert_asym)));
	l2_fs = vector_add(l2_fs, matmul(w_l2_absha_fs, vector_abs(l1.horz_asym)));
	l2_fs = vector_add(l2_fs, matmul(w_l2_absra_fs, vector_abs(l1.rotl_asym)));

	auto l2_va = matmul(w_l2_va_va, l1.vert_asym);
	l2_va = vector_add(l2_va, matmul(w_l2_fsxva_va, vector_clamp_mul(l1.full_symm, l1.vert_asym)));
	l2_va = vector_add(l2_va, matmul(w_l2_haxra_va, vector_clamp_mul(l1.horz_asym, l1.rotl_asym)));

	return SecondLayer{l2_fs, l2_va};
}


inline int eval_from_l2(const SecondLayer l2){
	return vector_dot(l2.vert_asym, w_final_va) + vector_dot(vector_clamp_mul(l2.full_symm, l2.vert_asym), w_final_fsxva);
}


template <bool wtm>
ForwardPassOutput forward_pass(const Board &board){
	auto l1 = board.ue.l1;
	l1.vert_asym = (wtm ? vector_add : vector_sub)(l1.vert_asym, w_l1_tempo_va);
	const auto l2 = l1_l2_transition(l1);
	const int eval_ = eval_from_l2(l2);
	return ForwardPassOutput{l1, l2, eval_};
}

template <bool wtm>
int eval(const Board &board)
{
	return (wtm ? 1 : -1) * forward_pass<wtm>(board).eval;
}

template int eval<true>(const Board&);
template int eval<false>(const Board&);
