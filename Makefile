CC = $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang
CFLAGS = -march=armv8-a -m32

LD = $(CC)
LDFLAGS = -m32 -shared -llog

SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
LIB_DIR = lib

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

LIB_SRC_FILES = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJ_FILES = $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRC_FILES))

TARGET = injector
LIB_TARGET = evil.so

INCLUDES = -I$(INC_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(LDFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES) main.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(LIB_TARGET): $(LIB_OBJ_FILES)
	$(LD) $(LDFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET) $(LIB_TARGET)
