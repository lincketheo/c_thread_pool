#pragma once

#include <stdlib.h>

typedef struct
{
  size_t rows;
  size_t cols;
  double **data;
} matrix;

int random_matrix (
    size_t rows,
    size_t cols,
    matrix *dest,
    double min,
    double max);

int zero_matrix (size_t rows, size_t cols, matrix *dest);

void print_matrix (matrix *m);

void free_matrix (matrix *m);

int matmul (const matrix *a, const matrix *b, matrix *dest);
