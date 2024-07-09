#include "closure.h"
#include "logging.h"
#include "matrix.h"

#include <stdio.h>

int main()
{
  closure* matmul_closure = NULL;
  closure* timer_closure = NULL;

  int ret = 0;

  matrix a;
  matrix b;
  matrix c;

  if (random_matrix(10, 20, &a, 0, 10)) {
    ret = 1;
    goto theend;
  }
  if (random_matrix(20, 30, &b, 0, 10)) {
    ret = 1;
    goto theend;
  }
  if (zero_matrix(10, 30, &c)) {
    ret = 1;
    goto theend;
  }
  if ((matmul_closure = matmul_closure_factory(&a, &b, &c)) == NULL) {
    ret = 1;
    goto theend;
  }
  if ((timer_closure = timeit_closure_factory(matmul_closure, 1)) == NULL) {
    ret = 1;
    goto theend;
  }

  clock_t duration = timeit_closure_execute(timer_closure);
  double seconds = (double)duration / (double)CLOCKS_PER_SEC;

  log_infoln("Elapsed time: %zu clocks or %f seconds", duration, seconds);

theend:
  free_timeit_closure(timer_closure);
  free_matmul_closure(matmul_closure);
  free_matrix(&a);
  free_matrix(&b);
  free_matrix(&c);

  return ret;
}
