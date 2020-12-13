#include "test.h"

int main() {
  ASSERT(0, 0);
  ASSERT(42, 42);
  ASSERT(21, 5+20-4);
  ASSERT(41,  12 + 34 - 5 );
  ASSERT(14, 2 + 3 * 4);
  ASSERT(3, 1 + 8 / 3);
  ASSERT(8, 2 * (1 + 3));
  ASSERT(8, - 5 + 13);
  ASSERT(1, 2 == 2);
  ASSERT(0, 1 == 2);
  ASSERT(1, 2 + 3 == 5);
  ASSERT(0, 2 != 2);
  ASSERT(1, 2 != 3);
  ASSERT(0, 2 + 3 != 5);
  ASSERT(1, 2 < 3);
  ASSERT(1, 2 <= 2);
  ASSERT(0, 3 < 2);
  ASSERT(0, 3 <= 2);
  ASSERT(1, 2 + 3 < 4 + 5);
  ASSERT(1, 3 > 2);
  ASSERT(1, 3 >= 2);
  ASSERT(0, 2 > 3);
  ASSERT(1, 2 >= 2);
  ASSERT(1, 4 + 5 > 2 + 3);
  ASSERT(4, sizeof(10));
  ASSERT(4, sizeof(10 + 20));
  return 0;
}
