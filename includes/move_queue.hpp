# pragma once

# include <tuple>
# include <queue>
# include "board.hpp"

template <bool white>
struct MoveQueue{
	MoveQueue();
	MoveQueue(const Move hint, const Move killer1, const Move killer2);

	bool empty() const;
	Move top() const;
	void pop();

	void push_knight_move(const Square from, const Square to, const HalfBoard &enemy);
	void push_bishop_move(const Square from, const Square to, const HalfBoard &enemy);
	void push_rook_move(const Square from, const Square to, const HalfBoard &enemy);
	void push_queen_move(const Square from, const Square to, const HalfBoard &enemy);
	void push_king_move(const Square from, const Square to, const HalfBoard &enemy);
	void push_castle_qs();
	void push_castle_ks();
	void push_single_pawn_move(const Square from);
	void push_double_pawn_move(const Square from);
	void push_pawn_capture_left(const Square from, const HalfBoard &enemy);
	void push_pawn_capture_right(const Square from, const HalfBoard &enemy);
	void push_ep_capture_left(const Square from);
	void push_ep_capture_right(const Square from);

	std::priority_queue<std::tuple<int, Move>> Queue;
	const Move Hint;
	const Move Killer1;
	const Move Killer2;

};
