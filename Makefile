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

# Verbose option, to output compile and link commands
export V := false
export CMD_PREFIX := @
ifeq ($(V),true)
	CMD_PREFIX :=
endif

GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)

# Combine compiler and linker flags
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)
test: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
test: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)
perft: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
perft: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)
tune_move_order: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS) -DTUNE_MOVE_ORDER
tune_move_order: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)

# Build and output paths
release: export BUILD_PATH := build/release
release: export BIN_PATH := bin/$(GIT_BRANCH)
test: export BUILD_PATH := build/debug
test: export BIN_PATH := bin
perft: export BUILD_PATH := build/debug
perft: export BIN_PATH := bin
tune_move_order: export BUILD_PATH := build/tune_move_order
tune_move_order: export BIN_PATH := bin
hello-world: export BUILD_PATH := build/debug
hello-world: export BIN_PATH := bin
install: export BIN_PATH := bin/$(GIT_BRANCH)

# Which main am I building?
release: export MAIN_NAME = uci
test: export MAIN_NAME = unittest
perft: export MAIN_NAME = perft
tune_move_order: export MAIN_NAME = tune
hello-world: export MAIN_NAME = hello-world

# OS detection
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	EXEC_FILE_NAME = $(MAIN_NAME).exe
	RUN_TEST_COMMAND = $(EXEC_FILE_NAME)
	MKLINK_COMMAND = mklink /d
else
	EXEC_FILE_NAME = $(MAIN_NAME)
	RUN_TEST_COMMAND = ./$(EXEC_FILE_NAME)
	MKLINK_COMMAND = ln -s
endif

# Find all source files in the source directory, sorted by most
# recently modified
ifeq ($(MAIN_NAME),hello-world)
	SOURCES_EX_MAIN =
else
	SOURCES_EX_MAIN = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
	SOURCES = $(SOURCES_EX_MAIN) $(MAINS_PATH)/$(MAIN_NAME).$(SRC_EXT)
endif

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES_EX_MAIN:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o) $(BUILD_PATH)/$(MAIN_NAME).o
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

# Standard, non-optimized release build
.PHONY: release
release: dirs
	@echo "Beginning release build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Debug build for gdb debugging
.PHONY: test
test: dirs
	@echo "Beginning test build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@pwd
	@ls -l
	@$(RUN_TEST_COMMAND)

# Test move generation for correctness
.PHONY: perft
perft: dirs
	@echo "Beginning test build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@$(RUN_TEST_COMMAND)

# Standard, non-optimized release build
.PHONY: tune_move_order
tune_move_order: dirs
	@echo "Beginning move order tuning build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

.PHONY: hello-world
hello-world: dirs
	@echo "Beginning test build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)
	@pwd
	@ls -l
	@$(RUN_TEST_COMMAND)

# Create the directories used in the build
.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

# Installs to the set path
.PHONY: install
install:
	@echo "Installing to $(DESTDIR)$(INSTALL_PREFIX)/bin"
	@$(INSTALL_PROGRAM) $(BIN_PATH)/$(RELEASE_MAIN) $(DESTDIR)$(INSTALL_PREFIX)/bin

# Uninstalls the program
.PHONY: uninstall
uninstall:
	@echo "Removing $(DESTDIR)$(INSTALL_PREFIX)/bin/$(RELEASE_MAIN)"
	@$(RM) $(DESTDIR)$(INSTALL_PREFIX)/bin/$(RELEASE_MAIN)

# Removes all build files
.PHONY: clean
clean:
	@echo "Deleting $(RELEASE_MAIN) symlink"
	@$(RM) $(RELEASE_MAIN)
	@echo "Deleting $(TEST_MAIN) symlink"
	@$(RM) $(TEST_MAIN)
	@echo "Deleting directories"
	@$(RM) -r build
	@$(RM) -r bin

# Main rule, checks the executable and symlinks to the output
all: $(BIN_PATH)/$(EXEC_FILE_NAME)$()
	@echo "Making symlink: $(EXEC_FILE_NAME) -> $<"
	@$(RM) $(EXEC_FILE_NAME)
	@$(MKLINK_COMMAND) $(BIN_PATH)/$(EXEC_FILE_NAME) $(EXEC_FILE_NAME)

# Link the executable
$(BIN_PATH)/$(EXEC_FILE_NAME): $(OBJECTS)
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

$(BUILD_PATH)/%.o: $(MAINS_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
	@echo -en "\t Compile time: "
	@$(END_TIME)
