#ifndef __9CC_H__
#define __9CC_H__

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QWORD_SIZE 8
#define DWORD_SIZE 4
#define BYTE_SIZE 1

// helper.c
char *strndup(const char *s, size_t n);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef struct {
  void **data;
  int len;
  int size;
} Vector;

Vector *vec_new();
void vec_add(Vector *vec, void *item);
int vec_len(Vector *vec);
void *vec_get(Vector *vec, int idx);
void *vec_last(Vector *vec);

typedef enum {
  TK_RESERVED,  // 記号
  TK_RETURN,    // "return"
  TK_IF,        // "if"
  TK_ELSE,      // "else"
  TK_WHILE,     // "while"
  TK_FOR,       // "for"
  TK_INT,       // "int"
  TK_CHAR,      // "char"
  TK_SIZEOF,    // "sizeof"
  TK_IDENT,     // 識別子
  TK_NUM,       // 整数
  TK_STRING,    // 文字列
  TK_EOF,       // EOF
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  TYPE_PTR,
  TYPE_INT,
  TYPE_CHAR,
  TYPE_ARRAY,
} TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  size_t size;
  struct Type *base;
  size_t array_size;
};

extern Type *int_type;
extern Type *char_type;

typedef struct Var {
  char *name;
  int len;
  int offset;
  Type *type;
  bool is_local;
} Var;

typedef struct String {
  char *name;
  char *string;
} String;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_ADDR,
  ND_DEREF,
  ND_PTR_ADD,
  ND_PTR_SUB,
  ND_PTR_DIFF,
  ND_EQ,
  ND_NEQ,
  ND_LT,
  ND_LTEQ,
  ND_ASSIGN,
  ND_LVAR,
  ND_GVAR,
  ND_NUM,
  ND_STRING,
  ND_SIZEOF,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_CALL,
  ND_FUNC,
  ND_VAR_DECLARE,
  ND_EXPR_STMT,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Vector *stmts;

  // if (<condition>) <consequence> else <alternative>
  // while (<condition>) <consequence>
  // for (<initialization>; <condition>; <increment>) <consequence>
  Node *condition;
  Node *consequence;
  Node *alternative;
  Node *initialization;
  Node *increment;

  int val;
  int offset;
  Type *type;

  char *name;
  Vector *params;
  Vector *args;
  Vector *lvars;
};

typedef struct {
  char *name;
  Type *type;
  Node *node;
} Function;

Vector *funcs;
Vector *gvars;
Vector *strings;

Token *token;
char *user_input;
char *filename;

// tokenize.c
void tokenize();

// parse.c
void parse();

// codegen.c
void emit_bss();
void emit_data();
void emit_text();

#endif
