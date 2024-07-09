#include "closure.h"
#include "logging.h"

#include <assert.h>
#include <stdlib.h>

void closure_execute(closure* cl)
{
  assert(cl);
  assert(cl->func);
  cl->func(cl->context);
}

// You don't __need__ to do this, but I think it makes the code
// more readable, that is, the actual function
// has no knowledge that it's in a closure
static clock_t timeit_execute(closure* cl, int verbose)
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
    log_infoln("Duration: %zu clocks, %f seconds",
        end - start,
        (double)(end - start) / (double)CLOCKS_PER_SEC);
  }
  return end - start;
}

typedef struct {
  closure* input;
  int verbose;
  clock_t output;
} timeit_context;

// Makes the responsibilities of this guy really small
static void timeit_wrapper(void* context)
{
  assert(context);
  timeit_context* tcontext = context;
  assert(tcontext->input);
  tcontext->output = timeit_execute(tcontext->input, tcontext->verbose);
}

closure* timeit_closure_factory(void* func, void* context, int verbose)
{
  assert(func);

  timeit_context* tcontext = NULL;
  closure* outer = NULL;
  closure* inner = NULL;

  if ((tcontext = malloc(sizeof *context)) == NULL) {
    log_errorln_errno("malloc timeit closure context");
    goto FAILED;
  }
  if ((inner = malloc(sizeof *inner)) == NULL) {
    log_errorln_errno("malloc timeit inner closure");
    goto FAILED;
  }
  if ((outer = malloc(sizeof *outer)) == NULL) {
    log_errorln_errno("malloc timeit outer closure");
    goto FAILED;
  }

  inner->func = func;
  inner->context = context;

  tcontext->input = inner;
  tcontext->verbose = verbose;

  outer->func = timeit_wrapper;
  outer->context = tcontext;

  return outer;

FAILED:
  if (tcontext)
    free(tcontext);
  if (outer)
    free(outer);
  if (inner)
    free(inner);
  log_errorln("Failed to create timeit closure");
  return NULL;
}

void free_timeit_closure(closure* c)
{
  if (c) {
    log_debugln("Free-ing timeit closure");
    if (c->context) {
      timeit_context* context = c->context;
      if (context) {
        closure* inner = context->input;
        free(inner);
        free(context);
        c->context = NULL;
      }
    }
    free(c);
  }
}

clock_t timeit_closure_execute(closure* c)
{
  closure_execute(c);
  timeit_context* tc = c->context;
  return tc->output;
}
