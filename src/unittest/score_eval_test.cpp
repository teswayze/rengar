# include "doctest.h"
# include "../tune/score_eval.hpp"
# include "../math.hpp"

TEST_CASE("Quantmoid"){
    CHECK(quantmoid(-250) == -126);
    CHECK(quantmoid(-200) == -121);
    CHECK(quantmoid(-150) == -105);
    CHECK(quantmoid(-100) == -80);
    CHECK(quantmoid(-50) == -45);
    CHECK(quantmoid(0) == 0);
    CHECK(quantmoid(50) == 45);
    CHECK(quantmoid(100) == 80);
    CHECK(quantmoid(150) == 105);
    CHECK(quantmoid(200) == 121);
    CHECK(quantmoid(250) == 126);
}

TEST_CASE("Double bongcloud"){
    auto e4 = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);
    auto e5 = move_from_squares(E7, E5, DOUBLE_PAWN_PUSH);
    auto Ke2 = move_from_squares(E1, E2, KING_MOVE);
    auto Ke7 = move_from_squares(E8, E7, KING_MOVE);
    auto Ke1 = move_from_squares(E2, E1, KING_MOVE);
    auto Ke8 = move_from_squares(E7, E8, KING_MOVE);
    auto game = {e4, e5, Ke2, Ke7, Ke1, Ke8};

    CHECK(score_eval(game, 0) < 10'000);
    CHECK(score_eval(game, 1) > 10'000);
    CHECK(score_eval(game, -1) > 10'000);
}

TEST_CASE("Quick queen trade"){
    auto e4 = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);
    auto d6 = move_from_squares(D7, D6, SINGLE_PAWN_PUSH);
    auto d4 = move_from_squares(D2, D4, DOUBLE_PAWN_PUSH);
    auto e5 = move_from_squares(E7, E5, DOUBLE_PAWN_PUSH);
    auto dxe5_w = move_from_squares(D4, E5, PAWN_CAPTURE);
    auto dxe5_b = move_from_squares(D6, E5, PAWN_CAPTURE);
    auto Qxe8 = move_from_squares(D1, D8, QUEEN_MOVE);
    auto Kxe8 = move_from_squares(E8, D8, KING_MOVE);
    auto game = {e4, d6, d4, e5, dxe5_w, dxe5_b, Qxe8, Kxe8};

    CHECK(score_eval(game, 0) < 1'000);
    CHECK(score_eval(game, 1) > 1'000);
    CHECK(score_eval(game, -1) > 1'000);
}

TEST_CASE("White blunders queen"){
    auto e4 = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);
    auto d5 = move_from_squares(D7, D5, DOUBLE_PAWN_PUSH);
    auto Qg4 = move_from_squares(D1, G4, QUEEN_MOVE);
    auto Bxg4 = move_from_squares(C8, G4, BISHOP_MOVE);
    auto h3 = move_from_squares(H2, H3, SINGLE_PAWN_PUSH);
    auto Bh5 = move_from_squares(G4, H5, BISHOP_MOVE);
    auto Bd3 = move_from_squares(F1, D3, BISHOP_MOVE);
    auto Nc6 = move_from_squares(B8, C6, KNIGHT_MOVE);
    auto Ne2 = move_from_squares(G1, E2, KNIGHT_MOVE);
    auto Nb4 = move_from_squares(C6, B4, KNIGHT_MOVE);
    auto game = {e4, d5, Qg4, Bxg4, h3, Bh5, Bd3, Nc6, Ne2, Nb4};

    CHECK(score_eval(game, 0) > 75'000);
    CHECK(score_eval(game, 1) > 75'000);
    CHECK(score_eval(game, -1) < 75'000);
}

TEST_CASE("Black blunders queen"){
    auto e4 = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);
    auto e5 = move_from_squares(E7, E5, DOUBLE_PAWN_PUSH);
    auto Nf3 = move_from_squares(G1, F3, KNIGHT_MOVE);
    auto Qh4 = move_from_squares(D8, H4, QUEEN_MOVE);
    auto Nxh4 = move_from_squares(F3, H4, KNIGHT_MOVE);
    auto Nf6 = move_from_squares(G8, F6, KNIGHT_MOVE);
    auto Nc3 = move_from_squares(B1, C3, KNIGHT_MOVE);
    auto Bc5 = move_from_squares(F8, C5, BISHOP_MOVE);
    auto d3 = move_from_squares(D2, D3, SINGLE_PAWN_PUSH);
    auto Nc6 = move_from_squares(B8, C6, KNIGHT_MOVE);
    auto game = {e4, e5, Nf3, Qh4, Nxh4, Nf6, Nc3, Bc5, d3, Nc6};

    CHECK(score_eval(game, 0) > 75'000);
    CHECK(score_eval(game, 1) < 75'000);
    CHECK(score_eval(game, -1) > 75'000);
}