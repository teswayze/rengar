# include <algorithm>
# include <stdexcept>
# include <iostream>
# include "move_queue.hpp"

const std::array<int, 64> starting_pawn_freq = {
	 176,  190,  203,  202,  236,  253,  292,  245,
	 200,  156,  162,  146,  172,  169,  208,  223,
	 149,  110,  110,  103,   88,  119,  132,  164,
	 106,  103,   33,  112,   73,   85,   83,  114,
	 -47,  -44,   98,  185,  127,  -69, -110,  -86,
	 -48,  -93,   47,   51,   88, -100,  -66,  -25,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
};
const std::array<int, 64> starting_knight_freq = {
	-110,  -90,  -85,  -54,  -37,  -52,  -62, -153,
	 -66,   -3,   -8,  -10,  -53,  -29,  -34,  -65,
	 -27,   24,  105,   55,  103,   69,   52,   21,
	 -35,    8,   89,   67,    5,  116,  -34,   13,
	 -82,   20,  104,  103,   99,   93,   54, -135,
	-111,  -19,  148,   52,   62,  187,   62,  -84,
	-206,  -80,  -24,  -35,  -26,   26,  -50, -130,
	-202, -264,  -94, -147, -193,  -98, -254, -240,
};
const std::array<int, 64> starting_bishop_freq = {
	-171, -115,  -73,  -67,  -63,  -63, -122, -173,
	-119,  -31,  -47,  -27,  -20,  -68,  -66, -101,
	 -99,  -29,   29,   17,   32,   38,  -28,  -74,
	 -44,   28,    6,   62,   38,   32,   -3,  -45,
	 -10,    0,   41,   55,   53,   32,   10,  -10,
	  -4,   23,   53,   71,   81,   34,   41, -101,
	 -83,   63,   -4,  -26,   23,   -5,  126, -105,
	-204, -121, -156,  -74, -117, -125, -158, -282,
};
const std::array<int, 64> starting_rook_freq = {
	  21,    5,   39,   33,   53,   32,   23,    2,
	  38,   36,   59,   45,   16,   -2,   16,   15,
	  18,   23,   21,    8,   -4,   11,   21,   11,
	   1,    0,   28,    9,    2,   34,   19,    6,
	 -18,  -15,   10,   -9,    7,    5,    4,  -23,
	 -54,  -30,  -37,  -55,  -28,    6,    7,  -45,
	-104,  -38,  -59,  -68,  -84,   -7,  -25,  -69,
	-114,  -94,  -13,   58,   18,  -83, -149, -115,
};
const std::array<int, 64> starting_queen_freq = {
	 -43,  -12,   29,   15,   34,  -30,  -36,  -12,
	 -14,   20,   38,   26,   36,  -25,  -73,  -74,
	 -24,    6,   20,   26,   50,   48,   29,    7,
	 -20,   17,   19,   38,   74,   58,   53,   -7,
	 -84,    6,   20,   74,   65,   40,   52,   11,
	 -72,  -10,    8,  -27,   32,   38,   78,  -23,
	-111,  -31,    8,  -47,  -14,   36,  -13, -100,
	 -90, -198, -143,  -72, -156, -134,  -95, -136,
};
const std::array<int, 64> starting_king_freq = {
	-155, -103, -116, -152, -166, -110,  -92, -189,
	-121,  -23,  -49,   -6,  -34,   13,  -38, -121,
	 -65,   53,   65,    2,   21,   89,  105,  -67,
	-111,   16,   40,   54,   41,   61,   13,  -86,
	-122,  -18,   12,   21,   54,   38,   -3, -104,
	 -97,  -27,    9,    7,   32,   32,   13, -103,
	-114,  -39,  -22,  -82,  -46,  -26,    4, -108,
	-186,  -46, -100, -139, -125, -160,  -79, -241,
};
const int starting_castle_qs_freq = 221;
const int starting_castle_ks_freq = 291;
const int starting_en_passant_freq = 264;

