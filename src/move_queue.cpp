# include <tuple>
# include <queue>
# include "board.hpp"
# include "move.hpp"
# include "eval.hpp"

# include <iostream>

const int REALLY_BIG_EVAL = 100000;

template <bool white>
struct MoveQueue{
	MoveQueue(const Board board, const Move hint) :
		Enemy(get_side<not white>(board)), Phase(phase_of_game(board)), Hint(hint) { }

	bool empty() const{ return Queue.empty(); }
	Move top() const{ return std::get<1>(Queue.top()); }
	void pop(){ Queue.pop(); }

	void push_null_move(){
		Queue.push(std::make_tuple(0, 0));
	}
	void push_knight_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, KNIGHT_MOVE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_knight_table[FlipIf(white, to)] - mg_knight_table[FlipIf(white, from)];
		const int pst_diff_eg = eg_knight_table[FlipIf(white, to)] - eg_knight_table[FlipIf(white, from)];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + capture_bonus(to);
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_bishop_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, BISHOP_MOVE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_bishop_table[FlipIf(white, to)] - mg_bishop_table[FlipIf(white, from)];
		const int pst_diff_eg = eg_bishop_table[FlipIf(white, to)] - eg_bishop_table[FlipIf(white, from)];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + capture_bonus(to);
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_rook_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, ROOK_MOVE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_rook_table[FlipIf(white, to)] - mg_rook_table[FlipIf(white, from)];
		const int pst_diff_eg = eg_rook_table[FlipIf(white, to)] - eg_rook_table[FlipIf(white, from)];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + capture_bonus(to);
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_queen_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, QUEEN_MOVE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_queen_table[FlipIf(white, to)] - mg_queen_table[FlipIf(white, from)];
		const int pst_diff_eg = eg_queen_table[FlipIf(white, to)] - eg_queen_table[FlipIf(white, from)];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + capture_bonus(to);
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_king_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, KING_MOVE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_king_table[FlipIf(white, to)] - mg_king_table[FlipIf(white, from)];
		const int pst_diff_eg = eg_king_table[FlipIf(white, to)] - eg_king_table[FlipIf(white, from)];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + capture_bonus(to);
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_castle_qs(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, C8), CASTLE_QUEENSIDE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_king_table[C8] + mg_rook_table[D8] - mg_king_table[E8] - mg_rook_table[A8];
		const int pst_diff_eg = eg_king_table[C8] + eg_rook_table[D8] - eg_king_table[E8] - eg_rook_table[A8];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg;
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_castle_ks(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, G8), CASTLE_KINGSIDE);
		if (move == Hint) { Queue.push(std::make_tuple(REALLY_BIG_EVAL, move)); return; }
		const int pst_diff_mg = mg_king_table[G8] + mg_rook_table[F8] - mg_king_table[E8] - mg_rook_table[H8];
		const int pst_diff_eg = eg_king_table[G8] + eg_rook_table[F8] - eg_king_table[E8] - eg_rook_table[H8];
		const int eval_diff = Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg;
		Queue.push(std::make_tuple(eval_diff, move));
	}
	void push_single_pawn_move(const Square from){
		pawn_move_helper<false, false>(from, white ? (from + 8) : (from - 8));
	}
	void push_double_pawn_move(const Square from){
		pawn_move_helper<false, true>(from, white ? (from + 16) : (from - 16));
	}
	void push_pawn_capture_left(const Square from){
		pawn_move_helper<true, false>(from, white ? (from + 7) : (from - 7));
	}
	void push_pawn_capture_right(const Square from){
		pawn_move_helper<true, false>(from, white ? (from + 9) : (from - 9));
	}
	void push_ep_capture_left(const Square from){
		pawn_move_helper<true, true>(from, white ? (from + 7) : (from - 7));
	}
	void push_ep_capture_right(const Square from){
		pawn_move_helper<true, true>(from, white ? (from + 9) : (from - 9));
	}

	const HalfBoard Enemy;
	const int Phase;
	std::priority_queue<std::tuple<int, Move>> Queue;
	const Move Hint;

	private:
		constexpr int capture_bonus(const Square to){
			if (not (Enemy.All & ToMask(to))){
				return 0;
			}
			if (Enemy.Pawn & ToMask(to)){
				return Phase * MG_PAWN + (24 - Phase) * EG_PAWN;
			}
			if (Enemy.Knight & ToMask(to)){
				return Phase * MG_KNIGHT + (24 - Phase) * EG_KNIGHT;
			}
			if (Enemy.Bishop & ToMask(to)){
				return Phase * MG_BISHOP + (24 - Phase) * EG_BISHOP;
			}
			if (Enemy.Rook & ToMask(to)){
				return Phase * MG_ROOK + (24 - Phase) * EG_ROOK;
			}
			if (Enemy.Queen & ToMask(to)){
				return Phase * MG_QUEEN + (24 - Phase) * EG_QUEEN;
			}
			throw std::logic_error("Unexpected capture (of king?)");
		}

		template <bool capture, bool special> // "special" means double push or EP capture
		inline void pawn_move_helper(const Square from, const Square to){
			if ((not special) and (white ? (to >= A8) : (to <= H1))){
				const int pawn_val= Phase * MG_PAWN + (24 - Phase) * EG_PAWN;
				const int eval_diff = pawn_move_eval_diff<capture, special>(from, to) - pawn_val;

				const Move move_n = move_from_squares(from, to, PROMOTE_TO_KNIGHT);
				Queue.push(std::make_tuple((move_n == Hint) ? REALLY_BIG_EVAL :
						(eval_diff + Phase * MG_KNIGHT + (24 - Phase) * EG_KNIGHT), move_n));
				const Move move_b = move_from_squares(from, to, PROMOTE_TO_BISHOP);
				Queue.push(std::make_tuple((move_b == Hint) ? REALLY_BIG_EVAL :
						(eval_diff + Phase * MG_BISHOP + (24 - Phase) * EG_BISHOP), move_b));
				const Move move_r = move_from_squares(from, to, PROMOTE_TO_ROOK);
				Queue.push(std::make_tuple((move_r == Hint) ? REALLY_BIG_EVAL :
						(eval_diff + Phase * MG_ROOK + (24 - Phase) * EG_ROOK), move_r));
				const Move move_q = move_from_squares(from, to, PROMOTE_TO_QUEEN);
				Queue.push(std::make_tuple((move_q == Hint) ? REALLY_BIG_EVAL :
						(eval_diff + Phase * MG_QUEEN + (24 - Phase) * EG_QUEEN), move_q));
			} else {
				const Move move = move_from_squares(from, to, capture ?
						(special ? EN_PASSANT_CAPTURE : PAWN_CAPTURE) :
						(special ? DOUBLE_PAWN_PUSH : SINGLE_PAWN_PUSH));
				const int eval_diff = (move == Hint) ? REALLY_BIG_EVAL :
						pawn_move_eval_diff<capture, special>(from, to);
				Queue.push(std::make_tuple(eval_diff, move));
			}
		}

		template <bool capture, bool special>
		inline int pawn_move_eval_diff(const Square from, const Square to){
			const int pst_diff_mg = mg_pawn_table[FlipIf(white, to)] - mg_pawn_table[FlipIf(white, from)];
			const int pst_diff_eg = eg_pawn_table[FlipIf(white, to)] - eg_pawn_table[FlipIf(white, from)];
			return Phase * pst_diff_mg + (24 - Phase) * pst_diff_eg + (capture ?
					(special ? (Phase * MG_PAWN + (24 - Phase) * EG_PAWN) : capture_bonus(to)) : 0);
		}
};

template struct MoveQueue<true>;
template struct MoveQueue<false>;
