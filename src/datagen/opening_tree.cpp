# include <iostream>
# include <exception>
# include <algorithm>

# include "opening_tree.hpp"
# include "../parse_format.hpp"
# include "../hashing.hpp"
# include "../movegen.hpp"
# include "../search.hpp"
# include "../hashtable.hpp"

int evaluated_positions = 0;

uint64_t get_key(const Board &board, bool wtm){
    return wtm ? (wtm_hash ^ board.EvalInfo.hash) : board.EvalInfo.hash;
}

OpeningTree init_opening_tree(){
    ht_init(16);

    Board board;
    parse_fen(STARTING_FEN, board);
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, StemNode> stem_node_map;
    std::map<uint64_t, StemNode> leaf_node_map;

    Move first_move = move_from_squares(E2, E4, DOUBLE_PAWN_PUSH);  // Doesn't matter, as it may be overridden
    StemNode root = StemNode{0ull, 0, first_move, 0};
    stem_node_map.insert(std::make_tuple(get_key(board, true), root));

    Board board_copy = board.copy();
    make_move<true>(board_copy, first_move);
    leaf_node_map.insert(std::make_tuple(get_key(board_copy, false), root));
    return OpeningTree{interior_node_map, stem_node_map, leaf_node_map, board.copy()};
}

template <typename NodeT>
bool OpeningTree::show_line_from_node(const NodeT node) const {
    if (node.parent_hash) {
        bool wtm = show_line_from_node(interior_node_map.at(node.parent_hash));
        std::cout << format_move_xboard(node.last_move) << " (" << node.evaluation * (wtm ? 1 : -1) << ") ";
        return not wtm;
    }
    return true;
}

void OpeningTree::show() const {
    std::cout << evaluated_positions << " positions evaluated" << std::endl;
    for (auto it = stem_node_map.begin(); it != stem_node_map.end(); it++) {
        show_line_from_node(it->second);
        std::cout << std::endl;
    }
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

size_t choose_move_by_explore_exploit(const std::vector<SpecAndCount> child_specs){
    size_t best_ix = 0;
    for (size_t i = 1; i < child_specs.size(); i++){
        auto eval_diff = child_specs[best_ix].spec.evaluation - child_specs[i].spec.evaluation;
        if (hl8_helper(eval_diff, child_specs[best_ix].visit_count + 1, child_specs[i].visit_count + 1)) {
            best_ix = i;
        }
    }
    return best_ix;
}

bool compare_spec_and_count(SpecAndCount left, SpecAndCount right){
    // Reverse order so that larger evals come first
    return left.spec.evaluation > right.spec.evaluation;
}

ChildSpec OpeningTree::evaluate_move(const int search_depth, const Board &board, const bool wtm, const Move move){
    Board board_copy = board.copy();
    History history;
    (wtm ? make_move<true> : make_move<false>)(board_copy, move);

    auto key = get_key(board_copy, not wtm);

    if (interior_node_map.count(key)) {
        auto node = interior_node_map.at(key);
        return ChildSpec{move, node.children[0].spec.child_move, node.evaluation};
    }

    if (stem_node_map.count(key)) {
        auto node = stem_node_map.at(key);
        return ChildSpec{move, node.next_move, node.evaluation};
    }

    evaluated_positions++;

    auto search_res = (wtm ? search_for_move_w_eval<false> : search_for_move_w_eval<true>)
        (board_copy, history, INT_MAX, search_depth, INT_MAX, INT_MAX);
    auto cs = ChildSpec{move, std::get<0>(search_res), -std::get<1>(search_res)};
    return cs;
}

void OpeningTree::convert_stem_to_interior(const int search_depth, const Board &board, const bool wtm){
    auto stem_key = get_key(board, wtm);
    StemNode stem_node = stem_node_map.at(stem_key);
    stem_node_map.erase(stem_key);

    Board board_copy = board.copy();
    (wtm ? make_move<true> : make_move<false>)(board_copy, stem_node.next_move);
    auto leaf_key = get_key(board_copy, not wtm);
    leaf_node_map.at(leaf_key); // Basically an assertion that the leaf key is in there
    leaf_node_map.erase(leaf_key);

    auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    MoveQueue move_queue = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);
    std::vector<SpecAndCount> child_spec_list;

    while (!move_queue.empty()) {
        Move move = move_queue.top();
        child_spec_list.push_back(SpecAndCount{evaluate_move(search_depth, board, wtm, move), 0});
        move_queue.pop();
    }

    // Stable sort ensures that move order breaks ties
    std::stable_sort(child_spec_list.begin(), child_spec_list.end(), compare_spec_and_count);

    child_spec_list[0].visit_count += 1;
    interior_node_map.insert(std::make_tuple(stem_key, 
        InteriorNode{child_spec_list, stem_node.parent_hash, stem_node.last_move, stem_node.evaluation}));
    
    Board board_copy2 = board.copy();
    (wtm ? make_move<true> : make_move<false>)(board_copy2, child_spec_list[0].spec.child_move);
    deepen_recursive(search_depth, board_copy2, not wtm, child_spec_list[0].spec, stem_key);
}

