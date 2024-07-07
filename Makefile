

C_FLAGS=-L. -I. -Wall -Werror -g

all: main_1

main_1: main_1.c libthreadpool.so libmat.so
	gcc -o $@ $< -lthreadpool -lmat $(C_FLAGS)

libthreadpool.so: thread_pool.c
	gcc -o $@ -shared -fPIC $< $(C_FLAGS)

libmat.so: matrix.c
	gcc -o $@ -shared -fPIC $< $(C_FLAGS)

clean:
	rm -f main main_1 *.so *.a *.o
	rm -rf *.dSYM

