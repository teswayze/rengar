include config.mk

# Generally should not need to edit below this line

# Function used to check variables. Use on the command line:
# make print-VARNAME
# Useful for debugging and adding features
print-%: ; @echo $*=$($*)

# Shell used in this makefile
# bash is used for 'echo -en'
SHELL = /bin/bash
# Clear built-in rules
.SUFFIXES:
# Programs for installation
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# Append pkg-config specific libraries if need be
ifneq ($(LIBS),)
	COMPILE_FLAGS += $(shell pkg-config --cflags $(LIBS))
	LINK_FLAGS += $(shell pkg-config --libs $(LIBS))
endif

# BMI and BMI2 instruction set
ifeq ($(no-BMI2),)
	COMPILE_FLAGS += -mbmi -mbmi2
endif

# Verbose option, to output compile and link commands
export V := false
export CMD_PREFIX := @
ifeq ($(V),true)
	CMD_PREFIX :=
endif

# Static linking
ifneq ($(static),)
	LINK_FLAGS += -static -static-libgcc -static-libstdc++
endif

# Version
ifneq ($(version),)
	COMPILE_FLAGS += -DRENGAR_VERSION="$(version)"
endif

GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)

# Combine compiler and linker flags
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)
test: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
test: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)
perft: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
perft: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)
tune_move_order: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) -DTUNE_MOVE_ORDER
tune_move_order: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)
tune_eval: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) -DTUNE_EVAL
tune_eval: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)

# Build and output paths
BUILD_PATH := build
BIN_PATH := bin/$(GIT_BRANCH)

# Which main am I building?
release: export SHORT_MAIN_NAME = uci
release: export LONG_MAIN_NAME = uci
test: export SHORT_MAIN_NAME = unittest
test: export LONG_MAIN_NAME = unittest
perft: export SHORT_MAIN_NAME = perft
perft: export LONG_MAIN_NAME = perft
tune_move_order: export SHORT_MAIN_NAME = tune
tune_move_order: export LONG_MAIN_NAME = tune_move_order
tune_eval: export SHORT_MAIN_NAME = tune
tune_eval: export LONG_MAIN_NAME = tune_eval

# Skip compiling test files except for the unit test build
test: export FILTER_OUT_TESTS =
release: export FILTER_OUT_TESTS = | grep -v _test.$(SRC_EXT)
perft: export FILTER_OUT_TESTS = | grep -v _test.$(SRC_EXT)
tune_move_order: export FILTER_OUT_TESTS = | grep -v _test.$(SRC_EXT)
tune_eval: export FILTER_OUT_TESTS = | grep -v _test.$(SRC_EXT)

# Find all source files in the source directory, sorted by most
# recently modified
SOURCES_EX_MAIN = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' $(FILTER_OUT_TESTS) | sort -k 1nr | cut -f2-)
SOURCES = $(SOURCES_EX_MAIN) $(MAINS_PATH)/$(SHORT_MAIN_NAME).$(SRC_EXT)

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES_EX_MAIN:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o) $(BUILD_PATH)/$(LONG_MAIN_NAME).o
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)

# Macros for timing compilation
CUR_TIME = awk 'BEGIN{srand(); print srand()}'
TIME_FILE = $(dir $@).$(notdir $@)_time
START_TIME = $(CUR_TIME) > $(TIME_FILE)
END_TIME = read st < $(TIME_FILE) ; \
	$(RM) $(TIME_FILE) ; \
	st=$$((`$(CUR_TIME)` - $$st)) ; \
	echo $$st

# Standard, optimized release build
.PHONY: release
release: dirs
	@echo "Beginning release build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Unit tests
.PHONY: test
test: dirs
	@echo "Beginning test build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@./$(LONG_MAIN_NAME)

# Test move generation for correctness
.PHONY: perft
perft: dirs
	@echo "Beginning test build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@./$(LONG_MAIN_NAME)

# Tune move order
.PHONY: tune_move_order
tune_move_order: dirs
	@echo "Beginning move order tuning build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Tune evaluation function
.PHONY: tune_eval
tune_eval: dirs
	@echo "Beginning evaluation tuning build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Create the directories used in the build
.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

# Removes all build files
.PHONY: clean
clean:
	@echo "Deleting symlinks"
	@$(RM) uci
	@$(RM) unittest
	@$(RM) perft
	@$(RM) tune_move_order
	@$(RM) tune_eval
	@echo "Deleting directories"
	@$(RM) -r build
	@$(RM) -r bin

# Main rule, checks the executable and symlinks to the output
all: $(BIN_PATH)/$(LONG_MAIN_NAME)
	@echo "Making symlink: $(LONG_MAIN_NAME) -> $<"
	@$(RM) $(LONG_MAIN_NAME)
	@ln -s $(BIN_PATH)/$(LONG_MAIN_NAME) $(LONG_MAIN_NAME)

# Link the executable
$(BIN_PATH)/$(LONG_MAIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@echo -en "\t Link time: "
	@$(END_TIME)

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
	@echo -en "\t Compile time: "
	@$(END_TIME)

$(BUILD_PATH)/$(LONG_MAIN_NAME).o: $(MAINS_PATH)/$(SHORT_MAIN_NAME).$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
	@echo -en "\t Compile time: "
	@$(END_TIME)
