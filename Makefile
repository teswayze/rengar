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

# Combine compiler and linker flags
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)
test: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
test: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)
selfplay-build: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
selfplay-build: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)

# Build and output paths
release: export BUILD_PATH := build/release
release: export BIN_PATH := bin/release
test: export BUILD_PATH := build/debug
test: export BIN_PATH := bin
install: export BIN_PATH := bin/release
selfplay-build: export BUILD_PATH := build/release
selfplay-build: export BIN_PATH := bin/release

# Which main am I building?
release: export MAIN_NAME = $(RELEASE_MAIN)
test: export MAIN_NAME = $(TEST_MAIN)
selfplay-build: export MAIN_NAME = $(SELFPLAY_MAIN)

# Find all source files in the source directory, sorted by most
# recently modified
SOURCES_EX_MAIN = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
SOURCES = $(SOURCES_EX_MAIN) $(MAINS_PATH)/$(MAIN_NAME).$(SRC_EXT)

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

# Output files for self play
SELFPLAY_OPENINGS = $(shell awk 'NF>1{print $NF}' $(BOOK_DIR)/$(SELFPLAY_BOOK).$(BOOK_EXT))
SELFPLAY_OUTPUTS = $(SELFPLAY_OPENINGS:%=$(SELFPLAY_DIR)/%.$(SELFPLAY_EXT))

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
	@./$(MAIN_NAME)

.PHONY: selfplay-build
selfplay-build: dirs
	@echo "Beginning selfplay build"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

.PHONY: selfplay
selfplay: $(SELFPLAY_OUTPUTS)

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
	@echo "Deleting $(SELFPLAY_MAIN) symlink"
	@$(RM) $(SELFPLAY_MAIN)
	@echo "Deleting directories"
	@$(RM) -r build
	@$(RM) -r bin

# Main rule, checks the executable and symlinks to the output
all: $(BIN_PATH)/$(MAIN_NAME)
	@echo "Making symlink: $(MAIN_NAME) -> $<"
	@$(RM) $(MAIN_NAME)
	@ln -s $(BIN_PATH)/$(MAIN_NAME) $(MAIN_NAME)

# Link the executable
$(BIN_PATH)/$(MAIN_NAME): $(OBJECTS)
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
	
# Selfplay outputs
$(SELFPLAY_DIR)/%.$(SELFPLAY_EXT): selfplay-build
	@echo "Running self play; storing output in $@"
	@$(START_TIME)
	./$(MAIN_NAME) $(SELFPLAY_BOOK) % $(SELFPLAY_NODES) | tee $@
	$echo -en "\t Game time: "
	@$(END_TIME)