# include "syzygy_probe.hpp"
# include "movegen.hpp"
# include "endgames.hpp"

// This is all copied and adapted from python-chess's syzygy.py (as of version 1.10.0)

const std::array<uint8_t, 64> TRIANGLE = {
    6, 0, 1, 2, 2, 1, 0, 6,
    0, 7, 3, 4, 4, 3, 7, 0,
    1, 3, 8, 5, 5, 8, 3, 1,
    2, 4, 5, 9, 9, 5, 4, 2,
    2, 4, 5, 9, 9, 5, 4, 2,
    1, 3, 8, 5, 5, 8, 3, 1,
    0, 7, 3, 4, 4, 3, 7, 0,
    6, 0, 1, 2, 2, 1, 0, 6,
};

constexpr bool offdiag(const Square square){ return (square >> 3) != (square & 7); }
constexpr bool abovediag(const Square square){ return (square >> 3) > (square & 7); }
constexpr Square flipdiag(const Square square){ return ((square >> 3) | (square << 3)) & 63; }

const std::array<uint8_t, 64> LOWER = {
    28,  0,  1,  2,  3,  4,  5,  6,
     0, 29,  7,  8,  9, 10, 11, 12,
     1,  7, 30, 13, 14, 15, 16, 17,
     2,  8, 13, 31, 18, 19, 20, 21,
     3,  9, 14, 18, 32, 22, 23, 24,
     4, 10, 15, 19, 22, 33, 25, 26,
     5, 11, 16, 20, 23, 25, 34, 27,
     6, 12, 17, 21, 24, 26, 27, 35,
};
const std::array<uint8_t, 64> DIAG = {
     0,  0,  0,  0,  0,  0,  0,  8,
     0,  1,  0,  0,  0,  0,  9,  0,
     0,  0,  2,  0,  0, 10,  0,  0,
     0,  0,  0,  3, 11,  0,  0,  0,
     0,  0,  0, 12,  4,  0,  0,  0,
     0,  0, 13,  0,  0,  5,  0,  0,
     0, 14,  0,  0,  0,  0,  6,  0,
    15,  0,  0,  0,  0,  0,  0,  7,
};
const std::array<uint8_t, 64> FLAP = {
    0,  0,  0,  0,  0,  0,  0, 0,
    0,  6, 12, 18, 18, 12,  6, 0,
    1,  7, 13, 19, 19, 13,  7, 1,
    2,  8, 14, 20, 20, 14,  8, 2,
    3,  9, 15, 21, 21, 15,  9, 3,
    4, 10, 16, 22, 22, 16, 10, 4,
    5, 11, 17, 23, 23, 17, 11, 5,
    0,  0,  0,  0,  0,  0,  0, 0,
};
const std::array<uint8_t, 64> PTWIST = {
     0,  0,  0,  0,  0,  0,  0,  0,
    47, 35, 23, 11, 10, 22, 34, 46,
    45, 33, 21,  9,  8, 20, 32, 44,
    43, 31, 19,  7,  6, 18, 30, 42,
    41, 29, 17,  5,  4, 16, 28, 40,
    39, 27, 15,  3,  2, 14, 26, 38,
    37, 25, 13,  1,  0, 12, 24, 36,
     0,  0,  0,  0,  0,  0,  0,  0,
};

const std::array<uint8_t, 8> FILE_TO_FILE = {0, 1, 2, 3, 3, 2, 1, 0};

