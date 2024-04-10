# include <iostream>
# include <cstdlib>

# include "opening_tree.hpp"
# include "../timer.hpp"
# include "../parse_format.hpp"

void send_error_message(){
    std::cout << "Usage: ./bookgen <target_file> <num_games> <search_depth> <chess324_id>" << std::endl;
}


int main(int argc, char **argv){
    if (argc != 5) {send_error_message(); return 1;}
    int num_games = std::atoi(argv[2]);
    int search_depth = std::atoi(argv[3]);
    int chess324_id = std::atoi(argv[4]);

    Timer t;
    t.start();
    auto fen = get_chess324_starting_fen(chess324_id);
    std::cout << "Building opening tree for " << fen << std::endl;
    OpeningTree tree = init_opening_tree(fen);
    for (int i = 1; i < num_games; i++) {
        tree.deepen(search_depth);
    }
    t.stop();

    std::cout << "Successuflly built tree in " << t.ms_elapsed() / 1000 << " seconds" << std::endl;
    tree.write_to_file(argv[1]);
    return 0;
}
