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

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 14 "2 + 3 * 4;"
try 3 "1 + 8 / 3;"
try 8 "2 * (1 + 3);"
try 8 "- 5 + 13;"
try 1 "2 == 2;"
try 0 "1 == 2;"
try 1 "2 + 3 == 5;"
try 0 "2 != 2;"
try 1 "2 != 3;"
try 0 "2 + 3 != 5;"
try 1 "2 < 3;"
try 1 "2 <= 2;"
try 0 "3 < 2;"
try 0 "3 <= 2;"
try 1 "2 + 3 < 4 + 5;"
try 1 "3 > 2;"
try 1 "3 >= 2;"
try 0 "2 > 3;"
try 1 "2 >= 2;"
try 1 "4 + 5 > 2 + 3;"
try 1 "0; 1;"
try 8 "a = 3; a + 5;"
try 3 "a = 5; z = -8; -z - a;"
try 8 "foo = 3; foo + 5;"
try 20 "foo = 12; bar = 8; foo + bar;"
try 20 "10; return 20; 30;"
try 10 "if (2 > 1) return 10; 20;"
try 20 "if (2 < 1) return 10; 20;"
try 10 "if (2 > 1) return 10; else return 20;"
try 20 "if (2 < 1) return 10; else return 20;"

echo OK
