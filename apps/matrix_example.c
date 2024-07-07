#include "thread_pool.h"
#include "matrix.h"

#include <stdio.h>

int main() {
  matrix a;
  matrix b;
  matrix c;

  random_matrix(2, 3, &a, 0, 10);
  random_matrix(3, 4, &b, 0, 10);
  zero_matrix(2, 4, & c);
  matmul(a, b, &c);

  print_matrix(&a);
  print_matrix(&b);
  print_matrix(&c);

  free_matrix(&a);
  free_matrix(&b);
  free_matrix(&c);
  return 0;
}
