#include "9cc.h"

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
    lvar->offset = last->offset + PTR_SIZE;
  else
    lvar->offset = PTR_SIZE;

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
  type->ptr_to = NULL;
  return type;
}

Type *new_type_int() {
  return new_type(TYPE_INT);
}

Type *new_type_ptr_to(Type *ptr_to) {
  Type *type = new_type(TYPE_PTR);
  type->ptr_to = ptr_to;
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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = new_type_int();
  return node;
}

Node *num() {
  return new_node_num(expect_number());
}

Node *expr();

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->name = token_copy_string(tok);
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
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar(tok);
      node->offset = lvar->offset;
      node->type = lvar->type;
      return node;
    }
  }

  return num();
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-")) {
    Node *node = new_node(ND_SUB, new_node_num(0), primary());
    node->type = node->rhs->type;
    return node;
  }
  if (consume("*")) {
    Node *node = new_node(ND_DEREF, unary(), NULL);
    node->type = node->lhs->type->ptr_to;
    return node;
  }
  if (consume("&")) {
    Node *node = new_node(ND_ADDR, unary(), NULL);
    node->type = new_type_ptr_to(node->lhs->type);
    return node;
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
      node->type = node->lhs->type;
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
      node->type = node->lhs->type;
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
      if (node->lhs->type->kind == TYPE_PTR)
        node->type = node->lhs->type;
      else if (node->rhs->type->kind == TYPE_PTR)
        node->type = node->rhs->type;
      else
        node->type = node->lhs->type;
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
      if (node->lhs->type->kind == TYPE_PTR)
        node->type = node->lhs->type;
      else if (node->rhs->type->kind == TYPE_PTR)
        node->type = node->rhs->type;
      else
        node->type = node->lhs->type;
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
      node->type = new_type_int();
    } else if (consume("<=")) {
      node->type = new_type_int();
      node = new_node(ND_LTEQ, node, add());
    } else if (consume(">")) {
      node->type = new_type_int();
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node->type = new_type_int();
      node = new_node(ND_LTEQ, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
      node->type = new_type_int();
    } else if (consume("!=")) {
      node = new_node(ND_NEQ, node, relational());
      node->type = new_type_int();
    }
    else
      return node;
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
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
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
    if (consume_kind(TK_ELSE))
      node->alternative = stmt();
    return node;
  } else if (consume_kind(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->condition = expr();
    expect(")");
    node->consequence = stmt();
    return node;
  } else if (consume_kind(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
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
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = vec_new();
    while (!at_eof()) {
      if (consume("}"))
        break;
      vec_add(node->stmts, (void *)stmt());
    }
    return node;
  } else if (consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume_kind(TK_INT)) {
    Type *type = new_type_int();
    while (!at_eof()) {
      if (consume("*")) {
        type = new_type_ptr_to(type);
        continue;
      }
      break;
    }
    Token *tok = expect_kind(TK_IDENT);
    new_lvar(tok, type);
    node = calloc(1, sizeof(Node));
    node->kind = ND_VAR_DECLARE;
    node->name = token_copy_string(tok);
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
  Type *type = new_type_int();
  while (!at_eof()) {
    if (consume("*")) {
      type = new_type_ptr_to(type);
      continue;
    }
    break;
  }
  Token *tok = expect_kind(TK_IDENT);
  expect("(");

  Function *fn = find_func(tok);
  if (!fn) {
    fn = calloc(1, sizeof(Function));
    fn->name = token_copy_string(tok);
    fn->type = type;
    vec_add(funcs, fn);
  }

  // parse params
  Vector *params = vec_new();
  lvars = vec_new();
  if (!consume(")")) {
    while (!at_eof()) {
      expect_kind(TK_INT);
      Type *type = new_type_int();
      while (!at_eof()) {
        if (consume("*")) {
          type = new_type_ptr_to(type);
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

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
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
