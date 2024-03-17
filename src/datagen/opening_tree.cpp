# include <iostream>
# include <exception>
# include <algorithm>

# include "opening_tree.hpp"
# include "../parse_format.hpp"
# include "../hashing.hpp"
# include "../movegen.hpp"

uint64_t get_key(const Board &board, bool wtm){
    return wtm ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
}

OpeningTree init_opening_tree(){
    Board board;
    parse_fen(STARTING_FEN, board);
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, StemNode> stem_node_map;
    std::map<uint64_t, StemNode> leaf_node_map;

    Move first_move = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);  // Doesn't matter, as it may be overridden
    StemNode root = StemNode{nullptr, 0, first_move, 0};
    stem_node_map[get_key(board, true)] = root;

    Board board_copy = board.copy();
    make_move<true>(board_copy, first_move);
    leaf_node_map[get_key(board, false)] = root;
    return OpeningTree{interior_node_map, stem_node_map, leaf_node_map, board.copy()};
}

template <typename NodeT>
void show_line_from_node(NodeT node){
    if (node.parent) {
        show_line_from_node(*(node.parent));
        std::cout << format_move_xboard(node.last_move) << " ";
    }
}

void OpeningTree::show() const {
    for (auto it = stem_node_map.begin(); it != stem_node_map.end(); it++) {
        show_line_from_node(it->second);
        std::cout << std::endl;
    }
}

bool compare_child_specs(ChildSpec left, ChildSpec right){
    // Reverse order so that larger evals come first
    return left.evaluation > right.evaluation;
}

void OpeningTree::convert_stem_to_interior(const int search_depth, Board &board, const bool wtm){
    auto stem_key = get_key(board, wtm);
    StemNode stem_node = stem_node_map[stem_key];
    stem_node_map.erase(stem_key);

    Board copy_for_leaf_key = board.copy();
    (wtm ? make_move<true> : make_move<false>)(copy_for_leaf_key, stem_node.next_move);
    auto leaf_key = get_key(board, not wtm);
    leaf_node_map.erase(leaf_key);

    auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    MoveQueue move_queue = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);
    std::vector<ChildSpec> child_spec_list;

    while (!move_queue.empty()) {
        Move move = move_queue.top();
        Board copy_for_search = board.copy();
        
        // TODO Search stuff
        move_queue.pop();
    }

    // Stable sort ensures that move order breaks ties
    std::stable_sort(child_spec_list.begin(), child_spec_list.end(), compare_child_specs);

    interior_node_map[stem_key] = InteriorNode{1, child_spec_list, stem_node.parent, stem_node.last_move, stem_node.evaluation};
    
    (wtm ? make_move<true> : make_move<false>)(board, child_spec_list.front().child_move);
    deepen_recursive(search_depth, board, not wtm, child_spec_list.front(), &(interior_node_map[stem_key]));
}

void OpeningTree::deepen_recursive(const int search_depth, Board &board, const bool wtm, 
        ChildSpec child_spec, InteriorNode *parent){
    auto key = get_key(board, wtm);

    if (interior_node_map.count(key)){
        show_line_from_node(*parent);
        show_line_from_node(interior_node_map[key]);
        throw std::logic_error("Not yet implemented (deepen_recursive -> interior)");
    } else if (stem_node_map.count(key)){
        convert_stem_to_interior(search_depth, board, wtm);
        deepen_recursive(search_depth, board, wtm, child_spec, parent);
    } else if (leaf_node_map.count(key)){
        show_line_from_node(*parent);
        show_line_from_node(leaf_node_map[key]);
        throw std::logic_error("Not yet implemented (deepen_recursive -> leaf)");
    } else {
        show_line_from_node(leaf_node_map[key]);
        throw std::logic_error("Not yet implemented (deepen_recursive -> miss)");
    }
}

void OpeningTree::deepen(const int search_depth) {
    Board board_copy = starting_board.copy();
    deepen_recursive(search_depth, board_copy, true, ChildSpec{0, move_from_squares(E2, E4, DOUBLE_PAWN_PUSH), 0}, nullptr);
}
