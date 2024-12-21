# include <string>
# include <list>
# include <tuple>

// Interpreted as an octal number with 5=Q, 4=R, 3=B, 2=N, 1=P
// Digits should be in descending order from most significant to least significant with no trailing zeros
// Maximum value is 0o55555 for KQQQQQvK
using HalfTbId = uint16_t;

struct TbId{
    // "stronger" means more pieces, then better best piece, then better second best piece, etc.
    // Because of the HalfTbId storage contract, this is equivalent to stronger >= weaker
    // Technically KPP is "stronger" than KQ
    HalfTbId stronger;
    HalfTbId weaker;

    std::string name() const;
    
    constexpr bool symmetric() const { return stronger == weaker; }
    constexpr bool has_pawns() const { return ((stronger & 7) == 1) or ((weaker & 7) == 1); }
    constexpr bool both_have_pawns() const { return ((stronger & 7) == 1) and ((weaker & 7) == 1); }
    size_t num() const;  // Total number of pieces, including kings
    bool enc_type_2() const;  // Use the K2 piece encoding

    std::tuple<int, int> pawn_counts() const;
    // First element of the tuple is the minimum positive pawn count 
    // Possible return values include (1, 0), (1, 2), (2, 0), etc., but not their reverses
};

std::list<TbId> all_tbs(const int max_num_pieces);

struct TableBaseError{};
