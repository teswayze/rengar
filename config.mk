#### PROJECT SETTINGS ####
# Compiler used
CXX ?= g++
# Extension of source files used in the project
SRC_EXT = cpp
# Path to the source directory, relative to the makefile
SRC_PATH = src
# Info about the mains available
MAINS_PATH = mains
RELEASE_MAIN = uci
TEST_MAIN = unittest
SELFPLAY_MAIN = selfplay
# Opening books
BOOK_DIR = openings
BOOK_EXT = book
SELFPLAY_BOOK = A
# Where to put the results of selfplay
SELFPLAY_ROOT = selfplay_results
SELFPLAY_NODES = 1000000
SELFPLAY_EXT = game
# Space-separated pkg-config libraries used by this project
LIBS =
# General compiler flags
COMPILE_FLAGS = -std=c++17 -Wall -Wextra -g -mbmi -mbmi2 -O3
# Additional release-specific flags
RCOMPILE_FLAGS = -D NDEBUG -DDOCTEST_CONFIG_DISABLE
# Additional debug-specific flags
DCOMPILE_FLAGS = -D DEBUG
# Add additional include paths
INCLUDES = -I includes
# General linker settings
LINK_FLAGS =
# Additional release-specific linker settings
RLINK_FLAGS =
# Additional debug-specific linker settings
DLINK_FLAGS =
# Destination directory, like a jail or mounted system
DESTDIR = /
# Install path (bin/ is appended automatically)
INSTALL_PREFIX = usr/local
