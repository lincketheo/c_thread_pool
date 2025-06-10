# Compiler & Flags
CC := gcc
CFLAGS := -Wall -Werror -g -I./libs -L.
LDFLAGS := -shared

# Targets
APPS := $(patsubst ./apps/%.c, %, $(wildcard ./apps/*.c))

.PHONY: all clean format

all: libthreadpool.so $(APPS)

# Compile all .c files in libs into a single shared library
libthreadpool.so: $(wildcard ./libs/*.c) $(wildcard ./tests/*.c)
	$(CC) -o $@ $(LDFLAGS) $^ $(CFLAGS)

# Build each app executable and link with the shared library
%: ./apps/%.c libthreadpool.so
	$(CC) -o $@ $< $(CFLAGS) -lthreadpool

# Format all .c and .h files recursively
format:
	find ./apps ./libs ./tests -name "*.c" -o -name "*.h" | xargs clang-format -i

# Clean
clean:
	rm -rf $(APPS) $(TESTS) libthreadpool.so *.o *.dSYM