const std::array<std::array<int16_t, 64>, 10> KK_IDX = {
    std::array<int16_t, 64>{
        -1,  -1,  -1,   0,   1,   2,   3,   4,
        -1,  -1,  -1,   5,   6,   7,   8,   9,
        10,  11,  12,  13,  14,  15,  16,  17,
        18,  19,  20,  21,  22,  23,  24,  25,
        26,  27,  28,  29,  30,  31,  32,  33,
        34,  35,  36,  37,  38,  39,  40,  41,
        42,  43,  44,  45,  46,  47,  48,  49,
        50,  51,  52,  53,  54,  55,  56,  57,
    },
    std::array<int16_t, 64>{
        58,  -1,  -1,  -1,  59,  60,  61,  62,
        63,  -1,  -1,  -1,  64,  65,  66,  67,
        68,  69,  70,  71,  72,  73,  74,  75,
        76,  77,  78,  79,  80,  81,  82,  83,
        84,  85,  86,  87,  88,  89,  90,  91,
        92,  93,  94,  95,  96,  97,  98,  99,
        100, 101, 102, 103, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115,
    },
    std::array<int16_t, 64>{
        116, 117,  -1,  -1,  -1, 118, 119, 120,
        121, 122,  -1,  -1,  -1, 123, 124, 125,
        126, 127, 128, 129, 130, 131, 132, 133,
        134, 135, 136, 137, 138, 139, 140, 141,
        142, 143, 144, 145, 146, 147, 148, 149,
        150, 151, 152, 153, 154, 155, 156, 157,
        158, 159, 160, 161, 162, 163, 164, 165,
        166, 167, 168, 169, 170, 171, 172, 173,
    },
    std::array<int16_t, 64>{
        174,  -1,  -1,  -1, 175, 176, 177, 178,
        179,  -1,  -1,  -1, 180, 181, 182, 183,
        184,  -1,  -1,  -1, 185, 186, 187, 188,
        189, 190, 191, 192, 193, 194, 195, 196,
        197, 198, 199, 200, 201, 202, 203, 204,
        205, 206, 207, 208, 209, 210, 211, 212,
        213, 214, 215, 216, 217, 218, 219, 220,
        221, 222, 223, 224, 225, 226, 227, 228,
    },
    std::array<int16_t, 64>{
        229, 230,  -1,  -1,  -1, 231, 232, 233,
        234, 235,  -1,  -1,  -1, 236, 237, 238,
        239, 240,  -1,  -1,  -1, 241, 242, 243,
        244, 245, 246, 247, 248, 249, 250, 251,
        252, 253, 254, 255, 256, 257, 258, 259,
        260, 261, 262, 263, 264, 265, 266, 267,
        268, 269, 270, 271, 272, 273, 274, 275,
        276, 277, 278, 279, 280, 281, 282, 283,
    },
    std::array<int16_t, 64>{
        284, 285, 286, 287, 288, 289, 290, 291,
        292, 293,  -1,  -1,  -1, 294, 295, 296,
        297, 298,  -1,  -1,  -1, 299, 300, 301,
        302, 303,  -1,  -1,  -1, 304, 305, 306,
        307, 308, 309, 310, 311, 312, 313, 314,
        315, 316, 317, 318, 319, 320, 321, 322,
        323, 324, 325, 326, 327, 328, 329, 330,
        331, 332, 333, 334, 335, 336, 337, 338,
    },
    std::array<int16_t, 64>{
        -1,  -1, 339, 340, 341, 342, 343, 344,
        -1,  -1, 345, 346, 347, 348, 349, 350,
        -1,  -1, 441, 351, 352, 353, 354, 355,
        -1,  -1,  -1, 442, 356, 357, 358, 359,
        -1,  -1,  -1,  -1, 443, 360, 361, 362,
        -1,  -1,  -1,  -1,  -1, 444, 363, 364,
        -1,  -1,  -1,  -1,  -1,  -1, 445, 365,
        -1,  -1,  -1,  -1,  -1,  -1,  -1, 446,
    },
    std::array<int16_t, 64>{
        -1,  -1,  -1, 366, 367, 368, 369, 370,
        -1,  -1,  -1, 371, 372, 373, 374, 375,
        -1,  -1,  -1, 376, 377, 378, 379, 380,
        -1,  -1,  -1, 447, 381, 382, 383, 384,
        -1,  -1,  -1,  -1, 448, 385, 386, 387,
        -1,  -1,  -1,  -1,  -1, 449, 388, 389,
        -1,  -1,  -1,  -1,  -1,  -1, 450, 390,
        -1,  -1,  -1,  -1,  -1,  -1,  -1, 451,
    },
    std::array<int16_t, 64>{
        452, 391, 392, 393, 394, 395, 396, 397,
        -1,  -1,  -1,  -1, 398, 399, 400, 401,
        -1,  -1,  -1,  -1, 402, 403, 404, 405,
        -1,  -1,  -1,  -1, 406, 407, 408, 409,
        -1,  -1,  -1,  -1, 453, 410, 411, 412,
        -1,  -1,  -1,  -1,  -1, 454, 413, 414,
        -1,  -1,  -1,  -1,  -1,  -1, 455, 415,
        -1,  -1,  -1,  -1,  -1,  -1,  -1, 456,
    },
    std::array<int16_t, 64>{
        457, 416, 417, 418, 419, 420, 421, 422,
        -1, 458, 423, 424, 425, 426, 427, 428,
        -1,  -1,  -1,  -1,  -1, 429, 430, 431,
        -1,  -1,  -1,  -1,  -1, 432, 433, 434,
        -1,  -1,  -1,  -1,  -1, 435, 436, 437,
        -1,  -1,  -1,  -1,  -1, 459, 438, 439,
        -1,  -1,  -1,  -1,  -1,  -1, 460, 440,
        -1,  -1,  -1,  -1,  -1,  -1,  -1, 461,
    }
};

constexpr int binom(uint8_t n, uint8_t k){
    assert(k > 0);
    int val = n;
    for (uint8_t i = 1; i < k; ) {
        val *= n-i;
        i++;
        val /= i;
    }
    return val;
}

const std::array<std::array<uint32_t, 24>, 5> PAWNIDX = {
    std::array<uint32_t, 24>{
             0,      1,      2,      3,      4,      5, 
             0,      1,      2,      3,      4,      5, 
             0,      1,      2,      3,      4,      5, 
             0,      1,      2,      3,      4,      5
    }, 
    std::array<uint32_t, 24>{
             0,     47,     92,    135,    176,    215, 
             0,     35,     68,     99,    128,    155, 
             0,     23,     44,     63,     80,     95, 
             0,     11,     20,     27,     32,     35
    }, 
    std::array<uint32_t, 24>{
             0,   1081,   2071,   2974,   3794,   4535, 
             0,    595,   1123,   1588,   1994,   2345, 
             0,    253,    463,    634,    770,    875, 
             0,     55,     91,    112,    122,    125
    }, 
    std::array<uint32_t, 24>{
             0,  16215,  30405,  42746,  53406,  62545,
             0,   6545,  12001,  16496,  20150,  23075, 
             0,   1771,   3101,   4070,   4750,   5205, 
             0,    165,    249,    284,    294,    295
    },
    std::array<uint32_t, 24>{
             0, 178365, 327360, 450770, 552040, 634291, 
             0,  52360,  93280, 124745, 148496, 166046, 
             0,   8855,  14840,  18716,  21096,  22461, 
             0,    330,    456,    491,    496,    496
    }
};
const std::array<std::array<uint32_t, 4>, 5> PFACTOR = {
    std::array<uint32_t, 4>{     6,      6,      6,      6},
    std::array<uint32_t, 4>{   252,    180,    108,     36},
    std::array<uint32_t, 4>{  5201,   2645,    953,    125},
    std::array<uint32_t, 4>{ 70315,  25375,   5491,    295},
    std::array<uint32_t, 4>{700336, 178696,  23176,    496}
};

