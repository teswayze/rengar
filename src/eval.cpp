# include "eval.hpp"

template <bool wtm>
inline FirstLayer processed_l1(const Board &board){
	return FirstLayer{
		vector_clamp(board.ue.l1.full_symm),
		vector_clamp((wtm ? vector_add : vector_sub)(board.ue.l1.vert_asym, w_l0_tempo_va)),
		vector_clamp(board.ue.l1.horz_asym),
		vector_clamp(board.ue.l1.rotl_asym),
	};
}


inline SecondLayer l1_l2_transition(const FirstLayer l1){
	auto l2_fs = w_l1_bias_fs;
	l2_fs = vector_add(l2_fs, matmul(w_l1_fs_fs, l1.full_symm));
	l2_fs = vector_add(l2_fs, matmul(w_l1_absva_fs, vector_abs(l1.vert_asym)));
	l2_fs = vector_add(l2_fs, matmul(w_l1_absha_fs, vector_abs(l1.horz_asym)));
	l2_fs = vector_add(l2_fs, matmul(w_l1_absra_fs, vector_abs(l1.rotl_asym)));

	auto l2_va = matmul(w_l1_va_va, l1.vert_asym);
	l2_va = vector_add(l2_va, matmul(w_l1_fsxva_va, vector_mul(l1.full_symm, l1.vert_asym)));
	l2_va = vector_add(l2_va, matmul(w_l1_haxra_va, vector_mul(l1.horz_asym, l1.rotl_asym)));

	return SecondLayer{vector_clamp(l2_fs), vector_clamp(l2_va)};
}


inline int eval_from_l2(const SecondLayer l2){
	return vector_dot(l2.vert_asym, w_l2_va) + vector_dot(vector_mul(l2.full_symm, l2.vert_asym), w_l2_fsxva);
}


template <bool wtm>
ForwardPassOutput forward_pass(const Board &board){
	const auto l1 = processed_l1<wtm>(board);
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
template ForwardPassOutput forward_pass<true>(Board const&);
template ForwardPassOutput forward_pass<false>(Board const&);
