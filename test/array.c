#include "test.h"

int main() {
  int a[2];
  *a = 10;
  *(a+1) = 20;
  ASSERT(10, a[0]);
  ASSERT(20, a[1]);
  ASSERT(10, *a);
  ASSERT(20, *(a+1));
  ASSERT(20, *(1+a));
  return 0;
}
