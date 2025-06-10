#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "matrix.h"

#define ASSERT_matrix(m) \
  ASSERT (m);            \
  ASSERT (m->data);      \
  ASSERT (m->rows > 0);  \
  ASSERT (m->cols > 0);

int
matmul (const matrix *a, const matrix *b, matrix *dest)
{
  ASSERT (a->data);
  ASSERT (b->data);
  ASSERT (a->cols == b->rows);
  ASSERT (dest);
  ASSERT (dest->data);
  ASSERT (dest->rows == a->rows);
  ASSERT (dest->cols == b->cols);

  for (int a_row = 0; a_row < a->rows; ++a_row)
    {
      for (int b_col = 0; b_col < b->cols; ++b_col)
        {
          dest->data[a_row][b_col] = 0;
          for (int b_row = 0; b_row < b->rows; ++b_row)
            {
              dest->data[a_row][b_col]
                  += a->data[a_row][b_row] * b->data[b_row][b_col];
            }
        }
    }

  return 0;
}

#define random_double(min, max) \
  ((min) + (rand () / (double)RAND_MAX) * ((max) - (min)))

static int
memalloc_matrix (size_t rows, size_t cols, matrix *dest)
{
  ASSERT (dest);
  ASSERT (rows > 0);
  ASSERT (cols > 0);

  dest->rows = rows;
  dest->cols = cols;
  if ((dest->data = malloc (rows * sizeof (double *))) == NULL)
    {
      perror ("malloc");
      goto FAILED;
    }
  for (int i = 0; i < rows; ++i)
    {
      if ((dest->data[i] = malloc (cols * sizeof (double))) == NULL)
        {
          perror ("malloc");
          goto FAILED;
        }
    }
  return 0;

FAILED:
  free_matrix (dest);
  return 1;
}

int
random_matrix (size_t rows, size_t cols, matrix *dest, double min, double max)
{
  if (memalloc_matrix (rows, cols, dest))
    return 1;

  ASSERT_matrix (dest);
  for (int i = 0; i < rows; ++i)
    {
      for (int j = 0; j < cols; ++j)
        {
          dest->data[i][j] = random_double (min, max);
        }
    }
  return 0;
}

int
zero_matrix (size_t rows, size_t cols, matrix *dest)
{
  if (memalloc_matrix (rows, cols, dest))
    return 1;

  ASSERT_matrix (dest);
  for (int i = 0; i < rows; ++i)
    {
      for (int j = 0; j < cols; ++j)
        {
          dest->data[i][j] = 0;
        }
    }
  return 0;
}

void
print_matrix (matrix *m)
{
  ASSERT_matrix (m);
  for (int i = 0; i < m->rows; ++i)
    {
      for (int j = 0; j < m->cols - 1; ++j)
        {
          printf ("%f, ", m->data[i][j]);
        }
      printf ("%f\n", m->data[i][m->cols - 1]);
    }
}

void
free_matrix (matrix *m)
{
  if (m)
    {
      if (m->data)
        {
          for (int i = 0; i < m->rows; ++i)
            {
              if (m->data[i])
                {
                  free (m->data[i]);
                  m->data[i] = NULL;
                }
            }
          free (m->data);
          m->data = NULL;
        }
    }
}
