#include "closure.h"

#include <stdio.h>

int main() {
  closure* example_closure = example_closure_factory(10000000000);
  closure* timer_closure = timeit_closure_factory(example_closure, 1);

  clock_t duration = timeit_closure_execute(timer_closure);
  double seconds = (double)duration / (double)CLOCKS_PER_SEC;

  printf("Elapsed time: %zu clocks or %f seconds\n", duration, seconds);

  return 0;
}
