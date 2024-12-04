# include <string>
# include <list>

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
