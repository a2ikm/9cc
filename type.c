#include "9cc.h"

Type *int_type   = &(Type){ TYPE_INT,   DWORD_SIZE, NULL };
Type *short_type = &(Type){ TYPE_SHORT, WORD_SIZE,  NULL };
Type *char_type  = &(Type){ TYPE_CHAR,  BYTE_SIZE,  NULL };

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

bool is_integer(Type *type) {
  return type->kind == TYPE_INT || type->kind == TYPE_SHORT || type->kind == TYPE_CHAR;
}

Type *max_type_by_size(Type *ltype, Type *rtype) {
  return ltype->size > rtype->size ? ltype : rtype;
}