std::array<int, 64> white_pawn_freq;
std::array<int, 64> white_knight_freq;
std::array<int, 64> white_bishop_freq;
std::array<int, 64> white_rook_freq;
std::array<int, 64> white_queen_freq;
std::array<int, 64> white_king_freq;
int white_castle_qs_freq;
int white_castle_ks_freq;
int white_en_passant_freq;

std::array<int, 64> black_pawn_freq;
std::array<int, 64> black_knight_freq;
std::array<int, 64> black_bishop_freq;
std::array<int, 64> black_rook_freq;
std::array<int, 64> black_queen_freq;
std::array<int, 64> black_king_freq;
int black_castle_qs_freq;
int black_castle_ks_freq;
int black_en_passant_freq;

int initialize_move_order_arrays(){
	for (int i=0; i<64; i++){
		black_pawn_freq[i] = white_pawn_freq[FlipIf(true, i)] = starting_pawn_freq[i];
		black_knight_freq[i] = white_knight_freq[FlipIf(true, i)] = starting_knight_freq[i];
		black_bishop_freq[i] = white_bishop_freq[FlipIf(true, i)] = starting_bishop_freq[i];
		black_rook_freq[i] = white_rook_freq[FlipIf(true, i)] = starting_rook_freq[i];
		black_queen_freq[i] = white_queen_freq[FlipIf(true, i)] = starting_queen_freq[i];
		black_king_freq[i] = white_king_freq[FlipIf(true, i)] = starting_king_freq[i];
	}
	black_castle_qs_freq = white_castle_qs_freq = starting_castle_qs_freq;
	black_castle_ks_freq = white_castle_ks_freq = starting_castle_ks_freq;
	black_en_passant_freq = white_en_passant_freq = starting_en_passant_freq;
	return 0;
}

const int _unused2 = initialize_move_order_arrays();

const std::array<int, 6> pawn_capture_freq = {0, 145, 329, 344, 353, 517};
const std::array<int, 6> knight_capture_freq = {-8, 103, 353, 336, 438, 529};
const std::array<int, 6> bishop_capture_freq = {-8, 120, 314, 391, 464, 596};
const std::array<int, 6> rook_capture_freq = {-13, 156, 207, 201, 378, 582};
const std::array<int, 6> queen_capture_freq = {-18, 134, 217, 224, 209, 483};
const std::array<int, 6> king_capture_freq = {-1, 212, 292, 328, 412, 342};

const int pawn_fear_pawn = 114;
const int pawn_fear_king = 88;
const int pawn_fear_knight = 54;
const int pawn_fear_rook = 52;
const int pawn_fear_bishop = 36;
const int pawn_fear_queen = 29;

const int knight_fear_pawn = 321;
const int knight_fear_king = 205;
const int knight_fear_bishop = 141;
const int knight_fear_rook = 127;
const int knight_fear_queen = 121;
const int knight_fear_knight = 80;

const int bishop_fear_pawn = 284;
const int bishop_fear_knight = 222;
const int bishop_fear_king = 156;
const int bishop_fear_rook = 145;
const int bishop_fear_bishop = 113;
const int bishop_fear_queen = 89;

const int rook_fear_pawn = 329;
const int rook_fear_bishop = 297;
const int rook_fear_knight = 290;
const int rook_fear_king = 206;
const int rook_fear_rook = 166;
const int rook_fear_queen = 71;

const int queen_fear_pawn = 440;
const int queen_fear_knight = 421;
const int queen_fear_bishop = 393;
const int queen_fear_rook = 363;
const int queen_fear_queen = 201;
const int queen_fear_king = 147;

const int pawn_evade_pawn = 50;
const int pawn_evade_knight = 38;
const int pawn_evade_bishop = 5;
//MOVE_ORDER_PARAM(pawn_evade_king, 0)
//MOVE_ORDER_PARAM(pawn_evade_rook, 0)
//MOVE_ORDER_PARAM(pawn_evade_queen, 0)

const int knight_evade_pawn = 247;
const int knight_evade_king = 83;
const int knight_evade_rook = 6;
//MOVE_ORDER_PARAM(knight_evade_bishop, 0)
//MOVE_ORDER_PARAM(knight_evade_knight, 0)
//MOVE_ORDER_PARAM(knight_evade_queen, 0)

