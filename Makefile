# Directories
BIN_SRC := ./apps
LIB_SRC := ./libs
TEST_SRC := ./tests

# Compiler & Flags
CC := gcc
CFLAGS := -Wall -Werror -g -fPIC -I$(LIB_SRC) -L.
LDFLAGS := -shared
TEST_LIBS := -lcunit

# Targets
LIBRARY := libthreadpool.so
BINARIES := $(patsubst $(BIN_SRC)/%.c, %, $(wildcard $(BIN_SRC)/*.c))
TESTS := $(patsubst $(TEST_SRC)/%.c, %, $(wildcard $(TEST_SRC)/*.c))

.PHONY: all clean format

all: $(LIBRARY) $(BINARIES) $(TESTS)

# Compile all .c files in libs into a single shared library
$(LIBRARY): $(wildcard $(LIB_SRC)/*.c)
	$(CC) -o $@ $(LDFLAGS) $^ $(CFLAGS)

# Build each app executable and link with the shared library
%: $(BIN_SRC)/%.c $(LIBRARY)
	$(CC) -o $@ $< $(CFLAGS) -lthreadpool

# Build each test in ./tests as an executable
%: $(TEST_SRC)/%.c $(LIBRARY)
	$(CC) -o $@ $< $(CFLAGS) $(TEST_LIBS) -lthreadpool

# Format all .c and .h files recursively
format:
	find $(BIN_SRC) $(LIB_SRC) $(TEST_SRC) -name "*.c" -o -name "*.h" | xargs clang-format -i

# Clean
clean:
	rm -f $(BINARIES) $(TESTS) $(LIBRARY) *.o *.dSYM

