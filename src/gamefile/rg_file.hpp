# include <string>
# include <fstream>
# include <vector>
# include "../board.hpp"

struct GameData {
    std::vector<Move> moves;
    char result;
};

struct RgFileWriter{
    std::vector<GameData> games;

    void add_game(GameData game);
    void write_to_file(const std::string path);
};

struct RgFileReader{
    std::ifstream input_file;
    size_t games_left;

    RgFileReader(const std::string path);
    GameData next_game();
};
