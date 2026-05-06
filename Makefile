CC = clang
CFLAGS = -Iinclude -Wall -O3
LDFLAGS = -framework CoreAudio -framework AudioToolbox  # Common Mac audio frameworks

# List your source files here
SRC = src/main.c
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
	./$(TARGET)
