# include "board.hpp"
# include "eval.hpp"
# include "pst.hpp"
# include "endgames.hpp"
# include "eval_param.hpp"

EVAL_PARAM(mg_bishop_atk, 3)
EVAL_PARAM(mg_rook_atk, 3)
EVAL_PARAM(mg_queen_atk, -1)
EVAL_PARAM(mg_tempo, 8)
EVAL_PARAM(eg_bishop_atk, 6)
EVAL_PARAM(eg_rook_atk, 4)
EVAL_PARAM(eg_queen_atk, 5)
EVAL_PARAM(eg_tempo, -3)
EVAL_PARAM(eval_divisor, 285)

template <bool wtm>
int eval(const Board &board)
{
	const int sign = wtm ? 1 : -1;
	if ((not board.White.Pawn) and (not board.Black.Pawn)) return mop_up_evaluation(board) * sign;

	const auto &info = board.EvalInfo;
	const int bishop_atk_cnt = __builtin_popcountll(board.WtAtk.Bishop) - __builtin_popcountll(board.BkAtk.Bishop);
	const int rook_atk_cnt = __builtin_popcountll(board.WtAtk.Rook) - __builtin_popcountll(board.BkAtk.Rook);
	const int queen_atk_cnt = __builtin_popcountll(board.WtAtk.Queen) - __builtin_popcountll(board.BkAtk.Queen);

	const auto mg_pst_eval = (board.White.King % 8 >= 4) ? 
		((board.Black.King % 8 >= 4) ? info.mg_kk : info.mg_kq) :
		((board.Black.King % 8 >= 4) ? info.mg_qk : info.mg_qq);
	const int mg_eval = mg_pst_eval + mg_bishop_atk * bishop_atk_cnt + mg_rook_atk * rook_atk_cnt + mg_queen_atk * queen_atk_cnt + mg_tempo * sign;
	const int eg_eval = info.eg + eg_bishop_atk * bishop_atk_cnt + eg_rook_atk * rook_atk_cnt + eg_queen_atk * queen_atk_cnt + eg_tempo * sign;
    const int raw_eval = eg_eval * 256 + info.phase_count * (mg_eval - eg_eval);

    return sign * make_endgame_adjustment(raw_eval, board) / eval_divisor;
}

template int eval<true>(const Board&);
template int eval<false>(const Board&);
