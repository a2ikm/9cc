#include "test.h"

int main() {
  int int42 = 42;
  ASSERT(42, int42);
  ASSERT(4, sizeof(int42));

  short short42 = 42;
  ASSERT(42, short42);
  ASSERT(2, sizeof(short42));

  char char42 = 42;
  ASSERT(42, char42);
  ASSERT(1, sizeof(char42));

  return 0;
}
