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
	PstEvalInfo top_eval_info() const;
	void pop();

	void push_knight_move(const Square from, const Square to, const Board &board);
	void push_bishop_move(const Square from, const Square to, const Board &board);
	void push_rook_move(const Square from, const Square to, const Board &board);
	void push_queen_move(const Square from, const Square to, const Board &board);
	void push_king_move(const Square from, const Square to, const Board &board);
	void push_castle_qs(const Board &board);
	void push_castle_ks(const Board &board);
	void push_single_pawn_move(const Square from, const Board &board);
	void push_double_pawn_move(const Square from, const Board &board);
	void push_pawn_capture_left(const Square from, const Board &board);
	void push_pawn_capture_right(const Square from, const Board &board);
	void push_ep_capture_left(const Square from, const Board &board);
	void push_ep_capture_right(const Square from, const Board &board);

	std::priority_queue<std::tuple<int, Move, PstEvalInfo>> Queue;
	const Move Hint;
	const Move Killer1;
	const Move Killer2;

};
