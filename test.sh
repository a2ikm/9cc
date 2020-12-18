#!/bin/bash
try() {
  expected="$1"
  input="$2"

  echo "$input" | ./9cc - > tmp.s
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "int main () { int a; a = 0; if (0) a = 1; return a; }"
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
try 4 "int main() { return sizeof(1); }"
try 4 "int main() { int x; return sizeof(x); }"
try 8 "int main() { int *x; return sizeof(x); }"
try 4 "int main() { return sizeof(sizeof(1)); }"
try 2 "int main() { int a[1]; *a = 2; return *a; }"
try 20 "int main() { int a[2]; *a = 10; *(a+1) = 20; return *(a+1); }"
try 20 "int main() { int a[2]; *a = 10; *(1+a) = 20; return *(1+a); }"
try 20 "int main() { int a[3]; *a = 10; *(a+2-1) = 20; return *(a+2-1); }"
try 10 "int main() { int a[1]; a[0] = 10; return a[0]; }"
try 42 "int main() { return 42; }"
try 42 "int x; int main() { x = 42; return x; }"
try 42 "int x; int main() { int a; x = 41; a = x + 1; return a; }"
try 10 "int main() { char a; a = 2; return a + 8; }"
try 10 "int main() { char a; a = 2; int b; b = 8; return a + b; }"
try 10 "int main() { char a[10]; a[0] = 10; return a[0]; } "
try 10 "char a() { char x; x = 42; return x; } int main() { if (42 == a()) { return 10; } else { return 20; } }"
try 10 "char a(char c) { return c + 1; } int main() { char c; c = 2; if (a(c) == 3) { return 10; } else { return 20; } }"
try 10 "char a(char c) { return c + 1; } int main() { if (a(2) == 3) { return 10; } else { return 20; } }"
try 1 "int main() { char a; a = 1; return sizeof(a); }"
try 8 'int main() { char *s; s = "foo"; return sizeof(s); }'
try 0 "int main() { /* this is block comment */ return 0; }"
try 0 "int main() {
  // this is line comment
  return 0;
}"

try_print_ok() {
  input="$1"

  gcc -c -o test/print_ok.o test/print_ok.c
  echo "$input" | ./9cc - > tmp.s
  gcc -c tmp.s
  gcc -static -o tmp test/print_ok.o tmp.o
  actual=$(./tmp)
  expected=OK
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try_print_ok "int print_ok(); int main() { print_ok(); return 0; }"
try_print_ok "int print_ok(); int main() { 10; print_ok(); return 0; }"
try_print_ok "int print_ok(); int main() { int a; a = 10; print_ok(); return 0; }"
try_print_ok "int print_ok(); int main() { int a; print_ok(); a = 10; return 0; }"

try_print() {
  expected="$1"
  input="$2"

  gcc -c -o test/print.o test/print.c
  echo "$input" | ./9cc - > tmp.s
  gcc -c tmp.s
  gcc -static -o tmp test/print.o tmp.o
  actual=$(./tmp)
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try_print "hello" "int print(char *s); int main() { print(\"hello\n\"); return 0; }"
try_print "hello" "int print(char *s); int main() { char *s; s = \"hello\n\"; print(s); return 0; }"

echo OK
