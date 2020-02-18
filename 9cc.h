#ifndef __9CC_H__
#define __9CC_H__

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT_SIZE 8

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

typedef enum {
  TK_RESERVED,  // 記号
  TK_RETURN,    // "return"
  TK_IF,        // "if"
  TK_ELSE,      // "else"
  TK_WHILE,     // "while"
  TK_FOR,       // "for"
  TK_IDENT,     // 識別子
  TK_NUM,       // 整数
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

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NEQ,
  ND_LT,
  ND_LTEQ,
  ND_ASSIGN,
  ND_LVAR,
  ND_NUM,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_CALL,
  ND_FUNC,
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

  char *fname;
  LVar *locals;
};

Token *token;
Node *code[100];

char *user_input;

// tokenize.c
void tokenize();
char *token_copy_string();

// parse.c
void parse();

// codegen.c
void gen(Node *node);

#endif
