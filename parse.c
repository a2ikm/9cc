#include "9cc.h"

typedef struct Env {
  Map *vars;
  struct Env *prev;
} Env;

Env *env;
Vector *lvars;

Env *new_env(Env *prev) {
  Env *env = calloc(1, sizeof(Env));
  env->vars = map_new();
  env->prev = prev;
  return env;
}

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

Var *new_var(Token *tok, Type *type) {
  Var *var = calloc(1, sizeof(Var));
  var->name = strndup(tok->str, tok->len);
  var->len = tok->len;
  var->type = type;
  return var;
}

Var *find_var(Token *tok) {
  for (Env *e = env; e; e = e->prev) {
    Var *var = map_get2(e->vars, tok->str, tok->len);
    if (var) {
      return var;
    }
  }
  return NULL;
}

Var *new_lvar(Token *tok, Type *type) {
  Var *lvar = new_var(tok, type);
  lvar->is_local = true;

  Var *last = vec_last(lvars);
  if (last)
    lvar->offset = last->offset + lvar->type->size;
  else
    lvar->offset = lvar->type->size;

  map_put2(env->vars, tok->str, tok->len, lvar);
  vec_add(lvars, lvar);
  return lvar;
}

Var *new_gvar(Token *tok, Type *type) {
  Var *gvar = new_var(tok, type);
  gvar->is_local = false;

  map_put2(env->vars, tok->str, tok->len, gvar);
  vec_add(gvars, gvar);
  return gvar;
}

String *new_string(Token *tok) {
  String *string = map_get2(strings, tok->str, tok->len);
  if (string) {
    return string;
  }

  string = calloc(1, sizeof(String));
  string->string = strndup(tok->str, tok->len);

  map_put2(strings, tok->str, tok->len, string);
  return string;
}

Function *find_func(Token *tok) {
  for (int i = 0; i < vec_len(funcs); i++) {
    Function *fn = vec_get(funcs, i);
    if (!memcmp(tok->str, fn->name, tok->len))
      return fn;
  }
  return NULL;
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

Type *detect_type() {
  if (consume_kind(TK_LONG))
    return long_type;
  else if (consume_kind(TK_INT))
    return int_type;
  else if (consume_kind(TK_SHORT))
    return short_type;
  else if (consume_kind(TK_CHAR))
    return char_type;
  else
    return NULL;
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

Node *new_var_node(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  node->type = var->type;
  return node;
}

Node *num() {
  return new_num(expect_number());
}

Node *expr();
Node *new_add(Node *lhs, Node *rhs);
Node *compound_stmt();

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = NULL;

  if (tok = consume_ident()) {
    if (consume("(")) {
      Function *fn = find_func(tok);
      if (!fn) {
        error_at(tok->str, "Calling unknown function");
      }

      Node *node = new_node(ND_CALL);
      node->name = strndup(tok->str, tok->len);
      node->type = fn->type;
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
      Var *var;
      Node *node;

      if (var = find_var(tok)) {
        node = new_var_node(var);
      } else {
        error_at(tok->str, "Unknown variable");
      }

      if (consume("[")) {
        node = new_add(node, expr());
        node->type = node->lhs->type;
        expect("]");

        node = new_unary(ND_DEREF, node);
        node->type = node->lhs->type->base;
      }

      return node;
    }
  }

  if (tok = consume_kind(TK_STRING)) {
    String *string = new_string(tok);
    Node *node = new_node(ND_STRING);
    node->string = string;
    node->type = pointer_to(char_type);
    return node;
  }

  return num();
}

// unary = "+" primary
//       | "-" primary
//       | "*" unary
//       | "&" unary
//       | "sizeof" unary
//       | primary
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
      node->type = max_type_by_size(node->lhs->type, node->rhs->type);
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

  if (is_integer(lhs->type) && is_integer(rhs->type)) {
    node = new_binary(ND_ADD, lhs, rhs);
    node->type = max_type_by_size(lhs->type, rhs->type);
  } else if (is_integer(lhs->type)) {
    node = new_binary(ND_PTR_ADD, rhs, lhs);
    node->type = rhs->type;
  } else if (is_integer(rhs->type)) {
    node = new_binary(ND_PTR_ADD, lhs, rhs);
    node->type = lhs->type;
  } else {
    error("kind mismatch: lhs=%d rhs=%d", lhs->type->kind, rhs->type->kind);
  }

  return node;
}

