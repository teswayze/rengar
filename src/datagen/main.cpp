# include <iostream>
# include <cstdlib>

# include "opening_tree.hpp"
# include "../timer.hpp"

void send_error_message(){
    std::cout << "Usage: ./datagen <num_games> <search_depth>" << std::endl;
}


int main(int argc, char **argv){
    if (argc != 3) {send_error_message(); return 1;}
    int num_games = std::atoi(argv[1]);
    int search_depth = std::atoi(argv[2]);

    Timer t;
    t.start();
    OpeningTree tree = init_opening_tree();
    for (int i = 1; i < num_games; i++) tree.deepen(search_depth);
    t.stop();

    std::cout << "Successuflly built tree in " << t.ms_elapsed() / 1000 << " seconds" << std::endl;
    tree.show();
}
