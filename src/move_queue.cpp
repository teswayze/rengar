# include <tuple>
# include <queue>
# include "board.hpp"
# include "pst.hpp"
# include "eval.hpp"

# ifdef TUNE_MOVE_ORDER
# include "register_params.hpp"
# define MOVE_ORDER_PARAM REGISTER_TUNABLE_PARAM
# define MOVE_ORDER_PARAM_ARRAY REGISTER_TUNABLE_PARAM_ARRAY
# else
# define MOVE_ORDER_PARAM(name, value) const int name = value;
# define MOVE_ORDER_PARAM_ARRAY(size, name, ...) const std::array<int, size> name = { __VA_ARGS__ };
# endif


MOVE_ORDER_PARAM_ARRAY(64, pawn_freq,
	 176,  190,  203,  202,  236,  253,  292,  245,
	 200,  156,  162,  146,  172,  169,  208,  223,
	 149,  110,  110,  103,   88,  119,  132,  164,
	 106,  103,   33,  112,   73,   85,   83,  114,
	 -47,  -44,   98,  185,  127,  -69, -110,  -86,
	 -48,  -93,   47,   51,   88, -100,  -66,  -25,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
)
MOVE_ORDER_PARAM_ARRAY(64, knight_freq,
	-110,  -90,  -85,  -54,  -37,  -52,  -62, -153,
	 -66,   -3,   -8,  -10,  -53,  -29,  -34,  -65,
	 -27,   24,  105,   55,  103,   69,   52,   21,
	 -35,    8,   89,   67,    5,  116,  -34,   13,
	 -82,   20,  104,  103,   99,   93,   54, -135,
	-111,  -19,  148,   52,   62,  187,   62,  -84,
	-206,  -80,  -24,  -35,  -26,   26,  -50, -130,
	-202, -264,  -94, -147, -193,  -98, -254, -240,
)
MOVE_ORDER_PARAM_ARRAY(64, bishop_freq,
	-171, -115,  -73,  -67,  -63,  -63, -122, -173,
	-119,  -31,  -47,  -27,  -20,  -68,  -66, -101,
	 -99,  -29,   29,   17,   32,   38,  -28,  -74,
	 -44,   28,    6,   62,   38,   32,   -3,  -45,
	 -10,    0,   41,   55,   53,   32,   10,  -10,
	  -4,   23,   53,   71,   81,   34,   41, -101,
	 -83,   63,   -4,  -26,   23,   -5,  126, -105,
	-204, -121, -156,  -74, -117, -125, -158, -282,
)
MOVE_ORDER_PARAM_ARRAY(64, rook_freq,
	  21,    5,   39,   33,   53,   32,   23,    2,
	  38,   36,   59,   45,   16,   -2,   16,   15,
	  18,   23,   21,    8,   -4,   11,   21,   11,
	   1,    0,   28,    9,    2,   34,   19,    6,
	 -18,  -15,   10,   -9,    7,    5,    4,  -23,
	 -54,  -30,  -37,  -55,  -28,    6,    7,  -45,
	-104,  -38,  -59,  -68,  -84,   -7,  -25,  -69,
	-114,  -94,  -13,   58,   18,  -83, -149, -115,
)
MOVE_ORDER_PARAM_ARRAY(64, queen_freq,
	 -43,  -12,   29,   15,   34,  -30,  -36,  -12,
	 -14,   20,   38,   26,   36,  -25,  -73,  -74,
	 -24,    6,   20,   26,   50,   48,   29,    7,
	 -20,   17,   19,   38,   74,   58,   53,   -7,
	 -84,    6,   20,   74,   65,   40,   52,   11,
	 -72,  -10,    8,  -27,   32,   38,   78,  -23,
	-111,  -31,    8,  -47,  -14,   36,  -13, -100,
	 -90, -198, -143,  -72, -156, -134,  -95, -136,
)
MOVE_ORDER_PARAM_ARRAY(64, king_freq,
	-155, -103, -116, -152, -166, -110,  -92, -189,
	-121,  -23,  -49,   -6,  -34,   13,  -38, -121,
	 -65,   53,   65,    2,   21,   89,  105,  -67,
	-111,   16,   40,   54,   41,   61,   13,  -86,
	-122,  -18,   12,   21,   54,   38,   -3, -104,
	 -97,  -27,    9,    7,   32,   32,   13, -103,
	-114,  -39,  -22,  -82,  -46,  -26,    4, -108,
	-186,  -46, -100, -139, -125, -160,  -79, -241,
)

