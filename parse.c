#include "9cc.h"

Type *int_type = &(Type){ TYPE_INT, DWORD_SIZE, NULL };

void dump_type(Node *node) {
  Type *type = node->type;
  fprintf(stderr, "%s: ", node->name);
  for (;type;) {
    if (type->kind == TYPE_PTR) {
      fprintf(stderr, "ptr->");
      type = type->base;
    } else if (type->kind == TYPE_INT) {
      fprintf(stderr, "int\n");
      return;
    }
  }
  fprintf(stderr, "null\n");
}

Vector *lvars;

LVar *find_lvar(Token *tok) {
  for (int i = 0; i < vec_len(lvars); i++) {
    LVar *lvar = vec_get(lvars, i);
    if (lvar->len == tok->len && !memcmp(tok->str, lvar->name, lvar->len))
      return lvar;
  }
  return NULL;
}

LVar *new_lvar(Token *tok, Type *type) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;

  LVar *last = vec_last(lvars);
  if (last)
    lvar->offset = last->offset + lvar->type->size;
  else
    lvar->offset = lvar->type->size;

  vec_add(lvars, lvar);
  return lvar;
}

Function *find_func(Token *tok) {
  for (int i = 0; i < vec_len(funcs); i++) {
    Function *fn = vec_get(funcs, i);
    if (!memcmp(tok->str, fn->name, tok->len))
      return fn;
  }
  return NULL;
}

Type *new_type(TypeKind kind) {
  Type *type = malloc(sizeof(Type));
  type->kind = kind;
  type->base = NULL;
  return type;
}

Type *pointer_to(Type *base) {
  Type *type = new_type(TYPE_PTR);
  type->size = QWORD_SIZE;
  type->base = base;
  return type;
}

Type *array_of(Type *base, size_t array_size) {
  Type *type = new_type(TYPE_ARRAY);
  type->base = base;
  type->array_size = array_size;
  type->size = array_size * base->size;
  return type;
}

bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_kind(TokenKind kind) {
  if (token->kind != kind)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_ident() {
  return consume_kind(TK_IDENT);
}

void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

Token *expect_kind(TokenKind kind) {
  if (token->kind != kind)
    error_at(token->str, "kindが異なります");
  Token *tok = token;
  token = token->next;
  return tok;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = int_type;
  return node;
}

Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr;
  node->rhs = NULL;
  return node;
}

Node *num() {
  return new_num(expect_number());
}

Node *expr();
Node *new_add(Node *lhs, Node *rhs);

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      Node *node = new_node(ND_CALL);
      node->name = strndup(tok->str, tok->len);
      node->type = find_func(tok)->type;
      node->args = vec_new();

      // parse args
      while (!consume(")")) {
        vec_add(node->args, expr());
        if (consume(","))
          continue;
        expect(")");
        break;
      }

      return node;
    } else {
      Node *node = new_node(ND_LVAR);

      LVar *lvar = find_lvar(tok);
      node->offset = lvar->offset;
      node->name = strndup(tok->str, tok->len);
      node->type = lvar->type;

      if (consume("[")) {
        node = new_add(node, expr());
        node->type = node->lhs->type;
        expect("]");

        node = new_unary(ND_DEREF, node);
        node->type = node->lhs->type->base;

        return node;
      } else {
        return node;
      }
    }
  }

  return num();
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-")) {
    Node *node = new_binary(ND_SUB, new_num(0), primary());
    node->type = node->rhs->type;
    return node;
  }
  if (consume("*")) {
    Node *node = new_unary(ND_DEREF, unary());
    node->type = node->lhs->type->base;
    return node;
  }
  if (consume("&")) {
    Node *node = new_unary(ND_ADDR, unary());
    node->type = pointer_to(node->lhs->type);
    return node;
  }
  if (consume_kind(TK_SIZEOF)) {
    Node *node = new_unary(ND_SIZEOF, unary());
    node->type = int_type;
    return node;
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
      node->type = node->lhs->type;
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
      node->type = node->lhs->type;
    } else {
      return node;
    }
  }
}

Node *new_add(Node *lhs, Node *rhs) {
  Node *node;

  if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
    node = new_binary(ND_ADD, lhs, rhs);
    node->type = int_type;
  } else if (lhs->type->kind == TYPE_INT) {
    node = new_binary(ND_PTR_ADD, rhs, lhs);
    node->type = rhs->type;
  } else if (rhs->type->kind == TYPE_INT) {
    node = new_binary(ND_PTR_ADD, lhs, rhs);
    node->type = lhs->type;
  } else {
    error("kind mismatch: lhs=%d rhs=%d", lhs->type->kind, rhs->type->kind);
  }

  return node;
}

