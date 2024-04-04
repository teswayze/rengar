# include <cassert>
# include "rg_file.hpp"

void RgFileWriter::add_game(GameData game){
    games.push_back(game);
}

void RgFileWriter::write_to_file(const std::string path){
    std::ofstream output_file;
    output_file.open(path, std::ios::binary | std::ios::out);
    assert(output_file);
    const size_t num_games = games.size();
    output_file.write((char*)&num_games, sizeof(num_games));
    assert(output_file);

    for (GameData game : games){
        output_file.write(&game.result, sizeof(game.result));
        assert(output_file);
        const size_t game_length = game.moves.size();
        output_file.write((char*)&game_length, sizeof(game_length));
        assert(output_file);
        output_file.write((char*)&game.moves[0], sizeof(Move) * game_length);
        assert(output_file);
    }
}

RgFileReader::RgFileReader(const std::string path){
    input_file.open(path, std::ios::binary | std::ios::in);
    input_file.read((char*)&games_left, sizeof(games_left));
}

GameData RgFileReader::next_game(){
    games_left--;

    char result;
    input_file.read(&result, sizeof(result));
    size_t game_length;
    input_file.read((char*)&game_length, sizeof(game_length));

    auto moves = std::vector<Move>(game_length, 0);
    input_file.read((char*)&moves[0], sizeof(Move) * game_length);

    return GameData{moves, result};
}