template <typename NodeT>
bool OpeningTree::reproduce_board_at(const NodeT node, Board &board){
    if (node.parent_hash == 0ull) return true;
    bool wtm = reproduce_board_at(interior_node_map.at(node.parent_hash), board);
    (wtm ? make_move<true> : make_move<false>)(board, node.last_move);
    return not wtm;
}

void OpeningTree::extend_leaf_parent(const int search_depth, const uint64_t leaf_hash){
    StemNode stem_node = leaf_node_map.at(leaf_hash);
    // We don't know how we got here, so we must traverse back up the tree to recreate the position
    Board board = starting_board.copy();
    bool wtm = reproduce_board_at(stem_node, board);
    convert_stem_to_interior(search_depth, board, wtm);
}

void OpeningTree::deepen_recursive(const int search_depth, Board &board, const bool wtm, 
        ChildSpec child_spec, uint64_t parent_hash){
    auto key = get_key(board, wtm);

    if (stem_node_map.count(key)){
        // We've hit a book exit position for a second time, which we now must extend further to differentiate the two lines
        // The call to convert_stem_to_interior extends the existing line one ply
        // The fall through to the next case adds another
        Board board_copy = board.copy();
        convert_stem_to_interior(search_depth, board_copy, wtm);
    }

    if (interior_node_map.count(key)){
        // Still in book, so we select from the available moves
        auto &node = interior_node_map.at(key);
        auto best_ix = choose_move_by_explore_exploit(node.children);
        node.children[best_ix].visit_count += 1;
        auto next_spec = node.children[best_ix].spec;
        (wtm ? make_move<true> : make_move<false>)(board, next_spec.child_move);
        deepen_recursive(search_depth, board, not wtm, next_spec, key);
    } else {

        // The position is not in our book
        // The book can exit here provided there's no transpositions, so we add a StemNode
        Board board_copy = board.copy();
        (wtm ? make_move<true> : make_move<false>)(board_copy, child_spec.best_reply);
        auto leaf_key = get_key(board_copy, not wtm);

        if (leaf_node_map.count(leaf_key)){
            // We can't add a stem and leaf because the leaf transposes to another
            // This corresponds to two book exit positions that would reach the same position after one move
            // For example: existing book line of 1. d4 d5 expecting 2. Nf3 collides with new line of 1. Nf3 d5 expecting 2. d4
            // The solution is to extend the existing line by one ply so that we end up in the miss -> stem case
            // Most likely we'll end up extending this line and the common child as well
            // However, this isn't certain to happen as the evaluation may change when deepening
            extend_leaf_parent(search_depth, leaf_key);
        }

        // No collision issue - we just add the new node
        auto new_node = StemNode{parent_hash, child_spec.child_move, child_spec.best_reply, child_spec.evaluation};
        stem_node_map.insert(std::make_tuple(key, new_node));
        leaf_node_map.insert(std::make_tuple(leaf_key, new_node));

        if (stem_node_map.count(leaf_key) or interior_node_map.count(leaf_key)){
            // The new line will transpose back to the book in one ply
            // For example: existing book line of 1. Nf3 Nf6 2. e3 collides with new line of 1. e3 Nf6 expecting 2. Nf3
            // The solution is to extend the new line by one ply
            // Most likely we'll end up extending this line again to differentiate the two
            // However, this isn't certain to happen as the evaluation may change when deepening
            convert_stem_to_interior(search_depth, board, wtm);            
        }

        if (leaf_node_map.count(key)) {
            // There is another line in the book that will transpose to this newly created one in one ply
            // For example: existing book line of 1. Nf3 d5 expecting 2. d4 collides with new line of 1. d4 d5 2. Nf3
            // The solution is to extend the existing line by one ply
            // Most likely we'll end up extending this line again to differentiate the two
            extend_leaf_parent(search_depth, key);
        }
    }
}

void OpeningTree::deepen(const int search_depth) {
    Board board_copy = starting_board.copy();
    deepen_recursive(search_depth, board_copy, true, ChildSpec{0, move_from_squares(E2, E4, DOUBLE_PAWN_PUSH), 0}, 0ull);
}
