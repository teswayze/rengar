# include <string>
# include <iostream>
# include <exception>

# include "board.hpp"

std::string format_square(Square square){
	return {"ABCDEFGH"[square % 8], "12345678"[square / 8]};
}

std::string file_letter_lower(Square square){
	return {"abcdefgh"[square % 8]};
}

std::string format_square_lower(Square square){
	return {"abcdefgh"[square % 8], "12345678"[square / 8]};
}

Square parse_square_lower(char file, char rank){
	return 8 * (rank - '1') + (file - 'a');
}

void dump_board(Board &board){
	for (int rank = 7; rank >= 0; rank--){
		for (int file = 0; file < 8; file++){
			auto mask = ToMask(rank * 8 + file);
			if (board.Occ & mask){
				bool white = board.White.All & mask;
				HalfBoard &half = white ? board.White : board.Black;
				if (half.Pawn & mask) {std::cout << (white ? "X" : "x");}
				if (half.Knight & mask) {std::cout << (white ? "N" : "n");}
				if (half.Bishop & mask) {std::cout << (white ? "B" : "b");}
				if (half.Rook & mask) {std::cout << (white ? "R" : "r");}
				if (half.Queen & mask) {std::cout << (white ? "Q" : "q");}
				if (half.King & mask) {std::cout << (white ? "K" : "k");}
			} else {
				std::cout << '.';
			}
			std::cout << " ";
		}
		std::cout << '\n';
	}
}

std::string format_move(Move move){
	std::string piece;
	bool is_promotion = false;
	std::string promotion_piece;

	switch (move_flags(move)){
	case NULL_MOVE:
		return "Pass";

	case KNIGHT_MOVE:
		piece = "Knight";
		break;
	case BISHOP_MOVE:
		piece = "Bishop";
		break;
	case ROOK_MOVE:
		piece = "Rook";
		break;
	case QUEEN_MOVE:
		piece = "Queen";
		break;
	case KING_MOVE:
		piece = "King";
		break;

	case CASTLE_QUEENSIDE:
		return "Castle Queenside";
	case CASTLE_KINGSIDE:
		return "Castle Kingside";

	case SINGLE_PAWN_PUSH:
	case DOUBLE_PAWN_PUSH:
	case PAWN_CAPTURE:
	case EN_PASSANT_CAPTURE:
		piece = "Pawn";
		break;

	case PROMOTE_TO_KNIGHT:
		piece = "Pawn";
		is_promotion = true;
		promotion_piece = "Knight";
		break;
	case PROMOTE_TO_BISHOP:
		piece = "Pawn";
		is_promotion = true;
		promotion_piece = "Bishop";
		break;
	case PROMOTE_TO_ROOK:
		piece = "Pawn";
		is_promotion = true;
		promotion_piece = "Rook";
		break;
	case PROMOTE_TO_QUEEN:
		piece = "Pawn";
		is_promotion = true;
		promotion_piece = "Queen";
		break;
	}

	std::string base = piece + " " + format_square(move_source(move)) + " " +
			format_square(move_destination(move));

	if (is_promotion){
		return base + " " + promotion_piece;
	} else {
		return base;
	}
}