MOVE_ORDER_PARAM_ARRAY(6, pawn_capture_freq,
	  0, 208, 295, 313, 293, 393,
)
MOVE_ORDER_PARAM_ARRAY(6, knight_capture_freq,
	  0,  58, 345, 329, 388, 374,
)
MOVE_ORDER_PARAM_ARRAY(6, bishop_capture_freq,
	  0, 118, 275, 385, 396, 430,
)
MOVE_ORDER_PARAM_ARRAY(6, rook_capture_freq,
	  0, 164, 187, 195, 380, 456,
)
MOVE_ORDER_PARAM_ARRAY(6, queen_capture_freq,
	  0, 144, 223, 237, 216, 490,
)
MOVE_ORDER_PARAM_ARRAY(6, king_capture_freq,
	  0, 210, 260, 277, 275, 168,
)

MOVE_ORDER_PARAM(pawn_fear_pawn, 92)
MOVE_ORDER_PARAM(pawn_fear_knight, 28)
MOVE_ORDER_PARAM(pawn_fear_king, 26)
MOVE_ORDER_PARAM(pawn_fear_rook, 23)
MOVE_ORDER_PARAM(pawn_fear_bishop, 21)
MOVE_ORDER_PARAM(pawn_fear_queen, 3)

MOVE_ORDER_PARAM(knight_fear_pawn, 258)
MOVE_ORDER_PARAM(knight_fear_king, 133)
MOVE_ORDER_PARAM(knight_fear_bishop, 94)
MOVE_ORDER_PARAM(knight_fear_rook, 80)
MOVE_ORDER_PARAM(knight_fear_knight, 62)
MOVE_ORDER_PARAM(knight_fear_queen, 51)

MOVE_ORDER_PARAM(bishop_fear_pawn, 245)
MOVE_ORDER_PARAM(bishop_fear_knight, 175)
MOVE_ORDER_PARAM(bishop_fear_king, 104)
MOVE_ORDER_PARAM(bishop_fear_rook, 104)
MOVE_ORDER_PARAM(bishop_fear_bishop, 99)
MOVE_ORDER_PARAM(bishop_fear_queen, 53)

MOVE_ORDER_PARAM(rook_fear_pawn, 327)
MOVE_ORDER_PARAM(rook_fear_knight, 263)
MOVE_ORDER_PARAM(rook_fear_bishop, 262)
MOVE_ORDER_PARAM(rook_fear_rook, 139)
MOVE_ORDER_PARAM(rook_fear_king, 137)
MOVE_ORDER_PARAM(rook_fear_queen, 47)

MOVE_ORDER_PARAM(queen_fear_pawn, 391)
MOVE_ORDER_PARAM(queen_fear_knight, 369)
MOVE_ORDER_PARAM(queen_fear_bishop, 351)
MOVE_ORDER_PARAM(queen_fear_rook, 301)
MOVE_ORDER_PARAM(queen_fear_queen, 166)
MOVE_ORDER_PARAM(queen_fear_king, 82)

MOVE_ORDER_PARAM(pawn_evade_pawn, 92)
MOVE_ORDER_PARAM(pawn_evade_knight, 28)
MOVE_ORDER_PARAM(pawn_evade_king, 26)
MOVE_ORDER_PARAM(pawn_evade_rook, 23)
MOVE_ORDER_PARAM(pawn_evade_bishop, 21)
MOVE_ORDER_PARAM(pawn_evade_queen, 3)

MOVE_ORDER_PARAM(knight_evade_pawn, 258)
MOVE_ORDER_PARAM(knight_evade_king, 133)
MOVE_ORDER_PARAM(knight_evade_bishop, 94)
MOVE_ORDER_PARAM(knight_evade_rook, 80)
MOVE_ORDER_PARAM(knight_evade_knight, 62)
MOVE_ORDER_PARAM(knight_evade_queen, 51)

MOVE_ORDER_PARAM(bishop_evade_pawn, 245)
MOVE_ORDER_PARAM(bishop_evade_knight, 175)
MOVE_ORDER_PARAM(bishop_evade_king, 104)
MOVE_ORDER_PARAM(bishop_evade_rook, 104)
MOVE_ORDER_PARAM(bishop_evade_bishop, 99)
MOVE_ORDER_PARAM(bishop_evade_queen, 53)

MOVE_ORDER_PARAM(rook_evade_pawn, 327)
MOVE_ORDER_PARAM(rook_evade_knight, 263)
MOVE_ORDER_PARAM(rook_evade_bishop, 262)
MOVE_ORDER_PARAM(rook_evade_rook, 139)
MOVE_ORDER_PARAM(rook_evade_king, 137)
MOVE_ORDER_PARAM(rook_evade_queen, 47)

MOVE_ORDER_PARAM(queen_evade_pawn, 391)
MOVE_ORDER_PARAM(queen_evade_knight, 369)
MOVE_ORDER_PARAM(queen_evade_bishop, 351)
MOVE_ORDER_PARAM(queen_evade_rook, 301)
MOVE_ORDER_PARAM(queen_evade_queen, 166)
// queen_evade_king is not possible, as the opponent would be in check

