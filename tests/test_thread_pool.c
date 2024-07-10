
#include <CUnit/CUnit.h>

#include "closure.h"
#include "logging.h"
#include "thread_pool.h"
#include <stdlib.h>
#include <unistd.h>

// To simulate cpu cycles
void busy_wait()
{
  for (volatile int i = 0; i < 50000000; ++i) { }
}

// For functional and demonstration of use, define
// three contexts - add, multiply and divide
// (shows thread_pool is arbitrary, not)
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

// Creates [num_tasks] random tasks.
// In order to validate (for the unit test)
// it also stores the types of each task in [types_dest]
// (0 - add, 1 - multiply, 2 - divide) and the
// task parameters in [args_dest]
//
// max_arg is just the maximum argument passed to each task
void create_thread_pool_for_test(
    size_t num_tasks,
    int max_arg,
    task** args_dest,
    int** types_dest,
    thread_pool** thread_pool_dest)
{
  thread_pool* w = create_thread_pool();
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
      CU_ASSERT(add_task(w, _add_task, &t[i].a) == 0);
      break;
    case 1:
      t[i].m.a = arg1;
      t[i].m.b = arg2;
      CU_ASSERT(add_task(w, multiply_task, &t[i].m) == 0);
      break;
    case 2:
      t[i].d.a = (float)arg1;
      t[i].d.b = (float)arg2;
      CU_ASSERT(add_task(w, divide_task, &t[i].d) == 0);
      break;
    default:
      CU_ASSERT(0);
    }
  }

  *args_dest = t;
  *thread_pool_dest = w;
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
      CU_ASSERT(0 && "rand() % 3 was greater than 2?");
      return;
    }
  }
}

typedef struct {
  thread_pool* w;
  size_t num_threads;
} context;

void execute_wrapper(void* w)
{
  context* c = w;
  thread_pool_execute_until_done(c->w, c->num_threads);
}

void timing_test(size_t num_threads)
{
  thread_pool* w_slow;
  task* tasks_slow;
  int* types_slow;

  thread_pool* w_fast;
  task* tasks_fast;
  int* types_fast;

  size_t num_tasks = 100;

  // Assumes:
  //  - Uniform distribution of tasks
  //  - Work should take the same time given the same conditions
  create_thread_pool_for_test(num_tasks, 1000, &tasks_slow, &types_slow, &w_slow);
  create_thread_pool_for_test(num_tasks, 1000, &tasks_fast, &types_fast, &w_fast);

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

  double slow_results = timeit_closure_execute(t_slow);
  double fast_results = timeit_closure_execute(t_fast);

  printf("\nT(1 thread) = %f seconds\n", slow_results);
  printf("T(%zu threads) = %f seconds\n", num_threads, fast_results);
  CU_ASSERT(fast_results <= slow_results);

  free_timeit_closure(t_slow);
  free_timeit_closure(t_fast);
  free(tasks_slow);
  free(types_slow);
  free(tasks_fast);
  free(types_fast);

  CU_ASSERT(free_thread_pool(w_slow) == 0);
  CU_ASSERT(free_thread_pool(w_fast) == 0);
}

void validation_test(size_t num_threads)
{
  thread_pool* w;
  task* tasks;
  int* types;

  size_t num_tasks = 100;
  create_thread_pool_for_test(num_tasks, 1000, &tasks, &types, &w);
  thread_pool_execute_until_done(w, num_threads);
  test_verify_tasks_correct(num_tasks, tasks, types);

  free(tasks);
  free(types);
  free_thread_pool(w);
}

int test_thread_pool()
{
  ssize_t num_threads = get_available_threads();

  if (num_threads == -1) {
    log_infoln("Doing nothing for thread test."
               "Couldn't figure out how many threads exist on machine");
    return 0;
  }

  if (num_threads > 4) {
    timing_test(num_threads);
  } else {
    log_infoln("Doing nothing for timing test for "
               "thread pool. Not enough threads");
  }

  validation_test(num_threads);

  return 0;
}

