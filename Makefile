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
	COMPILE_FLAGS += -mbmi -mbmi2 -mavx -mavx2
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
export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS)

# Build and output paths
BUILD_PATH := build
BIN_PATH := bin/$(GIT_BRANCH)

# Which main am I building?
release: export MODULE_NAME = uci
release: export BINARY_NAME = uci
release: export MODULE_CHOICES = uci
test: export MODULE_NAME = unittest
test: export BINARY_NAME = unittest
test: export MODULE_CHOICES = {unittest,bookgen,gamefile}
perft: export MODULE_NAME = perft
perft: export BINARY_NAME = perft
perft: export MODULE_CHOICES = perft
bookgen: export MODULE_NAME = bookgen
bookgen: export BINARY_NAME = bookgen
bookgen: export MODULE_CHOICES = {bookgen,gamefile}
game_cat: export MODULE_NAME = gamefile
game_cat: export BINARY_NAME = game_cat
game_cat: export MODULE_CHOICES = gamefile
selfplay: export MODULE_NAME = selfplay
selfplay: export BINARY_NAME = selfplay
selfplay: export MODULE_CHOICES = {selfplay,gamefile}
matetest: export MODULE_NAME = matetest
matetest: export BINARY_NAME = matetest
matetest: export MODULE_CHOICES = {selfplay,gamefile}

# Find all source files in the source directory, sorted by most
# recently modified
COMMON_SOURCES = $(shell ls $(SRC_PATH)/*.$(SRC_EXT) | sort -k 1nr | cut -f2-)
NETWORK_WEIGHTS = $(shell ls $(SRC_PATH)/weights/*.$(SRC_EXT) | sort -k 1nr | cut -f2-)
MODULE_SOURCES = $(shell ls $(SRC_PATH)/$(MODULE_CHOICES)/*.$(SRC_EXT) | sort -k 1nr | cut -f2- | grep -v main.cpp)
MAIN_SOURCE = $(SRC_PATH)/$(MODULE_NAME)/main.$(SRC_EXT)
SOURCES = $(COMMON_SOURCES) $(NETWORK_WEIGHTS) $(MODULE_SOURCES) $(MAIN_SOURCE)

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
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
	@$(RM) -r .unittest_tmp
	@mkdir .unittest_tmp
	@./$(BINARY_NAME)
	@$(RM) -r .unittest_tmp

# Test move generation for correctness
.PHONY: perft
perft: dirs
	@echo "Beginning perft build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@./$(BINARY_NAME)

# Generate opening book for training
.PHONY: bookgen
bookgen: dirs
	@echo "Beginning bookgen build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Show games in .rg file
.PHONY: game_cat
game_cat: dirs
	@echo "Beginning game_cat build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Generate training data from an opening book
.PHONY: selfplay
selfplay: dirs
	@echo "Beginning selfplay build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# See how Rengar does in won pawnless endgames
.PHONY: matetest
matetest: dirs
	@echo "Beginning matetest build"
	@$(START_TIME)
	@"$(MAKE)" all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@./all_mate_tests.sh

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
	@$(RM) bookgen
	@$(RM) game_cat
	@$(RM) selfplay
	@$(RM) matetest
	@echo "Deleting directories"
	@$(RM) -r build
	@$(RM) -r bin

# Main rule, checks the executable and symlinks to the output
all: $(BIN_PATH)/$(BINARY_NAME)
	@echo "Making symlink: $(BINARY_NAME) -> $<"
	@$(RM) $(BINARY_NAME)
	@ln -s $(BIN_PATH)/$(BINARY_NAME) $(BINARY_NAME)

# Link the executable
$(BIN_PATH)/$(BINARY_NAME): $(OBJECTS)
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
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) -MP -MMD -c $< -o $@
	@echo -en "\t Compile time: "
	@$(END_TIME)

# chess324 opening books
chess324_openings/startpos_%.rg: bookgen
	@mkdir -p chess324_openings
	@./bookgen $@ 3087 6 $*

NUMBERS := $(shell seq 0 323)
_HELPER := $(addsuffix .rg,${NUMBERS})
BOOKGEN_MILLION_TARGETS := $(addprefix chess324_openings/startpos_,${_HELPER})
.PHONY: bookgen-million
bookgen-million: $(BOOKGEN_MILLION_TARGETS)
	@ls chess324_openings
