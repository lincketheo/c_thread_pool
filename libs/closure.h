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
void free_timeit_closure(closure* c);

// An example closure that does a for loop for [iterations] iterations
closure* example_closure_factory(uint64_t iterations);
void free_example_closure(closure* c);

