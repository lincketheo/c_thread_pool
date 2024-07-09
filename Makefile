

BIN_DIR=.
BIN_SRC_DIR=./apps
LIB_DIR=./libs
C_FLAGS=-L$(LIB_DIR) -I$(LIB_DIR) -Wall -Werror -g

all: $(BIN_DIR)/matrix_example $(BIN_DIR)/closure_example

$(BIN_DIR)/matrix_example: $(BIN_SRC_DIR)/matrix_example.c $(LIB_DIR)/libmat.so 
	gcc -o $@ $< -lmat $(C_FLAGS)

$(BIN_DIR)/closure_example: $(BIN_SRC_DIR)/closure_example.c $(LIB_DIR)/libclosure.so $(LIB_DIR)/libmat.so
	gcc -o $@ $< -lclosure -lmat $(C_FLAGS)

$(LIB_DIR)/libthreadpool.so: $(LIB_DIR)/thread_pool.c
	gcc -o $@ -shared -fPIC $< $(C_FLAGS)

$(LIB_DIR)/libmat.so: $(LIB_DIR)/matrix.c
	gcc -o $@ -shared -fPIC $< $(C_FLAGS)

$(LIB_DIR)/libclosure.so: $(LIB_DIR)/closure.c
	gcc -o $@ -shared -fPIC $< $(C_FLAGS)

clean:
	rm -rf $(LIB_DIR)/*.so
	rm -rf $(LIB_DIR)/*.a
	rm -rf $(LIB_DIR)/*.o
	rm -f matrix_example
	rm -f closure_example 
	rm -rf $(LIB_DIR)/*.dSYM
	rm -rf *.dSYM

