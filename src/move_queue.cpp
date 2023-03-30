# include <tuple>
# include <queue>
# include "board.hpp"
# include "pst.hpp"
# include "eval.hpp"

const int REALLY_BIG_EVAL = 100000;
const int ONE_PAWN = 2400;

template <bool white>
struct MoveQueue{
	MoveQueue(const Board board, const Move hint, const Move killer1, const Move killer2) :
		Brd(board), CurrInfo(board.EvalInfo), Hint(hint), Killer1(killer1), Killer2(killer2) { }

	bool empty() const{ return Queue.empty(); }
	Move top() const{ return std::get<1>(Queue.top()); }
	PstEvalInfo top_eval_info() const{ return std::get<2>(Queue.top()); }
	void pop(){ Queue.pop(); }

	void push_knight_move(const Square from, const Square to){
		push_move_helper(move_from_squares(from, to, KNIGHT_MOVE));
	}
	void push_bishop_move(const Square from, const Square to){
		push_move_helper(move_from_squares(from, to, BISHOP_MOVE));
	}
	void push_rook_move(const Square from, const Square to){
		push_move_helper(move_from_squares(from, to, ROOK_MOVE));
	}
	void push_queen_move(const Square from, const Square to){
		push_move_helper(move_from_squares(from, to, QUEEN_MOVE));
	}
	void push_king_move(const Square from, const Square to){
		push_move_helper(move_from_squares(from, to, KING_MOVE));
	}
	void push_castle_qs(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, C8), CASTLE_QUEENSIDE);
		push_move_helper(move);
	}
	void push_castle_ks(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, G8), CASTLE_KINGSIDE);
		push_move_helper(move);
	}
	void push_single_pawn_move(const Square from){
		pawn_maybe_promote_helper(from, white ? (from + 8) : (from - 8), SINGLE_PAWN_PUSH);
	}
	void push_double_pawn_move(const Square from){
		push_move_helper(move_from_squares(from, white ? (from + 16) : (from - 16), DOUBLE_PAWN_PUSH));
	}
	void push_pawn_capture_left(const Square from){
		pawn_maybe_promote_helper(from, white ? (from + 7) : (from - 7), PAWN_CAPTURE);
	}
	void push_pawn_capture_right(const Square from){
		pawn_maybe_promote_helper(from, white ? (from + 9) : (from - 9), PAWN_CAPTURE);
	}
	void push_ep_capture_left(const Square from){
		push_move_helper(move_from_squares(from, white ? (from + 7) : (from - 7), EN_PASSANT_CAPTURE));
	}
	void push_ep_capture_right(const Square from){
		push_move_helper(move_from_squares(from, white ? (from + 9) : (from - 9), EN_PASSANT_CAPTURE));
	}

	const Board Brd;
	const PstEvalInfo CurrInfo;
	std::priority_queue<std::tuple<int, Move, PstEvalInfo>> Queue;
	const Move Hint;
	const Move Killer1;
	const Move Killer2;

	private:
		constexpr int match_bonus(const Move move){
			if (move == Hint) return REALLY_BIG_EVAL;
			if (move == Killer1) return ONE_PAWN;
			if (move == Killer2) return ONE_PAWN;
			return 0;
		}

		inline void push_move_helper(const Move move){
			const PstEvalInfo eval_info = adjust_eval<white>(CurrInfo, compute_eval_diff_for_move<white>(Brd, move));
			const int eval_value = (white ? 1 : -1) * eval_from_info(eval_info) + match_bonus(move);
			Queue.push(std::make_tuple(eval_value, move, eval_info));
		}

		inline void pawn_maybe_promote_helper(const Square from, const Square to, const MoveFlags flag_if_not_promote){
			if (white ? (to >= A8) : (to <= H1)){
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_KNIGHT));
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_BISHOP));
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_ROOK));
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_QUEEN));
			} else {
				push_move_helper(move_from_squares(from, to, flag_if_not_promote));
			}
		}
};

template struct MoveQueue<true>;
template struct MoveQueue<false>;
