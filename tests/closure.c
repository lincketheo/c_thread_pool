#include "closure.h"
#include "testing.h"

#include <stdlib.h>

typedef struct
{
  int *a;
  int *b;
  int *ret;
} add;

typedef struct
{
  int *a;
  int *b;
  int *ret;
} multiply;

static void
add_work (void *context)
{
  add *ccontext = context;
  test_fail_if_null (ccontext);
  *ccontext->ret = *ccontext->a + *ccontext->b;
}

static void
multiply_work (void *context)
{
  add *ccontext = context;
  test_fail_if_null (ccontext);
  *ccontext->ret = *ccontext->a * *ccontext->b;
}

TEST (closure)
{
  int ret1;
  int ret2;
  int a = 5;
  int b = 7;
  int c = 9;

  closure add_closure = (closure){
    .func = add_work,
    .context = &(add){
        .a = &a,
        .b = &b,
        .ret = &ret1,
    }
  };

  closure mult_closure
      = (closure){
          .func = multiply_work,
          .context = &(multiply){
              .a = ((add *)(add_closure.context))->ret,
              .b = &c,
              .ret = &ret2,
          }
        };

  closure_execute (&add_closure);
  closure_execute (&mult_closure);

  test_assert_equal (ret1, (5 + 7));
  test_assert_equal (ret2, (5 + 7) * 9);
}
