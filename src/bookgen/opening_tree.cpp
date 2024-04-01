# include <iostream>
# include <exception>
# include <algorithm>

# include "opening_tree.hpp"
# include "../parse_format.hpp"
# include "../hashing.hpp"
# include "../movegen.hpp"
# include "../search.hpp"
# include "../hashtable.hpp"
# include "../gamefile/rg_file.hpp"

int evaluated_positions = 0;

uint64_t get_key(const Board &board, bool wtm){
    return wtm ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
}

OpeningTree init_opening_tree(){
    ht_init(16);

    Board board;
    parse_fen(STARTING_FEN, board);
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, LeafNode> leaf_node_map;
    std::map<uint64_t, LeafNode> first_oob_map;
    std::map<uint64_t, SearchResult> search_cache;

    Move first_move = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);  // Doesn't matter, as it may be overridden
    LeafNode root = LeafNode{{}, SearchResult{first_move, 0}, true};
    leaf_node_map.insert(std::make_tuple(get_key(board, true), root));

    Board board_copy = board.copy();
    make_move<true>(board_copy, first_move);
    first_oob_map.insert(std::make_tuple(get_key(board_copy, false), root));
    return OpeningTree{interior_node_map, leaf_node_map, first_oob_map, board.copy()};
}

template <typename NodeT>
bool OpeningTree::show_line_from_node(const NodeT node) const {
    if (node.parents.size() == 0) return true; // Starting position

    auto parent = node.parents[0];
    bool wtm = show_line_from_node(interior_node_map.at(parent.hash));
    std::cout << format_move_xboard(parent.last_move) << " {" << node.get_evaluation() * (wtm ? 1 : -1) << "} ";
    return not wtm;
}

void OpeningTree::show() const {
    std::cout << evaluated_positions << " positions evaluated" << std::endl;
    for (auto it = leaf_node_map.begin(); it != leaf_node_map.end(); it++) {
        auto node = it->second;
        if (node.book_exit) {
            show_line_from_node(node);
            std::cout << "{" << format_move_xboard(node.search_result.best_move) << "}" << std::endl;
        }
    }
}

template <typename NodeT>
void OpeningTree::build_move_vector(const NodeT node, std::vector<Move> moves) const {
    if (node.parents.size() == 0) return; // Starting position

    auto parent = node.parents[0];
    moves.push_back(parent.last_move);
}

void OpeningTree::write_to_file(std::string path) const {
    RgFileWriter writer;
    for (auto it = leaf_node_map.begin(); it != leaf_node_map.end(); it++) {
        auto node = it->second;
        if (node.book_exit) {
            std::vector<Move> moves;
            build_move_vector(node, moves);
            writer.add_game({moves, 'U'});
        }
    }
    writer.write_to_file(path);
}

bool hl8_helper(int eval_diff, int common_count, int rare_count){
    // Check if 2 ** (eval_diff / 8) * rare_count < common_count
    bool first_loop = true;
    while (eval_diff != 0) {
        if (not first_loop) {
            // Square both sides
            eval_diff *= 2;
            auto to_shift = std::max(0, 17 - __builtin_clz(std::max(common_count, rare_count)));
            common_count = (common_count >> to_shift) * (common_count >> to_shift);
            rare_count = (rare_count >> to_shift) * (rare_count >> to_shift);
        }

        // Simplification of left hand side
        if (eval_diff >= 20 * 8) return false;
        rare_count = rare_count << (eval_diff / 8);
        eval_diff = eval_diff % 8;

        first_loop = false;
    }
    return rare_count < common_count;
}

size_t choose_move_by_explore_exploit(const std::vector<ChildInfo> child_info){
    size_t best_ix = 0;
    for (size_t i = 1; i < child_info.size(); i++){
        auto eval_diff = child_info[best_ix].evaluation - child_info[i].evaluation;
        if (hl8_helper(eval_diff, child_info[best_ix].visit_count + 1, child_info[i].visit_count + 1)) {
            best_ix = i;
        }
    }
    return best_ix;
}

bool compare_child_info(ChildInfo left, ChildInfo right){
    // Reverse order so that larger evals come first
    return left.evaluation > right.evaluation;
}

