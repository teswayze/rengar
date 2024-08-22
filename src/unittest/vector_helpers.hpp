# pragma once

# include "doctest.h"
# include "../Eigen/Core"

template <size_t n>
inline void check_equal(const Eigen::Vector<float, n> left, const Eigen::Vector<float, n> right){
   for (auto i = 0; i < n; i++) CHECK(std::abs(left[i] - right[i]) < 1e-6);
}

template <size_t n>
inline void check_opposite(const Eigen::Vector<float, n> left, const Eigen::Vector<float, n> right){
   for (auto i = 0; i < n; i++) CHECK(std::abs(left[i] + right[i]) < 1e-6);
}
