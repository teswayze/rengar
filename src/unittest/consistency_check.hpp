# pragma once

# include "../board.hpp"
# include "doctest.h"
# include "vector_helpers.hpp"

inline void check_consistent_hb(const HalfBoard &h){
	CHECK(h.All == (h.Pawn | h.Knight | h.Bishop | h.Rook | h.Queen | ToMask(h.King)));
}

inline void check_consistent_fb(const Board &b){
	check_consistent_hb(b.White);
	check_consistent_hb(b.Black);
	
	Board copy = b.EPMask ? from_sides_without_eval_ep(b.White, b.Black, TZCNT(b.EPMask)) : from_sides_without_eval(b.White, b.Black);
	CHECK(b.Occ == copy.Occ);

	CHECK(b.ue.hash == copy.ue.hash);
	check_equal(b.ue.l1.full_symm, copy.ue.l1.full_symm);
	check_equal(b.ue.l1.vert_asym, copy.ue.l1.vert_asym);
	check_equal(b.ue.l1.horz_asym, copy.ue.l1.horz_asym);
	check_equal(b.ue.l1.rotl_asym, copy.ue.l1.rotl_asym);
	CHECK(memcmp(&b.ue.l1v2, &b.ue.l1v2, sizeof(b.ue.l1v2)) == 0);

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