const std::array<uint8_t, 5> WDL_TO_MAP = {1, 3, 0, 2, 0};
const std::array<uint8_t, 5> PA_FLAGS = {8, 0, 0, 0, 4};

const std::array<char, 6> PCHR = {'-', 'P', 'N', 'B', 'R', 'Q'};

int htbid_piece_count(HalfTbId htbid){
    int count = 0;
    while (htbid & 7) { htbid = htbid >> 3; count += 1; }
    return count;
}
int htbid_pawn_count(HalfTbId htbid){
    int count = 0;
    while ((htbid & 7) == 1) { htbid = htbid >> 3; count += 1; }
    return count;
}
bool htbid_has_singleton(HalfTbId htbid){
    while (htbid){
        const auto piece = htbid & 7;
        int count = 0;
        while ((htbid & 7) == piece) { htbid = htbid >> 3; count += 1; }
        if (count == 1) return true;
    }
    return false;
}

std::string half_tb_name(HalfTbId htbid){
    std::string out = "";
    while (htbid > 0) {
        out = PCHR[htbid & 7] + out;
        htbid = htbid >> 3;
    }
    return out;
}

HalfTbId half_tb_id_from_half_board(const HalfBoard &hb){
    HalfTbId htbid = 0;

    for (int i = 0; i < __builtin_popcountll(hb.Queen); i++) { htbid = (htbid << 3) + 5; }
    for (int i = 0; i < __builtin_popcountll(hb.Rook); i++) { htbid = (htbid << 3) + 4; }
    for (int i = 0; i < __builtin_popcountll(hb.Bishop); i++) { htbid = (htbid << 3) + 3; }
    for (int i = 0; i < __builtin_popcountll(hb.Knight); i++) { htbid = (htbid << 3) + 2; }
    for (int i = 0; i < __builtin_popcountll(hb.Pawn); i++) { htbid = (htbid << 3) + 1; }

    return htbid;
}

std::list<HalfTbId> all_half_tbs(const int num_pieces){
    if (num_pieces == 0) return {0};
    if (num_pieces == 1) return {1, 2, 3, 4, 5};

    std::list<HalfTbId> output;
    const auto one_fewer = all_half_tbs(num_pieces - 1);
    for (const auto htbid : one_fewer) {
        const auto smallest_piece = htbid & 7;
        for (HalfTbId new_piece = 1; new_piece <= smallest_piece; new_piece++) 
            output.push_back(htbid * 8 + new_piece);
    }
    return output;
}

std::string TbId::name() const { return "K" + half_tb_name(stronger) + "vK" + half_tb_name(weaker); }
size_t TbId::num() const { return 2 + htbid_piece_count(stronger) + htbid_piece_count(weaker); }
bool TbId::enc_type_2() const { return not (has_pawns() or htbid_has_singleton(stronger) or htbid_has_singleton(weaker)); }
std::tuple<int, int> TbId::pawn_counts() const {
    int pc_strong = htbid_pawn_count(stronger);
    int pc_weak = htbid_pawn_count(weaker);
    if (pc_strong == 0) return std::make_tuple(pc_weak, 0);
    if (pc_weak == 0) return std::make_tuple(pc_strong, 0);
    if (pc_strong < pc_weak) return std::make_tuple(pc_strong, pc_weak);
    return std::make_tuple(pc_weak, pc_strong);
}

bool tbid_from_board(const Board &board, TbId &tbid){
    const HalfTbId w = half_tb_id_from_half_board(board.White);
    const HalfTbId b = half_tb_id_from_half_board(board.Black);
    const bool mirrored = w < b;
    tbid.stronger = mirrored ? b : w;
    tbid.weaker = mirrored ? w : b;
    return mirrored;
}

std::list<TbId> all_tbs(const int max_num_pieces){
    if (max_num_pieces <= 2) return std::list<TbId>();  // KvK is not a thing
    auto output = all_tbs(max_num_pieces - 1);

    for (auto weak_count = 0; weak_count * 2 + 2 <= max_num_pieces; weak_count++){
        auto strong_count = max_num_pieces - 2 - weak_count;
        for (const auto stronger : all_half_tbs(strong_count)) {
            for (const auto weaker : all_half_tbs(weak_count)) {
                if (stronger >= weaker) output.push_back(TbId{stronger, weaker});
            }
        }
    }

    return output;
}

constexpr int dtz_before_zeroing(int wdl){
    return ((wdl > 0) - (wdl < 0)) * ((std::abs(wdl) == 2) ? 1 : 101);
}

template <typename T>
uint32_t interpret_uint32_le(const T arr, const size_t offset){
    return 
        ((uint32_t) arr[offset + 0] << 0) | 
        ((uint32_t) arr[offset + 1] << 8) | 
        ((uint32_t) arr[offset + 2] << 16) | 
        ((uint32_t) arr[offset + 3] << 24);
}

template <typename T>
uint16_t interpret_uint16_le(const T arr, const size_t offset){
    return ((uint16_t) arr[offset + 0] << 0) | ((uint16_t) arr[offset + 1] << 8);
}

TableReader::TableReader(std::string path) {
    file.open(path, std::ios::binary | std::ios::in);
    if (not file) throw TableBaseError();
}

void TableReader::read_bytes_to(const size_t index, uint8_t *out_addr, size_t len){
    file.seekg(index, std::ios::beg);
    file.read((char*) out_addr, len);
}

uint8_t TableReader::read_byte(const size_t index){
    uint8_t out_var;
    read_bytes_to(index, &out_var, 1);
    return out_var;
}

uint32_t TableReader::read_uint32_le(const size_t index){
    std::array<uint8_t, 4> arr;
    read_bytes_to(index, arr.data(), 4);
    return interpret_uint32_le(arr, 0);
}