const int bishop_evade_pawn = 270;
const int bishop_evade_knight = 117;
const int bishop_evade_king = 83;
const int bishop_evade_queen = 36;
const int bishop_evade_rook = 32;
const int bishop_evade_bishop = 25;

const int rook_evade_pawn = 260;
const int rook_evade_bishop = 243;
const int rook_evade_knight = 229;
const int rook_evade_king = 127;
const int rook_evade_rook = 62;
const int rook_evade_queen = 52;

const int queen_evade_pawn = 282;
const int queen_evade_rook = 278;
const int queen_evade_bishop = 248;
const int queen_evade_knight = 246;
const int queen_evade_queen = 140;
// queen_evade_king is not possible, as the opponent would be in check

const int pawn_check = 200;
const int knight_check = 200;
const int bishop_check = 200;
const int rook_check = 200;
const int queen_check = 200;

const int underpromote_to_knight_freq = -450;
const int underpromote_to_bishop_freq = -638;
const int underpromote_to_rook_freq = -536;

ABCMask abc_for_halfboard(const HalfBoard &side){
	return ABCMask{side.Rook | side.Queen, side.Knight | side.Bishop, side.Pawn | side.Bishop | side.Queen};
}

Attacks checking_squares(const bool white, const Square enemy_king, const BitMask occ){
	const auto bishop_check = bishop_seen(enemy_king, occ);
	const auto rook_check = rook_seen(enemy_king, occ);
	return Attacks{
		(white ? pawn_attacks<false> : pawn_attacks<true>)(ToMask(enemy_king)),
		knight_lookup[enemy_king], bishop_check, rook_check, bishop_check | rook_check, EMPTY_BOARD
	};
}

bool MoveQueue::empty() const{ return queue_length == 0; }
Move MoveQueue::top() const{ return std::get<1>(move_array[0]); }
int MoveQueue::top_prio() const{ return std::get<0>(move_array[0]); }
void MoveQueue::pop(){
	std::pop_heap(move_array.data(), move_array.data() + queue_length);
	queue_length--;
	num_dequed_moves++;
}
void MoveQueue::heapify(){ std::make_heap(move_array.data(), move_array.data() + queue_length); }

constexpr int piece_at_square(const Square to, const ABCMask abc){
	const BitMask mask = ToMask(to);
	return ((abc.A & mask) ? 4 : 0) + ((abc.B & mask) ? 2 : 0) + ((abc.C & mask) ? 1 : 0);
}

constexpr int pawn_fear_penalty(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return pawn_fear_pawn;
	if (atk.King & mask) return pawn_fear_king;
	if (atk.Knight & mask) return pawn_fear_knight;
	if (atk.Rook & mask) return pawn_fear_rook;
	if (atk.Bishop & mask) return pawn_fear_bishop;
	if (atk.Queen & mask) return pawn_fear_queen;
	return 0;
}
constexpr int knight_fear_penalty(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return knight_fear_pawn;
	if (atk.King & mask) return knight_fear_king;
	if (atk.Bishop & mask) return knight_fear_bishop;
	if (atk.Rook & mask) return knight_fear_rook;
	if (atk.Queen & mask) return knight_fear_queen;
	if (atk.Knight & mask) return knight_fear_knight;
	return 0;
}
constexpr int bishop_fear_penalty(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return bishop_fear_pawn;
	if (atk.Knight & mask) return bishop_fear_knight;
	if (atk.King & mask) return bishop_fear_king;
	if (atk.Rook & mask) return bishop_fear_rook;
	if (atk.Bishop & mask) return bishop_fear_bishop;
	if (atk.Queen & mask) return bishop_fear_queen;
	return 0;
}
constexpr int rook_fear_penalty(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return rook_fear_pawn;
	if (atk.Bishop & mask) return rook_fear_bishop;
	if (atk.Knight & mask) return rook_fear_knight;
	if (atk.King & mask) return rook_fear_king;
	if (atk.Rook & mask) return rook_fear_rook;
	if (atk.Queen & mask) return rook_fear_queen;
	return 0;
}
constexpr int queen_fear_penalty(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return queen_fear_pawn;
	if (atk.Knight & mask) return queen_fear_knight;
	if (atk.Bishop & mask) return queen_fear_bishop;
	if (atk.Rook & mask) return queen_fear_rook;
	if (atk.Queen & mask) return queen_fear_queen;
	if (atk.King & mask) return queen_fear_king;
	return 0;
}

