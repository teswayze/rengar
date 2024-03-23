# include <vector>
# include <tuple>
# include <map>
# include <optional>

# include "../board.hpp"

struct SearchResult{
    Move best_move;
    int evaluation;
};

struct ChildInfo{
    Move child_move;
    int evaluation;
    int visit_count;
};

// Non-terminal positions covered by the opening book, typically explored at least twice
// All children have been evaluated
// The parent is not necessarily unique
struct InteriorNode{
    std::vector<ChildInfo> children;
    std::vector<uint64_t> parent_hashes;
    Move last_move;

    // A representative for finding a path to the root
    uint64_t get_parent() const { return parent_hashes[0]; }
    int get_evaluation() const { return -children[0].evaluation; }
};

// Book exit positions explored exactly once
// They have been evaluated but their children have not
// There is a unique parent and child of this node
struct StemNode{
    uint64_t parent_hash;
    Move last_move;
    Move next_move;
    int evaluation;

    uint64_t get_parent() const { return parent_hash; }
    int get_evaluation() const { return evaluation; }
};

// Leaf nodes are positions not in the book that technically are not explored, but will be reached by the first out of book move
// They have not been evaluated, but their (unique) parents have
// I don't actually create the LeafNode object, since it would just be a pointer to the parent


struct OpeningTree{
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, StemNode> stem_node_map;
    std::map<uint64_t, StemNode> leaf_node_map; // The hash key is the StemNode's likely leaf child
    std::map<uint64_t, SearchResult> search_cache; // Evaluation of unexplored potential book deviations
    const Board starting_board;

    void deepen(const int search_depth);
    void show() const;

    private:
        void convert_stem_to_interior(const int search_depth, const Board &board, const bool wtm);
        void deepen_recursive(const int search_depth, Board &board, const bool wtm, Move last_move, uint64_t parent_hash);
        template <typename NodeT>
        bool show_line_from_node(const NodeT node) const;
        void extend_leaf_parent(const int search_depth, const uint64_t leaf_hash);
        template <typename NodeT>
        bool reproduce_board_at(const NodeT node, Board &board);
        int evaluate_move(const int search_depth, const Board &board, const bool wtm, const Move move);
};

OpeningTree init_opening_tree();
