#include "test.h"

int g1;

int add_g1(int x) {
  g1 = g1 + x;
  return 0;
}

int main() {
  int a;
  a = 1;

  int z;
  z = -2;

  ASSERT(1, a);
  ASSERT(5, a + 4);
  ASSERT(2, -z);
  ASSERT(1, - z - a);
  ASSERT(4, sizeof(a));
  ASSERT(4, sizeof(a + z));

  int foo;
  foo = 1;

  int bar;
  bar = 2;

  ASSERT(3, foo + bar);

  g1 = 10;
  ASSERT(10, g1);

  add_g1(20);
  ASSERT(30, g1);

  a = g1 + 40;
  ASSERT(70, a);

  return 0;
}