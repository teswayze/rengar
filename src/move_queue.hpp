# pragma once

# include <tuple>
# include <array>
# include "board.hpp"

// Theoretical upper bound is 218, but maximum ever seen in a serious game is 79
const size_t move_array_max_size = 128;

struct ABCMask{
	BitMask A;
	BitMask B;
	BitMask C;
};

ABCMask abc_for_halfboard(const HalfBoard &side);

Attacks checking_squares(const bool white, const Square enemy_king, const BitMask occ);

int initialize_move_order_arrays();

struct MoveQueue{
	MoveQueue(const bool white, const Board &board, const Move hint, const Move killer1, const Move killer2) :
		Hint(hint), Killer1(killer1), Killer2(killer2), EnemyABC(abc_for_halfboard(white ? board.Black : board.White)),
		EnemyAtk(white ? board.BkAtk : board.WtAtk), 
		CheckSquares(checking_squares(white, white ? board.Black.King : board.White.King, board.Occ)) { }
	MoveQueue(const bool white, const Board &board) : EnemyABC(abc_for_halfboard(white ? board.Black : board.White)),
		EnemyAtk(white ? board.BkAtk : board.WtAtk),
		CheckSquares(checking_squares(white, white ? board.Black.King : board.White.King, board.Occ)) { }

	bool empty() const;
	Move top() const;
	int top_prio() const;
	void pop();
	void heapify();

	template <bool white>
	void push_knight_move(const Square from, const Square to);
	template <bool white>
	void push_bishop_move(const Square from, const Square to);
	template <bool white>
	void push_rook_move(const Square from, const Square to);
	template <bool white>
	void push_queen_move(const Square from, const Square to);
	template <bool white>
	void push_king_move(const Square from, const Square to);
	template <bool white>
	void push_castle_qs();
	template <bool white>
	void push_castle_ks();
	template <bool white, bool include_underpromotions>
	void push_single_pawn_move(const Square from);
	template <bool white>
	void push_double_pawn_move(const Square from);
	template <bool white, bool include_underpromotions>
	void push_pawn_capture_left(const Square from);
	template <bool white, bool include_underpromotions>
	void push_pawn_capture_right(const Square from);
	template <bool white>
	void push_ep_capture_left(const Square from);
	template <bool white>
	void push_ep_capture_right(const Square from);

	template <bool white>
	void update_frequency_for_beta_cutoff();

	private:
		std::array<std::tuple<int, Move>, move_array_max_size> move_array;
		size_t queue_length = 0;
		size_t num_dequed_moves = 0;
		const Move Hint = 0;
		const Move Killer1 = 0;
		const Move Killer2 = 0;
		const ABCMask EnemyABC;
		const Attacks &EnemyAtk;
		const Attacks CheckSquares;

		template <bool include_underpromotions>
		inline void handle_promotions(const Square from, const Square to, const int freq);
		inline void push_move_helper(int priority, const Move move);
};

void show_move_order_values();
