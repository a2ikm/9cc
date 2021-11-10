#include "9cc.h"

char *regsb[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *regsw[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
char *regsd[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *regsq[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int depth = 0;
unsigned int label_idx = 0;

void gen(Node *node);

void println(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
}

void push(char *arg)
{
  println("  push %s", arg);
  depth++;
}

void pop(char *arg)
{
  println("  pop %s", arg);
  depth--;
}

void gen_lval(Node *node)
{
  switch (node->kind)
  {
  case ND_VAR:
    if (node->var->is_local)
    {
      println("  mov rax, rbp");
      println("  sub rax, %d", node->var->offset);
      push("rax");
      return;
    }
    else
    {
      println("  lea rax, %s[rip]", node->var->name);
      push("rax");
      return;
    }
  case ND_DEREF:
    gen(node->lhs);
    return;
  }

  error("代入の左辺値が変数かアドレスのどちらでもありません");
}

void load(Type *type)
{
  pop("rax");

  switch (type->size)
  {
  case DWORD_SIZE:
    println("  movsxd rax, dword ptr [rax]");
    break;
  case WORD_SIZE:
    println("  movsx rax, word ptr [rax]");
    break;
  case BYTE_SIZE:
    println("  movsx rax, byte ptr [rax]");
    break;
  case QWORD_SIZE:
  default:
    println("  mov rax, [rax]");
  }

  push("rax");
}

void store(Type *type)
{
  pop("rdi");
  pop("rax");

  switch (type->size)
  {
  case DWORD_SIZE:
    println("  mov [rax], edi");
    break;
  case WORD_SIZE:
    println("  mov [rax], di");
    break;
  case BYTE_SIZE:
    println("  mov [rax], dil");
    break;
  case QWORD_SIZE:
  default:
    println("  mov [rax], rdi");
  }

  push("rdi");
}

void gen_num(Node *node)
{
  println("  mov rax, %d", node->val);
  push("rax");
  return;
}

void gen_string(Node *node)
{
  println("  lea rax, %s", node->string->name);
  push("rax");
  return;
}

void gen_return(Node *node)
{
  gen(node->lhs);
  pop("rax");
  println("  mov rsp, rbp");
  pop("rbp");
  println("  ret");
  return;
}

void gen_if(Node *node)
{
  unsigned int tmp_label_idx = label_idx++;

  gen(node->condition);
  pop("rax");
  println("  cmp rax, 0");
  println("  je  .Lelse%d", tmp_label_idx);
  gen(node->consequence);
  println("  jmp .Lend%d", tmp_label_idx);
  println(".Lelse%d:", tmp_label_idx);
  if (node->alternative)
  {
    gen(node->alternative);
  }
  println(".Lend%d:", tmp_label_idx);
  return;
}

void gen_while(Node *node)
{
  unsigned int tmp_label_idx = label_idx++;
  println(".Lbegin%d:", tmp_label_idx);
  gen(node->condition);
  pop("rax");
  println("  cmp rax, 0");
  println("  je  .Lend%d", tmp_label_idx);
  gen(node->consequence);
  println("  jmp .Lbegin%d", tmp_label_idx);
  println(".Lend%d:", tmp_label_idx);
  return;
}

void gen_for(Node *node)
{
  unsigned int tmp_label_idx = label_idx++;
  gen(node->initialization);
  println(".Lbegin%d:", tmp_label_idx);
  gen(node->condition);
  pop("rax");
  println("  cmp rax, 0");
  println("  je  .Lend%d", tmp_label_idx);
  gen(node->consequence);
  gen(node->increment);
  println("  jmp .Lbegin%d", tmp_label_idx);
  println(".Lend%d:", tmp_label_idx);
  return;
}

void gen_call(Node *node)
{
  for (int i = 0; i < vec_len(node->args); i++)
    gen((Node *)vec_get(node->args, i));

  for (int i = vec_len(node->args) - 1; i >= 0; i--)
    pop(regsq[i]);

  bool need_padding = depth % 2 == 1;
  if (need_padding)
    println("  sub rsp, 8");

  println("  mov al, 0"); // the number of floats in arguments
  println("  mov rax, 0");
  println("  call %s", node->name);

  if (need_padding)
    println("  add rsp, 8");

  push("rax");
  return;
}

void gen_func(Node *node)
{
  println(".global %s", node->name);
  println("%s:", node->name);
  push("rbp");
  println("  mov rbp, rsp");

  size_t frame_size = 0;
  for (int i = 0; i < vec_len(node->lvars); i++)
    frame_size += ((Var *)vec_get(node->lvars, i))->type->size;
  if (frame_size > 0)
    println("  sub rsp, %ld", frame_size);

  for (int i = 0; i < vec_len(node->params); i++)
  {
    Var *lvar = vec_get(node->params, i);
    switch (lvar->type->size)
    {
    case DWORD_SIZE:
      println("  mov [rbp-%d], %s", lvar->offset, regsd[i]);
      break;
    case WORD_SIZE:
      println("  mov [rbp-%d], %s", lvar->offset, regsd[i]);
      break;
    case BYTE_SIZE:
      println("  mov [rbp-%d], %s", lvar->offset, regsb[i]);
      break;
    case QWORD_SIZE:
    default:
      println("  mov [rbp-%d], %s", lvar->offset, regsq[i]);
    }
  }

  for (int i = 0; i < vec_len(node->stmts); i++)
  {
    gen((Node *)vec_get(node->stmts, i));
  }
  return;
}

void gen_binary(Node *node)
{
  gen(node->lhs);
  gen(node->rhs);

  pop("rdi");
  pop("rax");

  switch (node->kind)
  {
  case ND_ADD:
    println("  add rax, rdi");
    break;
  case ND_SUB:
    println("  sub rax, rdi");
    break;
  case ND_MUL:
    println("  imul rax, rdi");
    break;
  case ND_DIV:
    println("  cqo");
    println("  idiv rdi");
    break;
  case ND_PTR_ADD:
    println("  imul rdi, %ld", node->lhs->type->base->size);
    println("  add rax, rdi");
    break;
  case ND_PTR_SUB:
    println("  imul rdi, %ld", node->lhs->type->base->size);
    println("  sub rax, rdi");
    break;
  case ND_PTR_DIFF:
    println("  sub rax, rdi");
    println("  mov rdi, %ld", node->lhs->type->base->size);
    println("  cqo");
    println("  idiv rdi");
    break;
  case ND_EQ:
    println("  cmp rax, rdi");
    println("  sete al");
    println("  movzb rax, al");
    break;
  case ND_NEQ:
    println("  cmp rax, rdi");
    println("  setne al");
    println("  movzb rax, al");
    break;
  case ND_LT:
    println("  cmp rax, rdi");
    println("  setl al");
    println("  movzb rax, al");
    break;
  case ND_LTEQ:
    println("  cmp rax, rdi");
    println("  setle al");
    println("  movzb rax, al");
    break;
  }

  push("rax");
  return;
}

void gen(Node *node)
{
  switch (node->kind)
  {
  case ND_NUM:
    gen_num(node);
    return;
  case ND_STRING:
    gen_string(node);
    return;
  case ND_VAR:
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
  case ND_RETURN:
    gen_return(node);
    return;
  case ND_IF:
    gen_if(node);
    return;
  case ND_WHILE:
    gen_while(node);
    return;
  case ND_FOR:
    gen_for(node);
    return;
  case ND_BLOCK:
    for (int i = 0; i < vec_len(node->stmts); i++)
    {
      gen((Node *)vec_get(node->stmts, i));
    }
    return;
  case ND_CALL:
    gen_call(node);
    return;
  case ND_FUNC:
    gen_func(node);
    return;
  case ND_EXPR_STMT:
    gen(node->lhs);
    return;
  }

  gen_binary(node);
  return;
}

void emit_syntax()
{
  printf("  .intel_syntax noprefix\n");
}

void emit_bss()
{
  if (vec_len(gvars) == 0)
    return;

  println("  .bss");
  for (int i = 0; i < vec_len(gvars); i++)
  {
    Var *gvar = vec_get(gvars, i);
    println("%s:", gvar->name);
    println("  .zero %ld", gvar->type->size);
  }
}

void emit_data()
{
  Vector *keys = map_keys(strings);

  for (int i = 0; i < vec_len(keys); i++)
  {
    char *key = vec_get(keys, i);
    String *string = map_get(strings, key);

    int len = snprintf(NULL, 0, ".LC%d", i);
    string->name = (char *)malloc(sizeof(char) * (len + 1));
    sprintf(string->name, ".LC%d", i);

    println("  .data");
    println("%s:", string->name);
    println("  .string \"%s\"", string->string);
  }
}

void emit_text()
{
  println("  .text");
  for (int i = 0; i < vec_len(funcs); i++)
  {
    Function *fn = vec_get(funcs, i);
    if (fn->node)
      gen(fn->node);
  }
}
