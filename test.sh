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

try 0 "main() { return 0; }"
try 42 "main() { return 42; }"
try 21 "main() { return 5+20-4; }"
try 41 "main() { return  12 + 34 - 5 ; }"
try 14 "main() { return 2 + 3 * 4; }"
try 3 "main() { return 1 + 8 / 3; }"
try 8 "main() { return 2 * (1 + 3); }"
try 8 "main() { return - 5 + 13; }"
try 1 "main() { return 2 == 2; }"
try 0 "main() { return 1 == 2; }"
try 1 "main() { return 2 + 3 == 5; }"
try 0 "main() { return 2 != 2; }"
try 1 "main() { return 2 != 3; }"
try 0 "main() { return 2 + 3 != 5; }"
try 1 "main() { return 2 < 3; }"
try 1 "main() { return 2 <= 2; }"
try 0 "main() { return 3 < 2; }"
try 0 "main() { return 3 <= 2; }"
try 1 "main() { return 2 + 3 < 4 + 5; }"
try 1 "main() { return 3 > 2; }"
try 1 "main() { return 3 >= 2; }"
try 0 "main() { return 2 > 3; }"
try 1 "main() { return 2 >= 2; }"
try 1 "main() { return 4 + 5 > 2 + 3; }"
try 0 "main() { return 0; return 1; }"
try 8 "main() { a = 3; return a + 5; }"
try 3 "main() { a = 5; z = -8; return -z - a; }"
try 8 "main() { foo = 3; return foo + 5; }"
try 20 "main() { foo = 12; bar = 8; return foo + bar; }"
try 20 "main() { 10; return 20; return 30; }"
try 10 "main() { if (2 > 1) return 10; return 20; }"
try 20 "main() { if (2 < 1) return 10; return 20; }"
try 10 "main() { if (2 > 1) return 10; else return 20; }"
try 20 "main() { if (2 < 1) return 10; else return 20; }"
try 5 "main() { a = 0; while (a < 5) a = a + 1; return a; }"
try 10 "main() { a = 0; for (k = 0; k < 5; k = k + 1) a = a + 2; return a; }"
try 10 "main() { if (2 > 1)  { return 10; } return 20; }"
try 5 "main() { a = 0; while (a < 5) { a = a + 1; } return a; }"
try 10 "main() { a = 0; for (k = 0; k < 5; k = k + 1) { a = a + 2; } return a; }"
try 20 "main() { i = 0; a = 0; while (i < 5) { a = a + i * 2; i = i + 1; } return a; }"
try 10 "a() { return 10; } main() { b = a(); return b; }"
try 10 "a() { return 10; } main() { return a(); }"
try 3 "a() { return 1; } b() { return 2; } main() { return a() + b(); }"
try 1 "a(x) { return 1; } main() { return a(); }"

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

try_foo "main() { foo(); return 0; }"
try_foo "main() { 10; foo(); return 0; }"
try_foo "main() { a = 10; foo(); return 0; }"
try_foo "main() { foo(); a = 10; return 0; }"

echo OK