uint16_t TableReader::read_uint16_le(const size_t index){
    std::array<uint8_t, 2> arr;
    read_bytes_to(index, arr.data(), 2);
    return interpret_uint16_le(arr, 0);
}

void TableReader::check_magic(bool wdl) {
    const uint32_t expected_magic = wdl ? 0x5d23e871 : 0xa50c66d7;
    const uint32_t read_magic = read_uint32_le(0);
    if (read_magic != expected_magic) throw TableBaseError();
}

constexpr uint8_t half_byte_mask(const uint8_t byte, const bool lower_bits){
    return lower_bits ? (byte & 0x0f) : (byte >> 4);
}

void PairsData::set_norm_piece(const TbId &tbid){
    size_t i = tbid.enc_type_2() ? 2 : 3;
    norm[0] = i;

    while (i < tbid.num()){
        norm[i] = 0;
        for (size_t j = i; j < tbid.num() and pieces[i] == pieces[j]; j++) norm[i] += 1;
        i += norm[i];
    }
}

void PairsData::set_norm_pawn(const TbId &tbid){
    int pc0; int pc1;
    std::tie(pc0, pc1) = tbid.pawn_counts();

    norm[0] = pc0;
    if (pc1) norm[pc0] = pc1;

    size_t i = pc0 + pc1;
    while (i < tbid.num()){
        norm[i] = 0;
        for (size_t j = i; j < tbid.num() and pieces[i] == pieces[j]; j++) norm[i] += 1;
        i += norm[i];
    }
}

size_t PairsData::calc_factors_piece(const TbId &tbid, const uint8_t order){
    size_t fac = 1; 
    size_t k = 0; 
    size_t i = norm[0]; 
    size_t n = 64 - norm[0];

    while (i < tbid.num() or k == order) {
        if (k == order) {
            factor[0] = fac;
            fac *= tbid.enc_type_2() ? 462 : 31332;  // From the PIVFAC constant
        } else {
            factor[i] = fac;
            fac *= binom(n, norm[i]);
            n -= norm[i];
            i += norm[i];
        }
        k += 1;
    }

    return fac;
}

size_t PairsData::calc_factors_pawn(const TbId &tbid, const uint8_t order1, const uint8_t order2, const uint8_t file_no){
    size_t i = norm[0];
    if (order2 < 0x0f) i += norm[i];
    size_t n = 64 - i;
    size_t fac = 1;
    size_t k = 0;

    while (i < tbid.num() or k == order1 or k == order2){
        if (k == order1) {
            factor[0] = fac;
            fac *= PFACTOR[norm[0] - 1][file_no];
        } else if (k == order2) {
            factor[norm[0]] = fac;
            fac *= binom(48 - norm[0], norm[norm[0]]);
        } else {
            factor[i] = fac;
            fac *= binom(n, norm[i]);
            n -= norm[i];
            i += norm[i];
        }
        k += 1;
    }

    return fac;
}

void PairsData::setup_pieces_piece(TableReader &reader, const size_t p_data, const TbId &tbid, const bool lower_bits){
    for (size_t i = 0; i < tbid.num(); i++) pieces[i] = half_byte_mask(reader.read_byte(p_data + i + 1), lower_bits);
    const auto order = half_byte_mask(reader.read_byte(p_data), lower_bits);
    set_norm_piece(tbid);
    tb_size = calc_factors_piece(tbid, order);
}

void PairsData::setup_pieces_pawn(TableReader &reader, const size_t p_data, const TbId &tbid, const bool lower_bits, const size_t file_no){
    size_t j = tbid.both_have_pawns() ? 2 : 1;
    const auto order1 = half_byte_mask(reader.read_byte(p_data), lower_bits);
    const auto order2 = tbid.both_have_pawns() ? half_byte_mask(reader.read_byte(p_data + 1), lower_bits) : 0x0f;
    for (size_t i = 0; i < tbid.num(); i++) pieces[i] = half_byte_mask(reader.read_byte(p_data + i + j), lower_bits);
    set_norm_pawn(tbid);
    tb_size = calc_factors_pawn(tbid, order1, order2, file_no);
}

void PairsData::calc_symlen(const size_t s, std::vector<bool> &tmp, const std::vector<uint8_t> &sbytes){
    if (tmp[s]) return;
    size_t s1 = ((size_t) (sbytes[3 * s + 1] & 0xf)) * 256 + (size_t) (sbytes[3 * s]);
    size_t s2 = ((size_t) (sbytes[3 * s + 2])) * 16 + ((size_t) (sbytes[3 * s + 1])) / 16;
    s1_vec[s] = s1;
    s2_vec[s] = s2;
    if (s2 == 0x0fff) symlen[s] = 0;
    else {
        calc_symlen(s1, tmp, sbytes);
        calc_symlen(s2, tmp, sbytes);
        symlen[s] = symlen[s1] + symlen[s2] + 1;
    }
    tmp[s] = true;
}

