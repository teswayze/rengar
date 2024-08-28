# include "../external/doctest.h"
# include "../parse_format.hpp"
# include "../eval.hpp"
# include "../updatable.hpp"
# include "vector_helpers.hpp"

TEST_CASE("Starting Position"){
	Board board;
	parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
	CHECK(0 < eval<true>(board));
	CHECK(eval<true>(board) < 50);
}

const std::string KID_FEN = "r1bq1rk1/ppp1npbp/3p1np1/3Pp3/2P1P3/2N2N2/PP2BPPP/R1BQ1RK1 w - - 1 9";
const std::string KID_FEN_HFLIP = "1kr1qb1r/pbpn1ppp/1pn1p3/3pP3/3P1P2/2N2N2/PPPB2PP/1KR1QB1R w - - 1 9";
const std::string KID_FEN_VFLIP = "r1bq1rk1/pp2bppp/2n2n2/2p1p3/3pP3/3P1NP1/PPP1NPBP/R1BQ1RK1 b - - 1 9";
const std::string KID_FEN_ROT_180 = "1kr1qb1r/pppb2pp/2n2n2/3p1p2/3Pp3/1PN1P3/PBPN1PPP/1KR1QB1R b - - 1 9";

ForwardPassOutput fpo_from_fen(const std::string fen){
    Board board;
    const bool wtm = parse_fen(fen, board);
    return (wtm ? forward_pass<true> : forward_pass<false>)(board);
}

TEST_CASE("Evaluation symmetry"){
    const auto fpo = fpo_from_fen(KID_FEN);

    SUBCASE("Horizontal"){
        const auto fpo_hflip = fpo_from_fen(KID_FEN_HFLIP);

        check_equal<32>(fpo.l1.full_symm, fpo_hflip.l1.full_symm);
        check_equal<32>(fpo.l1.vert_asym, fpo_hflip.l1.vert_asym);
        check_opposite<32>(fpo.l1.horz_asym, fpo_hflip.l1.horz_asym);
        check_opposite<32>(fpo.l1.rotl_asym, fpo_hflip.l1.rotl_asym);

        check_equal<4>(fpo.l2.full_symm, fpo_hflip.l2.full_symm);
        check_equal<4>(fpo.l2.vert_asym, fpo_hflip.l2.vert_asym);

        CHECK(fpo.eval == fpo_hflip.eval);
    }

    SUBCASE("Vertical"){
        const auto fpo_vflip = fpo_from_fen(KID_FEN_VFLIP);

        check_equal<32>(fpo.l1.full_symm, fpo_vflip.l1.full_symm);
        check_opposite<32>(fpo.l1.vert_asym, fpo_vflip.l1.vert_asym);
        check_equal<32>(fpo.l1.horz_asym, fpo_vflip.l1.horz_asym);
        check_opposite<32>(fpo.l1.rotl_asym, fpo_vflip.l1.rotl_asym);

        check_equal<4>(fpo.l2.full_symm, fpo_vflip.l2.full_symm);
        check_opposite<4>(fpo.l2.vert_asym, fpo_vflip.l2.vert_asym);

        CHECK(fpo.eval == -fpo_vflip.eval);
    }

    SUBCASE("Rotational"){
        const auto fpo_rot_180 = fpo_from_fen(KID_FEN_ROT_180);

        check_equal<32>(fpo.l1.full_symm, fpo_rot_180.l1.full_symm);
        check_opposite<32>(fpo.l1.vert_asym, fpo_rot_180.l1.vert_asym);
        check_opposite<32>(fpo.l1.horz_asym, fpo_rot_180.l1.horz_asym);
        check_equal<32>(fpo.l1.rotl_asym, fpo_rot_180.l1.rotl_asym);

        check_equal<4>(fpo.l2.full_symm, fpo_rot_180.l2.full_symm);
        check_opposite<4>(fpo.l2.vert_asym, fpo_rot_180.l2.vert_asym);

        CHECK(fpo.eval == -fpo_rot_180.eval);
    }

}