int OpeningTree::evaluate_move(const int search_depth, const Board &board, const bool wtm, const ParentInfo parent){
    Board board_copy = board.copy();
    History history;
    (wtm ? make_move<true> : make_move<false>)(board_copy, parent.last_move);

    auto key = get_key(board_copy, not wtm);

    if (interior_node_map.count(key)) {
        auto &node = interior_node_map.at(key);
        node.parents.push_back(parent);
        return node.get_evaluation();
    }
    if (leaf_node_map.count(key)) {
        auto &node = leaf_node_map.at(key);
        node.parents.push_back(parent);
        return node.search_result.evaluation;
    }

	initialize_move_order_arrays();
    evaluated_positions++;
    auto search_res_tuple = (wtm ? search_for_move_w_eval<false> : search_for_move_w_eval<true>)
        (board_copy, history, INT_MAX, search_depth, INT_MAX, INT_MAX);
    auto new_leaf = LeafNode{{parent}, SearchResult{std::get<0>(search_res_tuple), -std::get<1>(search_res_tuple)}, false};
    leaf_node_map.insert(std::make_tuple(key, new_leaf));
    return new_leaf.search_result.evaluation;
}

size_t index_of_move(const std::vector<ChildInfo> children, const Move move){
    for (size_t move_idx = 0; move_idx < children.size(); move_idx++){
        if (children[move_idx].child_move == move) return move_idx;
    }
    throw std::logic_error("Out of legal moves!");
}

template <bool upwards>
void OpeningTree::update_evaluation(const ParentInfo parent, const int evaluation){
    auto &node = interior_node_map.at(parent.hash);

    size_t move_idx = index_of_move(node.children, parent.last_move);
    node.children[move_idx].evaluation = evaluation;

    bool propagate_adjustment;
    if (upwards) {
        while ((move_idx > 0) and (node.children[move_idx - 1].evaluation < evaluation)){
            std::swap(node.children[move_idx - 1], node.children[move_idx]);
            move_idx--;
        }
        propagate_adjustment = (move_idx == 0);

    } else {
        propagate_adjustment = (move_idx == 0);
        while ((move_idx < node.children.size() - 1) and (node.children[move_idx + 1].evaluation > evaluation)){
            std::swap(node.children[move_idx + 1], node.children[move_idx]);
            move_idx++;
        }
    }
    if (propagate_adjustment) {
        for (auto grandparent : node.parents) {
            update_evaluation<not upwards>(grandparent, node.get_evaluation());
        }
    }
}


// Maintains the number of book lines
void OpeningTree::convert_leaf_to_interior(const int search_depth, const Board &board, const bool wtm){
    auto leaf_key = get_key(board, wtm);
    LeafNode leaf_node = leaf_node_map.at(leaf_key);
    assert(leaf_node.book_exit);
    leaf_node_map.erase(leaf_key);

    Board board_copy = board.copy();
    (wtm ? make_move<true> : make_move<false>)(board_copy, leaf_node.search_result.best_move);
    auto first_oob_key = get_key(board_copy, not wtm);
    assert(first_oob_map.count(first_oob_key));
    first_oob_map.erase(first_oob_key);

    auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    MoveQueue move_queue = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);
    std::vector<ChildInfo> child_info_list;

    while (!move_queue.empty()) {
        Move move = move_queue.top();
        child_info_list.push_back(ChildInfo{move, evaluate_move(search_depth, board, wtm, ParentInfo{leaf_key, move}), 0});
        move_queue.pop();
    }

    // Stable sort ensures that move order breaks ties
    std::stable_sort(child_info_list.begin(), child_info_list.end(), compare_child_info);

    auto interior_node = InteriorNode{child_info_list, leaf_node.parents};
    interior_node_map.insert(std::make_tuple(leaf_key, interior_node));

    if (interior_node.get_evaluation() > leaf_node.get_evaluation()){
        for (auto parent : leaf_node.parents) {
            update_evaluation<true>(parent, interior_node.get_evaluation());
        }
    } else if (interior_node.get_evaluation() < leaf_node.get_evaluation()){
        for (auto parent : leaf_node.parents) {
            update_evaluation<false>(parent, interior_node.get_evaluation());
        }
    }

    // As is we've reduced the number of book lines by one, so we must add one back
    Board board_copy2 = board.copy();
    while (not deepen_recursive(search_depth, board_copy2, wtm)){
        board_copy2 = board.copy();
    }
}

