# pragma once

# include "doctest.h"
# include "../linalg.hpp"

inline void check_equal(const Vector left, const Vector right){
    auto left_it = vector_iterator(left).begin();
    auto right_it = vector_iterator(right).begin();
    const auto left_it_end = vector_iterator(left).end();
    int i = 0;
    while (left_it != left_it_end) {
        INFO(i);
        CHECK(*left_it == *right_it);
        left_it++;
        right_it++;
        i++;
    }
}

inline void check_opposite(const Vector left, const Vector right){
    auto left_it = vector_iterator(left).begin();
    auto right_it = vector_iterator(right).begin();
    const auto left_it_end = vector_iterator(left).end();
    int i = 0;
    while (left_it != left_it_end){
        INFO(i);
        CHECK(*left_it == -*right_it);
        left_it++;
        right_it++;
        i++;
    }
}
