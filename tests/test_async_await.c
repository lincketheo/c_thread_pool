#include "async_await.h"
#include "closure.h"
#include "thread_pool.h"
#include <CUnit/CUnit.h>

void expensive_calc(void* ret)
{
  int* r = ret;
  for (volatile int i = 0; i < 100000; ++i) { }
  *r = rand();
}

void _test_async_await(size_t num_threads)
{
  int num = 100000;
  int* ret = calloc(num, sizeof *ret);
  async_task** tasks = malloc(num * sizeof(async_task*));

  thread_pool* tp = create_thread_pool();
  CU_ASSERT_PTR_NOT_NULL_FATAL(tp);
  CU_ASSERT_FATAL(spin_thread_pool(tp, num_threads) == 0);

  for (int i = 0; i < num; ++i) {
    ret[i] = 0;
    tasks[i] = async(tp, expensive_calc, &ret[i]);
    CU_ASSERT_PTR_NOT_NULL_FATAL(tasks[i]);
  }

  for (int i = 0; i < num; ++i) {
    CU_ASSERT_FATAL(await(tasks[i]) == 0);
  }

  CU_ASSERT_FATAL(stop_thread_pool(tp) == 0);
  CU_ASSERT_FATAL(free_thread_pool(tp) == 0);
}

void wrapper(void* data)
{
  size_t* n = data;
  _test_async_await(*n);
}

void test_async_await()
{
  size_t slow = 1;
  size_t fast = 12;

  closure* tslow = timeit_closure_factory(wrapper, &slow, 0);
  closure* tfast = timeit_closure_factory(wrapper, &fast, 0);

  printf("\nAsync Test %zu Threads: %f\n", slow, timeit_closure_execute(tslow));
  printf("Async Test %zu Threads: %f\n", fast, timeit_closure_execute(tfast));
}