MOVE_ORDER_PARAM(castle_qs_freq, 186)
MOVE_ORDER_PARAM(castle_ks_freq, 278)
MOVE_ORDER_PARAM(en_passant_freq, 209)
MOVE_ORDER_PARAM(underpromote_to_knight_freq, -450)
MOVE_ORDER_PARAM(underpromote_to_bishop_freq, -638)
MOVE_ORDER_PARAM(underpromote_to_rook_freq, -536)


struct ABCMask{
	BitMask A;
	BitMask B;
	BitMask C;
};

ABCMask abc_for_halfboard(const HalfBoard &side){
	return ABCMask{side.Rook | side.Queen, side.Knight | side.Bishop, side.Pawn | side.Bishop | side.Queen};
}


template <bool white>
struct MoveQueue{
	MoveQueue(const Board &board, const Move hint, const Move killer1, const Move killer2) :
		Hint(hint), Killer1(killer1), Killer2(killer2), EnemyABC(abc_for_halfboard(get_side<not white>(board))),
		EnemyAtk(white ? board.BkAtk : board.WtAtk) { }
	MoveQueue(const Board &board) : Hint(0), Killer1(0), Killer2(0), EnemyABC(abc_for_halfboard(get_side<not white>(board))),
			EnemyAtk(white ? board.BkAtk : board.WtAtk){ }

	bool empty() const{ return Queue.empty(); }
	Move top() const{ return std::get<1>(Queue.top()); }
	int top_prio() const{ return std::get<0>(Queue.top()); }
	void pop(){ Queue.pop(); }

