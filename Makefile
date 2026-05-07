CC = clang
CFLAGS = -Iinclude -Wall -O3
LDFLAGS = -framework CoreAudio -framework AudioToolbox  # Common Mac audio frameworks

# List your source files here
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = bin/jr-dly-1

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: clean

run: $(TARGET)
	./$(TARGET) output.wav

########################
# Testing
########################

# 1. Collect all test source files
TEST_SRC = $(wildcard tests/*_test.c)
TEST_RUNNERS = $(TEST_SRC:.c=.out)

# 2. Logic: Collect all src files BUT exclude the one with main()
# This allows the test's main() to be the only one in the executable.
CORE_LOGIC_SRC = $(filter-out src/main.c, $(wildcard src/*.c))

# 3. Helpers: Collect any .c file in tests/ that ISN'T a test runner
TEST_HELPERS = $(filter-out tests/%_test.c, $(wildcard tests/*.c))

# 4. Main test rule
test: $(TEST_RUNNERS)
	@for test in $(TEST_RUNNERS); do ./$$test; done

# 5. Generic Pattern Rule
# This builds any "name_test.c" by linking it with all logic and helpers
tests/%_test.out: tests/%_test.c $(CORE_LOGIC_SRC) $(TEST_HELPERS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Clean up test binaries too
clean-tests:
	rm -f tests/*.out

.PHONY: test clean-tests
