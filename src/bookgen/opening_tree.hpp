# include <vector>
# include <tuple>
# include <map>
# include <optional>
# include <string>

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

struct ParentInfo{
    uint64_t hash;
    Move last_move;
};

// Non-terminal positions covered by the opening book, typically explored at least twice
// All children have been evaluated
struct InteriorNode{
    std::vector<ChildInfo> children;
    std::vector<ParentInfo> parents;

    int get_evaluation() const { return -children[0].evaluation; }
};

// Positions that have been evaluated, but the children have not
// Typically explored either once or zero times
// The book exit positions are leaf nodes explored once
// If book_exit == false, the position hasn't been added to the book yet
struct LeafNode{
    std::vector<ParentInfo> parents;
    SearchResult search_result;
    bool book_exit;

    int get_evaluation() const { return search_result.evaluation; }
};

// There is a hidden additional node type for positions resulting from the first move of 
// Leaf nodes are positions not in the book that technically are not explored, but will be reached by the first out of book move
// They have not been evaluated, but their (unique) parents have
// I don't actually create any object for these as it would just be a pointer to the parent
// Instead, I just store the LeafNode again in first_oob_map once it's been added to the book


struct OpeningTree{
    std::map<uint64_t, InteriorNode> interior_node_map;
    std::map<uint64_t, LeafNode> leaf_node_map;
    std::map<uint64_t, LeafNode> first_oob_map;
    const Board starting_board;

    void deepen(const int search_depth);
    void show() const;
    void write_to_dir(std::string path, size_t lines_per_file) const;

    private:
        void convert_leaf_to_interior(const int search_depth, const Board &board, const bool wtm);
        bool deepen_recursive(const int search_depth, Board &board, const bool wtm);
        template <typename NodeT>
        bool show_line_from_node(const NodeT node) const;
        void extend_first_oob_parent(const int search_depth, const uint64_t first_oob_hash);
        template <typename NodeT>
        bool reproduce_board_at(const NodeT node, Board &board);
        template <bool upwards>
        void update_evaluation(const ParentInfo parent, const int evaluation);
        template <typename NodeT>
        void build_move_vector(const NodeT node, std::vector<Move> &moves) const;
};

OpeningTree init_opening_tree();
