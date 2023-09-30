# include <climits>
# include "doctest.h"
# include "search.hpp"
# include "parse_format.hpp"
# include "hashtable.hpp"

const auto pawn_fork_fen_kf8 = "4rk2/1p2qp1p/p3pn1p/8/B1N5/8/3Q1PP1/5NK1 b - - 0 1";
const auto escape_check_fen = "4r1k1/4qp1p/p3pn2/1p4Q1/B1N5/8/5PP1/5NK1 b - - 2 3";
const auto pawn_fork_fen_kg8 = "4r1k1/1p2qp1p/p3pn2/8/B1N5/8/3Q1PP1/5NK1 b - - 0 1";

TEST_CASE("Black finds fork"){
    Board board;
    parse_fen(pawn_fork_fen_kf8, board);
    History history;
    ht_init(12);
    auto move = search_for_move<false>(board, history, INT_MAX, 6, INT_MAX, INT_MAX);
    CHECK(move == move_from_squares(B7, B5, DOUBLE_PAWN_PUSH));
}

TEST_CASE("Black escapes perpetual check"){
    Board board;
    parse_fen(escape_check_fen, board);
    History history;
    ht_init(12);
    auto move = search_for_move<false>(board, history, INT_MAX, 6, INT_MAX, INT_MAX);
    CHECK(move == move_from_squares(G8, H8, KING_MOVE));
}

TEST_CASE("Hash table pathology repro"){
    Board board;
    parse_fen(pawn_fork_fen_kg8, board);
    History history;
    ht_init(12);
    auto move = search_for_move<false>(board, history, INT_MAX, 6, INT_MAX, INT_MAX);
    CHECK(move == move_from_squares(B7, B5, DOUBLE_PAWN_PUSH));

    parse_fen(pawn_fork_fen_kf8, board);
    auto move2 = search_for_move<false>(board, history, INT_MAX, 6, INT_MAX, INT_MAX);
    CHECK(move2 == move_from_squares(B7, B5, DOUBLE_PAWN_PUSH));
}
