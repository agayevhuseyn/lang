#include "inc/var.h"

Var* init_var(char* name, AST* val, VariableType type, bool is_defined)
{
  Var* var = calloc(1, sizeof(Var));
  
  var->name = name;
  var->val = val;
  var->type = type;
  var->is_defined = is_defined;

  return var;
}
