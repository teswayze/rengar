# include "../external/Eigen/Core"
# include "doctest.h"

TEST_CASE("Eigen matmul"){
    const Eigen::Matrix2f mat {{1, 2}, {3, 4}};
    const Eigen::Vector2f vec {5, 6};
    const auto product = mat * vec;
    CHECK(product[0] == (1 * 5) + (2 * 6));
    CHECK(product[1] == (3 * 5) + (4 * 6));
}
