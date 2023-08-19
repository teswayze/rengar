# pragma once

# include "board.hpp"
# include "doctest.h"

inline void check_consistent_hb(const HalfBoard &h){
	CHECK(h.All == (h.Pawn | h.Knight | h.Bishop | h.Rook | h.Queen | ToMask(h.King)));
}

inline void check_consistent_fb(const Board &b){
	check_consistent_hb(b.White);
	check_consistent_hb(b.Black);
	
	Board copy = b.EPMask ? from_sides_without_eval_ep(b.White, b.Black, TZCNT(b.EPMask)) : from_sides_without_eval(b.White, b.Black);
	CHECK(b.Occ == copy.Occ);
	CHECK(b.EvalInfo.mg == copy.EvalInfo.mg);
	CHECK(b.EvalInfo.eg == copy.EvalInfo.eg);
	CHECK(b.EvalInfo.phase_count == copy.EvalInfo.phase_count);
	CHECK(b.EvalInfo.hash == copy.EvalInfo.hash);

	CHECK(b.WtAtk.Pawn == copy.WtAtk.Pawn);
	CHECK(b.WtAtk.Knight == copy.WtAtk.Knight);
	CHECK(b.WtAtk.Bishop == copy.WtAtk.Bishop);
	CHECK(b.WtAtk.Rook == copy.WtAtk.Rook);
	CHECK(b.WtAtk.Queen == copy.WtAtk.Queen);
	CHECK(b.WtAtk.King == copy.WtAtk.King);

	CHECK(b.BkAtk.Pawn == copy.BkAtk.Pawn);
	CHECK(b.BkAtk.Knight == copy.BkAtk.Knight);
	CHECK(b.BkAtk.Bishop == copy.BkAtk.Bishop);
	CHECK(b.BkAtk.Rook == copy.BkAtk.Rook);
	CHECK(b.BkAtk.Queen == copy.BkAtk.Queen);
	CHECK(b.BkAtk.King == copy.BkAtk.King);
}
