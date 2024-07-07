#include "closure.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

void closure_execute(closure* cl) {
  assert(cl);
  assert(cl->func);
  cl->func(cl->data);
}

clock_t timeit_execute(closure* cl, int verbose) {
  assert(cl);
  assert(cl->func);
  clock_t start, end;
  start = clock();
  if(verbose) {
    printf("Starting timer at clock: %zu\n", start);
  }
  closure_execute(cl);
  end = clock();
  if(verbose) {
    printf("Ending timer at clock: %zu\n", end);
    printf("Duration: %zu clocks, %f seconds\n", end - start, (double)(end - start) / (double)CLOCKS_PER_SEC);
  }
  return end - start;
}

void timeit_wrapper(void* data) {
  assert(data);
  timeit_data* tdata = data;
  assert(tdata->input);
  tdata->output = timeit_execute(tdata->input, tdata->verbose);
}

closure *timeit_closure_factory(closure* inner, int verbose) {
  assert(inner);
  assert(inner->func);

  timeit_data* data = NULL;
  closure* c = NULL;

  if((data = malloc(sizeof *data)) == NULL)
    goto FAILED;
  data->input = inner;
  data->verbose = verbose;

  if((c = malloc(sizeof *c)) == NULL)
    goto FAILED;

  c->func = timeit_wrapper;
  c->data = data;

  return c;

FAILED:
  if(data)
    free(data);
  if(c)
    free(c);
  return NULL;
}

void timeit_closure_free(closure* c) {
  if(c) {
    if(c->data) {
      timeit_data* data = c->data;
      if(data) {
        free(data);
        c->data = NULL;
      }
    }
    free(c);
  }
}

clock_t timeit_closure_execute(closure* c) {
  assert(c);
  closure_execute(c);
  timeit_data *d = c->data;
  assert(d);
  assert(d->input);
  return d->output;
}

void example_function_main(uint64_t input) {
  printf("Starting example closure with %" PRIu64 " iterations\n", input);
  for (volatile long i = 0; i < input; ++i); // Simulate some work
  printf("Ending example closure\n");
}

void example_closure_wrapper(void* data) {
  assert(data);
  example_closure_data* edata = data;
  example_function_main(edata->iterations);
}

closure* example_closure_factory(uint64_t iterations) {
  example_closure_data* data = NULL;
  closure* c = NULL;

  if((data = malloc(sizeof *data)) == NULL)
    goto FAILED;
  data->iterations = iterations;

  if((c = malloc(sizeof *c)) == NULL)
    goto FAILED;

  c->func = example_closure_wrapper;
  c->data = data;

  return c;

FAILED:
  if(data)
    free(data);
  if(c)
    free(c);
  return NULL;
}