Node *new_sub(Node *lhs, Node *rhs) {
  Node *node;

  if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
    node = new_binary(ND_SUB, lhs, rhs);
    node->type = int_type;
  } else if (rhs->type->kind == TYPE_INT) {
    node = new_binary(ND_PTR_SUB, lhs, rhs);
    node->type = lhs->type;
  } else if (lhs->type->kind == TYPE_PTR) {
    node = new_binary(ND_PTR_DIFF, lhs, rhs);
    node->type = int_type;
  } else {
    error("kind mismatch: lhs=%d rhs=%d", lhs->type->kind, rhs->type->kind);
  }

  return node;
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_add(node, mul());
    else if (consume("-"))
      node = new_sub(node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
      node->type = int_type;
    } else if (consume("<=")) {
      node->type = int_type;
      node = new_binary(ND_LTEQ, node, add());
    } else if (consume(">")) {
      node->type = int_type;
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node->type = int_type;
      node = new_binary(ND_LTEQ, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
      node->type = int_type;
    } else if (consume("!=")) {
      node = new_binary(ND_NEQ, node, relational());
      node->type = int_type;
    }
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_binary(ND_ASSIGN, node, assign());
    node->type = node->lhs->type;
  }
  return node;
}

Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;

  // end with stmt
  if (consume_kind(TK_IF)) {
    Node *node = new_node(ND_IF);
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
    if (consume_kind(TK_ELSE))
      node->alternative = stmt();
    return node;
  } else if (consume_kind(TK_WHILE)) {
    Node *node = new_node(ND_WHILE);
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
    return node;
  } else if (consume_kind(TK_FOR)) {
    Node *node = new_node(ND_FOR);
    expect("(");
    node->initialization = expr();
    expect(";");
    node->condition = expr();
    expect(";");
    node->increment = expr();
    expect(")");
    node->consequence = stmt();
    return node;
  } else if (consume("{")) {
    Node *node = new_node(ND_BLOCK);
    node->stmts = vec_new();
    while (!at_eof()) {
      if (consume("}"))
        break;
      vec_add(node->stmts, (void *)stmt());
    }
    return node;
  } else if (consume_kind(TK_RETURN)) {
    Node *node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume_kind(TK_INT)) {
    Type *type = int_type;
    while (!at_eof()) {
      if (consume("*")) {
        type = pointer_to(type);
        continue;
      }
      break;
    }
    Token *tok = expect_kind(TK_IDENT);
    if (consume("[")) {
      int array_size = expect_number();
      type = array_of(type, array_size);
      expect("]");
    }
    new_lvar(tok, type);
    Node *node = new_node(ND_VAR_DECLARE);
    node->name = strndup(tok->str, tok->len);
    node->type = type;
    expect(";");
    return node;
  }

  node = expr();
  expect(";");
  return node;
}

void func() {
  expect_kind(TK_INT);
  Type *type = int_type;
  while (!at_eof()) {
    if (consume("*")) {
      type = pointer_to(type);
      continue;
    }
    break;
  }
  Token *tok = expect_kind(TK_IDENT);
  expect("(");

  Function *fn = find_func(tok);
  if (!fn) {
    fn = calloc(1, sizeof(Function));
    fn->name = strndup(tok->str, tok->len);
    fn->type = type;
    vec_add(funcs, fn);
  }

  // parse params
  Vector *params = vec_new();
  lvars = vec_new();
  if (!consume(")")) {
    while (!at_eof()) {
      expect_kind(TK_INT);
      Type *type = int_type;
      while (!at_eof()) {
        if (consume("*")) {
          type = pointer_to(type);
          continue;
        }
        break;
      }
      tok = expect_kind(TK_IDENT);
      vec_add(params, new_lvar(tok, type));
      if (consume(","))
        continue;
      break;
    }
    expect(")");
  }

  if (consume(";"))
    return;

  Node *node = new_node(ND_FUNC);
  node->name = fn->name;
  node->type = fn->type;
  node->params = params;
  node->stmts = vec_new();
  node->lvars = lvars;
  fn->node = node;

  expect("{");
  while (!at_eof()) {
    if (consume("}"))
      break;
    vec_add(node->stmts, (void *)stmt());
  }
  return;
}

void program() {
  while (!at_eof())
    func();
}

void parse() {
  program();
}