constexpr int pawn_evade_bonus(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return pawn_evade_pawn;
	if (atk.Knight & mask) return pawn_evade_knight;
	if (atk.Bishop & mask) return pawn_evade_bishop;
	//if (atk.King & mask) return pawn_evade_king;
	//if (atk.Rook & mask) return pawn_evade_rook;
	//if (atk.Queen & mask) return pawn_evade_queen;
	return 0;
}
constexpr int knight_evade_bonus(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return knight_evade_pawn;
	if (atk.King & mask) return knight_evade_king;
	if (atk.Rook & mask) return knight_evade_rook;
	//if (atk.Bishop & mask) return knight_evade_bishop;
	//if (atk.Knight & mask) return knight_evade_knight;
	//if (atk.Queen & mask) return knight_evade_queen;
	return 0;
}
constexpr int bishop_evade_bonus(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return bishop_evade_pawn;
	if (atk.Knight & mask) return bishop_evade_knight;
	if (atk.King & mask) return bishop_evade_king;
	if (atk.Queen & mask) return bishop_evade_queen;
	if (atk.Rook & mask) return bishop_evade_rook;
	if (atk.Bishop & mask) return bishop_evade_bishop;
	return 0;
}
constexpr int rook_evade_bonus(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return rook_evade_pawn;
	if (atk.Bishop & mask) return rook_evade_bishop;
	if (atk.Knight & mask) return rook_evade_knight;
	if (atk.King & mask) return rook_evade_king;
	if (atk.Rook & mask) return rook_evade_rook;
	if (atk.Queen & mask) return rook_evade_queen;
	return 0;
}
constexpr int queen_evade_bonus(const Square to, const Attacks& atk){
	const BitMask mask = ToMask(to);
	if (atk.Pawn & mask) return queen_evade_pawn;
	if (atk.Rook & mask) return queen_evade_rook;
	if (atk.Bishop & mask) return queen_evade_bishop;
	if (atk.Knight & mask) return queen_evade_knight;
	if (atk.Queen & mask) return queen_evade_queen;
	return 0;
}

inline void MoveQueue::push_move_helper(int priority, const Move move){
	if (queue_length >= move_array_max_size) {
		throw std::runtime_error("Move array full!");
	}

	if (move == Hint) priority += 100000;
	if (move == Killer1) priority += 100;
	if (move == Killer2) priority += 100;
	
	move_array[queue_length] = std::make_tuple(priority, move);
	queue_length++;
}

template <bool include_underpromotions>
inline void MoveQueue::handle_promotions(const Square from, const Square to, const int freq){
	if (include_underpromotions) {
		const Move n = move_from_squares(from, to, PROMOTE_TO_KNIGHT);
		push_move_helper(freq + underpromote_to_knight_freq + (CheckSquares.Knight & ToMask(to) ? knight_check : 0), n);
		const Move b = move_from_squares(from, to, PROMOTE_TO_BISHOP);
		push_move_helper(freq + underpromote_to_bishop_freq + (CheckSquares.Bishop & ToMask(to) ? bishop_check : 0), b);
		const Move r = move_from_squares(from, to, PROMOTE_TO_ROOK);
		push_move_helper(freq + underpromote_to_rook_freq + (CheckSquares.Rook & ToMask(to) ? rook_check : 0), r);
	}
	const Move q = move_from_squares(from, to, PROMOTE_TO_QUEEN);
	push_move_helper(freq + (CheckSquares.Queen & ToMask(to) ? queen_check : 0), q);
}