size_t PairsData::setup_pairs(TableReader &reader, const size_t data_ptr, const bool wdl) {
    flags = reader.read_byte(data_ptr);
    if (flags & 0x80) {
        idxbits = 0;
        if (wdl) min_len = reader.read_byte(data_ptr + 1);
        else min_len = 0;
        size[0] = 0; size[1] = 0; size[2] = 0;
        return data_ptr + 2;
    }

    blocksize = reader.read_byte(data_ptr + 1);
    idxbits = reader.read_byte(data_ptr + 2);

    const size_t real_num_blocks = reader.read_uint32_le(data_ptr + 4);
    const size_t num_blocks = real_num_blocks + reader.read_byte(data_ptr + 3);
    const size_t max_len = reader.read_byte(data_ptr + 8);
    min_len = reader.read_byte(data_ptr + 9);
    const size_t h = max_len - min_len + 1;
    const size_t num_syms = reader.read_uint16_le(data_ptr + 10 + 2 * h);

    const size_t num_indices = (tb_size + (1ull << idxbits) - 1) >> idxbits;
    size[0] = 6 * num_indices;
    size[1] = 2 * num_blocks;
    size[2] = (1ull << blocksize) * real_num_blocks;

    auto sbytes = std::vector(3 * num_syms, (uint8_t) 0);
    reader.read_bytes_to(data_ptr + 12 + 2 * h, sbytes.data(), 3 * num_syms);
    symlen = std::vector(num_syms, (size_t) 0);
    s1_vec = std::vector(num_syms, (size_t) 0);
    s2_vec = std::vector(num_syms, (size_t) 0);
    std::vector<bool> tmp = std::vector(num_syms, false);
    for (size_t i = 0; i < num_syms; i++) calc_symlen(i, tmp, sbytes);

    offset_data = std::vector(h, (size_t) 0);
    for (size_t i = 0; i < h; i++) offset_data[i] = reader.read_uint16_le(data_ptr + 10 + i * 2);
    base = std::vector(h, (size_t) 0);
    base[h - 1] = 0;
    for (int i = h - 2; i >= 0; i--) {
        base[i] = (base[i + 1] + offset_data[i] - offset_data[i+1]) / 2;
    }
    for (size_t i = 0; i < h; i++) base[i] = base[i] << (64 - (min_len + i));

    return data_ptr + 12 + 2 * h + 3 * num_syms + (num_syms & 1);
}

size_t PairsData::encode(std::array<Square, 7> &p, const TbId &tbid) const {
    const size_t n = tbid.num();

    if (p[0] & 4){ for (size_t i = 0; i < n; i ++) p[i] ^= 7; }
    if (not tbid.has_pawns()){
        if (p[0] & 32){ for (size_t i = 0; i < n; i ++) p[i] ^= 56; }
        bool set_diag_yet = false;
        for (size_t i = 0; i < (tbid.enc_type_2() ? 2 : 3) and not set_diag_yet; i++) {
            if (offdiag(p[i])) {
                if (abovediag(p[i])) { for (size_t j = 0; j < n; j++) p[j] = flipdiag(p[j]); }
                set_diag_yet = true;
            }
        }
    }

    size_t idx;
    size_t i;
    bool secondary_pawn = false;
    if (tbid.has_pawns()) {
        const auto pawn_counts = tbid.pawn_counts();
        i = std::get<0>(pawn_counts);
        secondary_pawn = std::get<1>(pawn_counts) > 0;

        idx = PAWNIDX[i - 1][FLAP[p[0]]];
        for (size_t j = 1; j < i; j++) idx += binom(PTWIST[p[j]], i - j);
    } else if (tbid.enc_type_2()) { 
        idx = KK_IDX[TRIANGLE[p[0]]][p[1]];
        i = 2;
    } else {
        size_t p1_offset = (p[1] > p[0]) ? 1 : 0;
        size_t p2_offset = ((p[2] > p[0]) ? 1 : 0) + ((p[2] > p[1]) ? 1 : 0);

        if (offdiag(p[0])) { 
            idx = TRIANGLE[p[0]] * 63 * 62 + (p[1] - p1_offset) * 62 + (p[2] - p2_offset); 
        } else if (offdiag(p[1])) { 
            idx = 6 * 63 * 62 + DIAG[p[0]] * 28 * 62 + LOWER[p[1]] * 62 + p[2] - p2_offset; 
        } else if (offdiag(p[2])) { 
            idx = 6 * 63 * 62 + 4 * 28 * 62 + (DIAG[p[0]]) * 7 * 28 + (DIAG[p[1]] - p1_offset) * 28 + LOWER[p[2]]; 
        } else {
            idx = 6 * 63 * 62 + 4 * 28 * 62 + 4 * 7 * 28 + (DIAG[p[0]] * 7 * 6) + (DIAG[p[1]] - p1_offset) * 6 + (DIAG[p[2]] - p2_offset);
        }
        i = 3;
    }
    idx *= factor[0];

    while (i < n) {
        const size_t t = norm[i];
        for (size_t j = i; j < i + t; j++) {
            for (size_t k = j + 1; k < i + t; k++) { 
                if (p[j] > p[k]) std::swap(p[j], p[k]);
            }
        }

        size_t s = 0;
        for (size_t m = i; m < i + t; m++){
            Square pj = p[m] - (secondary_pawn ? 8 : 0);
            for (size_t l = 0; l < i; l++) { if (p[m] > p[l]) pj--; }
            s += binom(pj, m - i + 1);
        }

        idx += s * factor[i];
        i += t;
        secondary_pawn = false;
    }

    return idx;
}

