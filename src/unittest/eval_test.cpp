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

const Vector a = vector_set(216, 984, -22, -270, 504, 479, 865, 87, -882, 1002, 394, 8, -704, -194, 535, 314);
const Vector b = vector_set(-510, 897, -707, -814, 265, -295, 654, -396, -592, 1003, -390, -567, -481, 72, 388, 455);

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
TEST_CASE("Vector dot"){
    const int dot = vector_dot(a, b);
    auto a_it = vector_iterator(a).begin();
    auto b_it = vector_iterator(b).begin();
    int expected = 0;
    for (auto i = 0; i < 16; i++) {
        expected += *a_it * *b_it;
        a_it++; b_it++;
    }
    CHECK(expected / 256 == dot);
}

const Matrix M = {
    vector_set(918, 562, 314, -508, -59, -231, -526, 44, 589, 27, 821, 337, -402, -140, 298, 467),
    vector_set(-1008, 65, -827, -998, 368, 739, 472, 904, 283, 666, 617, 23, 778, 708, -24, 127),
    vector_set(280, 382, 357, -868, -90, -710, -428, 167, -375, 289, 419, 779, 280, 656, -272, 88),
    vector_set(794, 968, -780, 349, -437, 191, 303, -95, 729, 152, -328, 322, 370, 846, 772, -127),
    vector_set(1017, 254, -562, 775, -459, -554, 455, 515, 589, 988, -874, -308, -760, -946, -311, 76),
    vector_set(333, 44, -491, 543, 549, -152, 776, 814, -59, -454, 753, 509, -813, -927, -754, 11),
    vector_set(-744, 893, -174, 904, -134, -871, 715, 477, 661, -607, 141, 194, -417, 573, -534, -871),
    vector_set(339, -140, -158, -893, 493, 733, -408, 867, -757, 695, 865, -534, 851, -553, 199, -317),
    vector_set(-696, 1000, -847, 567, 16, -122, 402, -853, -1003, -393, 945, -745, -538, 910, 1010, -397),
    vector_set(-414, 694, 853, 11, 97, -840, -706, 569, 730, -618, 232, 97, -518, 980, 26, 498),
    vector_set(-113, -882, 213, 629, -212, 647, 718, -327, 956, -112, 537, -4, 138, -573, -824, 822),
    vector_set(326, -603, -154, -263, -1002, 297, -946, -804, -334, -654, 304, 462, 425, 463, 558, 856),
    vector_set(-367, 925, 235, 558, -572, -139, 323, -49, 303, -639, -610, -960, -180, 548, -74, -546),
    vector_set(-478, -381, 469, 596, 49, 823, 289, -770, -355, -968, -301, 288, 29, -801, 331, 137),
    vector_set(-914, 612, 33, 521, 597, -50, -461, -432, 170, -191, 621, 212, -999, 114, -21, -235),
    vector_set(-979, 272, 671, -548, -113, 500, 788, 252, -357, -788, -937, -632, -284, 54, 432, -145),
};

TEST_CASE("Matmul helper 1"){
    const auto mmh1 = _matmul_helper2(M.data(), a);
    for (int i = 0; i < 4; i++) {
        const int expected = vector_dot(M[i], a);
        const int result = ((int*)&mmh1)[i];
        CHECK(expected == result);
    }
}

TEST_CASE("Matrix Multiply"){
    const auto Ma = matmul(M, a);
    auto ma_it = vector_iterator(Ma).begin();
    for (int i = 0; i < 16; i++){
        CHECK(vector_dot(M[i], a) == *ma_it);
        ma_it++;
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
