CC = $NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang
CFLAGS = -march=armv8-a -m32

SRC_DIR = module
INC_DIR = include
OBJ_DIR = obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = injector

INCLUDES = -I$(INC_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJ_FILES) main.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
