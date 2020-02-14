#include "9cc.h"

void extend(Vector *vec, int count) {
  if (vec->len + count <= vec->size)
    return;
  vec->size *= 2;
  vec->data = realloc(vec->data, vec->size);
}

Vector *vec_new() {
  Vector *vec = malloc(sizeof(Vector));
  vec->len = 0;
  vec->size = 16;
  vec->data = malloc(sizeof(void *) * vec->size);
  return vec;
}

void vec_add(Vector *vec, void *item) {
  extend(vec, 1);
  vec->data[vec->len++] = item;
}

int vec_len(Vector *vec) {
  return vec->len;
}

void *vec_get(Vector *vec, int idx) {
  return vec->data[idx];
}
