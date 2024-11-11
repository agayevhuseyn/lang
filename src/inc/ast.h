#ifndef AST_H
#define AST_H

#include "token.h"
#include <stdbool.h>

typedef enum {
  VAR_INT,
  VAR_FLOAT,
  VAR_STRING,
  VAR_BOOL,
} VariableType;

typedef enum {
  AST_TYPE_NOOP,
  AST_COMPOUND,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_BOOL,
  AST_BINARY,
  AST_UNARY,
  AST_VARIABLE_DECLARATION,
  AST_VARIABLE_ASSIGN,
  AST_VARIABLE,
  AST_FUNCTION_DECLARATION,
  AST_FUNCTION_CALL,
  AST_IF,
  AST_ELSE,
  AST_WHILE,
  AST_FOR,
  AST_RETURN,
  AST_RETURN_VAL,
  AST_SKIP,
  AST_STOP,
  AST_INCLUDE,
  AST_MODULE_FUNCTION_CALL,
  AST_OBJECT_DECLARATION,
} TypeAST;

typedef struct AST {
  TypeAST type;
  bool is_return;
  union {
    struct {
      struct AST** statements;
      size_t statement_size;
      enum {
        COMPOUND_ENTRY,
        COMPOUND_IF,
        COMPOUND_WHILE,
        COMPOUND_FOR,
        COMPOUND_FUNCTION,
      } type;
      struct AST* return_val;
    } compound;

    struct {
      int val;
    } integer;

    struct {
      float val;
    } floating;

    struct {
      char* val;
    } string;

    struct {
      bool val;
    } boolean;

    struct {
      TokenType op;
      struct AST* left;
      struct AST* right;
    } binary;

    struct {
      TokenType op;
      struct AST* expr;
    } unary;

    struct {
      char** names;
      struct AST** values;
      bool* is_defined;
      size_t size;
      VariableType type;
    } variable_declaration;

    struct {
      char* name;
      struct AST* assign_val;
      TokenType op;
    } variable_assign;

    struct {
      char* name;
    } variable;

    struct {
      char* name;
      bool has_return;
      VariableType return_type;
      struct AST** args;
      VariableType* arg_types;
      size_t arg_size;
      struct AST* compound;
    } function_declaration;

    struct {
      char* name;
      struct AST** args;
      size_t arg_size;
    } function_call;

    struct {
      char* module_name;
      struct AST* func;
    } module_function_call;

    struct {
      struct AST* cond;
      struct AST* compound;
      struct AST* else_block;
      bool got_else;
    } if_block;

    struct {
      struct AST* compound;
    } else_block;

    struct {
      struct AST* cond;
      struct AST* compound; } while_block;

    struct {
      bool has_first, has_second, has_third;
      struct AST *first, *second, *third;
      struct AST* compound;
    } for_block;

    struct {
      bool is_empty_return;
      struct AST* expr;
    } return_expr;

    struct {
      struct AST* val;
    } return_val;

    struct {
      char* module_name;
      bool is_alias;
      char* module_alias_name;
    } include;

    struct {
      char* name;
      VariableType* field_types;
      char** field_names;
      size_t field_size;
    } object_declaration;
  };
} AST;

AST* init_ast(TypeAST type);
AST* get_ast_noop();
AST* get_ast_true();
AST* get_ast_false();

char* ast_name(TypeAST type);
char* var_type_name(VariableType type);
char* compound_name(int type);

#endif