template <bool white>
void MoveQueue::push_knight_move(const Square from, const Square to){
	const Move move = move_from_squares(from, to, KNIGHT_MOVE);
	int base_prio = (white ? white_knight_freq : black_knight_freq)[to] 
		+ knight_capture_freq[piece_at_square(to, EnemyABC)]
		- knight_fear_penalty(to, EnemyAtk) + knight_evade_bonus(from, EnemyAtk);
	if (CheckSquares.Knight & ToMask(to)) base_prio += knight_check;
	push_move_helper(base_prio, move);
}
template <bool white>
void MoveQueue::push_bishop_move(const Square from, const Square to){
	const Move move = move_from_squares(from, to, BISHOP_MOVE);
	int base_prio = (white ? white_bishop_freq : black_bishop_freq)[to] 
		+ bishop_capture_freq[piece_at_square(to, EnemyABC)]
		- bishop_fear_penalty(to, EnemyAtk) + bishop_evade_bonus(from, EnemyAtk);
	if (CheckSquares.Bishop & ToMask(to)) base_prio += bishop_check;
	push_move_helper(base_prio, move);
}
template <bool white>
void MoveQueue::push_rook_move(const Square from, const Square to){
	const Move move = move_from_squares(from, to, ROOK_MOVE);
	int base_prio = (white ? white_rook_freq : black_rook_freq)[to] 
		+ rook_capture_freq[piece_at_square(to, EnemyABC)]
		- rook_fear_penalty(to, EnemyAtk) + rook_evade_bonus(from, EnemyAtk);
	if (CheckSquares.Rook & ToMask(to)) base_prio += rook_check;
	push_move_helper(base_prio, move);
}
template <bool white>
void MoveQueue::push_queen_move(const Square from, const Square to){
	const Move move = move_from_squares(from, to, QUEEN_MOVE);
	int base_prio = (white ? white_queen_freq : black_queen_freq)[to] 
		+ queen_capture_freq[piece_at_square(to, EnemyABC)]
		- queen_fear_penalty(to, EnemyAtk) + queen_evade_bonus(from, EnemyAtk);
	if (CheckSquares.Queen & ToMask(to)) base_prio += queen_check;
	push_move_helper(base_prio, move);
}
template <bool white>
void MoveQueue::push_king_move(const Square from, const Square to){
	const Move move = move_from_squares(from, to, KING_MOVE);
	const int base_prio = (white ? white_king_freq : black_king_freq)[to] + king_capture_freq[piece_at_square(to, EnemyABC)];
	push_move_helper(base_prio, move);
}
template <bool white>
void MoveQueue::push_castle_qs(){
	const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, C8), CASTLE_QUEENSIDE);
	const int freq = white ? white_castle_qs_freq : black_castle_qs_freq;
	push_move_helper(freq + (CheckSquares.Rook & ToMask(white ? D1 : D8) ? rook_check : 0), move);
}
template <bool white>
void MoveQueue::push_castle_ks(){
	const Move move = move_from_squares(FlipIf(white, E8), FlipIf(white, G8), CASTLE_KINGSIDE);
	const int freq = white ? white_castle_ks_freq : black_castle_ks_freq;
	push_move_helper(freq + (CheckSquares.Rook & ToMask(white ? F1 : F8) ? rook_check : 0), move);
}
template <bool white, bool include_underpromotions>
void MoveQueue::push_single_pawn_move(const Square from){
	const Square to = white ? (from + 8) : (from - 8);
	int freq = (white ? white_pawn_freq : black_pawn_freq)[to] 
		- pawn_fear_penalty(to, EnemyAtk) + pawn_evade_bonus(from, EnemyAtk);
	if (white ? (to >= A8) : (to <= H1)) {
		handle_promotions<include_underpromotions>(from, to, freq);
	} else {
		const Move move = move_from_squares(from, to, SINGLE_PAWN_PUSH);
		if (CheckSquares.Pawn & ToMask(to)) freq += pawn_check;
		push_move_helper(freq, move);
	}
}
template <bool white>
void MoveQueue::push_double_pawn_move(const Square from){
	const Square to = white ? (from + 16) : (from - 16);
	const Move move = move_from_squares(from, to, DOUBLE_PAWN_PUSH);
	int move_prio = (white ? white_pawn_freq : black_pawn_freq)[to]
		- pawn_fear_penalty(to, EnemyAtk) + pawn_evade_bonus(from, EnemyAtk);
	if (CheckSquares.Pawn & ToMask(to)) move_prio += pawn_check;
	push_move_helper(move_prio, move);
}
template <bool white, bool include_underpromotions>
void MoveQueue::push_pawn_capture_left(const Square from){
	const Square to = white ? (from + 7) : (from - 7);
	int freq = (white ? white_pawn_freq : black_pawn_freq)[to] + pawn_capture_freq[piece_at_square(to, EnemyABC)] 
		- pawn_fear_penalty(to, EnemyAtk) + pawn_evade_bonus(from, EnemyAtk);
	if (white ? (to >= A8) : (to <= H1)) {
		handle_promotions<include_underpromotions>(from, to, freq);
	} else {
		const Move move = move_from_squares(from, to, PAWN_CAPTURE);
		if (CheckSquares.Pawn & ToMask(to)) freq += pawn_check;
		push_move_helper(freq, move);
	}
}
template <bool white, bool include_underpromotions>
void MoveQueue::push_pawn_capture_right(const Square from){
	const Square to = white ? (from + 9) : (from - 9);
	int freq =(white ? white_pawn_freq : black_pawn_freq)[to] + pawn_capture_freq[piece_at_square(to, EnemyABC)] 
		- pawn_fear_penalty(to, EnemyAtk) + pawn_evade_bonus(from, EnemyAtk);
	if (white ? (to >= A8) : (to <= H1)) {
		handle_promotions<include_underpromotions>(from, to, freq);
	} else {
		const Move move = move_from_squares(from, to, PAWN_CAPTURE);
		if (CheckSquares.Pawn & ToMask(to)) freq += pawn_check;
		push_move_helper(freq, move);
	}
}
template <bool white>
void MoveQueue::push_ep_capture_left(const Square from){
	const Square to = white ? (from + 7) : (from - 7);
	const Move move = move_from_squares(from, to, EN_PASSANT_CAPTURE);
	int freq = white ? white_en_passant_freq : black_en_passant_freq;
	if (CheckSquares.Pawn & ToMask(to)) freq += pawn_check;
	push_move_helper(freq, move);
}
template <bool white>
void MoveQueue::push_ep_capture_right(const Square from){
	const Square to = white ? (from + 9) : (from - 9);
	const Move move = move_from_squares(from, to, EN_PASSANT_CAPTURE);
	int freq = white ? white_en_passant_freq : black_en_passant_freq;
	if (CheckSquares.Pawn & ToMask(to)) freq += pawn_check;
	push_move_helper(freq, move);
}