std::string format_move_san(Move move){
	std::string promotion_piece;

	switch (move_flags(move)){
	case NULL_MOVE:
		return "Pass";

	case KNIGHT_MOVE:
		return "N" + format_square_lower(move_source(move)) + format_square_lower(move_destination(move));
	case BISHOP_MOVE:
		return "B" + format_square_lower(move_source(move)) + format_square_lower(move_destination(move));
	case ROOK_MOVE:
		return "R" + format_square_lower(move_source(move)) + format_square_lower(move_destination(move));
	case QUEEN_MOVE:
		return "Q" + format_square_lower(move_source(move)) + format_square_lower(move_destination(move));
	case KING_MOVE:
		return "K" + format_square_lower(move_source(move)) + format_square_lower(move_destination(move));

	case CASTLE_QUEENSIDE:
		return "O-O-O";
	case CASTLE_KINGSIDE:
		return "O-O";

	case SINGLE_PAWN_PUSH:
	case DOUBLE_PAWN_PUSH:
		return format_square_lower(move_destination(move));
	case PAWN_CAPTURE:
	case EN_PASSANT_CAPTURE:
		return file_letter_lower(move_source(move)) + "x" + format_square_lower(move_destination(move));

	case PROMOTE_TO_KNIGHT:
		promotion_piece = "N";
		break;
	case PROMOTE_TO_BISHOP:
		promotion_piece = "B";
		break;
	case PROMOTE_TO_ROOK:
		promotion_piece = "R";
		break;
	case PROMOTE_TO_QUEEN:
		promotion_piece = "Q";
		break;
	}

	std::string suffix = format_square_lower(move_destination(move)) + "=" + promotion_piece;
	std::string source_file = file_letter_lower(move_source(move));
	if (source_file == file_letter_lower(move_destination(move))) {
		return suffix;
	}
	return source_file + "x" + suffix;
}

std::string format_move_xboard(Move move){

	switch (move_flags(move)){
	case NULL_MOVE:
		return "0000";

	case KNIGHT_MOVE:
	case BISHOP_MOVE:
	case ROOK_MOVE:
	case QUEEN_MOVE:
	case KING_MOVE:
	case CASTLE_QUEENSIDE:
	case CASTLE_KINGSIDE:
	case SINGLE_PAWN_PUSH:
	case DOUBLE_PAWN_PUSH:
	case PAWN_CAPTURE:
	case EN_PASSANT_CAPTURE:
		return format_square_lower(move_source(move)) + format_square_lower(move_destination(move));
	case PROMOTE_TO_KNIGHT:
		return format_square_lower(move_source(move)) + format_square_lower(move_destination(move)) + "n";
	case PROMOTE_TO_BISHOP:
		return format_square_lower(move_source(move)) + format_square_lower(move_destination(move)) + "b";
	case PROMOTE_TO_ROOK:
		return format_square_lower(move_source(move)) + format_square_lower(move_destination(move)) + "r";
	case PROMOTE_TO_QUEEN:
		return format_square_lower(move_source(move)) + format_square_lower(move_destination(move)) + "q";
	}
	throw std::logic_error("Unexpected move flag");
}

Move parse_move_xboard(std::string move_str, Board &board, bool wtm){
	HalfBoard &friendly = wtm ? board.White : board.Black;
	HalfBoard &enemy = wtm ? board.Black : board.White;

	Square from_square = parse_square_lower(move_str[0], move_str[1]);
	Square to_square = parse_square_lower(move_str[2], move_str[3]);

	MoveFlags flag = NULL_MOVE;
	if (friendly.Pawn & ToMask(from_square)){
		if (wtm ? (to_square >= A8) : (to_square <= H1)){
			switch (move_str[4]){
			case 'n': flag = PROMOTE_TO_KNIGHT; break;
			case 'b': flag = PROMOTE_TO_BISHOP; break;
			case 'r': flag = PROMOTE_TO_ROOK; break;
			case 'q': flag = PROMOTE_TO_QUEEN; break;
			}
		} else {
			if ((to_square - from_square) == (wtm ? 8 : -8)){ flag = SINGLE_PAWN_PUSH; }
			if ((to_square - from_square) == (wtm ? 16 : -16)){ flag = DOUBLE_PAWN_PUSH; }
			if (ToMask(to_square) & enemy.All){ flag = PAWN_CAPTURE; }
			if (ToMask(to_square + (wtm ? -8 : 8)) & board.EPMask){ flag = EN_PASSANT_CAPTURE; }
		}
	}
	if (friendly.Knight & ToMask(from_square)){ flag = KNIGHT_MOVE; }
	if (friendly.Bishop & ToMask(from_square)){ flag = BISHOP_MOVE; }
	if (friendly.Rook & ToMask(from_square)){ flag = ROOK_MOVE; }
	if (friendly.Queen & ToMask(from_square)){ flag = QUEEN_MOVE; }
	if (friendly.King & ToMask(from_square)){
		flag = KING_MOVE;
		if ((from_square == (wtm ? E1 : E8)) and (to_square == (wtm ? C1 : C8))){ flag = CASTLE_QUEENSIDE;}
		if ((from_square == (wtm ? E1 : E8)) and (to_square == (wtm ? G1 : G8))){ flag = CASTLE_KINGSIDE;}
	}
	if (flag == 0){ throw std::invalid_argument("Illegal move " + move_str); }
	return move_from_squares(from_square, to_square, flag);
}

