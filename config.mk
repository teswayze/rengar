#### PROJECT SETTINGS ####
# Compiler used
CXX ?= g++
# Extension of source files used in the project
SRC_EXT = cpp
# Path to the source directory, relative to the makefile
SRC_PATH = src
# Space-separated pkg-config libraries used by this project
LIBS =
# General compiler flags
COMPILE_FLAGS = -std=c++17 -Wall -Wextra -g -O3 -D NDEBUG
# General linker settings
LINK_FLAGS =
# For downloading Eigen
EIGEN_VERSION = 3.4.0
# For downloading doctest
DOCTEST_VERSION = v2.4.11
