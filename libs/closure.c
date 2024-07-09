#include "closure.h"
#include "logging.h"
#include "matrix.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void closure_execute(closure* cl)
{
  assert(cl);
  assert(cl->func);
  cl->func(cl->data);
}

clock_t timeit_execute(closure* cl, int verbose)
{
  assert(cl);
  assert(cl->func);
  clock_t start, end;
  start = clock();
  if (verbose) {
    log_infoln("Starting timer at clock: %zu", start);
  }
  closure_execute(cl);
  end = clock();
  if (verbose) {
    log_infoln("Ending timer at clock: %zu", end);
    log_infoln("Duration: %zu clocks, %f seconds", end - start, (double)(end - start) / (double)CLOCKS_PER_SEC);
  }
  return end - start;
}

void timeit_wrapper(void* data)
{
  assert(data);
  timeit_data* tdata = data;
  assert(tdata->input);
  tdata->output = timeit_execute(tdata->input, tdata->verbose);
}

closure* timeit_closure_factory(closure* inner, int verbose)
{
  assert(inner);
  assert(inner->func);

  timeit_data* data = NULL;
  closure* c = NULL;

  if ((data = malloc(sizeof *data)) == NULL){
    log_errorln_errno("malloc timeit closure data");
    goto FAILED;
  }
  data->input = inner;
  data->verbose = verbose;

  if ((c = malloc(sizeof *c)) == NULL){
    log_errorln_errno("malloc timeit closure");
    goto FAILED;
  }

  c->func = timeit_wrapper;
  c->data = data;

  return c;

FAILED:
  if (data)
    free(data);
  if (c)
    free(c);
  log_errorln("Failed to create timeit closure");
  return NULL;
}

void free_timeit_closure(closure* c)
{
  if (c) {
    log_debugln("Free-ing timeit closure");
    if (c->data) {
      timeit_data* data = c->data;
      if (data) {
        free(data);
        c->data = NULL;
      }
    }
    free(c);
  }
}

clock_t timeit_closure_execute(closure* c)
{
  assert(c);
  closure_execute(c);
  timeit_data* d = c->data;
  assert(d);
  assert(d->input);
  return d->output;
}

void example_function_main(uint64_t input)
{
  log_infoln("Starting example closure with %" PRIu64 " iterations", input);
  for (volatile long i = 0; i < input; ++i)
    ; // Simulate some work
  log_infoln("Ending example closure");
}

// You don't __need__ to do this, but I think it makes the code
// more readable, that is, the actual function (example_function_main)
// has no knowledge that it's in a closure
void example_closure_wrapper(void* data)
{
  assert(data);
  uint64_t* iterations = data;
  assert(iterations);
  example_function_main(*iterations);
}

closure* example_closure_factory(uint64_t iterations)
{
  uint64_t* data = NULL;
  closure* c = NULL;

  if ((data = malloc(sizeof *data)) == NULL){
    log_errorln_errno("malloc example closure data");
    goto FAILED;
  }
  *data = iterations;

  if ((c = malloc(sizeof *c)) == NULL){
    log_errorln_errno("malloc example closure");
    goto FAILED;
  }

  c->func = example_closure_wrapper;
  c->data = data;

  return c;

FAILED:
  if (data)
    free(data);
  if (c)
    free(c);
  log_errorln("Failed to create example closure");
  return NULL;
}

void free_example_closure(closure* c)
{
  if (c) {
    log_debugln("Free-ing example closure");
    if (c->data) {
      uint64_t* data = c->data;
      if (data) {
        free(data);
        c->data = NULL;
      }
    }
    free(c);
  }
}

static void matrix_closure_wrapper(void* data)
{
  matmul_data* md = data;

  const matrix* left = md->left;
  const matrix* right = md->right;
  matrix* dest = md->output;
  assert(left);
  assert(right);
  assert(dest);

  md->return_code = matmul(left, right, dest);
}

closure* matmul_closure_factory(
    const matrix* left,
    const matrix* right,
    matrix* output)
{
  if (left == NULL || right == NULL || output == NULL) {
    log_errorln("All matrices must be non null");
    errno = EINVAL;
    return NULL;
  }

  closure* c = NULL;
  matmul_data* data = NULL;

  if ((c = malloc(sizeof *c)) == NULL){
    log_errorln_errno("malloc matmul closure data");
    goto FAILED;
  }
  if ((data = malloc(sizeof *data)) == NULL){
    log_errorln_errno("malloc matmul closure");
    goto FAILED;
  }

  data->left = left;
  data->right = right;
  data->output = output;
  data->return_code = 0;
  c->func = matrix_closure_wrapper;
  c->data = data;
  return c;

FAILED:
  if (c)
    free(c);
  if (data)
    free(data);
  log_errorln("Failed to create matrix closure");
  return NULL;
}

void free_matmul_closure(closure* c)
{
  if (c) {
    log_debugln("Free-ing matmul closure");
    if (c->data) {
      free(c->data);
    }
    free(c);
  }
}
