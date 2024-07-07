#pragma once

#include <time.h>
#include <stdint.h>

typedef struct {
  void (*func)(void*);
  void* data;
} closure;

void closure_execute(closure* cl);

// Some useful closures
// A timer closure that computes the time it takes to run the [input] closure
// This uses clock cycles, not time, so sleep() won't work
typedef struct {
  closure* input;
  int verbose;
  clock_t output;
} timeit_data;

closure *timeit_closure_factory(closure* inner, int verbose);
clock_t timeit_closure_execute(closure* c);
void timeit_closure_free(closure* c);

// An example closure that does a for loop for [iterations] iterations
typedef struct {
  uint64_t iterations;
} example_closure_data;

closure* example_closure_factory(uint64_t iterations);
void example_closure_free(closure* c);

