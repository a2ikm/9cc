#ifndef __9CC_H__
#define __9CC_H__

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QWORD_SIZE 8
#define DWORD_SIZE 4
#define WORD_SIZE 2
#define BYTE_SIZE 1

// vector.c
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

// map.c
typedef struct {
  char *key;
  int keylen;
  void *data;
} MapEntry;

typedef struct {
  MapEntry *buckets;
  int size;
  int len;
} Map;

Map *map_new();
void *map_get(Map *map, char *key);
void *map_get2(Map *map, char *key, int keylen);
void map_put(Map *map, char *key, void *data);
void map_put2(Map *map, char *key, int keylen, void *data);
void map_delete(Map *map, char *key);
void map_delete2(Map *map, char *key, int keylen);
Vector *map_keys(Map *map);
void map_test();

// helper.c
char *strndup(const char *s, size_t n);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef enum {
  TK_PUNC,      // 記号
  TK_RETURN,    // "return"
  TK_IF,        // "if"
  TK_ELSE,      // "else"
  TK_WHILE,     // "while"
  TK_FOR,       // "for"
  TK_LONG,      // "long"
  TK_INT,       // "int"
  TK_SHORT,     // "short"
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

// type.c

typedef enum {
  TYPE_PTR,
  TYPE_LONG,
  TYPE_INT,
  TYPE_SHORT,
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

extern Type *long_type;
extern Type *int_type;
extern Type *short_type;
extern Type *char_type;

Type *new_type(TypeKind kind);
Type *pointer_to(Type *base);
Type *array_of(Type *base, size_t array_size);
bool is_integer(Type *type);
Type *max_type_by_size(Type *ltype, Type *rtype);

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
  ND_VAR,
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
  Type *type;
  Var *var;
  String *string;

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
Map *strings;

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

#define unreachable() error("unreachable %s:%d", __FILE__, __LINE__)

#endif