Move parse_move_san(std::string move_str, Board &board, bool wtm){
	HalfBoard &friendly = wtm ? board.White : board.Black;
	HalfBoard &enemy = wtm ? board.Black : board.White;

	switch (move_str[0]) {
	case 'N':
		return move_from_squares(parse_square_lower(move_str[1], move_str[2]), parse_square_lower(move_str[3], move_str[4]), KNIGHT_MOVE);
	case 'B':
		return move_from_squares(parse_square_lower(move_str[1], move_str[2]), parse_square_lower(move_str[3], move_str[4]), BISHOP_MOVE);
	case 'R':
		return move_from_squares(parse_square_lower(move_str[1], move_str[2]), parse_square_lower(move_str[3], move_str[4]), ROOK_MOVE);
	case 'Q':
		return move_from_squares(parse_square_lower(move_str[1], move_str[2]), parse_square_lower(move_str[3], move_str[4]), QUEEN_MOVE);
	case 'K':
		return move_from_squares(parse_square_lower(move_str[1], move_str[2]), parse_square_lower(move_str[3], move_str[4]), KING_MOVE);

	case 'O':
		if (move_str == "O-O") { return move_from_squares(wtm ? E1 : E8, wtm ? G1 : G8, CASTLE_KINGSIDE); }
		if (move_str == "O-O-O") { return move_from_squares(wtm ? E1 : E8, wtm ? C1 : C8, CASTLE_QUEENSIDE); }
		throw std::invalid_argument("Non castling move starts with 'O': " + move_str);
	}

	Square from;
	Square to;
	MoveFlags flags;
	size_t index_of_equals_sign;
	// Pawm move!
	if (move_str[1] == 'x') {
		to = parse_square_lower(move_str[2], move_str[3]);
		from = parse_square_lower(move_str[0], move_str[3] + (wtm ? -1 : 1));
		flags = (enemy.All & ToMask(to)) ? PAWN_CAPTURE : EN_PASSANT_CAPTURE;
		index_of_equals_sign = 4;
	} else {
		to = parse_square_lower(move_str[0], move_str[1]);
		from = to + (wtm ? -8 : 8);
		if (friendly.Pawn & ToMask(from)) { flags = SINGLE_PAWN_PUSH; }
		else { from = to + (wtm ? -16 : 16); flags = DOUBLE_PAWN_PUSH; }
		index_of_equals_sign = 2;
	}

	if (wtm ? (to >= A8) : (to <= H1)) {
		switch (move_str[index_of_equals_sign + 1]) {
		case 'N': flags = PROMOTE_TO_KNIGHT; break;
		case 'B': flags = PROMOTE_TO_BISHOP; break;
		case 'R': flags = PROMOTE_TO_ROOK; break;
		case 'Q': flags = PROMOTE_TO_QUEEN; break;
		}
	}

	return move_from_squares(from, to, flags);
}

