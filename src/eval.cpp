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
