#include "async_await.h"
#include "threadpool.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define t_diff(start, end) \
  ((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9)

typedef struct
{
  int iters;
  u64 result;
} context;

void
expensive_calculation (void *data)
{
  context *ctx = (context *)data;
  fprintf (stdout, "%d Starting\n", ctx->iters);
  int ret = 0;
  for (int i = 0; i < ctx->iters; ++i)
    {
      ret += i;
    }
  ctx->result = ret;
  fprintf (stdout, "%d Done\n", ctx->iters);
  free (ctx);
}

int
main ()
{
  double time1, time2;

  // With Threading - keep spinning
  {
    thread_pool *w = create_thread_pool ();
    spin_thread_pool (w, get_available_threads ());

    if (w == NULL)
      {
        fprintf (stderr, "Failed to create thread pool\n");
        return -1;
      }

    async_task **tasks = malloc (1000 * sizeof (async_task *));

    // Ignores system calls of thread set up
    struct timespec start, end;
    clock_gettime (CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 1000; ++i)
      {
        context *a = malloc (sizeof *a);
        if (a == NULL)
          {
            fprintf (stderr, "Failed to allocate\n");
          }
        a->iters = 10000000 + i;
        tasks[i] = async (w, expensive_calculation, a);
        if (tasks[i] == NULL)
          {
            fprintf (stderr, "Failed to add task\n");
            return -1;
          }
      }

    for (int i = 0; i < 1000; ++i)
      {
        if (await (tasks[i]))
          {
            fprintf (stderr, "Failed to await task\n");
            return -1;
          }
      }

    stop_thread_pool (w);

    clock_gettime (CLOCK_MONOTONIC, &end);
    time1 = t_diff (start, end);

    free (tasks);
    free_thread_pool (w);
  }

  // Without Threading
  {

    struct timespec start, end;
    clock_gettime (CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 1000; ++i)
      {
        context *a = malloc (sizeof *a);
        if (a == NULL)
          {
            fprintf (stderr, "Failed to allocate\n");
          }
        a->iters = 10000000 + i;

        expensive_calculation (a);
      }

    clock_gettime (CLOCK_MONOTONIC, &end);
    time2 = t_diff (start, end);
  }

  fprintf (stdout, "Time Multi Threaded: %f seconds\n", time1);
  fprintf (stdout, "Time Single Threaded: %f seconds\n", time2);

  return 0;
}
