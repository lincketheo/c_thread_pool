#pragma once

#include <stdint.h>
#include <time.h>

typedef struct {
  void (*func)(void*);
  void* context;
} closure;

// Executes closure.
// Closure construction is responsible for providing
// address space for input / output inside closure.context
void closure_execute(closure* cl);

closure* timeit_closure_factory(
    void* func,
    void* data,
    int verbose) __attribute__((malloc));

void free_timeit_closure(closure* c);

// Hides the details of casting timer data details
clock_t timeit_closure_execute(closure* c);
