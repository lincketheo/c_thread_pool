#include "async_await.h"
#include "closure.h"
#include "testing.h"
#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>

void
expensive_calc (void *ret)
{
  int *r = ret;
  for (volatile int i = 0; i < 100000; ++i)
    {
    }
  *r = rand ();
}

void
_test_async_await (int num_threads)
{
  int num = 100000;
  int *ret = calloc (num, sizeof *ret);
  async_task **tasks = malloc (num * sizeof (async_task *));

  thread_pool *tp = create_thread_pool ();

  test_fail_if_null (tp);

  test_assert_equal (spin_thread_pool (tp, num_threads), 0);

  for (int i = 0; i < num; ++i)
    {
      ret[i] = 0;
      tasks[i] = async (tp, expensive_calc, &ret[i]);
      test_fail_if_null (tasks[i]);
    }

  for (int i = 0; i < num; ++i)
    {
      test_assert_equal (await (tasks[i]), 0);
    }

  test_assert_equal (stop_thread_pool (tp), 0);
  test_assert_equal (free_thread_pool (tp), 0);
  free (ret);
  free (tasks);
}

void
wrapper (void *data)
{
  size_t *n = data;
  _test_async_await (*n);
}

TEST (async_await)
{
  size_t slow = 1;
  size_t fast = 12;

  closure *tslow = timeit_closure_factory (wrapper, &slow, 0);
  closure *tfast = timeit_closure_factory (wrapper, &fast, 0);

  printf ("\nAsync Test %zu Threads: %f\n", slow,
          timeit_closure_execute (tslow));
  printf ("Async Test %zu Threads: %f\n", fast,
          timeit_closure_execute (tfast));
}
