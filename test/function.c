#include "test.h"

int ret10() {
  return 10;
  return 20;
}

int ret_given(int x) {
  return x;
}

int add(int x, int y) {
  return x + y;
}

int fib(int x) {
  if (x == 0) {
    return 1;
  } else if (x == 1) {
    return 1;
  } else {
    return fib(x-2) + fib(x-1);
  }
}

int main() {
  ASSERT(10, ret10());

  int a;
  a = ret10();
  ASSERT(10, a);

  ASSERT(10, ret_given(10));
  ASSERT(10, ret_given(4 + 6));
  ASSERT(10, add(4, 6));
  ASSERT(8, fib(5));

  return 0;
}
