# include <tuple>
# include <queue>
# include "board.hpp"
# include "pst.hpp"
# include "eval.hpp"


const std::array<int, 64> pawn_freq = {
	165, 194, 191, 187, 220, 218, 250, 216,
	191, 189, 170, 166, 151, 194, 187, 205,
	146, 109,  89, 113,  77, 116, 116, 154,
	110,  93,  14,  67,  72,  75,  71, 107,
	-11, -40,  88, 161, 104, -41, -95, -53,
	-11, -69,  62, 109, 115, -76, -39,  12,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const std::array<int, 64> knight_freq = {
	-105, -155,  -58,  -43,  -33,  -30, -141, -137,
	 -63,   -7,   -8,    7,  -21,  -25,  -52,  -53,
	  -9,  -40,   27,   41,    9,    6,  -19,   -4,
	 -22,   -9,   57,   17,    7,   40,  -46,   17,
	 -53,  -11,   57,   24,   27,   62,   24, -116,
	-119,    9,  141,   39,   65,  189,   63, -159,
	-195,  -69,    3,  -13,  -17,   25,  -45, -146,
	-227, -192,  -88, -107, -163,  -74, -290, -243,
};
const std::array<int, 64> bishop_freq = {
	-164, -199,  -88, -107,  -62, -108, -150, -163,
	 -80,  -36,  -52,  -61,  -21,  -75,  -50, -123,
	-145,  -34,    4,   -6,  -48,   15,  -75, -149,
	 -38,   -1,  -16,   18,   20,    0,   -3,  -67,
	  -6,  -17,   33,   34,   10,   17,  -27,   -1,
	 -20,   27,   47,   67,   87,   37,   28,  -76,
	 -77,   72,    0,   10,   47,    0,  136,  -15,
	-147,  -84,  -90,  -51,  -52,  -99, -135, -219,
};
const std::array<int, 64> rook_freq = {
	  28,   11,   32,   23,   40,   19,   11,   -4,
	  31,   42,   48,   32,   -1,   -7,    4,    5,
	  18,   19,   -5,  -13,  -51,  -52,  -25,  -26,
	   3,  -17,    1,  -41,  -34,  -13,  -31,  -12,
	 -18,  -38,  -12,  -43,  -37,  -29,  -33,  -19,
	 -56,  -38,  -44,  -61,  -39,  -16,  -34,  -33,
	-102,  -76,  -64,  -54,  -62,  -20,  -48,  -54,
	 -82,  -63,   23,   70,   39,  -59, -127,  -93,
};
const std::array<int, 64> queen_freq = {
	 -41,  -43,  -26,  -33,   -8,  -36,  -52,  -22,
	 -25,   19,    1,  -11,   -4,  -28,  -53,  -57,
	 -33,  -53,  -33,  -37,  -17,  -25,  -55,  -33,
	 -35,  -29,  -34,  -42,   10,  -13,  -18,  -35,
	 -51,  -47,  -20,   -8,    2,   -2,   -9,  -22,
	 -68,  -17,  -23,  -46,  -10,   17,   50,  -14,
	-114,  -51,   16,  -25,   -9,   34,  -19,  -80,
	 -65, -169, -101,  -39, -153, -146, -114, -128,
};
const std::array<int, 64> king_freq = {
	-169, -119,  -92, -129, -136,  -92,  -86, -180,
	 -99,   -6,  -30,    9,  -13,   35,  -23,  -85,
	 -57,   52,   69,    5,   27,   96,  103,  -51,
	-101,   11,   47,   57,   55,   60,   32,  -72,
	-123,   -9,   23,   27,   63,   48,   15,  -74,
	-100,  -20,   13,   20,   45,   46,   32,  -82,
	 -92,   -5,    1,  -57,  -17,    4,   28,  -68,
	-167,  -23,  -42,  -91,  -90, -113,  -46, -190,
};

const std::array<int, 6> pawn_capture_freq = {
		  0, 207, 325, 332, 291, 402,
};
const std::array<int, 6> knight_capture_freq = {
		  0,  33, 262, 283, 350, 434,
};
const std::array<int, 6> bishop_capture_freq = {
		  0,  79, 198, 338, 370, 404,
};
const std::array<int, 6> rook_capture_freq = {
		  0, 101, 112, 149, 311, 432,
};
const std::array<int, 6> queen_capture_freq = {
		  0,  63, 115, 165, 145, 334,
};
const std::array<int, 6> king_capture_freq = {
		  0, 209, 288, 293, 279, 165,
};

const int castle_qs_freq = 270;
const int castle_ks_freq = 331;
const int en_passant_freq = 309;

const int underpromote_to_knight_freq = -429;
const int underpromote_to_bishop_freq = -785;
const int underpromote_to_rook_freq = -590;

int capture_bonus_index(const HalfBoard &side, const Square square){
	const BitMask mask = ToMask(square);
	if (not (mask & side.All)) return 0;
	if (mask & side.Pawn) return 1;
	if (mask & side.Knight) return 2;
	if (mask & side.Bishop) return 3;
	if (mask & side.Rook) return 4;
	if (mask & side.Queen) return 5;

	throw std::logic_error("Something went wrong - trying to capture king? 'All' out of sync with other masks?");
}


template <bool white>
struct MoveQueue{
	MoveQueue(const Move hint, const Move killer1, const Move killer2) :
		Hint(hint), Killer1(killer1), Killer2(killer2) { }
	MoveQueue() : Hint(0), Killer1(0), Killer2(0) { }

	bool empty() const{ return Queue.empty(); }
	Move top() const{ return std::get<1>(Queue.top()); }
	void pop(){ Queue.pop(); }

	void push_knight_move(const Square from, const Square to, const HalfBoard &enemy){
		const Move move = move_from_squares(from, to, KNIGHT_MOVE);
		Queue.push(std::make_tuple(knight_freq[FlipIf(white, to)] + knight_capture_freq[capture_bonus_index(enemy, to)] + match_bonus(move), move));
	}
	void push_bishop_move(const Square from, const Square to, const HalfBoard &enemy){
		const Move move = move_from_squares(from, to, BISHOP_MOVE);
		Queue.push(std::make_tuple(bishop_freq[FlipIf(white, to)] + bishop_capture_freq[capture_bonus_index(enemy, to)] + match_bonus(move), move));
	}
	void push_rook_move(const Square from, const Square to, const HalfBoard &enemy){
		const Move move = move_from_squares(from, to, ROOK_MOVE);
		Queue.push(std::make_tuple(rook_freq[FlipIf(white, to)] + rook_capture_freq[capture_bonus_index(enemy, to)] + match_bonus(move), move));
	}
	void push_queen_move(const Square from, const Square to, const HalfBoard &enemy){
		const Move move = move_from_squares(from, to, QUEEN_MOVE);
		Queue.push(std::make_tuple(queen_freq[FlipIf(white, to)] + queen_capture_freq[capture_bonus_index(enemy, to)] + match_bonus(move), move));
	}
	void push_king_move(const Square from, const Square to, const HalfBoard &enemy){
		const Move move = move_from_squares(from, to, KING_MOVE);
		Queue.push(std::make_tuple(king_freq[FlipIf(white, to)] + king_capture_freq[capture_bonus_index(enemy, to)] + match_bonus(move), move));
	}
	void push_castle_qs(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, C8), CASTLE_QUEENSIDE);
		Queue.push(std::make_tuple(castle_qs_freq + match_bonus(move), move));
	}
	void push_castle_ks(){
		const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, G8), CASTLE_KINGSIDE);
		Queue.push(std::make_tuple(castle_ks_freq + match_bonus(move), move));
	}
	void push_single_pawn_move(const Square from){
		const Square to = white ? (from + 8) : (from - 8);
		const int freq = pawn_freq[FlipIf(white, to)];
		if (white ? (to >= A8) : (to <= H1)) {
			handle_promotions(from, to, freq);
		} else {
			const Move move = move_from_squares(from, to, SINGLE_PAWN_PUSH);
			Queue.push(std::make_tuple(freq + match_bonus(move), move));
		}
	}
	void push_double_pawn_move(const Square from){
		const Square to = white ? (from + 16) : (from - 16);
		const Move move = move_from_squares(from, to, DOUBLE_PAWN_PUSH);
		Queue.push(std::make_tuple(pawn_freq[FlipIf(white, to)] + match_bonus(move), move));
	}
	void push_pawn_capture_left(const Square from, const HalfBoard &enemy){
		const Square to = white ? (from + 7) : (from - 7);
		const int freq = pawn_freq[FlipIf(white, to)] + pawn_capture_freq[capture_bonus_index(enemy, to)];
		if (white ? (to >= A8) : (to <= H1)) {
			handle_promotions(from, to, freq);
		} else {
			const Move move = move_from_squares(from, to, PAWN_CAPTURE);
			Queue.push(std::make_tuple(freq + match_bonus(move), move));
		}
	}
	void push_pawn_capture_right(const Square from, const HalfBoard &enemy){
		const Square to = white ? (from + 9) : (from - 9);
		const int freq = pawn_freq[FlipIf(white, to)] + pawn_capture_freq[capture_bonus_index(enemy, to)];
		if (white ? (to >= A8) : (to <= H1)) {
			handle_promotions(from, to, freq);
		} else {
			const Move move = move_from_squares(from, to, PAWN_CAPTURE);
			Queue.push(std::make_tuple(freq + match_bonus(move), move));
		}
	}
	void push_ep_capture_left(const Square from){
		const Square to = white ? (from + 7) : (from - 7);
		const Move move = move_from_squares(from, to, EN_PASSANT_CAPTURE);
		Queue.push(std::make_tuple(en_passant_freq + match_bonus(move), move));
	}
	void push_ep_capture_right(const Square from){
		const Square to = white ? (from + 9) : (from - 9);
		const Move move = move_from_squares(from, to, EN_PASSANT_CAPTURE);
		Queue.push(std::make_tuple(en_passant_freq + match_bonus(move), move));
	}

	std::priority_queue<std::tuple<int, Move>> Queue;
	const Move Hint;
	const Move Killer1;
	const Move Killer2;

	private:
		constexpr int match_bonus(const Move move){
			if (move == Hint) return 100000;
			if (move == Killer1) return 100;
			if (move == Killer2) return 100;
			return 0;
		}

		inline void handle_promotions(const Square from, const Square to, const int freq){
			const Move n = move_from_squares(from, to, PROMOTE_TO_KNIGHT);
			Queue.push(std::make_tuple(freq + underpromote_to_knight_freq + match_bonus(n), n));
			const Move b = move_from_squares(from, to, PROMOTE_TO_BISHOP);
			Queue.push(std::make_tuple(freq + underpromote_to_bishop_freq + match_bonus(b), b));
			const Move r = move_from_squares(from, to, PROMOTE_TO_ROOK);
			Queue.push(std::make_tuple(freq + underpromote_to_rook_freq + match_bonus(r), r));
			const Move q = move_from_squares(from, to, PROMOTE_TO_QUEEN);
			Queue.push(std::make_tuple(freq + match_bonus(q), q));
		}
};

template struct MoveQueue<true>;
template struct MoveQueue<false>;
