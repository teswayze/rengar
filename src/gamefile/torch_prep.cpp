# include <fstream>
# include <exception>
# include "torch_prep.hpp"
# include "rg_file.hpp"
# include "../movegen.hpp"
# include "../parse_format.hpp"

struct PytorchCsvWriters {
    std::ofstream pst_idx;
    std::ofstream color_sign;
    std::ofstream sob_sign;
    std::ofstream wtm;
    std::ofstream game_result;

    PytorchCsvWriters(const std::string path){
        pst_idx.open(path + ".pst_idx.csv");
        assert(pst_idx);
        color_sign.open(path + ".color_sign.csv");
        assert(color_sign);
        sob_sign.open(path + ".sob_sign.csv");
        assert(sob_sign);
        wtm.open(path + ".wtm.csv");
        assert(wtm);
        game_result.open(path + ".game_result.csv");
        assert(game_result);

        for (auto i = 0; i < 32; i++) {
            pst_idx << ",pst_idx" << i;
            color_sign << ",color_sign" << i;
            sob_sign << ",sob_sign" << i;
        }
        pst_idx << "\n";
        color_sign << "\n";
        sob_sign << "\n";
        wtm << ",wtm\n";
        game_result << ",game_result\n";
    }

    template <bool white, Piece piece>
    void handle_piece(const Square square){
        const bool flip_h = square & 4;
        const size_t idx = calculate_pst_idx<white, piece>(square);
        pst_idx << "," << idx;
        color_sign << (white ? ",1.0" : ",-1.0");
        sob_sign << (flip_h ? ",1.0" : ",-1.0");
    }

    template <bool white>
    int handle_halfboard(const HalfBoard &hb){
        int pieces_seen = 0;
        Bitloop(hb.Pawn, x) { handle_piece<white, PAWN>(TZCNT(x)); pieces_seen++; }
        Bitloop(hb.Knight, x) { handle_piece<white, KNIGHT>(TZCNT(x)); pieces_seen++; }
        Bitloop(hb.Bishop, x) { handle_piece<white, BISHOP>(TZCNT(x)); pieces_seen++; }
        Bitloop(hb.Rook, x) { handle_piece<white, ROOK>(TZCNT(x)); pieces_seen++; }
        Bitloop(hb.Queen, x) { handle_piece<white, QUEEN>(TZCNT(x)); pieces_seen++; }
        handle_piece<white, KING>(hb.King); pieces_seen++;
        return pieces_seen;
    }

    void handle_position(const bool wtm_, const Board &board, const float game_result_, const int position_no){
        pst_idx << position_no;
        color_sign << position_no;
        sob_sign << position_no;

        int piece_count = handle_halfboard<true>(board.White) + handle_halfboard<false>(board.Black);
        while (piece_count < 32) {
            pst_idx << ",184";
            color_sign << ",0.0";
            sob_sign << ",0.0";
            piece_count++; 
        }

        pst_idx << "\n";
        color_sign << "\n";
        sob_sign << "\n";
        wtm << position_no << (wtm_ ? ",1.0\n" : ",-1.0\n");
        game_result << position_no << "," << game_result_ << "\n";
    }

    int handle_game(const GameData game, int position_no, const int chess324_id){
        Board board;
        parse_fen(get_chess324_starting_fen(chess324_id), board);
        bool wtm_ = true;
        
        float game_result_;
        switch (game.result){
            case 'W': game_result_ = 1.0; break;
            case 'D': game_result_ = 0.0; break;
            case 'B': game_result_ = -1.0; break;
            default: throw std::logic_error("Bad game result");
        }

        for (auto move : game.moves) {
            if (move_flags(move) < PAWN_CAPTURE and not (ToMask(move_destination(move)) & board.Occ)) {
                const auto cnp = (wtm_ ? checks_and_pins<true> : checks_and_pins<false>)(board);
                if (cnp.CheckMask == FULL_BOARD) {
                    handle_position(wtm_, board, game_result_, position_no);
                    position_no++;
                }
            }

            (wtm_ ? make_move<true> : make_move<false>)(board, move);
            wtm_ = not wtm_;
        }

        return position_no;
    }
};

void prep_games_for_pytorch(const std::string file_path, const int chess324_id){
    auto reader = RgFileReader(file_path);
    auto writers = PytorchCsvWriters(file_path);
    int position_no = 0;
    while (reader.games_left) {
        position_no = writers.handle_game(reader.next_game(), position_no, chess324_id);
    }
}