template <typename NodeT>
bool OpeningTree::reproduce_board_at(const NodeT node, Board &board){
    if (node.parents.size() == 0) return true; // Starting position
    auto parent = node.parents[0];
    bool wtm = reproduce_board_at(interior_node_map.at(parent.hash), board);
    (wtm ? make_move<true> : make_move<false>)(board, parent.last_move);
    return not wtm;
}

void OpeningTree::extend_first_oob_parent(const int search_depth, const uint64_t first_oob_hash){
    LeafNode leaf_node = first_oob_map.at(first_oob_hash);
    // We don't know how we got here, so we must traverse back up the tree to recreate the position
    Board board = starting_board.copy();
    bool wtm = reproduce_board_at(leaf_node, board);
    convert_leaf_to_interior(search_depth, board, wtm);
}

bool OpeningTree::deepen_recursive(const int search_depth, Board &board, const bool wtm){
    auto key = get_key(board, wtm);

    if (interior_node_map.count(key)){
        // Still in book, and not at a book exit
        auto &node = interior_node_map.at(key);

        // Select one of the available moves to deepen
        auto best_ix = choose_move_by_explore_exploit(node.children);
        const Move move = node.children[best_ix].child_move;
        (wtm ? make_move<true> : make_move<false>)(board, move);
        bool created_new_line = deepen_recursive(search_depth, board, not wtm);
        if (created_new_line) {
            size_t move_idx = index_of_move(node.children, move);
            node.children[move_idx].visit_count += 1;
        }
        return created_new_line;
    }

    auto &node = leaf_node_map.at(key);

    if (node.book_exit){
        // We've hit a book exit position for a second time, which we now must extend further to differentiate the two lines
        // The call to convert_leaf_to_interior extends the existing line one ply
        // By returning false we indicate that we must deepen again from the root
        // Most likely we'll end up extending this line again, but a new ply may change the evaluations
        // We don't want to double down on a refuted position
        convert_leaf_to_interior(search_depth, board, wtm);
        return false;
    }
    
    // A leaf node that is not yet in the book
    // We can add a book exit here provided there's no transpositions related to the first out-of-book move

    if (first_oob_map.count(key)) {
        // There is another line in the book that will transpose to this newly created one in one ply
        // For example: existing book line of 1. Nf3 d5 expecting 2. d4 collides with new line of 1. d4 d5 2. Nf3
        // The solution is to extend the existing line by one ply
        // Most likely we'll end up extending this line again to differentiate the two
        extend_first_oob_parent(search_depth, key);
        return false;
    }

    Board board_copy = board.copy();
    (wtm ? make_move<true> : make_move<false>)(board_copy, node.search_result.best_move);
    auto first_oob_key = get_key(board_copy, not wtm);

    if (first_oob_map.count(first_oob_key)){
        // If we add a book exit here, this book exit and another will converge after one move
        // For example: existing book line of 1. d4 d5 expecting 2. Nf3 collides with new line of 1. Nf3 d5 expecting 2. d4
        // The solution is to extend the existing line by one ply
        // Most likely we'll end up extending this line and the common child as well
        extend_first_oob_parent(search_depth, first_oob_key);
        return false;
    }

    node.book_exit = true;
    first_oob_map.insert(std::make_tuple(first_oob_key, node));

    if (interior_node_map.count(first_oob_key) or 
        (leaf_node_map.count(first_oob_key) and leaf_node_map.at(first_oob_key).book_exit)){
        // The new line will transpose back to the book in one ply
        // For example: existing book line of 1. Nf3 Nf6 2. e3 collides with new line of 1. e3 Nf6 expecting 2. Nf3
        // The solution is to extend the new line by one ply
        // Most likely we'll end up extending this line again to differentiate the two
        convert_leaf_to_interior(search_depth, board, wtm);
    }

    return true;
}

void OpeningTree::deepen(const int search_depth) {
    Board board_copy = starting_board.copy();
    while (not deepen_recursive(search_depth, board_copy, true)) {
        board_copy = starting_board.copy();
    }
}
