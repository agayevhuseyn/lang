#ifndef VISITOR_H
#define VISITOR_H

#include "parser.h"
#include "var.h"
#include "scope.h"
#include "module.h"

typedef struct {
  Scope* global_scope;
  // function definitons
  AST** function_declarations;
  size_t function_size;
  // included modules
  Module** modules;
  size_t module_size;
} Visitor;

Visitor* init_visitor(Parser* parser);

void visitor_check_types(bool is_declared, Var* var, TokenType op, AST* var_val);

AST* visitor_visit(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_compound(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_int(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_float(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_string(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_bool(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_binary(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_unary(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_binary_bool(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_function(Visitor* visitor, Scope* scope, AST* f, AST* f_call);
AST* visitor_visit_function_call(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_variable_declaration(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_variable(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_variable_assign(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_if(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_else(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_while(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_for(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_return(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_return_val(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_skip(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_stop(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_include(Visitor* visitor, Scope* scope, AST* node);
AST* visitor_visit_module_function_call(Visitor* visitor, Scope* scope, AST* node);

#endif
