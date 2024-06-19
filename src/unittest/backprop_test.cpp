# include "doctest.h"
# include "vector_helpers.hpp"
# include "../trainer/backprop.hpp"

const Vector a = vector_set(216, 984, -22, -270, 504, 479, 865, 87, -882, 1002, 394, 8, -704, -194, 535, 314);
const Vector b = vector_set(-510, 897, -707, -814, 265, -295, 654, -396, -592, 1003, -390, -567, -481, 72, 388, 455);

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
const Matrix N = {
	vector_set(-104, -584, -737, -283, -287, 428, -27, -880, 541, 248, 650, 935, 494, 590, 795, 126),
	vector_set(452, -660, 808, -412, 25, 484, 282, -667, 842, -743, -551, -750, -221, -192, 827, -490),
	vector_set(598, 249, -744, 110, 331, -368, -282, -602, -929, 647, -545, -281, 1022, -94, -183, 760),
	vector_set(-564, 672, -661, -778, 30, -897, 475, 678, -758, 992, 128, -330, 594, 986, 19, 483),
	vector_set(841, -53, 641, -862, 94, -52, -157, -1, -46, -692, -48, -538, 504, 338, -186, 268),
	vector_set(771, 890, 923, 937, -998, 588, 360, -902, -627, 727, -683, 417, -966, 567, 2, -458),
	vector_set(788, 591, 766, -980, -940, 202, -78, 933, -989, -633, -260, 891, 22, -90, 979, -175),
	vector_set(199, -560, 112, 935, -406, 78, -70, -235, -254, -591, -593, 310, -481, 290, -184, 180),
	vector_set(-716, -721, -395, 351, 854, 559, -60, 233, -358, -5, -270, 1001, -214, -681, 706, 612),
	vector_set(-447, -988, 589, 279, -194, -578, -107, 1001, -920, -465, 561, -931, 78, -122, 19, -754),
	vector_set(-697, 95, 326, -19, 343, 930, 837, -241, 191, 687, 678, -563, -980, 455, -471, 412),
	vector_set(300, 900, -1006, -976, -79, 205, 432, -224, -752, 466, -756, 810, -510, -233, -921, -968),
	vector_set(-250, 876, 964, -750, 818, 369, 793, 188, 692, 594, 86, -709, 257, 595, 978, -483),
	vector_set(-1008, 613, -539, -723, -672, 706, -753, 33, -975, 387, -916, -107, 649, 914, -969, -940),
	vector_set(627, -349, -439, -424, 87, -934, -662, 832, 718, 377, 944, 271, -327, 681, 695, 593),
	vector_set(807, 26, -582, 217, -686, 64, 115, 166, 735, 962, -246, -402, 828, -273, -638, -743),
};

int vector_sum(const Vector x){
    int sum = 0;
    for (auto elem : vector_iterator(x)) sum += elem;
    return sum;
}

TEST_CASE("Learn sum through backprop"){
    Vector a_copy = a;
    auto adjuster = SGDAdjuster(&a_copy);
    const int learning_rate = 1 << 7;

    for (int epoch_no = 0; epoch_no < 100; epoch_no++){
        for (int idx = 0; idx < 16; idx++){
            const Vector input = M[idx];
            vector_dot_back_prop(input, adjuster, vector_sum(input) - vector_dot(input, a_copy), learning_rate);
        }
    }

    for (auto elem : vector_iterator(a_copy)) {
        CHECK(elem > 206);
        CHECK(elem < 306);
    }
}

TEST_CASE("Learn identity through backprop"){
    Matrix M_copy = M;
    auto adjusters = SGDAdjusterArr<16>(M_copy.data());
    const int learning_rate = 1 << 7;

    for (int epoch_no = 0; epoch_no < 200; epoch_no++){
        for (int idx = 0; idx < 16; idx++){
            const Vector input = N[idx];
            matmul_back_prop(input, adjusters, vector_sub(input, matmul(M_copy, input)), learning_rate);
        }
    }

    auto ptr = (int16_t*) M_copy.data();
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int expected = (i == j) ? 256 : 0;
            CHECK(*ptr > expected - 50);
            CHECK(*ptr < expected + 50);
            ptr++;
        }
    }
}

TEST_CASE("abs backprop"){
    Vector a_copy = a;
    auto adjuster = SGDAdjuster(&a_copy);
    const int learning_rate = 1 << 20;

    for (int epoch_no = 0; epoch_no < 100; epoch_no++){
        const Vector backprop_grad = vector_abs_back_prop(a_copy, vector_sub(vector_abs(b), vector_abs(a_copy)));
        adjuster.update(backprop_grad, learning_rate);
    }

    check_equal(vector_abs(a_copy), vector_abs(b));
}

TEST_CASE("clamp backprop"){
    Vector a_copy = a;
    auto adjuster = SGDAdjuster(&a_copy);
    const int learning_rate = 1 << 20;
    const Vector doubler = _mm256_set1_epi16(512);

    for (int epoch_no = 0; epoch_no < 100; epoch_no++){
        const Vector backprop_grad = vector_clamp_back_prop(
            vector_mul(a_copy, doubler), 
            vector_sub(vector_clamp(vector_mul(b, doubler)), vector_clamp(vector_mul(a_copy, doubler)))
            );
        adjuster.update(backprop_grad, learning_rate);
    }

    auto a_ptr = (int16_t*) &a;
    auto param_ptr = (int16_t*) &a_copy;
    auto expected = vector_clamp(vector_mul(b, doubler));
    auto expected_ptr = (int16_t*) &expected;
    auto result = vector_clamp(vector_mul(a_copy, doubler));
    auto result_ptr = (int16_t*) &result;
    for (int i = 0; i < 16; i++) {
        if (std::abs(a_ptr[i]) < 512) CHECK(expected_ptr[i] == result_ptr[i]);
        else CHECK(a_ptr[i] == param_ptr[i]);
    }
}

TEST_CASE("mul backprop"){
    Vector a_copy = a;
    auto adjuster = SGDAdjuster(&a_copy);
    const int learning_rate = 1 << 12;

    for (int epoch_no = 0; epoch_no < 100; epoch_no++){
        for (int idx = 0; idx < 16; idx++){
            const Vector backprop_grad = std::get<0>(vector_mul_back_prop(a_copy, M[idx], 
                vector_sub(vector_mul(b, M[idx]), vector_mul(a_copy, M[idx]))));
            adjuster.update(backprop_grad, learning_rate);
        }
    }

    check_equal(a_copy, b);
}
# include "../parse_format.hpp"
TEST_CASE("dot backprop"){
    Vector a_copy = a;
    Matrix M_copy = M;
    auto unused_mat_adjusters = SGDAdjusterArr<16>(M_copy.data());
    auto adjuster = SGDAdjuster(&a_copy);
    const int learning_rate = 1 << 12;

    for (int epoch_no = 0; epoch_no < 100; epoch_no++){
        for (int idx = 0; idx < 16; idx++){
            const Vector backprop_grad = vector_dot_back_prop(a_copy, unused_mat_adjusters.data[idx],
                ((int16_t*) &b)[idx] - vector_dot(a_copy, M[idx]), 0);
            adjuster.update(backprop_grad, learning_rate);
        }
        if (epoch_no % 10 == 9) print_vector(a_copy);
    }

    check_equal(matmul(M, a_copy), b);
}