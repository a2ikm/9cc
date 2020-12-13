#include "test.h"

int main() {
  int a;
  int i;

  a = 0;
  if (1) a = 1;
  ASSERT(1, a);

  a = 0;
  if (0) a = 1;
  ASSERT(0, a);

  //a = 0;
  if (1) a = 1;
  ASSERT(1, a);

  a = 0;
  if (0) a = 1; else a = 2;
  ASSERT(2, a);

  a = 0;
  if (1 > 0) a = 1; else a = 2;
  ASSERT(1, a);

  a = 0;
  if (1 < 0) a = 1; else a = 2;
  ASSERT(2, a);

  a = 0;
  while (a < 5) a = a + 1;
  ASSERT(5, a);

  a = 0;
  for (i = 0; i < 5; i = i + 1) a = a + 2;
  ASSERT(10, a);

  return 0;
}
