#include "9cc.h"

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

char *read_ident(char *p) {
  if (isalpha(*p) || *p == '_') {
    p++;
    while (isalpha(*p) || isdigit(*p) || *p == '_') {
      p++;
    }
    return p;
  } else {
    return NULL;
  }
}

bool is_alnum(char c) {
  return isalpha(c) || isdigit(c) || c == '_';
}

bool match(char *p, char *kwd) {
  int len = strlen(kwd);
  return (!strncmp(p, kwd, len) && !is_alnum(p[len]));
}

void tokenize() {
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "//", 2) == 0) {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    if (strncmp(p, "/*", 2) == 0) {
      p += 2;
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "コメントが閉じられていません");
      p = q + 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '{' || *p == '}' ||
        *p == '[' || *p == ']' ||
        *p == '&' || *p == ',' || *p == ';') {
      cur = new_token(TK_PUNC, cur, p++, 1);
      continue;
    }

    if (*p == '=') {
      if (*(p+1) == '=') {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      } else {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '!' && *(p+1) == '=') {
      cur = new_token(TK_PUNC, cur, p++, 2);
      p++;
      continue;
    }

    if (*p == '<') {
      if (*(p+1) == '=') {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      } else {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '>') {
      if (*(p+1) == '=') {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      } else {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '"') {
      char *start = ++p;
      char *end = NULL;

      while (*p) {
        if (*p == '"') {
          end = p;
          break;
        }
        p++;
      }

      if (!end)
        error_at(start, "double quotation not found");

      cur = new_token(TK_STRING, cur, start, end - start);
      p++;
      continue;
    }

    if (*p == '\'') {
      char *start = ++p;

      p++;
      if (*p != '\'')
        error_at(start, "single quotation not found");

      cur = new_token(TK_NUM, cur, start, 1);
      cur->val = *start;

      p++;
      continue;
    }

    if (match(p, "return")) {
      cur = new_token(TK_KW, cur, p, 6);
      p += 6;
      continue;
    }

    if (match(p, "if")) {
      cur = new_token(TK_KW, cur, p, 2);
      p += 2;
      continue;
    }

    if (match(p, "else")) {
      cur = new_token(TK_KW, cur, p, 4);
      p += 4;
      continue;
    }

    if (match(p, "while")) {
      cur = new_token(TK_KW, cur, p, 5);
      p += 5;
      continue;
    }

    if (match(p, "for")) {
      cur = new_token(TK_KW, cur, p, 3);
      p += 3;
      continue;
    }

    if (match(p, "long")) {
      cur = new_token(TK_KW, cur, p, 4);
      p += 4;
      continue;
    }

    if (match(p, "int")) {
      cur = new_token(TK_KW, cur, p, 3);
      p += 3;
      continue;
    }

    if (match(p, "short")) {
      cur = new_token(TK_KW, cur, p, 5);
      p += 5;
      continue;
    }

    if (match(p, "char")) {
      cur = new_token(TK_KW, cur, p, 4);
      p += 4;
      continue;
    }

    if (match(p, "sizeof")) {
      cur = new_token(TK_KW, cur, p, 6);
      p += 6;
      continue;
    }

    if (isalpha(*p)) {
      char *old_p = p;
      p = read_ident(p);
      cur = new_token(TK_IDENT, cur, old_p, p - old_p);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *old_p = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - old_p;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  token = head.next;
}
