# include "doctest.h"
# include "../parse_format.hpp"
# include "../eval.hpp"
# include "../updatable.hpp"
# include "vector_helpers.hpp"

TEST_CASE("Vector zero"){
    for (auto val : vector_iterator(vector_zero)) CHECK(val == 0);
}

const Vector x = vector_set(
    29734, 236, -20595, -27575, -256, 17290, 10956, -24954, 
    19280, -11327, 145, -12158, 16333, -25016, 10990, -1539);
const Vector y = vector_set(
    3463, -742, 25779, 21781, -11009, 12646, 2963, 18901, 
    31628, -24323, -12821, 4765, 24478, -9841, 16947, -5563);

TEST_CASE("Vector add"){
    const auto sum = vector_add(x, y);
    auto x_it = vector_iterator(x).begin();
    auto y_it = vector_iterator(y).begin();
    for (auto sum_val : vector_iterator(sum)){
        CHECK(std::clamp(*x_it + *y_it, -32768, 32767) == sum_val);
        x_it++; y_it++;
    }
}
TEST_CASE("Vector sub"){
    const auto diff = vector_sub(x, y);
    auto x_it = vector_iterator(x).begin();
    auto y_it = vector_iterator(y).begin();
    for (auto diff_val : vector_iterator(diff)){
        CHECK(std::clamp(*x_it - *y_it, -32768, 32767) == diff_val);
        x_it++; y_it++;
    }
}
TEST_CASE("Vector abs"){
    const auto abs_x = vector_abs(x);
    auto x_it = vector_iterator(x).begin();
    for (auto abs_val : vector_iterator(abs_x)){
        CHECK(std::abs(*x_it) == abs_val);
        x_it++;
    }
}

Vector a = vector_set(216, 984, -22, -270, 504, 479, 865, 87, -882, 1002, 394, 8, -704, -194, 535, 314);
Vector b = vector_set(-510, 897, -707, -814, 265, -295, 654, -396, -592, 1003, -390, -567, -481, 72, 388, 455);

TEST_CASE("Vector clamp mul"){
    const auto activated = vector_clamp_mul(a, b);
    auto a_it = vector_iterator(a).begin();
    auto b_it = vector_iterator(b).begin();
    for (auto act_val : vector_iterator(activated)){
        int product = std::clamp(*a_it / 4, -181, 181) * std::clamp(*b_it / 4, -181, 181);
        CHECK(product / 16 == act_val);
        a_it++; b_it++;
    }
}

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

        check_equal(fpo.l1.full_symm, fpo_hflip.l1.full_symm);
        check_equal(fpo.l1.vert_asym, fpo_hflip.l1.vert_asym);
        check_opposite(fpo.l1.horz_asym, fpo_hflip.l1.horz_asym);
        check_opposite(fpo.l1.rotl_asym, fpo_hflip.l1.rotl_asym);

        check_equal(fpo.l2.full_symm, fpo_hflip.l2.full_symm);
        check_equal(fpo.l2.vert_asym, fpo_hflip.l2.vert_asym);

        CHECK(fpo.eval == fpo_hflip.eval);
    }

    SUBCASE("Vertical"){
        const auto fpo_vflip = fpo_from_fen(KID_FEN_VFLIP);

        check_equal(fpo.l1.full_symm, fpo_vflip.l1.full_symm);
        check_opposite(fpo.l1.vert_asym, fpo_vflip.l1.vert_asym);
        check_equal(fpo.l1.horz_asym, fpo_vflip.l1.horz_asym);
        check_opposite(fpo.l1.rotl_asym, fpo_vflip.l1.rotl_asym);

        check_equal(fpo.l2.full_symm, fpo_vflip.l2.full_symm);
        check_opposite(fpo.l2.vert_asym, fpo_vflip.l2.vert_asym);

        CHECK(fpo.eval == -fpo_vflip.eval);
    }

    SUBCASE("Rotational"){
        const auto fpo_rot_180 = fpo_from_fen(KID_FEN_ROT_180);

        check_equal(fpo.l1.full_symm, fpo_rot_180.l1.full_symm);
        check_opposite(fpo.l1.vert_asym, fpo_rot_180.l1.vert_asym);
        check_opposite(fpo.l1.horz_asym, fpo_rot_180.l1.horz_asym);
        check_equal(fpo.l1.rotl_asym, fpo_rot_180.l1.rotl_asym);

        check_equal(fpo.l2.full_symm, fpo_rot_180.l2.full_symm);
        check_opposite(fpo.l2.vert_asym, fpo_rot_180.l2.vert_asym);

        CHECK(fpo.eval == -fpo_rot_180.eval);
    }

}
