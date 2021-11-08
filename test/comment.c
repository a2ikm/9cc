// clang-format off
#include "test.h"

int main() {
  int a;
  a = /* this is block comment */ 10;
  ASSERT(10, a);

  // this is line comment
  // return 20;

  return 0;
}
