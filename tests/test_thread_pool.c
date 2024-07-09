
#include "CUnit/CUnit.h"
#include "closure.h"
#include "logging.h"
#include "thread_pool.h"
#include <stdlib.h>
#include <unistd.h>

void busy_wait()
{
  for (volatile int i = 0; i < 100000; ++i) { }
}

typedef struct {
  int a;
  int b;
  int ret;
} add;

typedef struct {
  int a;
  int b;
  int ret;
} multiply;

typedef struct {
  float a;
  float b;
  float ret;
} divide;

void _add_task(void* data)
{
  add* d = data;
  busy_wait();
  d->ret = d->a + d->b;
}

void multiply_task(void* data)
{
  multiply* d = data;
  busy_wait();
  d->ret = d->a * d->b;
}

void divide_task(void* data)
{
  divide* d = data;
  busy_wait();
  d->ret = d->a / d->b;
}

typedef union {
  add a;
  multiply m;
  divide d;
} task;

#define CU_ASSERT_ADD_CLOSURE_END_STATE_CORRECT(t) \
  do {                                             \
  } while (0);

#define CU_ASSERT_MULT_CLOSURE_END_STATE_CORRECT(t) \
  CU_ASSERT(t.a* t.b == t.ret);

#define CU_ASSERT_DIV_CLOSURE_END_STATE_CORRECT(t) \
  do {                                             \
    add* _a = t.context;                           \
    CU_ASSERT_PTR_NOT_NULL_FATAL(_a);              \
    CU_ASSERT(_a->a / _a->b == _a->ret);           \
  } while (0);

void create_work_for_test(
    size_t num_tasks,
    int max_arg,
    task** args_dest,
    int** types_dest,
    work** work_dest)
{
  work* w = create_work();
  task* t = malloc(num_tasks * sizeof *t);
  int* types = malloc(num_tasks * sizeof *types);

  CU_ASSERT_PTR_NOT_NULL_FATAL(w);
  CU_ASSERT_PTR_NOT_NULL_FATAL(t);
  CU_ASSERT_PTR_NOT_NULL_FATAL(types);

  for (int i = 0; i < num_tasks; ++i) {
    int arg1 = rand() % max_arg;
    int arg2 = rand() % max_arg;
    types[i] = (int)(rand() % 3);

    switch (types[i]) {
    case 0:
      t[i].a.a = arg1;
      t[i].a.b = arg2;
      CU_ASSERT_FATAL(add_task(w, _add_task, &t[i].a) == 0);
      break;
    case 1:
      t[i].m.a = arg1;
      t[i].m.b = arg2;
      CU_ASSERT_FATAL(add_task(w, multiply_task, &t[i].m) == 0);
      break;
    case 2:
      t[i].d.a = (float)arg1;
      t[i].d.b = (float)arg2;
      CU_ASSERT_FATAL(add_task(w, divide_task, &t[i].d) == 0);
      break;
    default:
      CU_FAIL("rand() % 3 was greater than 2?");
    }
  }

  *args_dest = t;
  *work_dest = w;
  *types_dest = types;
}

void test_verify_tasks_correct(
    size_t num_tasks,
    task* args,
    int* types)
{
  for (int i = 0; i < num_tasks; ++i) {
    switch (types[i]) {
    case 0:
      CU_ASSERT(args[i].a.a + args[i].a.b == args[i].a.ret);
      break;
    case 1:
      CU_ASSERT(args[i].m.a * args[i].m.b == args[i].m.ret);
      break;
    case 2:
      CU_ASSERT(args[i].d.a / args[i].d.b == args[i].d.ret);
      break;
    default:
      CU_FAIL("rand() % 3 was greater than 2?");
      return;
    }
  }
}

typedef struct {
  work* w;
  size_t num_threads;
} context;

void execute_wrapper(void* w)
{
  context* c = w;
  thread_pool_execute(c->w, c->num_threads);
}

void timing_test(size_t num_threads)
{
  work* w_slow;
  task* tasks_slow;
  int* types_slow;

  work* w_fast;
  task* tasks_fast;
  int* types_fast;

  size_t num_tasks = 100000;
  create_work_for_test(num_tasks, 1000, &tasks_slow, &types_slow, &w_slow);
  create_work_for_test(num_tasks, 1000, &tasks_fast, &types_fast, &w_fast);

  context c_slow = (context) {
    .w = w_slow,
    .num_threads = 1,
  };
  context c_fast = (context) {
    .w = w_fast,
    .num_threads = num_threads,
  };

  closure* t_slow = timeit_closure_factory(execute_wrapper, &c_slow, 0);
  closure* t_fast = timeit_closure_factory(execute_wrapper, &c_fast, 0);

  clock_t slow_results = timeit_closure_execute(t_slow);
  clock_t fast_results = timeit_closure_execute(t_fast);
  printf("%zu %zu\n", slow_results, fast_results);
  exit(1);
  CU_ASSERT(fast_results < slow_results);

  free_timeit_closure(t_slow);
  free_timeit_closure(t_fast);
  free(tasks_slow);
  free(types_slow);
  free(tasks_fast);
  free(types_fast);

  CU_ASSERT(free_work(w_slow) == 0);
  CU_ASSERT(free_work(w_fast) == 0);
}

void validation_test(size_t num_threads)
{
  work* w;
  task* tasks;
  int* types;

  size_t num_tasks = 100;
  create_work_for_test(num_tasks, 1000, &tasks, &types, &w);
  thread_pool_execute(w, num_threads);
  test_verify_tasks_correct(num_tasks, tasks, types);

  free(tasks);
  free(types);
  free_work(w);
}

void test_thread_pool()
{
  long num_processors = sysconf(_SC_NPROCESSORS_ONLN);
  if (num_processors == -1) {
    log_infoln("Skipping thread test. Couldn't figure out how many threads exist");
    return;
  }
  if (num_processors > 2) {
    timing_test(num_processors);
  } else {
    log_infoln("Skipping timing test for thread pool. Not enough threads");
  }

  validation_test(num_processors);

  return;
}