	void push_knight_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, KNIGHT_MOVE);
		Queue.push(std::make_tuple(knight_freq[FlipIf(white, to)] + knight_capture_freq[piece_at_square(to)]
			- knight_fear_penalty(to) + knight_evade_bonus(from) + match_bonus(move), move));
	}
	void push_bishop_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, BISHOP_MOVE);
		Queue.push(std::make_tuple(bishop_freq[FlipIf(white, to)] + bishop_capture_freq[piece_at_square(to)]
			- bishop_fear_penalty(to) + bishop_evade_bonus(from) + match_bonus(move), move));
	}
	void push_rook_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, ROOK_MOVE);
		Queue.push(std::make_tuple(rook_freq[FlipIf(white, to)] + rook_capture_freq[piece_at_square(to)]
			- rook_fear_penalty(to) + rook_evade_bonus(from) + match_bonus(move), move));
	}
	void push_queen_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, QUEEN_MOVE);
		Queue.push(std::make_tuple(queen_freq[FlipIf(white, to)] + queen_capture_freq[piece_at_square(to)]
			- queen_fear_penalty(to) + queen_evade_bonus(from) + match_bonus(move), move));
	}
	void push_king_move(const Square from, const Square to){
		const Move move = move_from_squares(from, to, KING_MOVE);
		Queue.push(std::make_tuple(king_freq[FlipIf(white, to)] + king_capture_freq[piece_at_square(to)] + 
			match_bonus(move), move));
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
		const int freq = pawn_freq[FlipIf(white, to)] - pawn_fear_penalty(to) + pawn_evade_bonus(from);
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
		Queue.push(std::make_tuple(pawn_freq[FlipIf(white, to)] - pawn_fear_penalty(to) + pawn_evade_bonus(from)
			+ match_bonus(move), move));
	}
	void push_pawn_capture_left(const Square from){
		const Square to = white ? (from + 7) : (from - 7);
		const int freq = pawn_freq[FlipIf(white, to)] + pawn_capture_freq[piece_at_square(to)] - pawn_fear_penalty(to)
			+ pawn_evade_bonus(from);
		if (white ? (to >= A8) : (to <= H1)) {
			handle_promotions(from, to, freq);
		} else {
			const Move move = move_from_squares(from, to, PAWN_CAPTURE);
			Queue.push(std::make_tuple(freq + match_bonus(move), move));
		}
	}
	void push_pawn_capture_right(const Square from){
		const Square to = white ? (from + 9) : (from - 9);
		const int freq = pawn_freq[FlipIf(white, to)] + pawn_capture_freq[piece_at_square(to)] - pawn_fear_penalty(to)
			+ pawn_evade_bonus(from);
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

	private:
		std::priority_queue<std::tuple<int, Move>> Queue;
		const Move Hint;
		const Move Killer1;
		const Move Killer2;
		const ABCMask EnemyABC;
		const Attacks& EnemyAtk;

		constexpr int match_bonus(const Move move){
			if (move == Hint) return 100000;
			if (move == Killer1) return 100;
			if (move == Killer2) return 100;
			return 0;
		}

		constexpr int piece_at_square(const Square to){
			const BitMask mask = ToMask(to);
			return ((EnemyABC.A & mask) ? 4 : 0) + ((EnemyABC.B & mask) ? 2 : 0) + ((EnemyABC.C & mask) ? 1 : 0);
		}

		constexpr int pawn_fear_penalty(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return pawn_fear_pawn;
			if (EnemyAtk.Knight & mask) return pawn_fear_knight;
			if (EnemyAtk.King & mask) return pawn_fear_king;
			if (EnemyAtk.Rook & mask) return pawn_fear_rook;
			if (EnemyAtk.Bishop & mask) return pawn_fear_bishop;
			if (EnemyAtk.Queen & mask) return pawn_fear_queen;
			return 0;
		}
		constexpr int knight_fear_penalty(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return knight_fear_pawn;
			if (EnemyAtk.King & mask) return knight_fear_king;
			if (EnemyAtk.Bishop & mask) return knight_fear_bishop;
			if (EnemyAtk.Rook & mask) return knight_fear_rook;
			if (EnemyAtk.Knight & mask) return knight_fear_knight;
			if (EnemyAtk.Queen & mask) return knight_fear_queen;
			return 0;
		}
		constexpr int bishop_fear_penalty(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return bishop_fear_pawn;
			if (EnemyAtk.Knight & mask) return bishop_fear_knight;
			if (EnemyAtk.King & mask) return bishop_fear_king;
			if (EnemyAtk.Rook & mask) return bishop_fear_rook;
			if (EnemyAtk.Bishop & mask) return bishop_fear_bishop;
			if (EnemyAtk.Queen & mask) return bishop_fear_queen;
			return 0;
		}
		constexpr int rook_fear_penalty(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return rook_fear_pawn;
			if (EnemyAtk.Knight & mask) return rook_fear_knight;
			if (EnemyAtk.Bishop & mask) return rook_fear_bishop;
			if (EnemyAtk.Rook & mask) return rook_fear_rook;
			if (EnemyAtk.King & mask) return rook_fear_king;
			if (EnemyAtk.Queen & mask) return rook_fear_queen;
			return 0;
		}
		constexpr int queen_fear_penalty(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return queen_fear_pawn;
			if (EnemyAtk.Knight & mask) return queen_fear_knight;
			if (EnemyAtk.Bishop & mask) return queen_fear_bishop;
			if (EnemyAtk.Rook & mask) return queen_fear_rook;
			if (EnemyAtk.Queen & mask) return queen_fear_queen;
			if (EnemyAtk.King & mask) return queen_fear_king;
			return 0;
		}

		constexpr int pawn_evade_bonus(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return pawn_evade_pawn;
			if (EnemyAtk.Knight & mask) return pawn_evade_knight;
			if (EnemyAtk.King & mask) return pawn_evade_king;
			if (EnemyAtk.Rook & mask) return pawn_evade_rook;
			if (EnemyAtk.Bishop & mask) return pawn_evade_bishop;
			if (EnemyAtk.Queen & mask) return pawn_evade_queen;
			return 0;
		}
		constexpr int knight_evade_bonus(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return knight_evade_pawn;
			if (EnemyAtk.King & mask) return knight_evade_king;
			if (EnemyAtk.Bishop & mask) return knight_evade_bishop;
			if (EnemyAtk.Rook & mask) return knight_evade_rook;
			if (EnemyAtk.Knight & mask) return knight_evade_knight;
			if (EnemyAtk.Queen & mask) return knight_evade_queen;
			return 0;
		}
		constexpr int bishop_evade_bonus(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return bishop_evade_pawn;
			if (EnemyAtk.Knight & mask) return bishop_evade_knight;
			if (EnemyAtk.King & mask) return bishop_evade_king;
			if (EnemyAtk.Rook & mask) return bishop_evade_rook;
			if (EnemyAtk.Bishop & mask) return bishop_evade_bishop;
			if (EnemyAtk.Queen & mask) return bishop_evade_queen;
			return 0;
		}
		constexpr int rook_evade_bonus(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return rook_evade_pawn;
			if (EnemyAtk.Knight & mask) return rook_evade_knight;
			if (EnemyAtk.Bishop & mask) return rook_evade_bishop;
			if (EnemyAtk.Rook & mask) return rook_evade_rook;
			if (EnemyAtk.King & mask) return rook_evade_king;
			if (EnemyAtk.Queen & mask) return rook_evade_queen;
			return 0;
		}
		constexpr int queen_evade_bonus(const Square to){
			const BitMask mask = ToMask(to);
			if (EnemyAtk.Pawn & mask) return queen_evade_pawn;
			if (EnemyAtk.Knight & mask) return queen_evade_knight;
			if (EnemyAtk.Bishop & mask) return queen_evade_bishop;
			if (EnemyAtk.Rook & mask) return queen_evade_rook;
			if (EnemyAtk.Queen & mask) return queen_evade_queen;
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
