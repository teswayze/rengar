# include <iostream>
# include <cstdlib>

# include "opening_tree.hpp"

void send_error_message(){
    std::cout << "Usage: ./datagen <num_games> <search_depth>" << std::endl;
}


int main(int argc, char **argv){
    if (argc != 3) {send_error_message(); return 1;}
    int num_games = std::atoi(argv[1]);
    int search_depth = std::atoi(argv[2]);

    OpeningTree tree = init_opening_tree();
    for (int i = 1; i < num_games; i++) tree.deepen(search_depth);
    tree.show();
}