template void MoveQueue::push_knight_move<true>(const Square, const Square);
template void MoveQueue::push_knight_move<false>(const Square, const Square);
template void MoveQueue::push_bishop_move<true>(const Square, const Square);
template void MoveQueue::push_bishop_move<false>(const Square, const Square);
template void MoveQueue::push_rook_move<true>(const Square, const Square);
template void MoveQueue::push_rook_move<false>(const Square, const Square);
template void MoveQueue::push_queen_move<true>(const Square, const Square);
template void MoveQueue::push_queen_move<false>(const Square, const Square);
template void MoveQueue::push_king_move<true>(const Square, const Square);
template void MoveQueue::push_king_move<false>(const Square, const Square);
template void MoveQueue::push_castle_qs<true>();
template void MoveQueue::push_castle_qs<false>();
template void MoveQueue::push_castle_ks<true>();
template void MoveQueue::push_castle_ks<false>();
template void MoveQueue::push_single_pawn_move<true, true>(const Square);
template void MoveQueue::push_single_pawn_move<true, false>(const Square);
template void MoveQueue::push_single_pawn_move<false, true>(const Square);
template void MoveQueue::push_single_pawn_move<false, false>(const Square);
template void MoveQueue::push_double_pawn_move<true>(const Square);
template void MoveQueue::push_double_pawn_move<false>(const Square);
template void MoveQueue::push_pawn_capture_left<true, true>(const Square);
template void MoveQueue::push_pawn_capture_left<true, false>(const Square);
template void MoveQueue::push_pawn_capture_left<false, true>(const Square);
template void MoveQueue::push_pawn_capture_left<false, false>(const Square);
template void MoveQueue::push_pawn_capture_right<true, true>(const Square);
template void MoveQueue::push_pawn_capture_right<true, false>(const Square);
template void MoveQueue::push_pawn_capture_right<false, true>(const Square);
template void MoveQueue::push_pawn_capture_right<false, false>(const Square);
template void MoveQueue::push_ep_capture_left<true>(const Square);
template void MoveQueue::push_ep_capture_left<false>(const Square);
template void MoveQueue::push_ep_capture_right<true>(const Square);
template void MoveQueue::push_ep_capture_right<false>(const Square);

