# include "lookup.hpp"
# include "board.hpp"
# include "eval.hpp"


int eval(const Board board)
{
    int mg = 0;
    int eg = 0;
    int game_phase = 0;

    Bitloop(board.White.Pawn, x){
    	mg += mg_pawn_table[Flip(SquareOf(x))] + MG_PAWN;
    	eg += eg_pawn_table[Flip(SquareOf(x))] + EG_PAWN;
    }
    Bitloop(board.White.Knight, x){
    	mg += mg_knight_table[Flip(SquareOf(x))] + MG_KNIGHT;
    	eg += eg_knight_table[Flip(SquareOf(x))] + EG_KNIGHT;
    	game_phase += 1;
    }
    Bitloop(board.White.Bishop, x){
    	mg += mg_bishop_table[Flip(SquareOf(x))] + MG_BISHOP;
    	eg += eg_bishop_table[Flip(SquareOf(x))] + EG_BISHOP;
    	game_phase += 1;
    }
    Bitloop(board.White.Rook, x){
    	mg += mg_rook_table[Flip(SquareOf(x))] + MG_ROOK;
    	eg += eg_rook_table[Flip(SquareOf(x))] + EG_ROOK;
    	game_phase += 2;
    }
    Bitloop(board.White.Queen, x){
    	mg += mg_queen_table[Flip(SquareOf(x))] + MG_QUEEN;
    	eg += eg_queen_table[Flip(SquareOf(x))] + EG_QUEEN;
    	game_phase += 4;
    }
    Bitloop(board.White.King, x){
    	mg += mg_king_table[Flip(SquareOf(x))];
    	eg += eg_king_table[Flip(SquareOf(x))];
    }

    Bitloop(board.Black.Pawn, x){
    	mg -= mg_pawn_table[SquareOf(x)] + MG_PAWN;
    	eg -= eg_pawn_table[SquareOf(x)] + EG_PAWN;
    }
    Bitloop(board.Black.Knight, x){
    	mg -= mg_knight_table[SquareOf(x)] + MG_KNIGHT;
    	eg -= eg_knight_table[SquareOf(x)] + EG_KNIGHT;
    	game_phase += 1;
    }
    Bitloop(board.Black.Bishop, x){
    	mg -= mg_bishop_table[SquareOf(x)] + MG_BISHOP;
    	eg -= eg_bishop_table[SquareOf(x)] + EG_BISHOP;
    	game_phase += 1;
    }
    Bitloop(board.Black.Rook, x){
    	mg -= mg_rook_table[SquareOf(x)] + MG_ROOK;
    	eg -= eg_rook_table[SquareOf(x)] + EG_ROOK;
    	game_phase += 2;
    }
    Bitloop(board.Black.Queen, x){
    	mg -= mg_queen_table[SquareOf(x)] + MG_QUEEN;
    	eg -= eg_queen_table[SquareOf(x)] + EG_QUEEN;
    	game_phase += 4;
    }
    Bitloop(board.Black.King, x){
    	mg -= mg_king_table[SquareOf(x)];
    	eg -= eg_king_table[SquareOf(x)];
    }

    // In case of early promotion
    const int mg_phase = (game_phase > 24) ? 24 : game_phase;
    const int eg_phase = 24 - mg_phase;
    return mg * mg_phase + eg * eg_phase;
}

int phase_of_game(const Board board){
	const int minor_count = __builtin_popcountll(
			board.White.Knight | board.White.Bishop | board.Black.Knight | board.Black.Bishop);
    const int rook_count = __builtin_popcountll(board.White.Rook | board.Black.Rook);
    const int queen_count = __builtin_popcountll(board.White.Queen | board.Black.Queen);
    const int game_phase = minor_count + 2 * rook_count + 4 * queen_count;
    return (game_phase > 24) ? 24 : game_phase;
}

# include "doctest.h"
# include "parse_format.hpp"

TEST_CASE("Starting Position"){
	Board board = std::get<1>(parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
	CHECK(eval(board) == 0);
	CHECK(phase_of_game(board) == 24);
}

TEST_CASE("Albin Countergambit Missed Brilliancy"){
	Board board = std::get<1>(parse_fen("rnbqk1nr/ppp2ppp/8/4P3/1BP5/8/PP2K1PP/RN1Q1BqR w kq - 0 8"));
	CHECK(eval(board) == -25032);
	CHECK(phase_of_game(board) == 24);
}

TEST_CASE("Pieceless Endgame"){
	Board board = std::get<1>(parse_fen("8/8/8/4k3/8/4K3/4P3/8 w - - 0 1"));
	CHECK(eval(board) == 2472);
	CHECK(phase_of_game(board) == 0);
}

TEST_CASE("Berlin Endgame"){
	Board board = std::get<1>(parse_fen("r1bk1b1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNB2RK1 w - - 0 9"));
	CHECK(eval(board) == 656);
	CHECK(phase_of_game(board) == 14);
}

TEST_CASE("Fried Liver"){
	Board board = std::get<1>(parse_fen("r1bq1b1r/ppp3pp/2n1k3/3np3/2B5/2N2Q2/PPPP1PPP/R1B1K2R b KQ - 3 8"));
	CHECK(eval(board) == -5102);
	CHECK(phase_of_game(board) == 23);
}

TEST_CASE("Grunfeld Exchange Sacrifice"){
	Board board = std::get<1>(parse_fen("r2q1rk1/pp2p2p/4bpp1/n2P4/4P3/3BBP2/P3N1PP/Q4RK1 w - - 0 16"));
	CHECK(eval(board) == -1292);
	CHECK(phase_of_game(board) == 19);
}

TEST_CASE("Positional Sveshnikov"){
	Board board = std::get<1>(parse_fen("r1bqk2r/5ppp/p1np1b2/1p1Np3/4P3/N7/PPP2PPP/R2QKB1R w KQkq - 0 11"));
	CHECK(eval(board) == -1160);
	CHECK(phase_of_game(board) == 22);
}

TEST_CASE("Rook and Bishop Pawn vs Bishop Fortress"){
	Board board = std::get<1>(parse_fen("8/8/6B1/8/2k5/2p5/7r/2K5 w KQkq - 0 1"));
	CHECK(eval(board) == -9891);
	CHECK(phase_of_game(board) == 3);
}
