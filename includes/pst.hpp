# pragma once

# include "bitboard.hpp"

struct PstEvalInfo{
	int mg_kk;
	int mg_qk;
	int mg_kq;
	int mg_qq;
	int eg;
	int phase_count;

	uint64_t hash;

	PstEvalInfo() = default;
	PstEvalInfo(const PstEvalInfo&) = delete;

	PstEvalInfo copy() const {
		return PstEvalInfo{ mg_kk, mg_qk, mg_kq, mg_qq, eg, phase_count, hash };
	}

	template <bool white>
	void move_pawn(const Square from, const Square to);
	template <bool white>
	void move_knight(const Square from, const Square to);
	template <bool white>
	void move_bishop(const Square from, const Square to);
	template <bool white>
	void move_rook(const Square from, const Square to);
	template <bool white>
	void move_queen(const Square from, const Square to);
	template <bool white>
	void move_king(const Square from, const Square to);

	template <bool white>
	void remove_pawn(const Square square);
	template <bool white>
	void remove_knight(const Square square);
	template <bool white>
	void remove_bishop(const Square square);
	template <bool white>
	void remove_rook(const Square square);
	template <bool white>
	void remove_queen(const Square square);

	template <bool white>
	void promote_pawn_to_knight(const Square from, const Square to);
	template <bool white>
	void promote_pawn_to_bishop(const Square from, const Square to);
	template <bool white>
	void promote_pawn_to_rook(const Square from, const Square to);
	template <bool white>
	void promote_pawn_to_queen(const Square from, const Square to);

	template <bool white>
	void castle_queenside();
	template <bool white>
	void castle_kingside();
};

PstEvalInfo half_to_full_eval_info(const PstEvalInfo &w, const PstEvalInfo &b);

template <bool white>
PstEvalInfo static_eval_info(const BitMask pawn, const BitMask knight, const BitMask bishop,
		const BitMask rook, const BitMask queen, const Square king, const BitMask castle);
