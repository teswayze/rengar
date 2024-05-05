# include "doctest.h"
# include "../parse_format.hpp"
# include "../eval.hpp"
# include "../updatable.hpp"

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

FirstLayer fl_from_fen(const std::string fen){
    Board board;
    parse_fen(fen, board);
    return initialize_ue(board.White, board.Black).l1;
}

void check_equal(const __m256i left, const __m256i right){
    int16_t *left_ptr = (int16_t*) &left;
    int16_t *right_ptr = (int16_t*) &right;
    for (auto i = 0; i < 16; i ++) CHECK(left_ptr[i] == right_ptr[i]);
}

void check_opposite(const __m256i left, const __m256i right){
    int16_t *left_ptr = (int16_t*) &left;
    int16_t *right_ptr = (int16_t*) &right;
    for (auto i = 0; i < 16; i ++) CHECK(left_ptr[i] == -right_ptr[i]);
}

TEST_CASE("First layer symmetry"){
    const auto fl = fl_from_fen(KID_FEN);

    SUBCASE("Horizontal"){
        const auto fl_hflip = fl_from_fen(KID_FEN_HFLIP);
        check_equal(fl.full_symm, fl_hflip.full_symm);
        check_equal(fl.vert_asym, fl_hflip.vert_asym);
        check_opposite(fl.horz_asym, fl_hflip.horz_asym);
        check_opposite(fl.rotl_asym, fl_hflip.rotl_asym);
    }

    SUBCASE("Vertical"){
        const auto fl_vflip = fl_from_fen(KID_FEN_VFLIP);
        check_equal(fl.full_symm, fl_vflip.full_symm);
        check_opposite(fl.vert_asym, fl_vflip.vert_asym);
        check_equal(fl.horz_asym, fl_vflip.horz_asym);
        check_opposite(fl.rotl_asym, fl_vflip.rotl_asym);
    }

    SUBCASE("Rotational"){
        const auto fl_rot_180 = fl_from_fen(KID_FEN_ROT_180);
        check_equal(fl.full_symm, fl_rot_180.full_symm);
        check_opposite(fl.vert_asym, fl_rot_180.vert_asym);
        check_opposite(fl.horz_asym, fl_rot_180.horz_asym);
        check_equal(fl.rotl_asym, fl_rot_180.rotl_asym);
    }

}
