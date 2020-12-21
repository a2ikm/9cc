#include "test.h"

char add(char x, char y) {
  return x + y;
}

int main() {
  char c1;
  c1 = 2;
  ASSERT(2, c1);
  ASSERT(6, c1+4);
  ASSERT(1, sizeof(c1));
  ASSERT(8, sizeof(&c1));

  char c2;
  char c3;
  c2 = 10;
  c3 = 20;
  ASSERT(30, add(c2, c3));

  char s1[10];
  s1[0] = 10;
  s1[1] = 20;
  s1[2] = 30;
  ASSERT(10, s1[0]);
  ASSERT(20, s1[1]);
  ASSERT(30, s1[2]);
  ASSERT(10, sizeof(s1));

  char *s2;
  s2 = "bar";
  ASSERT(98, s2[0]);
  ASSERT(97, s2[1]);
  ASSERT(114, s2[2]);
  ASSERT(0, s2[3]);
  ASSERT(8, sizeof(s2));

  char *s3 = "baz";
  ASSERT(98, s3[0]);
  ASSERT(97, s3[1]);
  ASSERT(122, s3[2]);
  ASSERT(0, s2[3]);

  return 0;
}
