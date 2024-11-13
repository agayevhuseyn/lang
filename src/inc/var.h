#ifndef VAR_H
#define VAR_H

#include "ast.h"

typedef struct Var {
  char* name;
  union {
    AST* val;
    struct {
      AST* declaration;
      struct Var** vars;
      size_t size;
    } object;
  };
  bool is_defined;
  VariableType type;
} Var;

Var* init_var(bool is_object, char* name, AST* val, VariableType type, bool is_defined);

#endif
