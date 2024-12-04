# include <string>
# include <list>

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
};

std::list<TbId> all_tbs(const int max_num_pieces);
