#include "9cc.h"

bool equal(Token *tok, char *str)
{
  return strlen(str) == tok->len && memcmp(tok->str, str, tok->len) == 0;
}
