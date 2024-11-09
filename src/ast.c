#include "inc/ast.h"

AST* AST_NOOP = (void*)0;
AST* AST_TRUE = (void*)0;
AST* AST_FALSE = (void*)0;

inline AST* init_ast(TypeAST type)
{
  AST* ast = calloc(1, sizeof(AST));
  ast->type = type;
  return ast;
}

inline AST* get_ast_noop()
{
  if (!AST_NOOP) {
    AST_NOOP = init_ast(AST_TYPE_NOOP);
  }
  return AST_NOOP;
}

inline AST* get_ast_true()
{
  if (!AST_TRUE) {
    AST_TRUE = init_ast(AST_BOOL);
    AST_TRUE->boolean.val = true;
  }
  return AST_TRUE;
}

inline AST* get_ast_false()
{
  if (!AST_FALSE) {
    AST_FALSE = init_ast(AST_BOOL);
    AST_FALSE->boolean.val = false;
  }
  return AST_FALSE;
}

char* ast_name(TypeAST type)
{
  switch (type) {
    case AST_COMPOUND: return "AST_COMPOUND";
    case AST_INT: return "AST_INT";
    case AST_BINARY: return "AST_BINARY";
    case AST_TYPE_NOOP: return "AST_TYPE_NOOP";
    case AST_FUNCTION_CALL: return "AST_FUNCTION_CALL";
    case AST_MODULE_FUNCTION_CALL: return "AST_MODULE_FUNCTION_CALL";
    case AST_FUNCTION_DECLARATION: return "AST_FUNCTION_DECLARATION";
    case AST_STRING: return "AST_STRING";
    case AST_VARIABLE_DECLARATION: return "AST_VARIABLE_DECLARATION";
    case AST_VARIABLE: return "AST_VARIABLE";
    case AST_FLOAT: return "AST_FLOAT";
    case AST_VARIABLE_ASSIGN: return "AST_VARIABLE_ASSIGN";
    case AST_BOOL: return "AST_BOOL";
    case AST_IF: return "AST_IF";
    case AST_ELSE: return "AST_ELSE";
    case AST_WHILE: return "AST_WHILE";
    case AST_FOR: return "AST_FOR";
    case AST_RETURN: return "AST_RETURN";
    case AST_RETURN_VAL: return "AST_RETURN_VAL";
    case AST_UNARY: return "AST_UNARY";
    case AST_SKIP: return "AST_SKIP";
    case AST_STOP: return "AST_STOP";
    case AST_INCLUDE: return "AST_INCLUDE";
    case AST_OBJECT_DECLARATION: return "AST_OBJECT_DECLARATION";
  }
}

char* var_type_name(VariableType type)
{
  switch (type) {
    case VAR_INT: return "VAR_INT";
    case VAR_FLOAT: return "VAR_FLOAT";
    case VAR_STRING: return "VAR_STRING";
    case VAR_BOOL: return "VAR_BOOL";
  }
}

char* compound_name(int type)
{
  switch (type) {
    case COMPOUND_ENTRY: return "COMPOUND_ENTRY";
    case COMPOUND_IF: return "COMPOUND_IF";
    case COMPOUND_WHILE: return "COMPOUND_WHILE";
    case COMPOUND_FOR: return "COMPOUND_FOR";
    case COMPOUND_FUNCTION: return "COMPOUND_FUNCTION";
  }
  return (void*)0;
}
