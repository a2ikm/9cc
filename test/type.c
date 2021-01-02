#include "test.h"

int main() {
  ASSERT(8, sizeof(long));
  ASSERT(4, sizeof(int));
  ASSERT(2, sizeof(short));
  ASSERT(1, sizeof(char));

  long long42 = 42;
  ASSERT(42, long42);
  ASSERT(8, sizeof(long42));

  int int42 = 42;
  ASSERT(42, int42);
  ASSERT(4, sizeof(int42));

  short short42 = 42;
  ASSERT(42, short42);
  ASSERT(2, sizeof(short42));

  char char42 = 42;
  ASSERT(42, char42);
  ASSERT(1, sizeof(char42));

  //
  // long * int
  //

  ASSERT(sizeof(long42), sizeof(long42 + int42));
  ASSERT(sizeof(long42), sizeof(int42 + long42));

  ASSERT(sizeof(long42), sizeof(long42 * int42));
  ASSERT(sizeof(long42), sizeof(int42 * long42));

  ASSERT(sizeof(long42), sizeof(long42 - int42));
  ASSERT(sizeof(long42), sizeof(int42 - long42));


  //
  // int * short
  //

  ASSERT(sizeof(int42), sizeof(int42 + short42));
  ASSERT(sizeof(int42), sizeof(short42 + int42));

  ASSERT(sizeof(int42), sizeof(int42 * short42));
  ASSERT(sizeof(int42), sizeof(short42 * int42));

  ASSERT(sizeof(int42), sizeof(int42 - short42));
  ASSERT(sizeof(int42), sizeof(short42 - int42));

  //
  // short * char
  //

  ASSERT(sizeof(short42), sizeof(short42 + char42));
  ASSERT(sizeof(short42), sizeof(char42 + short42));

  ASSERT(sizeof(short42), sizeof(short42 * char42));
  ASSERT(sizeof(short42), sizeof(char42 * short42));

  ASSERT(sizeof(short42), sizeof(short42 - char42));
  ASSERT(sizeof(short42), sizeof(char42 - short42));

  return 0;
}
