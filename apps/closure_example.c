#include "closure.h"

#include <stdio.h>

int main() {
  closure* example_closure = NULL;
  closure* timer_closure = NULL;
  int ret = 0;

  if((example_closure = example_closure_factory(10000000000)) == NULL) {
    ret = 1;
    goto theend;
  }
  if((timer_closure = timeit_closure_factory(example_closure, 1)) == NULL) {
    ret = 1;
    goto theend;
  }

  clock_t duration = timeit_closure_execute(timer_closure);
  double seconds = (double)duration / (double)CLOCKS_PER_SEC;

  printf("Elapsed time: %zu clocks or %f seconds\n", duration, seconds);

theend:
  free_example_closure(example_closure);
  free_timeit_closure(timer_closure);

  return ret;
}
