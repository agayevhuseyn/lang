#include "inc/scope.h"
#include <string.h>

Scope* init_scope()
{
  Scope* scope = calloc(1, sizeof(Scope));

  scope->vars = (void*)0;
  scope->var_size = 0;
  scope->is_global = false;
  scope->prev = (void*)0;

  return scope;
}

void print_scope(Scope* scope)
{
  for (int i = 0; i < scope->var_size; i++) {
    #include <stdio.h>
    printf("var name: %s\n", scope->vars[i]->name);
  }
}

void scope_add_var(Scope* scope, Var* var)
{
  scope->var_size++;
  scope->vars = realloc(scope->vars, scope->var_size * sizeof(Var*));
  scope->vars[scope->var_size - 1] = var;
}

bool scope_is_var_declared(Scope* scope, char* name)
{
  for (int i = 0; i < scope->var_size; i++) {
    Var* var = scope->vars[i];
    if (strcmp(var->name, name) == 0) {
      return true;
    }
  }
  return false;
}

Var* scope_get_var(Scope* scope, char* name)
{
  for (int i = 0; i < scope->var_size; i++) {
    Var* var = scope->vars[i];
    if (strcmp(var->name, name) == 0) {
      return var;
    }
  }
  return (void*)0;
}

void scope_free(Scope* scope)
{
  while (scope) {
    for (int i = 0; i < scope->var_size; i++) {
      free(scope->vars[i]);
    }
    free(scope->vars);
    free(scope);
    scope = scope->prev;
  }
}
