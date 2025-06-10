#include "testing.h"
#include "types.h"

int
main ()
{
  for (u64 i = 0; i < ntests; ++i)
    {
      test_func func = tests[i].test;
      func ();
    }
}
