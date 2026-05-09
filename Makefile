CC = clang
CXX = clang++
CFLAGS = -Iinclude -Ilibs/link/include -Ilibs/link/modules/asio-standalone/asio/include -Wall -O3
CXXFLAGS = -Iinclude -Ilibs/link/include -Ilibs/link/modules/asio-standalone/asio/include -Wall -O3 -std=c++17 -fPIC
LDFLAGS = -framework CoreAudio -framework AudioToolbox -lpthread

# List your source files here
# C sources
C_SRC = $(filter-out src/data_callback.c src/device.c src/encoder.c src/miniaudio.c src/miniaudio_utils.c, $(wildcard src/*.c))
# C++ sources
CXX_SRC = $(wildcard src/*.cpp)

# Object files
C_OBJ = $(C_SRC:src/%.c=build/%.o)
CXX_OBJ = $(CXX_SRC:src/%.cpp=build/%.o)
OBJ = $(C_OBJ) $(CXX_OBJ)
TARGET = bin/jr-dly-1

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: clean

run: $(TARGET)
	./$(TARGET)

########################
# Testing
########################

# 1. Collect all test source files
TEST_SRC = $(wildcard tests/*_test.c)
TEST_RUNNERS = $(TEST_SRC:.c=.out)

# 2. Logic: Collect all src files BUT exclude main() and removed files
CORE_LOGIC_SRC = $(filter-out src/main.c src/link_bridge.cpp, $(wildcard src/*.c src/*.cpp))

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
