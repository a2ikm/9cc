#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "int main() { return 0; }"
try 42 "int main() { return 42; }"
try 21 "int main() { return 5+20-4; }"
try 41 "int main() { return  12 + 34 - 5 ; }"
try 14 "int main() { return 2 + 3 * 4; }"
try 3 "int main() { return 1 + 8 / 3; }"
try 8 "int main() { return 2 * (1 + 3); }"
try 8 "int main() { return - 5 + 13; }"
try 1 "int main() { return 2 == 2; }"
try 0 "int main() { return 1 == 2; }"
try 1 "int main() { return 2 + 3 == 5; }"
try 0 "int main() { return 2 != 2; }"
try 1 "int main() { return 2 != 3; }"
try 0 "int main() { return 2 + 3 != 5; }"
try 1 "int main() { return 2 < 3; }"
try 1 "int main() { return 2 <= 2; }"
try 0 "int main() { return 3 < 2; }"
try 0 "int main() { return 3 <= 2; }"
try 1 "int main() { return 2 + 3 < 4 + 5; }"
try 1 "int main() { return 3 > 2; }"
try 1 "int main() { return 3 >= 2; }"
try 0 "int main() { return 2 > 3; }"
try 1 "int main() { return 2 >= 2; }"
try 1 "int main() { return 4 + 5 > 2 + 3; }"
try 0 "int main() { return 0; return 1; }"
try 8 "int main() { int a; a = 3; return a + 5; }"
try 3 "int main() { int a; int z; a = 5; z = -8; return -z - a; }"
try 8 "int main() { int foo; foo = 3; return foo + 5; }"
try 20 "int main() { int foo; int bar; foo = 12; bar = 8; return foo + bar; }"
try 20 "int main() { 10; return 20; return 30; }"
try 10 "int main() { if (2 > 1) return 10; return 20; }"
try 20 "int main() { if (2 < 1) return 10; return 20; }"
try 10 "int main() { if (2 > 1) return 10; else return 20; }"
try 20 "int main() { if (2 < 1) return 10; else return 20; }"
try 5 "int main() { int a; a = 0; while (a < 5) a = a + 1; return a; }"
try 10 "int main() { int a; int k; a = 0; for (k = 0; k < 5; k = k + 1) a = a + 2; return a; }"
try 10 "int main() { if (2 > 1)  { return 10; } return 20; }"
try 5 "int main() { int a; a = 0; while (a < 5) { a = a + 1; } return a; }"
try 10 "int main() { int a; int k; a = 0; for (k = 0; k < 5; k = k + 1) { a = a + 2; } return a; }"
try 20 "int main() { int i; int a; i = 0; a = 0; while (i < 5) { a = a + i * 2; i = i + 1; } return a; }"
try 10 "int a() { return 10; } int main() { int b; b = a(); return b; }"
try 10 "int a() { return 10; } int main() { return a(); }"
try 3 "int a() { return 1; } int b() { return 2; } int main() { return a() + b(); }"
try 1 "int a(int x) { return 1; } int main() { return a(); }"
try 1 "int a(int x) { return x; } int main() { return a(1); }"
try 3 "int a(int x) { return x; } int main() { return a(1+2); }"
try 3 "int a(int x) { return x + 2; } int main() { return a(1); }"
try 8 "int fb(int x) { if (x == 0) { return 1; } else if (x == 1) { return 1; } else { return fb(x-2) + fb(x-1); } } int main() { return fb(5); }"
try 3 "int a(int x, int y) { return x + y; } int main() { return a(1, 2); }"
try 9 "int a(int x, int y, int z) { return x + y * z; } int main() { return a(1, 2, 4); }"
try 3 "int main() { int x; int *y; x = 3; y = &x; return *y; }"
try 3 "int main() { int x; int y; int *z; x = 3; y = 42; z = &y + 1; return *z; }"
try 42 "int main() { int x; int y; int *z; x = 3; y = 42; z = &x - 1; return *z; }"
try 0 "int main() { int x; int y; y = &x - &x; return y; }"
try 1 "int main() { int x; int y; int z; z = &x - &y; return z; }"
try 3 "int main() { int x; int *y; y = &x; *y = 3; return 3; }"
try 3 "int main() { int x; int *y; int **z; x = 3; y = &x; z = &y; return **z; }"

try_foo() {
  input="$1"

  gcc -c -o test/foo.o test/foo.c
  ./9cc "$input" > tmp.s
  gcc -c tmp.s
  gcc -o tmp test/foo.o tmp.o
  actual=$(./tmp)
  expected=OK
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try_foo "int foo(); int main() { foo(); return 0; }"
try_foo "int foo(); int main() { 10; foo(); return 0; }"
try_foo "int foo(); int main() { int a; a = 10; foo(); return 0; }"
try_foo "int foo(); int main() { int a; foo(); a = 10; return 0; }"

echo OK
