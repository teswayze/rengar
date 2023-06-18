# pragma once

# include <tuple>
# include <queue>
# include "board.hpp"

struct ABCMask{
	BitMask A;
	BitMask B;
	BitMask C;
};


template <bool white>
struct MoveQueue{
	MoveQueue(const Board &board);
	MoveQueue(const Board &board, const Move hint, const Move killer1, const Move killer2);

	bool empty() const;
	Move top() const;
	int top_prio() const;
	void pop();

	void push_knight_move(const Square from, const Square to);
	void push_bishop_move(const Square from, const Square to);
	void push_rook_move(const Square from, const Square to);
	void push_queen_move(const Square from, const Square to);
	void push_king_move(const Square from, const Square to);
	void push_castle_qs();
	void push_castle_ks();
	void push_single_pawn_move(const Square from);
	void push_double_pawn_move(const Square from);
	void push_pawn_capture_left(const Square from);
	void push_pawn_capture_right(const Square from);
	void push_ep_capture_left(const Square from);
	void push_ep_capture_right(const Square from);

	std::priority_queue<std::tuple<int, Move>> Queue;
	const Move Hint;
	const Move Killer1;
	const Move Killer2;
	const ABCMask EnemyABC;
	const ABCMask GuardABC;

};