bool parse_fen(std::string fen, Board &out_board){
	size_t index = 0;

	// Main board
	BitMask wp=0, wn=0, wb=0, wr=0, wq=0, wk=0, bp=0, bn=0, bb=0, br=0, bq=0, bk=0;
	int rank_no = 8;
	int file_no = 0;
	while (rank_no > 0){
		Square square = (rank_no - 1) * 8 + file_no;
		switch (fen[index]){
		case 'P': wp |= ToMask(square); file_no++; break;
		case 'N': wn |= ToMask(square); file_no++; break;
		case 'B': wb |= ToMask(square); file_no++; break;
		case 'R': wr |= ToMask(square); file_no++; break;
		case 'Q': wq |= ToMask(square); file_no++; break;
		case 'K': wk |= ToMask(square); file_no++; break;

		case 'p': bp |= ToMask(square); file_no++; break;
		case 'n': bn |= ToMask(square); file_no++; break;
		case 'b': bb |= ToMask(square); file_no++; break;
		case 'r': br |= ToMask(square); file_no++; break;
		case 'q': bq |= ToMask(square); file_no++; break;
		case 'k': bk |= ToMask(square); file_no++; break;

		case '1': file_no += 1; break;
		case '2': file_no += 2; break;
		case '3': file_no += 3; break;
		case '4': file_no += 4; break;
		case '5': file_no += 5; break;
		case '6': file_no += 6; break;
		case '7': file_no += 7; break;
		case '8': file_no += 8; break;

		case ' ':
		case '/':
			if ((rank_no == 1) == (fen[index] == '/')) {
				throw std::invalid_argument("Wrong number of rows in FEN: " + fen);
			}
			if (file_no != 8) {
				throw std::invalid_argument("Wrong number of columns in one row of FEN: " + fen);
			}
			rank_no--; file_no = 0; break;

		default: throw std::invalid_argument("Unexpected character in FEN board: " + fen);
		}
		index++;
	}
	if (__builtin_popcountll(wk) != 1){
		throw std::invalid_argument("Wrong number of white kings in FEN: " + fen);
	}
	if (__builtin_popcountll(bk) != 1){
		throw std::invalid_argument("Wrong number of white kings in FEN: " + fen);
	}

	bool wtm;

	// Side to move
	switch (fen[index]){
	case 'w': wtm = true; break;
	case 'b': wtm = false; break;
	default: throw std::invalid_argument("Unexpected character in FEN side to move: " + fen);
	}
	index += 2;

	// Castling rights
	BitMask w_cas=0, b_cas=0;
	while (fen[index] != ' '){
		switch (fen[index]){
		case '-': break;
		case 'Q': w_cas |= ToMask(A1); break;
		case 'K': w_cas |= ToMask(H1); break;
		case 'q': b_cas |= ToMask(A8); break;
		case 'k': b_cas |= ToMask(H8); break;
		default: throw std::invalid_argument("Unexpected character in FEN castling rights: " + fen);
		}
		index++;
	}
	index++;

	const HalfBoard white = from_masks(wp, wn, wb, wr, wq, wk, w_cas);
	const HalfBoard black = from_masks(bp, bn, bb, br, bq, bk, b_cas);

	// En Passant target
	switch (fen[index]){
	case '-': out_board = from_sides_without_eval(white, black); break;
	case 'a': out_board = from_sides_without_eval_ep(white, black, wtm ? A5 : A4); break;
	case 'b': out_board = from_sides_without_eval_ep(white, black, wtm ? B5 : B4); break;
	case 'c': out_board = from_sides_without_eval_ep(white, black, wtm ? C5 : C4); break;
	case 'd': out_board = from_sides_without_eval_ep(white, black, wtm ? D5 : D4); break;
	case 'e': out_board = from_sides_without_eval_ep(white, black, wtm ? E5 : E4); break;
	case 'f': out_board = from_sides_without_eval_ep(white, black, wtm ? F5 : F4); break;
	case 'g': out_board = from_sides_without_eval_ep(white, black, wtm ? G5 : G4); break;
	case 'h': out_board = from_sides_without_eval_ep(white, black, wtm ? H5 : H4); break;
	default: throw std::invalid_argument("Unexpected character in FEN en passant target: " + fen);
	}

	return wtm;
}

