# include <iostream>
# include <cstdlib>
# include <filesystem>

# include "opening_tree.hpp"
# include "../timer.hpp"

void send_error_message(){
    std::cout << "Usage: ./bookgen <target_dir> <num_games> <search_depth> <num_batches>" << std::endl;
}


int main(int argc, char **argv){
    if (argc != 5) {send_error_message(); return 1;}
    int num_games = std::atoi(argv[2]);
    int search_depth = std::atoi(argv[3]);
    int num_batches = std::atoi(argv[4]);
    if (num_games % num_batches) {
        std::cout << "Error: num_games must be a multiple of num_batches" << std::endl;
        return 1;
    }

    Timer t;
    t.start();
    OpeningTree tree = init_opening_tree();
    for (int i = 1; i < num_games; i++) {
        tree.deepen(search_depth);
        if (i % 1000 == 0) std::cout << "Progress: " << i << " / " << num_games << std::endl;
    }
    t.stop();

    std::cout << "Successuflly built tree in " << t.ms_elapsed() / 1000 << " seconds" << std::endl;
    std::filesystem::create_directory(argv[1]);
    tree.write_to_dir(argv[1], num_games / num_batches);
    return 0;
}
