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
