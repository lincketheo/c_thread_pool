#include "thread_pool.h"
#include "matrix.h"

#include <stdio.h>

int main() {
  matrix a;
  matrix b;
  matrix c;
  int ret = 0;

  if(random_matrix(2, 3, &a, 0, 10)) {
    ret = 1;
    goto theend;
  }
  if(random_matrix(3, 4, &b, 0, 10)) {
    ret = 1;
    goto theend;
  }
  if(zero_matrix(2, 4, & c)) {
    ret = 1;
    goto theend;
  }
  if(matmul(a, b, &c)) {
    ret = 1;
    goto theend;
  }

  printf("Matrix a:\n");
  print_matrix(&a);
  printf("Matrix b:\n");
  print_matrix(&b);
  printf("Matrix a x b:\n");
  print_matrix(&c);

theend:
  free_matrix(&a);
  free_matrix(&b);
  free_matrix(&c);
  return ret;
}
