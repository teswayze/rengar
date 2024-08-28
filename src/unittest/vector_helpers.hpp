# pragma once

# include "../external/doctest.h"
# include "../external/Eigen/Core"

template <size_t n>
inline void check_equal(const Eigen::Vector<float, n> left, const Eigen::Vector<float, n> right){
   for (size_t i = 0; i < n; i++) CHECK(std::abs(left[i] - right[i]) < 1e-5);
}

template <size_t n>
inline void check_opposite(const Eigen::Vector<float, n> left, const Eigen::Vector<float, n> right){
   for (size_t i = 0; i < n; i++) CHECK(std::abs(left[i] + right[i]) < 1e-5);
}
