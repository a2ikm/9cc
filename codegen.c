#include "9cc.h"

char *regsd[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
char *regsq[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

unsigned int label_idx = 0;

void gen_lval(Node *node) {
  switch(node->kind) {
    case ND_GVAR:
      printf("  lea rax, %s[rip]\n", node->name);
      printf("  push rax\n");
      return;
    case ND_LVAR:
    case ND_ADDR:
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", node->offset);
      printf("  push rax\n");
      return;
    case ND_DEREF:
      gen(node->lhs);
      return;
  }

  error("代入の左辺値が変数かアドレスのどちらでもありません");
}

void load(Type *type) {
  printf("  pop rax\n");

  if (type->size == DWORD_SIZE)
    printf("  movsxd rax, dword ptr [rax]\n");
  else
    printf("  mov rax, [rax]\n");

  printf("  push rax\n");
}

void store(Type *type) {
  printf("  pop rdi\n");
  printf("  pop rax\n");

  if (type->size == DWORD_SIZE)
    printf("  mov [rax], edi\n");
  else
    printf("  mov [rax], rdi\n");

  printf("  push rdi\n");
}

void gen(Node *node) {
  unsigned int tmp_label_idx;
  int frame_size = 0;

  switch(node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_GVAR:
    case ND_LVAR:
      gen_lval(node);
      if (node->type->kind != TYPE_ARRAY)
        load(node->type);
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      if (node->type->kind != TYPE_ARRAY)
        load(node->type);
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      store(node->type);
      return;
    case ND_SIZEOF:
      printf("  push %ld\n", node->lhs->type->size);
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
        printf("  pop %s\n", regsq[i]);

      tmp_label_idx = label_idx++;
      printf("  mov rax, rsp\n");
      printf("  and rax, 0xF\n");
      printf("  jnz .L.call.%d\n", tmp_label_idx);
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->name);
      printf("  jmp .L.end.%d\n", tmp_label_idx);
      printf(".L.call.%d:\n", tmp_label_idx);
      printf("  sub rsp, 8\n");
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->name);
      printf("  add rsp, 8\n");
      printf(".L.end.%d:\n", tmp_label_idx);
      printf("  push rax\n");
      return;
    case ND_FUNC:
      printf(".global %s\n", node->name);
      printf("%s:\n", node->name);
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");

      size_t frame_size = 0;
      for (int i = 0; i < vec_len(node->lvars); i++)
        frame_size += ((Var *)vec_get(node->lvars, i))->type->size;
      if (frame_size > 0)
        printf("  sub rsp, %ld\n", frame_size);

      for (int i = 0; i < vec_len(node->params); i++) {
        Var *lvar = vec_get(node->params, i);
        if (lvar->type->size == DWORD_SIZE)
          printf("  mov [rbp-%d], %s\n", lvar->offset, regsd[i]);
        else
          printf("  mov [rbp-%d], %s\n", lvar->offset, regsq[i]);
      }

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
    case ND_PTR_ADD:
      printf("  imul rdi, %ld\n", node->lhs->type->base->size);
      printf("  add rax, rdi\n");
      break;
    case ND_PTR_SUB:
      printf("  imul rdi, %ld\n", node->lhs->type->base->size);
      printf("  sub rax, rdi\n");
      break;
    case ND_PTR_DIFF:
      printf("  sub rax, rdi\n");
      printf("  mov rdi, %ld\n", node->lhs->type->base->size);
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

void gen_gvar(Var *gvar) {
  printf("%s:\n", gvar->name);
  printf("  .zero %ld\n", gvar->type->size);
}