size_t PairsData::decompress_pairs(TableReader &reader, size_t idx) const {
    if (idxbits == 0) return min_len;

    const size_t mainidx = idx >> idxbits;
    std::array<uint8_t, 16> indextable_bytes;
    reader.read_bytes_to(indextable + 6 * (mainidx - 1), indextable_bytes.data(), 16);
    size_t block = interpret_uint32_le(indextable_bytes, 6);
    int64_t litidx = (idx & (1ull << idxbits) - 1) - (1ull << (idxbits - 1)) + interpret_uint16_le(indextable_bytes, 10);

    if (litidx < 0) {
        const size_t prev_block = (mainidx == 0) ? 0 : interpret_uint32_le(indextable_bytes, 0);
        std::vector<uint8_t> sizetable_bytes = std::vector(2 * (block - prev_block), (uint8_t) 0);
        reader.read_bytes_to(sizetable + 2 * prev_block, sizetable_bytes.data(), 2 * (block - prev_block));

        while (litidx < 0) {
            block--; 
            litidx += interpret_uint16_le(sizetable_bytes, 2 * (block - prev_block)) + 1;
        }
    } else {
        const size_t next_block = (mainidx == size[0] / 6 - 1) ? (size[1] / 2 - 1) : interpret_uint32_le(indextable_bytes, 12);
        const size_t initial_block = block;
        std::vector<uint8_t> sizetable_bytes = std::vector(2 * (next_block - block + 1), (uint8_t) 0);
        reader.read_bytes_to(sizetable + 2 * block, sizetable_bytes.data(), 2 * (next_block - block + 1));

        int64_t last = interpret_uint16_le(sizetable_bytes, 0);
        while (litidx > last) {
            litidx -= last + 1;
            block++;
            last = interpret_uint16_le(sizetable_bytes, 2 * (block - initial_block));
        }
    }
    size_t ulitidx = litidx;

    std::vector<uint8_t> code_bytes = std::vector(1ull << blocksize, (uint8_t) 0);
    reader.read_bytes_to(data + (block << blocksize), code_bytes.data(), 1ull << blocksize);
    uint64_t code = 0;
    size_t ptr = 0;
    size_t bitcnt = 64;
    size_t sym;

    while (true) {
        while (bitcnt >= 8) {
            bitcnt -= 8;
            code |= ((size_t) code_bytes[ptr]) << bitcnt;
            ptr++;
        }
        size_t l = min_len;
        while (code < base[l - min_len]) l++;
        sym = ((code - base[l - min_len]) >> (64 - l)) + offset_data[l - min_len];
        if (ulitidx < symlen[sym] + 1) break;
        ulitidx -= symlen[sym] + 1;
        code <<= l;
        bitcnt += l;
    }
    while (true) {
        size_t s1 = s1_vec[sym];
        if (symlen[sym] == 0) return s1;
        if (ulitidx < symlen[s1] + 1) sym = s1;
        else { 
            ulitidx -= symlen[s1] + 1; 
            sym = s2_vec[sym]; 
        }
    }
}

WdlTable::WdlTable(const TbId &tbid_, const std::string syzygy_path) : tbid(tbid_), reader(syzygy_path + "/" + tbid.name() + ".rtbw") {
    reader.check_magic(true);

    const bool split = not tbid.symmetric();
    const bool has_pawns = tbid.has_pawns();

    size_t data_ptr = 5;

    // Setup pieces
    if (has_pawns) {
        const size_t gap = tbid.num() + (tbid.both_have_pawns() ? 2 : 1);
        const size_t split_mult = split ? 2 : 1;
        for (size_t f = 0; f < 4; f++){
            pairs_data[split_mult * f].setup_pieces_pawn(reader, data_ptr, tbid, true, f);
            if (split) pairs_data[2 * f + 1].setup_pieces_pawn(reader, data_ptr, tbid, false, f);
            data_ptr += gap;
        }
    } else {
        pairs_data[0].setup_pieces_piece(reader, data_ptr, tbid, true);
        if (split) pairs_data[1].setup_pieces_piece(reader, data_ptr, tbid, false);
        data_ptr += tbid.num() + 1;
    }
    data_ptr += data_ptr & 0x01;
    
    const size_t n_pairs = (split ? 2 : 1) * (has_pawns ? 4 : 1);
    for (size_t i = 0; i < n_pairs; i++) { data_ptr = pairs_data[i].setup_pairs(reader, data_ptr, true); }
    for (size_t i = 0; i < n_pairs; i++) { 
        pairs_data[i].indextable = data_ptr; 
        data_ptr += pairs_data[i].size[0]; 
    }
    for (size_t i = 0; i < n_pairs; i++) { 
        pairs_data[i].sizetable = data_ptr; 
        data_ptr += pairs_data[i].size[1]; 
    }
    for (size_t i = 0; i < n_pairs; i++) {
        data_ptr = (data_ptr + 0x3f) & ~0x3f;
        pairs_data[i].data = data_ptr; 
        data_ptr += pairs_data[i].size[2]; 
    }
}

DtzTable::DtzTable(const TbId &tbid_, const std::string syzygy_path) : tbid(tbid_), reader(syzygy_path + "/" + tbid.name() + ".rtbz") {
    reader.check_magic(false);

    const bool has_pawns = tbid.has_pawns();

    size_t data_ptr = 5;

    // Setup pieces
    if (has_pawns) {
        const size_t gap = tbid.num() + (tbid.both_have_pawns() ? 2 : 1);
        for (size_t f = 0; f < 4; f++){
            pairs_data[f].setup_pieces_pawn(reader, data_ptr, tbid, true, f);
            data_ptr += gap;
        }
    } else {
        pairs_data[0].setup_pieces_piece(reader, data_ptr, tbid, true);
        data_ptr += tbid.num() + 1;
    }
    data_ptr += data_ptr & 0x01;
    
    const size_t n_pairs = has_pawns ? 4 : 1;
    for (size_t i = 0; i < n_pairs; i++) { data_ptr = pairs_data[i].setup_pairs(reader, data_ptr, true); }

    for (size_t i = 0; i < n_pairs; i++) {
        if (pairs_data[i].flags & 2) {
            if (pairs_data[i].flags & 16) {
                data_ptr += data_ptr & 0x01;
                for (size_t j = 0; j < 4; j++) {
                    pairs_data[i].map_idx[j] = data_ptr + 2;
                    data_ptr += reader.read_uint16_le(data_ptr);
                }
            } else {
                for (size_t j = 0; j < 4; j++) {
                    pairs_data[i].map_idx[j] = data_ptr + 1;
                    data_ptr += reader.read_byte(data_ptr);
                }
            }
        }
    }
    data_ptr += data_ptr & 0x01;

    for (size_t i = 0; i < n_pairs; i++) { 
        pairs_data[i].indextable = data_ptr; 
        data_ptr += pairs_data[i].size[0]; 
    }
    for (size_t i = 0; i < n_pairs; i++) { 
        pairs_data[i].sizetable = data_ptr; 
        data_ptr += pairs_data[i].size[1]; 
    }
    for (size_t i = 0; i < n_pairs; i++) {
        data_ptr = (data_ptr + 0x3f) & ~0x3f;
        pairs_data[i].data = data_ptr; 
        data_ptr += pairs_data[i].size[2]; 
    }
}

