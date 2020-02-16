#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  tokenize();
  parse();

  printf(".intel_syntax noprefix\n");

  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }

  return 0;
}
