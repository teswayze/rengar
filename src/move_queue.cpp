# include <tuple>
# include <queue>
# include "board.hpp"
# include "pst.hpp"
# include "eval.hpp"

const int REALLY_BIG_EVAL = 100000;
const int ONE_PAWN = 100 * PC_TOTAL;

template <bool white>
struct MoveQueue{
	MoveQueue(const Move hint, const Move killer1, const Move killer2) :
		Hint(hint), Killer1(killer1), Killer2(killer2) { }
	MoveQueue() : Hint(0), Killer1(0), Killer2(0) { }

	bool empty() const{ return Queue.empty(); }
	Move top() const{ return std::get<1>(Queue.top()); }
	PstEvalInfo top_eval_info() const{ return std::get<2>(Queue.top()); }
	void pop(){ Queue.pop(); }

	void push_knight_move(const Square from, const Square to, const Board &board){
		push_move_helper(move_from_squares(from, to, KNIGHT_MOVE), board);
	}
	void push_bishop_move(const Square from, const Square to, const Board &board){
		push_move_helper(move_from_squares(from, to, BISHOP_MOVE), board);
	}
	void push_rook_move(const Square from, const Square to, const Board &board){
		push_move_helper(move_from_squares(from, to, ROOK_MOVE), board);
	}
	void push_queen_move(const Square from, const Square to, const Board &board){
		push_move_helper(move_from_squares(from, to, QUEEN_MOVE), board);
	}
	void push_king_move(const Square from, const Square to, const Board &board){
		push_move_helper(move_from_squares(from, to, KING_MOVE), board);
	}
	void push_castle_qs(const Board &board){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, C8), CASTLE_QUEENSIDE);
		push_move_helper(move, board);
	}
	void push_castle_ks(const Board &board){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, G8), CASTLE_KINGSIDE);
		push_move_helper(move, board);
	}
	void push_single_pawn_move(const Square from, const Board &board){
		pawn_maybe_promote_helper(from, white ? (from + 8) : (from - 8), SINGLE_PAWN_PUSH, board);
	}
	void push_double_pawn_move(const Square from, const Board &board){
		push_move_helper(move_from_squares(from, white ? (from + 16) : (from - 16), DOUBLE_PAWN_PUSH), board);
	}
	void push_pawn_capture_left(const Square from, const Board &board){
		pawn_maybe_promote_helper(from, white ? (from + 7) : (from - 7), PAWN_CAPTURE, board);
	}
	void push_pawn_capture_right(const Square from, const Board &board){
		pawn_maybe_promote_helper(from, white ? (from + 9) : (from - 9), PAWN_CAPTURE, board);
	}
	void push_ep_capture_left(const Square from, const Board &board){
		push_move_helper(move_from_squares(from, white ? (from + 7) : (from - 7), EN_PASSANT_CAPTURE), board);
	}
	void push_ep_capture_right(const Square from, const Board &board){
		push_move_helper(move_from_squares(from, white ? (from + 9) : (from - 9), EN_PASSANT_CAPTURE), board);
	}

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

		inline void push_move_helper(const Move move, const Board &board){
			const PstEvalInfo eval_info = adjust_eval<white>(board.EvalInfo, compute_eval_diff_for_move<white>(board, move));
			const int eval_value = (white ? 1 : -1) * eval_from_info(eval_info) + match_bonus(move);
			Queue.push(std::make_tuple(eval_value, move, eval_info));
		}

		inline void pawn_maybe_promote_helper(const Square from, const Square to, const MoveFlags flag_if_not_promote, const Board &board){
			if (white ? (to >= A8) : (to <= H1)){
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_KNIGHT), board);
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_BISHOP), board);
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_ROOK), board);
				push_move_helper(move_from_squares(from, to, PROMOTE_TO_QUEEN), board);
			} else {
				push_move_helper(move_from_squares(from, to, flag_if_not_promote), board);
			}
		}
};

template struct MoveQueue<true>;
template struct MoveQueue<false>;
