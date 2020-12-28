#include "test.h"

int ret30() {
  return 30;
}

int main() {
  int a[2];
  *a = 10;
  *(a+1) = 20;
  ASSERT(10, a[0]);
  ASSERT(20, a[1]);
  ASSERT(10, *a);
  ASSERT(20, *(a+1));
  ASSERT(20, *(1+a));
  ASSERT(8, sizeof(a));

  int b[3] = { 10, 20, ret30() };
  ASSERT(10, b[0]);
  ASSERT(20, b[1]);
  ASSERT(30, b[2]);
  ASSERT(12, sizeof(b));

  int c[0] = {};
  ASSERT(0, sizeof(c));

  int d[1] = {};
  ASSERT(0, d[0]);
  ASSERT(4, sizeof(d));

  int e[3] = { 10 };
  ASSERT(10, e[0]);
  ASSERT(0, e[1]);
  ASSERT(0, e[2]);
  ASSERT(12, sizeof(e));

  int f[] = {};
  ASSERT(0, sizeof(f));

  int g[] = { 10 };
  ASSERT(0, d[0]);
  ASSERT(4, sizeof(g));

  return 0;
}
