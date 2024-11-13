#include "inc/var.h"

Var* init_var(bool is_object, char* name, AST* val, VariableType type, bool is_defined)
{
  Var* var = calloc(1, sizeof(Var));
  
  var->name = name;
  var->type = type;
  var->is_defined = is_defined;
  
  if (!is_object) {
    var->val = val;
  } else {
    var->object.declaration = val;
    var->object.size = 0;
    var->object.vars = (void*)0;
  }

  return var;
}