uint8_t reorder_pawns(std::array<Square, 7> &p, const size_t num_pawns){
    for (size_t i = 0; i < num_pawns; i++) {
        for (size_t j = i+1; j < num_pawns; j++) {
            if (FLAP[p[i]] > FLAP[p[j]]) std::swap(p[i], p[j]);
        }
    }
    return FILE_TO_FILE[p[0] & 7];
}

int WdlTable::probe(const bool wtm, const bool should_mirror, const Board &board){
    uint8_t cmirror; Square smirror; size_t pairs_idx;

    if (tbid.symmetric()) { 
        cmirror = wtm ? 0 : 8;
        smirror = wtm ? 0 : 56;
        pairs_idx = 0;
    } else {
        cmirror = should_mirror ? 8 : 0;
        smirror = should_mirror ? 56 : 0;
        pairs_idx = (should_mirror ^ wtm) ? 0 : 1;
    }

    std::array<Square, 7> p;
    size_t i = 0;
    bool first_pawn_loop = tbid.has_pawns();
    while (i < tbid.num()) {
        uint8_t piece_color = (pairs_data[pairs_idx].pieces[i] ^ cmirror) >> 3;
        const HalfBoard &hb = piece_color ? board.Black : board.White;

        uint8_t piece_type = pairs_data[pairs_idx].pieces[i] & 7;
        BitMask bb;
        switch (piece_type) {
            case 1: bb = hb.Pawn; break;
            case 2: bb = hb.Knight; break;
            case 3: bb = hb.Bishop; break;
            case 4: bb = hb.Rook; break;
            case 5: bb = hb.Queen; break;
            case 6: bb = ToMask(hb.King); break;
            default: throw TableBaseError(); 
        }

        Bitloop(bb, loop_var){ p[i] = TZCNT(loop_var) ^ smirror; i++; }

        if (first_pawn_loop) {
            // The pawn_file function
            const uint8_t file_no = reorder_pawns(p, i);
            pairs_idx += (tbid.symmetric() ? 1 : 2) * file_no;
        }
        first_pawn_loop = false;
    }

    const size_t idx = pairs_data[pairs_idx].encode(p, tbid);
    const size_t res = pairs_data[pairs_idx].decompress_pairs(reader, idx);
    return (int)(res & 0xf) - 2;
}

int DtzTable::probe(const bool wtm, const bool should_mirror, const Board &board, int wdl){
    uint8_t cmirror; Square smirror; uint8_t bside;

    if (tbid.symmetric()) { 
        cmirror = wtm ? 0 : 8;
        smirror = wtm ? 0 : 56;
        bside = 0;
    } else {
        cmirror = should_mirror ? 8 : 0;
        smirror = should_mirror ? 56 : 0;
        bside = (should_mirror ^ wtm) ? 0 : 1;
    }

    if ((not tbid.has_pawns()) and ((pairs_data[0].flags & 1) != bside)) return 0;

    std::array<Square, 7> p;
    size_t i = 0;
    bool first_pawn_loop = tbid.has_pawns();
    size_t pairs_idx = 0;
    while (i < tbid.num()) {
        uint8_t piece_color = (pairs_data[pairs_idx].pieces[i] ^ cmirror) >> 3;
        const HalfBoard &hb = piece_color ? board.Black : board.White;

        uint8_t piece_type = pairs_data[pairs_idx].pieces[i] & 7;
        BitMask bb;
        switch (piece_type) {
            case 1: bb = hb.Pawn; break;
            case 2: bb = hb.Knight; break;
            case 3: bb = hb.Bishop; break;
            case 4: bb = hb.Rook; break;
            case 5: bb = hb.Queen; break;
            case 6: bb = ToMask(hb.King); break;
            default: throw TableBaseError(); 
        }

        Bitloop(bb, loop_var){ p[i] = TZCNT(loop_var) ^ smirror; i++; }

        if (first_pawn_loop) {
            // The pawn_file function
            pairs_idx = reorder_pawns(p, i);
            if ((pairs_data[pairs_idx].flags & 1) != bside) return 0;
        }
        first_pawn_loop = false;
    }

    const size_t idx = pairs_data[pairs_idx].encode(p, tbid);
    size_t res = pairs_data[pairs_idx].decompress_pairs(reader, idx);

    if (pairs_data[pairs_idx].flags & 2) {
        size_t data_ptr = pairs_data[pairs_idx].map_idx[WDL_TO_MAP[wdl + 2]];
        if (pairs_data[pairs_idx].flags & 16) res = reader.read_uint16_le(data_ptr + 2 * res);
        else res = reader.read_byte(data_ptr + res);
    }

    if ((not (pairs_data[pairs_idx].flags & PA_FLAGS[wdl + 2])) or (wdl & 1)) res *= 2;

    return res;
}

