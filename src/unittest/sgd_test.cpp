# include "doctest.h"
# include "vector_helpers.hpp"
# include "../trainer/backprop.hpp"

const Vector a = vector_set(216, 984, -22, -270, 504, 479, 865, 87, -882, 1002, 394, 8, -704, -194, 535, 314);
const Vector b = vector_set(-510, 897, -707, -814, 265, -295, 654, -396, -592, 1003, -390, -567, -481, 72, 388, 455);


TEST_CASE("SGD weight adjustment by zero"){
    Vector b_copy = b;
    auto adjuster = SGDAdjuster(&b_copy);

    adjuster.update(vector_zero, 1 << 21);
    check_equal(b, b_copy);

    adjuster.update(a, 0);
    check_equal(b, b_copy);
}

TEST_CASE("SGD weight adjustment"){
    Vector b_copy = b;
    auto adjuster = SGDAdjuster(&b_copy);

    adjuster.update(a, 1 << 21);
    int16_t *a_ptr = (int16_t*) &a;
    int16_t *b_ptr = (int16_t*) &b;
    int16_t *b_copy_ptr = (int16_t*) &b_copy;
    for (int i = 0; i < 16; i++){
        CHECK(b_copy_ptr[i] == std::clamp(a_ptr[i] + b_ptr[i], -1023, 1023));
    }
}

TEST_CASE("SGD weight adjustment in two steps"){
    Vector b_copy = b;
    auto adjuster = SGDAdjuster(&b_copy);
    const Vector all_ones = _mm256_set1_epi16(1);

    adjuster.update(all_ones, 3 << 18);
    check_equal(b, b_copy);

    adjuster.update(all_ones, 3 << 18);
    check_equal(vector_add(b, all_ones), b_copy);
}

const Matrix M = {
    vector_set(918, 562, 314, -508, -59, -231, -526, 44, 589, 27, 821, 337, -402, -140, 298, 467),
    vector_set(-1008, 65, -827, -998, 368, 739, 472, 904, 283, 666, 617, 23, 778, 708, -24, 127),
    vector_set(280, 382, 357, -868, -90, -710, -428, 167, -375, 289, 419, 779, 280, 656, -272, 88),
    vector_set(794, 968, -780, 349, -437, 191, 303, -95, 729, 152, -328, 322, 370, 846, 772, -127),
    vector_set(1017, 254, -562, 775, -459, -554, 455, 515, 589, 988, -874, -308, -760, -946, -311, 76),
    vector_set(333, 44, -491, 543, 549, -152, 776, 814, -59, -454, 753, 509, -813, -927, -754, 11),
    vector_set(-744, 893, -174, 904, -134, -871, 715, 477, 661, -607, 141, 194, -417, 573, -534, -871),
    vector_set(339, -140, -158, -893, 493, 733, -408, 867, -757, 695, 865, -534, 851, -553, 199, -317),
    vector_set(-696, 1000, -847, 567, 16, -122, 402, -853, -1003, -393, 945, -745, -538, 910, 1010, -397),
    vector_set(-414, 694, 853, 11, 97, -840, -706, 569, 730, -618, 232, 97, -518, 980, 26, 498),
    vector_set(-113, -882, 213, 629, -212, 647, 718, -327, 956, -112, 537, -4, 138, -573, -824, 822),
    vector_set(326, -603, -154, -263, -1002, 297, -946, -804, -334, -654, 304, 462, 425, 463, 558, 856),
    vector_set(-367, 925, 235, 558, -572, -139, 323, -49, 303, -639, -610, -960, -180, 548, -74, -546),
    vector_set(-478, -381, 469, 596, 49, 823, 289, -770, -355, -968, -301, 288, 29, -801, 331, 137),
    vector_set(-914, 612, 33, 521, 597, -50, -461, -432, 170, -191, 621, 212, -999, 114, -21, -235),
    vector_set(-979, 272, 671, -548, -113, 500, 788, 252, -357, -788, -937, -632, -284, 54, 432, -145),
};

TEST_CASE("SGD weight array adjustment"){
    Matrix M_copy = M;
    auto adjusters = SGDAdjusterArr<16>(M_copy.data());

    for (int j = 0; j < 16; j++){
        adjusters.data[j].update(a, 1 << 21);
        int16_t *a_ptr = (int16_t*) &a;
        int16_t *Mj_ptr = (int16_t*) (M.data() + j);
        int16_t *Mj_copy_ptr = (int16_t*) (M_copy.data() + j);
        for (int i = 0; i < 16; i++){
            CHECK(Mj_copy_ptr[i] == std::clamp(a_ptr[i] + Mj_ptr[i], -1023, 1023));
        }
    }
}
