#ifndef SCOPE_H
#define SCOPE_H

#include "var.h"

typedef struct Scope {
  Var** vars;
  size_t var_size;
  bool is_global;
  struct Scope* prev;
} Scope;

Scope* init_scope();

void print_scope(Scope* scope);

void scope_add_var(Scope* scope, Var* var);
bool scope_is_var_declared(Scope* scope, char* name);
Var* scope_get_var(Scope* scope, char* name);
void scope_free(Scope* scope);

#endif
