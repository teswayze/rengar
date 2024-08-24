# pragma once

# include "../Eigen/Core"

extern const Eigen::Vector<float, 32> w_l0_tempo_va;

extern const Eigen::Vector4f w_l1_fs_bias;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_fs;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absva;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absha;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_absra;

extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_va;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_fsxva;
extern const Eigen::Matrix<float, 4, 32, Eigen::RowMajor> w_l1_haxra;

extern const Eigen::Vector4f w_l2_va;
extern const Eigen::Vector4f w_l2_fsxva;