Tablebase::Tablebase(const int max_num_pieces, const std::string syzygy_path){
    const auto tbid_list = all_tbs(max_num_pieces);
    for (const auto tbid : tbid_list) {
        wdl_tables.insert(std::pair{tbid, WdlTable(tbid, syzygy_path)});
        dtz_tables.insert(std::pair{tbid, DtzTable(tbid, syzygy_path)});
    }
}

bool Tablebase::ready() const {
    for (auto it = wdl_tables.begin(); it != wdl_tables.end(); it++){
        if (not it->second.ready()) return false;
    }
    return true;
}

int Tablebase::probe_wdl(const bool wtm, const Board &board){
    return std::get<0>(probe_wdl_ab(wtm, board, -2, 2));
}

std::tuple<int, bool> Tablebase::probe_wdl_ab(const bool wtm, const Board &board, int alpha, int beta){
    if (is_insufficient_material(board)) return {0, false};

    // Generating all moves is suboptimal, but we're so far from the critical path that it doesn't cost us much
    const auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    auto moves = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);

    if (moves.empty()) {
        if (cnp.CheckMask != FULL_BOARD) return {-2, false};  // Checkmate
        return {0, false};  // Stalemate
    }

    bool seen_non_ep_move = false;
    while (not moves.empty()) {
        const Move move = moves.top();
        if (move_flags(move) != EN_PASSANT_CAPTURE) seen_non_ep_move = true;

        const bool is_non_ep_captuere = ToMask(move_destination(move)) & (wtm ? board.Black.All : board.White.All);
        if (is_non_ep_captuere or (move_flags(move) == EN_PASSANT_CAPTURE)) {
            auto b2 = board.copy();
            (wtm ? make_move<true> : make_move<false>)(b2, move);
            const int rec_probe_res = std::get<0>(probe_wdl_ab(not wtm, b2, -beta, -alpha));
            alpha = std::max(alpha, -rec_probe_res);
            if (alpha >= beta) return {beta, true};
        }

        moves.pop();
    }

    // Only legal moves are en passant, so we can't trust the TB entry
    if (not seen_non_ep_move) return {alpha, alpha > 0};

    TbId tbid;
    const bool mirrored = tbid_from_board(board, tbid);
    if (wdl_tables.count(tbid) != 1) throw TableBaseError();
    const int tb_probe_res = wdl_tables.at(tbid).probe(wtm, mirrored, board);

    return {std::max(alpha, tb_probe_res), (alpha >= tb_probe_res) and (alpha > 0)};
}

bool is_non_capture_pawn_move(const Move move, const BitMask occ){
    switch (move_flags(move)) {
        case SINGLE_PAWN_PUSH:
        case DOUBLE_PAWN_PUSH:
        return true;

        case PROMOTE_TO_KNIGHT:
        case PROMOTE_TO_BISHOP:
        case PROMOTE_TO_ROOK:
        case PROMOTE_TO_QUEEN:
        return not (ToMask(move_destination(move)) & occ);

        default:
        return false;
    }
}

int Tablebase::probe_dtz(const bool wtm, const Board &board){
    int wdl; bool winning_capture;
    std::tie(wdl, winning_capture) = probe_wdl_ab(wtm, board, -2, 2);

    if (wdl == 0) return 0;  // Draw
    if (winning_capture) return dtz_before_zeroing(wdl);  // Winning capture

    if ((wdl > 0) and (wtm ? board.White.Pawn : board.Black.Pawn)) {  // Look for winning pawn move
        const auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
        auto moves = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);

        while (not moves.empty()) {
            const Move move = moves.top();
            if (is_non_capture_pawn_move(move, board.Occ)) {
                auto b2 = board.copy();
                (wtm ? make_move<true> : make_move<false>)(b2, move);
                const int v = -probe_wdl(not wtm, b2);
                if (v == wdl) return (v == 2) ? 1 : 101;
            }
            moves.pop();
        }
    }

    TbId tbid;
    const bool mirrored = tbid_from_board(board, tbid);
    int dtz = dtz_tables.at(tbid).probe(wtm, mirrored, board, wdl);
    if (dtz) return dtz_before_zeroing(wdl) + ((wdl > 0) ? dtz : -dtz);  // Successful probe

    if (wdl > 0) { // Take minimum dtz of all legal to find shortest conversion
       int best = 0xffff;

        const auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
        auto moves = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);

        while (not moves.empty()) {
            const Move move = moves.top();
            if (not is_irreversible(board, move)) {
                auto b2 = board.copy();
                (wtm ? make_move<true> : make_move<false>)(b2, move);
                const int v = -probe_dtz(not wtm, b2);

                if (v == 1) {
                    // Need to check for checkmate
                    const auto cnp2 = (wtm ? checks_and_pins<false> : checks_and_pins<true>)(b2);
                    if (cnp2.CheckMask != FULL_BOARD) { // Check
                        auto moves2 = (wtm ? generate_moves<false> : generate_moves<true>)(b2, cnp2, 0, 0, 0);
                        if (moves2.empty()) return 1;  // Checkmate
                    }
                }

                if (v > 0) best = std::min(v + 1, best);
            }
            moves.pop();
        }

        return best;
    }

    // Take maximum dtz of all legal to stall conversion as long as possible
    int best = (wdl == -2) ? -1 : -101;

    const auto cnp = (wtm ? checks_and_pins<true> : checks_and_pins<false>)(board);
    auto moves = (wtm ? generate_moves<true> : generate_moves<false>)(board, cnp, 0, 0, 0);

    while (not moves.empty()) {
        const Move move = moves.top();

        if (not is_irreversible(board, move)) {
            auto b2 = board.copy();
            (wtm ? make_move<true> : make_move<false>)(b2, move);
            best = std::min(best, -probe_dtz(not wtm, b2) - 1);
        }
        moves.pop();
    }
    moves.pop();

    return best;
}
