#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  funcs = vec_new();
  gvars = vec_new();
  strings = vec_new();

  user_input = argv[1];
  tokenize();
  parse();

  printf("  .intel_syntax noprefix\n");
  emit_bss();
  emit_data();
  emit_text();

  return 0;
}