Node *new_sub(Node *lhs, Node *rhs) {
  Node *node;

  if (is_integer(lhs->type) && is_integer(rhs->type)) {
    node = new_binary(ND_SUB, lhs, rhs);
    node->type = max_type_by_size(lhs->type, rhs->type);
  } else if (is_integer(rhs->type)) {
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

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_binary(ND_ASSIGN, node, assign());
    node->type = node->lhs->type;
  }
  return node;
}

// expr = assign
Node *expr() {
  return assign();
}

// expr-stmt = expr? ";"
Node *expr_stmt() {
  Node *node = new_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

// stmt = "if" "(" expr ")" ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr ";" expr ";" expr ")" stmt
//      | "{" compound-stmt
//      | "return" expr ";"
//      | expr-stmt
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
    return compound_stmt();
  } else if (consume_kind(TK_RETURN)) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  return expr_stmt();
}

Node *lvar_initializer(Var *var) {
  Node *node = new_var_node(var);

  if (consume("{")) {
    Vector *exprs = vec_new();

    int i = 0;
    bool closed = false;
    while(!at_eof()) {
      if (consume("}")) {
        closed = true;
        break;
      }
      if (i > 0) {
        expect(",");
      }

      vec_add(exprs, assign());
      i++;
    }

    if (!closed) {
      expect("}");
    }

    if (var->type->array_size == -1) {
      var->type = array_of(var->type->base, vec_len(exprs));
    } else if (var->type->array_size < vec_len(exprs)) {
      error_at(token->str, "Too many initializers");
    } else {
      for (; vec_len(exprs) < var->type->array_size;) {
        vec_add(exprs, new_num(0));
      }
    }

    Vector *stmts = vec_new();
    for (int j = 0; j < vec_len(exprs); j++) {
      Node *init = new_add(node, new_num(j));
      init->type = init->lhs->type;
      init = new_unary(ND_DEREF, init);
      init->type = init->lhs->type->base;

      init = new_binary(ND_ASSIGN, init, vec_get(exprs, j));
      init->type = init->lhs->type;
      init = new_unary(ND_EXPR_STMT, init);

      vec_add(stmts, init);
    }

    node = new_node(ND_BLOCK);
    node->stmts = stmts;
  } else {
    node = new_binary(ND_ASSIGN, node, assign());
    node->type = node->lhs->type;
    node = new_unary(ND_EXPR_STMT, node);
  }

  return node;
}

Node *declaration(Type *type) {
  Node *node = NULL;
  Var *var = NULL;

  while (!at_eof()) {
    if (consume("*")) {
      type = pointer_to(type);
      continue;
    }
    break;
  }
  Token *tok = expect_kind(TK_IDENT);
  if (consume("[")) {
    if (token->kind == TK_NUM) {
      type = array_of(type, expect_number());
    } else {
      type = array_of(type, -1);
    }
    expect("]");
  }

  var = new_lvar(tok, type);
  if (consume("=")) {
    node = lvar_initializer(var);
  }

  expect(";");

  return node;
}

// compound-stmt = (declaration | stmt)* "}"
Node *compound_stmt() {
  Node *node = new_node(ND_BLOCK);
  node->stmts = vec_new();
  while(!at_eof()) {
    if (consume("}"))
      break;
    Type *type = detect_type();
    if (type) {
      Node *init = declaration(type);
      if (init)
        vec_add(node->stmts, init);
    } else {
      vec_add(node->stmts, stmt());
    }
  }
  return node;
}

// func = typename ident "(" (typename ident ("," typename ident)*)? ")" "{" compound-stmt
//      | typename ident "(" (typename ident ("," typename ident)*)? ")" ";"
void func() {
  Type *type = detect_type();
  if (!type)
    error_at(token->str, "Type detection failure");

  while (!at_eof()) {
    if (consume("*")) {
      type = pointer_to(type);
      continue;
    }
    break;
  }
  Token *tok = expect_kind(TK_IDENT);

  if (consume("(")) {
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
        Type *type = detect_type();
        if (!type)
          error_at(token->str, "Type detection failure");
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
    node->stmts = compound_stmt()->stmts;
  } else {
    if (consume("[")) {
      int array_size = expect_number();
      type = array_of(type, array_size);
      expect("]");
    }
    expect(";");

    new_gvar(tok, type);
  }
}

void program() {
  while (!at_eof())
    func();
}

void parse() {
  env = new_env(NULL);
  program();
}
