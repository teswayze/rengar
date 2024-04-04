# include "doctest.h"
# include "../bookgen/opening_tree.hpp"
# include "../parse_format.hpp"
# include <iostream>

ChildInfo find_matching_child(const std::vector<ChildInfo> children, const Move move){
    for (auto child : children){
        if (child.child_move == move) return child;
    }
    throw std::logic_error("Out of legal moves!");
}

TEST_CASE("Opening tree validation"){
    const int n = 20;

    OpeningTree tree = init_opening_tree(STARTING_FEN);
    for (int i = 1; i < n; i++) tree.deepen(6);

    CHECK(tree.first_oob_map.size() == n);
    for (auto it = tree.first_oob_map.begin(); it != tree.first_oob_map.end(); it++) {
        CHECK(it->second.book_exit);
        auto key = it->first;
        if (tree.leaf_node_map.count(key)) CHECK(not tree.leaf_node_map.at(key).book_exit);
        CHECK(tree.interior_node_map.count(it->first) == 0);
    }

    int parent_count = 0;
    for (auto it = tree.leaf_node_map.begin(); it != tree.leaf_node_map.end(); it++) {
        CHECK(tree.interior_node_map.count(it->first) == 0);
        auto node = it->second;
        int visit_count = 0;
        for (auto parent : node.parents){
            parent_count++;
            auto child = find_matching_child(tree.interior_node_map.at(parent.hash).children, parent.last_move);
            CHECK(child.evaluation == node.get_evaluation());
            visit_count += child.visit_count;
        }
        CHECK(visit_count == (node.book_exit ? 1 : 0));
    }

    int child_count = 0;
    for (auto it = tree.interior_node_map.begin(); it != tree.interior_node_map.end(); it++) {
        auto node = it->second;
        int visit_count = 0;
        for (auto parent : node.parents){
            parent_count++;
            auto child = find_matching_child(tree.interior_node_map.at(parent.hash).children, parent.last_move);
            CHECK(child.evaluation == node.get_evaluation());
            visit_count += child.visit_count;
        }
        
        int child_visit_sum = 0;
        for (auto child : node.children) {
            child_count++;
            child_visit_sum += child.visit_count;
        }
        
        if (node.parents.size() == 0) CHECK(child_visit_sum == n); // Root node
        else CHECK(child_visit_sum == visit_count);
    }

    CHECK(parent_count == child_count);
}