template <bool white>
inline void adjust_frequency_param_for_move(const Move move, const int change){
	switch (move_flags(move)){
		case KNIGHT_MOVE: (white ? white_knight_freq : black_knight_freq)[move_destination(move)] += change; break;
		case BISHOP_MOVE: (white ? white_bishop_freq : black_bishop_freq)[move_destination(move)] += change; break;
		case ROOK_MOVE: (white ? white_rook_freq : black_rook_freq)[move_destination(move)] += change; break;
		case QUEEN_MOVE: (white ? white_queen_freq : black_queen_freq)[move_destination(move)] += change; break;
		case KING_MOVE: (white ? white_king_freq : black_king_freq)[move_destination(move)] += change; break;
		case CASTLE_QUEENSIDE: (white ? white_castle_qs_freq : black_castle_qs_freq) += change; break;
		case CASTLE_KINGSIDE: (white ? white_castle_ks_freq : black_castle_ks_freq) += change; break;
		case EN_PASSANT_CAPTURE: (white ? white_en_passant_freq : black_en_passant_freq) += change; break;
		default: (white ? white_pawn_freq : black_pawn_freq)[move_destination(move)] += change;
	}
}

template <bool white>
void MoveQueue::update_frequency_for_beta_cutoff(){
	adjust_frequency_param_for_move<white>(top(), num_dequed_moves);
	for (size_t i = 0; i < num_dequed_moves; i++){
		adjust_frequency_param_for_move<white>(std::get<1>(move_array[queue_length + i]), -1);
	}
}

template void MoveQueue::update_frequency_for_beta_cutoff<true>();
template void MoveQueue::update_frequency_for_beta_cutoff<false>();

void show_board_array(std::array<int, 64> arr){
	for (size_t i = 0; i < 64; i++){
		std::cout << "\t" << arr[i] << ",";
		if (i%8 == 7) std::cout << std::endl;
	}
}

void show_move_order_values(){
	std::cout << "white pawn:" << std::endl;
	show_board_array(white_pawn_freq);
	std::cout << "white knight:" << std::endl;
	show_board_array(white_knight_freq);
	std::cout << "white bishop:" << std::endl;
	show_board_array(white_bishop_freq);
	std::cout << "white rook:" << std::endl;
	show_board_array(white_rook_freq);
	std::cout << "white queen:" << std::endl;
	show_board_array(white_queen_freq);
	std::cout << "white king:" << std::endl;
	show_board_array(white_king_freq);
	std::cout << "white castle queenside:" << white_castle_qs_freq << std::endl;
	std::cout << "white castle kingside:" << white_castle_ks_freq << std::endl;
	std::cout << "white en passant:" << white_en_passant_freq << std::endl;

	std::cout << "black pawn:" << std::endl;
	show_board_array(black_pawn_freq);
	std::cout << "black knight:" << std::endl;
	show_board_array(black_knight_freq);
	std::cout << "black bishop:" << std::endl;
	show_board_array(black_bishop_freq);
	std::cout << "black rook:" << std::endl;
	show_board_array(black_rook_freq);
	std::cout << "black queen:" << std::endl;
	show_board_array(black_queen_freq);
	std::cout << "black king:" << std::endl;
	show_board_array(black_king_freq);
	std::cout << "black castle queenside:" << black_castle_qs_freq << std::endl;
	std::cout << "black castle kingside:" << black_castle_ks_freq << std::endl;
	std::cout << "black en passant:" << black_en_passant_freq << std::endl;

}
