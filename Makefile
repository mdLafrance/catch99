BUILD_SYSTEM = Ninja
BUILD_TYPE = debug
BUILD_DIR = build
TEST_TARGET = build/test

# Default rule: build, compile, and run
all: build compile test

# Rule to generate build system artifacts in the ./build directory
build:
	@echo "Generating build system artifacts in $(BUILD_DIR) directory"
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G "$(BUILD_SYSTEM)" -S . -B $(BUILD_DIR)

# Rule to clean up build artifacts
clean:
	@echo "Removing all build artifacts."
	rm -rf $(BUILD_DIR)

# Rule to compile the build artifacts
compile:
	@echo "Compiling build artifacts with $(BUILD_SYSTEM)."
	ninja -C $(BUILD_DIR)

# Rule to run the generated binary
test:
	@echo "Running the generated binary."
	$(TEST_TARGET)

.PHONY: all build clean compile test

