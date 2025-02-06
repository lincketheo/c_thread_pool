#include "closure.h"
#include "logging.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

void
closure_execute (closure *cl)
{
  assert (cl);
  assert (cl->func);
  cl->func (cl->context);
}

#define loose_time(tsp) (tsp.tv_sec + tsp.tv_nsec / 1e9)
#define t_diff(start, end)                                                    \
  ((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9)

// You don't __need__ to do this, but I think it makes the code
// more readable, that is, the actual function
// has no knowledge that it's in a closure
static double
timeit_execute (closure *cl, int verbose)
{
  assert (cl);
  assert (cl->func);
  struct timespec start, end;
  clock_gettime (CLOCK_MONOTONIC, &start);
  if (verbose)
    {
      log_infoln ("Starting timer at clock: %f", loose_time (start));
    }
  closure_execute (cl);
  clock_gettime (CLOCK_MONOTONIC, &end);
  if (verbose)
    {
      log_infoln ("Ending timer at clock: %f", loose_time (end));
      log_infoln ("Duration: %f seconds", t_diff (start, end));
    }
  return t_diff (start, end);
}

typedef struct
{
  closure *input;
  int verbose;
  double output;
} timeit_context;

// Makes the responsibilities of this guy really small
static void
timeit_wrapper (void *context)
{
  assert (context);
  timeit_context *tcontext = context;
  assert (tcontext->input);
  tcontext->output = timeit_execute (tcontext->input, tcontext->verbose);
}

closure *
timeit_closure_factory (void *func, void *context, int verbose)
{
  assert (func);

  timeit_context *tcontext = NULL;
  closure *outer = NULL;
  closure *inner = NULL;

  if ((tcontext = malloc (sizeof *tcontext)) == NULL)
    {
      log_errorln_errno ("malloc timeit closure context");
      goto FAILED;
    }
  if ((inner = malloc (sizeof *inner)) == NULL)
    {
      log_errorln_errno ("malloc timeit inner closure");
      goto FAILED;
    }
  if ((outer = malloc (sizeof *outer)) == NULL)
    {
      log_errorln_errno ("malloc timeit outer closure");
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
    free (tcontext);
  if (outer)
    free (outer);
  if (inner)
    free (inner);
  log_errorln ("Failed to create timeit closure");
  return NULL;
}

void
free_timeit_closure (closure *c)
{
  if (c)
    {
      if (c->context)
        {
          timeit_context *context = c->context;
          if (context)
            {
              closure *inner = context->input;
              if (inner)
                free (inner);
              free (context);
            }
        }
      free (c);
    }
}

double
timeit_closure_execute (closure *c)
{
  closure_execute (c);
  timeit_context *tc = c->context;
  return tc->output;
}
