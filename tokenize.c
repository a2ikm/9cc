#include "9cc.h"

Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

char *read_ident(char *p)
{
  if (isalpha(*p) || *p == '_')
  {
    p++;
    while (isalpha(*p) || isdigit(*p) || *p == '_')
    {
      p++;
    }
    return p;
  }
  else
  {
    return NULL;
  }
}

bool is_alnum(char c)
{
  return isalpha(c) || isdigit(c) || c == '_';
}

bool is_keyword(Token *tok)
{
  return equal(tok, "return") ||
         equal(tok, "if") ||
         equal(tok, "else") ||
         equal(tok, "while") ||
         equal(tok, "for") ||
         equal(tok, "sizeof") ||
         equal(tok, "long") ||
         equal(tok, "int") ||
         equal(tok, "short") ||
         equal(tok, "unsigned") ||
         equal(tok, "char");
}

void tokenize()
{
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (strncmp(p, "//", 2) == 0)
    {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    if (strncmp(p, "/*", 2) == 0)
    {
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
        *p == '&' || *p == ',' || *p == ';')
    {
      cur = new_token(TK_PUNC, cur, p++, 1);
      continue;
    }

    if (*p == '=')
    {
      if (*(p + 1) == '=')
      {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      }
      else
      {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '!' && *(p + 1) == '=')
    {
      cur = new_token(TK_PUNC, cur, p++, 2);
      p++;
      continue;
    }

    if (*p == '<')
    {
      if (*(p + 1) == '=')
      {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      }
      else
      {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '>')
    {
      if (*(p + 1) == '=')
      {
        cur = new_token(TK_PUNC, cur, p++, 2);
        p++;
        continue;
      }
      else
      {
        cur = new_token(TK_PUNC, cur, p++, 1);
        continue;
      }
    }

    if (*p == '"')
    {
      char *start = ++p;
      char *end = NULL;

      while (*p)
      {
        if (*p == '"')
        {
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

    if (*p == '\'')
    {
      char *start = ++p;

      p++;
      if (*p != '\'')
        error_at(start, "single quotation not found");

      cur = new_token(TK_NUM, cur, start, 1);
      cur->val = *start;

      p++;
      continue;
    }

    if (isalpha(*p))
    {
      char *old_p = p;
      p = read_ident(p);
      cur = new_token(TK_IDENT, cur, old_p, p - old_p);

      if (is_keyword(cur))
        cur->kind = TK_KW;

      continue;
    }

    if (isdigit(*p))
    {
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
