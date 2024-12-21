# include <string>
# include <tuple>
# include <list>
# include <fstream>
# include <array>
# include <vector>

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

struct TableReader{
    std::ifstream file;

    TableReader(const std::string path);

    uint8_t read_byte(const size_t index);
    uint64_t read_uint64_be(const size_t index);
    uint32_t read_uint32_be(const size_t index);
    uint32_t read_uint32_le(const size_t index);
    uint16_t read_uint16_le(const size_t index);
    void check_magic(bool wdl);
};

struct PairsData{
    // Assigned in one of the setup_pieces_* helpers
    std::array<size_t, 7> pieces;
    std::array<size_t, 7> norm;
    std::array<size_t, 7> factor;
    size_t tb_size;

    // Assigned in setup_pairs
    size_t idxbits;
    size_t blocksize;

    size_t offset;
    size_t sympat;
    size_t min_len;

    std::array<size_t, 3> size;
    std::vector<size_t> symlen;
    std::vector<size_t> base;

    // Assigned after setup_pairs
    size_t indextable;
    size_t sizetable;
    size_t data;

    void set_norm_piece(const TbId &tbid);
    void set_norm_pawn(const TbId &tbid);
    size_t calc_factors_piece(const TbId &tbid, const uint8_t order);
    size_t calc_factors_pawn(const TbId &tbid, const uint8_t order1, const uint8_t order2, const uint8_t file_no);
    void setup_pieces_piece(TableReader &reader, const size_t p_data, const TbId &tbid, const bool lower_bits);
    void setup_pieces_pawn(TableReader &reader, const size_t p_data, const TbId &tbid, const bool lower_bits, const size_t file_no);
    void calc_symlen(TableReader &reader, size_t s, std::vector<size_t> tmp);
    size_t setup_pairs(TableReader &reader, const size_t data_ptr, const bool wdl);
};

struct WdlTable{
    TbId tbid;
    TableReader reader;
    std::array<PairsData, 8> pairs_data;

    bool ready() const { return reader.file ? true : false; }
    WdlTable(const TbId &tbid_, const std::string syzygy_path);
};
