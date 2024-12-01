# include "eval.hpp"
# include "weights/hidden.hpp"
# include "endgames.hpp"

# define vector_clamp(x) ((x).cwiseMin(0.9921875).cwiseMax(-0.9921875))


template <bool wtm>
inline FirstLayer processed_l1(const Board &board){
	return FirstLayer(
		vector_clamp(board.ue.l1.full_symm),
		wtm ? vector_clamp(board.ue.l1.vert_asym + w_l0_tempo_va).eval() : vector_clamp(board.ue.l1.vert_asym - w_l0_tempo_va).eval(),
		vector_clamp(board.ue.l1.horz_asym),
		vector_clamp(board.ue.l1.rotl_asym)
	);
}


inline SecondLayer l1_l2_transition(const FirstLayer &l1){
	auto l2_fs = (
		w_l1_fs_bias +
		w_l1_fs * l1.full_symm +
		w_l1_absva * l1.vert_asym.cwiseAbs() +
		w_l1_absha * l1.horz_asym.cwiseAbs() +
		w_l1_absra * l1.rotl_asym.cwiseAbs()
	);

	auto l2_va = (
		w_l1_va * l1.vert_asym +
		w_l1_fsxva * l1.full_symm.cwiseProduct(l1.vert_asym) +
		w_l1_haxra * l1.horz_asym.cwiseProduct(l1.rotl_asym)
	);

	return SecondLayer{vector_clamp(l2_fs), l2_va};
}


inline int eval_from_l2(const SecondLayer l2){
	float nn_output = w_l2_va.dot(l2.vert_asym) + w_l2_fsxva.dot(l2.full_symm.cwiseProduct(l2.vert_asym));
	return (int) (nn_output * 128);
}


template <bool wtm>
ForwardPassOutput forward_pass(const Board &board){
	const auto l1 = processed_l1<wtm>(board);
	const auto l2 = l1_l2_transition(l1);
	const int eval_ = eval_from_l2(l2);
	return ForwardPassOutput{l1, l2, eval_};
}

bool mop_up_mode_active = false;
void set_mop_up_mode(const bool activate_mop_up_mode) {
	mop_up_mode_active = activate_mop_up_mode;
}
bool is_mop_up_mode(){ return mop_up_mode_active; }


template <bool wtm>
int eval(const Board &board)
{
	const int sign = wtm ? 1 : -1;
	if (mop_up_mode_active) return mop_up_evaluation(board) * sign;
	const int result = forward_pass<wtm>(board).eval;
	if (only_has_minor(result >= 0 ? board.White : board.Black)) return 0;
	return sign * result;
}

template int eval<true>(const Board&);
template int eval<false>(const Board&);
template ForwardPassOutput forward_pass<true>(Board const&);
template ForwardPassOutput forward_pass<false>(Board const&);
