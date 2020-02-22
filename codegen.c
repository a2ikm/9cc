#include "9cc.h"

char *regs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

unsigned int label_idx = 0;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR && node->kind != ND_ADDR) {
    fprintf(stderr, "代入の左辺値が変数かアドレスのどちらでもありません\n");
    exit(1);
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  unsigned int tmp_label_idx;
  int frame_size = 0;

  switch(node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      tmp_label_idx = label_idx++;

      if (node->alternative) {
        gen(node->condition);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", tmp_label_idx);
        gen(node->consequence);
        printf("  jmp .Lend%d\n", tmp_label_idx);
        printf(".Lelse%d:\n", tmp_label_idx);
        gen(node->alternative);
        printf(".Lend%d:\n", tmp_label_idx);
      } else {
        gen(node->condition);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", tmp_label_idx);
        gen(node->consequence);
        printf(".Lend%d:\n", tmp_label_idx);
      }
      return;
    case ND_WHILE:
      tmp_label_idx = label_idx++;
      printf(".Lbegin%d:\n", tmp_label_idx);
      gen(node->condition);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", tmp_label_idx);
      gen(node->consequence);
      printf("  jmp .Lbegin%d\n", tmp_label_idx);
      printf(".Lend%d:\n", tmp_label_idx);
      return;
    case ND_FOR:
      tmp_label_idx = label_idx++;
      gen(node->initialization);
      printf(".Lbegin%d:\n", tmp_label_idx);
      gen(node->condition);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", tmp_label_idx);
      gen(node->consequence);
      gen(node->increment);
      printf("  jmp .Lbegin%d\n", tmp_label_idx);
      printf(".Lend%d:\n", tmp_label_idx);
      return;
    case ND_BLOCK:
      for (int i = 0; i < vec_len(node->stmts); i++) {
        gen((Node *)vec_get(node->stmts, i));
        printf("  pop rax\n");
      }
      printf("  push rax\n");
      return;
    case ND_CALL:
      for (int i = 0; i < vec_len(node->args); i++)
        gen((Node *)vec_get(node->args, i));

      for (int i = vec_len(node->args) - 1; i >= 0; i--)
        printf("  pop %s\n", regs[i]);

      printf("  push rsp\n");
      printf("  push [rsp]\n");
      printf("  and rsp, -0x10\n");
      printf("  call %s\n", node->name);
      printf("  mov rsp, [rsp+8]\n");
      printf("  push rax\n");
      return;
    case ND_FUNC:
      printf(".global %s\n", node->name);
      printf("%s:\n", node->name);
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");

      for (int i = 0; i < vec_len(node->params); i++)
        printf("  push %s\n", regs[i]);

      frame_size = (vec_len(node->lvars) - vec_len(node->params)) * INT_SIZE;
      printf("  sub rsp, %d\n", frame_size);

      for (int i = 0; i < vec_len(node->stmts); i++) {
        gen((Node *)vec_get(node->stmts, i));
        printf("  pop rax\n");
      }
      return;
    case ND_VAR_DECLARE:
      printf("  push rax\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LTEQ:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
