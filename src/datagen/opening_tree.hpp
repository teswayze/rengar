# include <vector>
# include <tuple>
# include <map>
# include <optional>

# include "../board.hpp"

struct ChildSpec{
    Move child_move;
    Move best_reply;
    int evaluation;
};

struct SpecAndCount{
    ChildSpec spec;
    int visit_count;
};

// Non-terminal positions covered by the opening book, typically explored at least twice
// All children have been evaluated
// The parent is not necessarily unique; just a representative is given here
struct InteriorNode{
    std::vector<SpecAndCount> children;
    InteriorNode *parent;
    Move last_move;
    int evaluation;
};

// Book exit positions explored exactly once
// They have been evaluated but their children have not
// There is a unique parent and child of this node
struct StemNode{
    InteriorNode *parent;
    Move last_move;
    Move next_move;
    int evaluation;
};

// Leaf nodes are positions not in the book that technically are not explored, but will be reached by the first out of book move
// They have not been evaluated, but their (unique) parents have
// I don't actually create the LeafNode object, since it would just be a pointer to the parent


struct OpeningTree{
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, StemNode> stem_node_map;
    std::map<uint64_t, StemNode> leaf_node_map; // The hash key is the StemNode's likely leaf child
    const Board starting_board;

    void deepen(const int search_depth);
    void show() const;

    private:
        void convert_stem_to_interior(const int search_depth, const Board &board, const bool wtm);
        void deepen_recursive(const int search_depth, Board &board, const bool wtm, ChildSpec child_spec, InteriorNode *parent);
};

OpeningTree init_opening_tree();
