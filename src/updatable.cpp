# include "updatable.hpp"
# include "bmi2_fallback.hpp"

template <bool white>
void initialize_helper(EfficientlyUpdatable &ue, const HalfBoard &hb){
    Bitloop(hb.Pawn, x) ue.add_piece<white, PAWN>(TZCNT(x));
    Bitloop(hb.Knight, x) ue.add_piece<white, KNIGHT>(TZCNT(x));
    Bitloop(hb.Bishop, x) ue.add_piece<white, BISHOP>(TZCNT(x));
    Bitloop(hb.Rook, x) ue.add_piece<white, ROOK>(TZCNT(x));
    Bitloop(hb.Queen, x) ue.add_piece<white, QUEEN>(TZCNT(x));
    ue.add_piece<white, KING>(hb.King);

    if (hb.Castle & ToMask(white ? A1 : A8)){ ue.hash ^= (white ? white_cqs_hash : black_cqs_hash); }
	if (hb.Castle & ToMask(white ? H1 : H8)){ ue.hash ^= (white ? white_cks_hash : black_cks_hash); }

}

EfficientlyUpdatable initialize_ue(const HalfBoard &white, const HalfBoard &black){
    EfficientlyUpdatable ue = EfficientlyUpdatable{FirstLayer{vector_zero, vector_zero, vector_zero, vector_zero}, 0ull};
    initialize_helper<true>(ue, white);
    initialize_helper<false>(ue, black);
    return ue;
}
