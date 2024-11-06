#ifndef VAR_H
#define VAR_H

#include "ast.h"

typedef struct {
  char* name;
  struct AST* val;
  bool is_defined;
  VariableType type;
} Var;

Var* init_var(char* name, AST* val, VariableType type, bool is_defined);

#endif
