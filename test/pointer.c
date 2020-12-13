#include "test.h"

int main() {
  int *ptr2int;
  *ptr2int = 1;
  ASSERT(1, *ptr2int);

  int x;
  int *ptr2x;
  x = 10;
  ptr2x = &x;
  ASSERT(10, *ptr2x);

  int y;
  int *ptr2y;
  int **ptr2ptr2y;
  y = 20;
  ptr2y = &y;
  ptr2ptr2y = &ptr2y;
  ASSERT(20, **ptr2ptr2y);

  ASSERT(4, sizeof(y));
  ASSERT(8, sizeof(ptr2y));
  ASSERT(8, sizeof(ptr2ptr2y));

  return 0;
}
