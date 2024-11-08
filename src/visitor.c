#include "inc/visitor.h"
#include "inc/ast.h"
#include "inc/module.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Visitor* init_visitor(Parser* parser)
{
  Visitor* visitor = calloc(1, sizeof(Visitor));

  visitor->global_scope = init_scope();
  visitor->global_scope->is_global = true;
  visitor->function_declarations = parser->function_declarations;
  visitor->function_size = parser->function_size;

  return visitor;
}

static AST* visitor_error(char* msg)
{
  printf("Visitor-> Error: %s\n", msg);
  exit(1);
  return get_ast_noop();
}

static AST* builtin_write(Visitor* visitor, Scope* scope, AST** args, size_t arg_size)
{
  for (int i = 0; i < arg_size; i++) {
    AST* arg = visitor_visit(visitor, scope, args[i]);

    switch (arg->type) {
      case AST_STRING:
        printf("%s ", arg->string.val);
        break;
      case AST_INT:
        printf("%d ", arg->integer.val);
        break;
      case AST_FLOAT:
        printf("%f ", arg->floating.val);
        break;
      case AST_BOOL:
        printf("%s ", arg->boolean.val ? "true" : "false");
        break;
      default: {
        char msg[64]; sprintf(msg, "unexpected %d indexed arg at function write: '%s'", i, ast_name(arg->type));
        return visitor_error(msg);
      }
    }
  }
  printf("\n");

  return get_ast_noop();
}

static AST* builtin_read(Visitor* visitor, Scope* scope, AST** args, size_t arg_size)
{
  if (arg_size > 1) {
    char msg[64]; sprintf(msg, "function read: at most 1 argument, got %lu", arg_size);
    return visitor_error(msg);
  } else if (arg_size == 0) goto no_arg;

  AST* arg = visitor_visit(visitor, scope, args[0]);
  switch (arg->type) {
    case AST_STRING:
      printf("%s", arg->string.val);
      break;
    default: {
      char msg[64]; sprintf(msg, "unexpected %d indexed arg at function read: '%s'", 0, ast_name(arg->type));
      return visitor_error(msg);
    }
  }

  no_arg:

  char buffer[1024];
  fgets(buffer, sizeof(buffer), stdin);
  buffer[strlen(buffer) - 1] = '\0';
  AST* ast = init_ast(AST_STRING);
  ast->string.val = calloc(strlen(buffer) + 1, sizeof(char));
  strcpy(ast->string.val, buffer);
  return ast;
}

static AST* builtin_int(Visitor* visitor, Scope* scope, AST** args, size_t arg_size)
{
  if (arg_size != 1) {
    char msg[64]; sprintf(msg, "function int: expected 1 argument, got %lu", arg_size);
    return visitor_error(msg);
  }

  AST* arg = visitor_visit(visitor, scope, args[0]);

  switch (arg->type) {
    case AST_STRING: {
      AST* ret = init_ast(AST_INT);
      ret->integer.val = atoi(arg->string.val);
      return ret;
    }
    case AST_INT:
      return arg;
    case AST_FLOAT: {
      AST* ret = init_ast(AST_INT);
      ret->integer.val = (int)arg->floating.val;
      return ret;
    }
    case AST_BOOL: {
      AST* ret = init_ast(AST_INT);
      ret->integer.val = arg->boolean.val;
      return ret;
    }
    default: {
      char msg[64]; sprintf(msg, "unexpected arg at function int: '%s'", ast_name(arg->type));
      return visitor_error(msg);
    }
  }
}

static AST* builtin_float(Visitor* visitor, Scope* scope, AST** args, size_t arg_size)
{
  if (arg_size != 1) {
    char msg[64]; sprintf(msg, "function float: expected 1 argument, got %lu", arg_size);
    return visitor_error(msg);
  }

  AST* arg = visitor_visit(visitor, scope, args[0]);

  switch (arg->type) {
    case AST_STRING: {
      AST* ret = init_ast(AST_FLOAT);
      ret->floating.val = atof(arg->string.val);
      return ret;
    }
    case AST_INT: {
      AST* ret = init_ast(AST_FLOAT);
      ret->floating.val = (float)arg->integer.val;
      return ret;
    }
    case AST_FLOAT:
      return arg;
    case AST_BOOL: {
      AST* ret = init_ast(AST_FLOAT);
      ret->floating.val = arg->boolean.val;
      return ret;
    }
    default: {
      char msg[64]; sprintf(msg, "unexpected arg at function float: '%s'", ast_name(arg->type));
      return visitor_error(msg);
    }
  }
}

static AST* builtin_string(Visitor* visitor, Scope* scope, AST** args, size_t arg_size)
{
  if (arg_size != 1) {
    char msg[64]; sprintf(msg, "function string: expected 1 argument, got %lu", arg_size);
    return visitor_error(msg);
  }

  AST* arg = visitor_visit(visitor, scope, args[0]);

  switch (arg->type) {
    case AST_STRING:
      return arg;
    case AST_INT:
      return get_ast_noop();
    case AST_FLOAT:
      return get_ast_noop();
    case AST_BOOL: {
      AST* ret = init_ast(AST_STRING);
      ret->string.val = arg->boolean.val ? "true" : "false";
      return ret;
    }
    default: {
      char msg[64]; sprintf(msg, "unexpected arg at function string: '%s'", ast_name(arg->type));
      return visitor_error(msg);
    }
  }
}

void visitor_check_types(bool is_declared, Var* var, TokenType op, AST* var_val)
{
  if (!var->is_defined && op != TOKEN_ASSIGN) {
    char msg[96];
    sprintf(msg, "invalid '%s' operator for undefined '%s' variable", token_name(op), var->name);
    visitor_error(msg);
    return;
  }
  if (var_val->type == AST_INT && var->type == VAR_INT) {
    switch (op) {
      case TOKEN_ASSIGN: {
        if (is_declared || !var->is_defined) {
          AST* ast = init_ast(AST_INT);
          ast->integer.val = var_val->integer.val;
          var->val = ast;
          break;
        } else {
          var->val->integer.val = var_val->integer.val;
          break;
        }
      }
      case TOKEN_PLUSEQ:
        var->val->integer.val += var_val->integer.val;
        break;
      case TOKEN_MINUSEQ:
        var->val->integer.val -= var_val->integer.val;
        break;
      case TOKEN_MULEQ:
        var->val->integer.val *= var_val->integer.val;
        break;
      case TOKEN_DIVEQ:
        var->val->integer.val /= var_val->integer.val;
        break;
      case TOKEN_MODEQ:
        var->val->integer.val %= var_val->integer.val;
        break;
      default:
        break;
    }
  } else if (var_val->type == AST_FLOAT && var->type == VAR_FLOAT) {
    switch (op) {
      case TOKEN_ASSIGN: {
        if (is_declared || !var->is_defined) {
          AST* ast = init_ast(AST_FLOAT);
          ast->floating.val = var_val->floating.val;
          var->val = ast;
          break;
        } else {
          var->val->floating.val = var_val->floating.val;
          break;
        }
      }
      case TOKEN_PLUSEQ:
        var->val->floating.val += var_val->floating.val;
        break;
      case TOKEN_MINUSEQ:
        var->val->floating.val -= var_val->floating.val;
        break;
      case TOKEN_MULEQ:
        var->val->floating.val *= var_val->floating.val;
        break;
      case TOKEN_DIVEQ:
        var->val->floating.val /= var_val->floating.val;
        break;
      case TOKEN_MODEQ:
        char msg[64];
        sprintf(msg, "'%%' operator cannot be applied to floating values");
        visitor_error(msg);
        break;
      default:
        break;
    }
  }
  else if (var_val->type == AST_INT && var->type == VAR_FLOAT) {
    switch (op) {
      case TOKEN_ASSIGN: {
        if (is_declared || !var->is_defined) {
          AST* ast = init_ast(AST_FLOAT);
          ast->floating.val = (float)var_val->integer.val;
          var->val = ast;
          break;
        } else {
          var->val->floating.val = var_val->integer.val;
          break;
        }
      }
      case TOKEN_PLUSEQ:
        var->val->floating.val += var_val->integer.val;
        break;
      case TOKEN_MINUSEQ:
        var->val->floating.val -= var_val->integer.val;
        break;
      case TOKEN_MULEQ:
        var->val->floating.val *= var_val->integer.val;
        break;
      case TOKEN_DIVEQ:
        var->val->floating.val /= var_val->integer.val;
        break;
      case TOKEN_MODEQ:
        char msg[64];
        sprintf(msg, "'%%' operator cannot be applied to floating values");
        visitor_error(msg);
        break;
      default:
        break;
    }
  } else if (var_val->type == AST_FLOAT && var->type == VAR_INT) {
    switch (op) {
      case TOKEN_ASSIGN: {
        if (is_declared || !var->is_defined) {
          AST* ast = init_ast(AST_INT);
          ast->integer.val = (int)var_val->floating.val;
          var->val = ast;
          break;
        } else {
          var->val->integer.val = var_val->floating.val;
          break;
        }
      }
      case TOKEN_PLUSEQ:
        var->val->integer.val += var_val->floating.val;
        break;
      case TOKEN_MINUSEQ:
        var->val->integer.val -= var_val->floating.val;
        break;
      case TOKEN_MULEQ:
        var->val->integer.val *= var_val->floating.val;
        break;
      case TOKEN_DIVEQ:
        var->val->integer.val /= var_val->floating.val;
        break;
      case TOKEN_MODEQ:
        char msg[64];
        sprintf(msg, "'%%' operator cannot be applied to floating values");
        visitor_error(msg);
        break;
      default:
        break;
    }
  } else if (var_val->type == AST_STRING && var->type == VAR_STRING) {
    if (op != TOKEN_ASSIGN) {
      char msg[64];
      sprintf(msg, "strings can only get = operator");
      visitor_error(msg);
      return;
    }
    if (is_declared || !var->is_defined) {
      AST* ast = init_ast(AST_STRING);
      ast->string.val = var_val->string.val;
      var->val = ast;
    } else {
      var->val->string.val = var_val->string.val;
    }
  } else if (var_val->type == AST_BOOL && var->type == VAR_BOOL) {
    if (op != TOKEN_ASSIGN) {
      char msg[64];
      sprintf(msg, "bools can only get = operator");
      visitor_error(msg);
      return;
    }
    if (is_declared || !var->is_defined) {
      AST* ast = init_ast(AST_BOOL);
      ast->boolean.val = var_val->boolean.val;
      var->val = ast;
    } else {
      var->val->boolean.val = var_val->boolean.val;
    }
  } else {
    char msg[64];
    sprintf(msg, "variable '%s' type error: '%s', '%s'", var->name, var_type_name(var->type), ast_name(var_val->type));
    visitor_error(msg);
  }

  var->is_defined = true;
}

AST* visitor_visit(Visitor* visitor, Scope* scope, AST* node)
{
  switch (node->type) {
    case AST_TYPE_NOOP: return get_ast_noop();
    case AST_COMPOUND: return visitor_visit_compound(visitor, scope, node);
    case AST_INT: return visitor_visit_int(visitor, scope, node);
    case AST_FLOAT: return visitor_visit_float(visitor, scope, node);
    case AST_STRING: return visitor_visit_string(visitor, scope, node);
    case AST_BINARY: return visitor_visit_binary(visitor, scope, node);
//    case AST_FUNCTION_DECLARATION: return visitor_visit_function(visitor, scope, node);
    case AST_FUNCTION_CALL: return visitor_visit_function_call(visitor, scope, node);
    case AST_VARIABLE_DECLARATION: return visitor_visit_variable_declaration(visitor, scope, node);
    case AST_VARIABLE: return visitor_visit_variable(visitor, scope, node);
    case AST_VARIABLE_ASSIGN: return visitor_visit_variable_assign(visitor, scope, node);
    case AST_BOOL: return visitor_visit_bool(visitor, scope, node);
    case AST_IF: return visitor_visit_if(visitor, scope, node);
    case AST_ELSE: return visitor_visit_else(visitor, scope, node);
    case AST_WHILE: return visitor_visit_while(visitor, scope, node);
    case AST_FOR: return visitor_visit_for(visitor, scope, node);
    case AST_RETURN: return visitor_visit_return(visitor, scope, node);
    case AST_RETURN_VAL: return visitor_visit_return_val(visitor, scope, node);
    case AST_UNARY: return visitor_visit_unary(visitor, scope, node);
    case AST_SKIP: return visitor_visit_skip(visitor, scope, node);
    case AST_STOP: return visitor_visit_stop(visitor, scope, node);
    case AST_INCLUDE: return visitor_visit_include(visitor, scope, node);
    case AST_MODULE_FUNCTION_CALL: return visitor_visit_module_function_call(visitor, scope, node);
  }
}

AST* visitor_visit_compound(Visitor* visitor, Scope* scope, AST* node)
{
  for (int i = 0; i < node->compound.statement_size; i++) {
    AST* statement = node->compound.statements[i];
    AST* visited = visitor_visit(visitor, scope, statement);
    switch (visited->type) {
      case AST_RETURN_VAL:
      case AST_SKIP:
      case AST_STOP:
        return visited;
      default:
        break;
    }
  }
  return get_ast_noop();
}

AST* visitor_visit_int(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_float(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_string(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_bool(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_binary(Visitor* visitor, Scope* scope, AST* node)
{
  AST* ast = init_ast(AST_FLOAT);

  AST* bin_left = visitor_visit(visitor, scope, node->binary.left);
  AST* bin_right = visitor_visit(visitor, scope, node->binary.right);

  if (bin_left->type == AST_INT && bin_right->type == AST_INT) { 
    ast->type = AST_INT;
    int left = bin_left->integer.val, right = bin_right->integer.val;

    switch (node->binary.op) {
      case TOKEN_PLUS:
        ast->integer.val = left + right;
        break;
      case TOKEN_MINUS:
        ast->integer.val = left - right;
        break;
      case TOKEN_MUL:
        ast->integer.val = left * right;
        break;
      case TOKEN_DIV:
        ast->integer.val = left / right;
        break;
      case TOKEN_MOD:
        ast->integer.val = left % right;
        break;
      case TOKEN_EQ:
        return left == right ? get_ast_true() : get_ast_false();
      case TOKEN_NE:
        return left != right ? get_ast_true() : get_ast_false();
      case TOKEN_GT:
        return left > right ? get_ast_true() : get_ast_false();
      case TOKEN_GE:
        return left >= right ? get_ast_true() : get_ast_false();
      case TOKEN_LT:
        return left < right ? get_ast_true() : get_ast_false();
      case TOKEN_LE:
        return left <= right ? get_ast_true() : get_ast_false();
      default:
        return get_ast_noop();
    }
  } else if (bin_left->type == AST_FLOAT && bin_right->type == AST_FLOAT) { 
    float left = bin_left->floating.val, right = bin_right->floating.val;

    switch (node->binary.op) {
      case TOKEN_PLUS:
        ast->floating.val = left + right;
        break;
      case TOKEN_MINUS:
        ast->floating.val = left - right;
        break;
      case TOKEN_MUL:
        ast->floating.val = left * right;
        break;
      case TOKEN_DIV:
        ast->floating.val = left / right;
        break;
      case TOKEN_MOD: {
        char msg[64];
        sprintf(msg, "'%%' operator cannot be applied to floating values");
        return visitor_error(msg);
      }
      case TOKEN_EQ:
        return left == right ? get_ast_true() : get_ast_false();
      case TOKEN_NE:
        return left != right ? get_ast_true() : get_ast_false();
      case TOKEN_GT:
        return left > right ? get_ast_true() : get_ast_false();
      case TOKEN_GE:
        return left >= right ? get_ast_true() : get_ast_false();
      case TOKEN_LT:
        return left < right ? get_ast_true() : get_ast_false();
      case TOKEN_LE:
        return left <= right ? get_ast_true() : get_ast_false();
      default:
        return get_ast_noop();
    }
  } else if ((bin_left->type == AST_INT || bin_left->type == AST_FLOAT) &&
              bin_right->type == AST_INT || bin_right->type == AST_FLOAT) {
    float left = bin_left->type == AST_FLOAT ? bin_left->floating.val : (float)bin_left->integer.val,
          right = bin_right->type == AST_FLOAT ? bin_right->floating.val : (float)bin_right->integer.val;

    switch (node->binary.op) {
      case TOKEN_PLUS:
        ast->floating.val = left + right;
        break;
      case TOKEN_MINUS:
        ast->floating.val = left - right;
        break;
      case TOKEN_MUL:
        ast->floating.val = left * right;
        break;
      case TOKEN_DIV:
        ast->floating.val = left / right;
        break;
      case TOKEN_MOD: {
        char msg[64];
        sprintf(msg, "'%%' operator cannot be applied to floating values");
        return visitor_error(msg);
      }
      case TOKEN_EQ:
        return left == right ? get_ast_true() : get_ast_false();
      case TOKEN_NE:
        return left != right ? get_ast_true() : get_ast_false();
      case TOKEN_GT:
        return left > right ? get_ast_true() : get_ast_false();
      case TOKEN_GE:
        return left >= right ? get_ast_true() : get_ast_false();
      case TOKEN_LT:
        return left < right ? get_ast_true() : get_ast_false();
      case TOKEN_LE:
        return left <= right ? get_ast_true() : get_ast_false();
      default:
        return get_ast_noop();
    }
  } else if (bin_left->type == AST_STRING && bin_right->type == AST_STRING) {
    switch (node->binary.op) {
      case TOKEN_EQ:
        return strcmp(bin_left->string.val, bin_right->string.val) == 0 ? get_ast_true() : get_ast_false();
      case TOKEN_NE:
        return strcmp(bin_left->string.val, bin_right->string.val) != 0 ? get_ast_true() : get_ast_false();
      default: {
        char msg[64];
        sprintf(msg, "%s operator cannot be applied to string", token_name(node->binary.op));
        return visitor_error(msg);
      }
    } 
  } else if (bin_left->type == AST_BOOL && bin_right->type == AST_BOOL) {
    switch (node->binary.op) {
      case TOKEN_AND:
        return bin_left->boolean.val && bin_right->boolean.val ? get_ast_true() : get_ast_false();
      case TOKEN_OR:
        return bin_left->boolean.val || bin_right->boolean.val ? get_ast_true() : get_ast_false();
      case TOKEN_EQ:
        return bin_left->boolean.val == bin_right->boolean.val ? get_ast_true() : get_ast_false();
      case TOKEN_NE:
        return bin_left->boolean.val != bin_right->boolean.val ? get_ast_true() : get_ast_false();
      default: {
        char msg[64];
        sprintf(msg, "%s operator cannot be applied to bool", token_name(node->binary.op));
        return visitor_error(msg);
      }
    }
  } else {
    char msg[64];
    sprintf(msg, "unexpected types in binary: left: %s, right: %s", ast_name(bin_left->type), ast_name(bin_right->type));
    return visitor_error(msg);
  }

  return ast;
}

AST* visitor_visit_unary(Visitor* visitor, Scope* scope, AST* node)
{
  AST* expr = visitor_visit(visitor, scope, node->unary.expr);
  switch (node->unary.op) {
    case TOKEN_MINUS:
      if (expr->type == AST_INT) {
        AST* ast = init_ast(AST_INT);
        ast->integer.val = -expr->integer.val;
        return ast;
      } else if (expr->type == AST_FLOAT) {
        AST* ast = init_ast(AST_FLOAT);
        ast->floating.val = -expr->floating.val;
        return ast;
      } else {
        char msg[128];
        sprintf(msg, "'-' unary operator cannot be applied to %s", ast_name(expr->type));
        return visitor_error(msg);
      }
    case TOKEN_NOT:
      if (expr->type == AST_BOOL) {
        return expr->boolean.val ? get_ast_false() : get_ast_true();
      } else {
        char msg[128];
        sprintf(msg, "'not' unary operator cannot be applied to %s", ast_name(expr->type));
        return visitor_error(msg);
      }
    default:
      return get_ast_noop();
  }
}

AST* visitor_visit_function(Visitor* visitor, Scope* scope, AST* f, AST* f_call)
{
  if (f_call->function_call.arg_size != f->function_declaration.arg_size) {
    char msg[128];
    sprintf(msg, "function %s: expected %lu arg(s), but got %lu",
            f->function_declaration.name,
            f->function_declaration.arg_size,
            f_call->function_call.arg_size);
    return visitor_error(msg);
  }
  Scope* local_scope = init_scope();
  for (int i = 0; i < f->function_declaration.arg_size; i++) {
    VariableType var_type = f->function_declaration.arg_types[i];
    AST* var_val = visitor_visit(visitor, scope, f_call->function_call.args[i]);
    /*
    if (!((var_val->type == AST_INT && var_type == VAR_INT)||
        (var_val->type == AST_FLOAT && var_type == VAR_FLOAT)||
        (var_val->type == AST_STRING && var_type == VAR_STRING)||
        (var_val->type == AST_BOOL && var_type == VAR_BOOL))) {
      char msg[128];
      sprintf(msg, "'%s' function call arg error\n\targ %d: expected: %s, got: %s",
              f->function_declaration.name, i, var_type_name(var_type), ast_name(var_val->type));
      return visitor_error(msg);
    }
    */
    Var* var = init_var(f->function_declaration.args[i]->variable.name, var_val, var_type, true);

    visitor_check_types(true, var, TOKEN_ASSIGN, var_val);

    scope_add_var(local_scope, var);
  }
  scope = local_scope;

  AST* compound = f->function_declaration.compound;

  AST* return_val_unvisited = visitor_visit(visitor, scope, compound);
  AST* return_val = visitor_visit(visitor, scope, return_val_unvisited);
/*
  if (!f->function_declaration.has_return && return_val->type != AST_TYPE_NOOP) {
    char msg[128];
    sprintf(msg, "'%s' function return error: expected: nothing, got: %s",
            f->function_declaration.name, ast_name(return_val->type));
    return visitor_error(msg);
  } else */

  // dont get return val for now
//  return get_ast_noop();
  
  if (!f->function_declaration.has_return) {
    if (return_val->type != AST_TYPE_NOOP) {
      char msg[128];
      sprintf(msg, "'%s' function return error: expected no type, got: %s",
              f->function_declaration.name, ast_name(return_val->type));
      return visitor_error(msg);
    }
    return get_ast_noop();
  }
  
  VariableType return_type = f->function_declaration.return_type;
  if (!((return_val->type == AST_INT && return_type == VAR_INT)||
      (return_val->type == AST_FLOAT && return_type == VAR_FLOAT)||
      (return_val->type == AST_STRING && return_type == VAR_STRING)||
      (return_val->type == AST_BOOL && return_type == VAR_BOOL))) {
    char msg[128];
    sprintf(msg, "'%s' function return error: expected: %s, got: %s",
            f->function_declaration.name, var_type_name(return_type), ast_name(return_val->type));
    return visitor_error(msg);
  }
  return return_val;
}

AST* visitor_visit_function_call(Visitor* visitor, Scope* scope, AST* node)
{
  if (strcmp(node->function_call.name, "write") == 0) {
    return builtin_write(visitor, scope, node->function_call.args, node->function_call.arg_size);
  } else if (strcmp(node->function_call.name, "read") == 0) {
    return builtin_read(visitor, scope, node->function_call.args, node->function_call.arg_size);
  } else if (strcmp(node->function_call.name, "int") == 0) {
    return builtin_int(visitor, scope, node->function_call.args, node->function_call.arg_size);
  } else if (strcmp(node->function_call.name, "float") == 0) {
    return builtin_float(visitor, scope, node->function_call.args, node->function_call.arg_size);
  } else if (strcmp(node->function_call.name, "string") == 0) {
    return builtin_string(visitor, scope, node->function_call.args, node->function_call.arg_size);
  } else {
    for (int i = 0; i < visitor->function_size; i++) {
      AST* function = visitor->function_declarations[i];
      if (strcmp(node->function_call.name, function->function_declaration.name) == 0) {
        return visitor_visit_function(visitor, scope, function, node);
      }
    }
  }
  
  char msg[64]; sprintf(msg, "call to undeclared function named: '%s'", node->function_call.name);
  return visitor_error(msg);
}

AST* visitor_visit_variable_declaration(Visitor* visitor, Scope* scope, AST* node)
{
  for (int i = 0; i < node->variable_declaration.size; i++) {
    if (scope_is_var_declared(scope, node->variable_declaration.names[i])) {
      char msg[64];
      sprintf(msg, "variable '%s' has already been declared", node->variable_declaration.names[i]);
      return visitor_error(msg);
    }

    AST* var_val = node->variable_declaration.is_defined[i] ? visitor_visit(visitor, scope, node->variable_declaration.values[i]) : (void*)0;

    Var* var = init_var(node->variable_declaration.names[i], var_val, node->variable_declaration.type, node->variable_declaration.is_defined[i]);

    if (node->variable_declaration.is_defined[i]) {
      visitor_check_types(true, var, TOKEN_ASSIGN, var_val);
    }

    scope_add_var(scope, var);
  }

  return get_ast_noop();
}


AST* visitor_visit_variable(Visitor* visitor, Scope* scope, AST* node)
{
  do {
    Var* var = scope_get_var(scope, node->variable.name);
    if (var) {
      if (!var->is_defined) {
        char msg[96];
        sprintf(msg, "use of value of undefined variable: '%s'", node->variable.name);
        return visitor_error(msg);
      }
      return var->val;
    }
    if (scope->is_global) break;
    if (!scope->prev) {
      scope = visitor->global_scope;
    } else {
      scope = scope->prev;
    }
  } while (scope);

  char msg[64];
  sprintf(msg, "use of undeclared variable: '%s'", node->variable.name);
  return visitor_error(msg);
}

AST* visitor_visit_variable_assign(Visitor* visitor, Scope* scope, AST* node)
{
  Var* var;
  Scope* main_scope = scope;
  do {
    var = scope_get_var(scope, node->variable_assign.name);
    if (var) {
      goto assign;
    }
    if (scope->is_global) break;
    if (!scope->prev) {
      scope = visitor->global_scope;
    } else {
      scope = scope->prev;
    }
  } while (scope);
  char msg[64];
  sprintf(msg, "use of undeclared variable: '%s'", node->variable_assign.name);
  return visitor_error(msg);

  assign:

  TokenType op = node->variable_assign.op;
  AST* var_val = visitor_visit(visitor, main_scope, node->variable_assign.assign_val);
  visitor_check_types(false, var, op, var_val);
  
  return visitor_visit(visitor, scope, var->val);
}

AST* visitor_visit_if(Visitor* visitor, Scope* scope, AST* node)
{
  AST* cond = visitor_visit(visitor, scope, node->if_block.cond);
  if (cond->type != AST_BOOL) {
    char msg[64];
    sprintf(msg, "if requires bool but got: '%s'", ast_name(cond->type));
    return visitor_error(msg);
  }
  if (cond->boolean.val == true) {
    Scope* local_scope = init_scope();
    if (!scope->is_global) {
      local_scope->prev = scope;
    }
    scope = local_scope;
    return visitor_visit(visitor, scope, node->if_block.compound);
  } else {
    if (node->if_block.got_else == true) {
      return visitor_visit(visitor, scope, node->if_block.else_block);
    }
  }

  return get_ast_noop();
}

AST* visitor_visit_else(Visitor* visitor, Scope* scope, AST* node)
{
  Scope* local_scope = init_scope();
  if (!scope->is_global) {
    local_scope->prev = scope;
  }
  scope = local_scope;
  return visitor_visit(visitor, scope, node->else_block.compound);
}

AST* visitor_visit_while(Visitor* visitor, Scope* scope, AST* node)
{
  AST* cond = visitor_visit(visitor, scope, node->while_block.cond);
  if (cond->type != AST_BOOL) {
    char msg[128];
    sprintf(msg, "while requires bool but got: '%s'", ast_name(cond->type));
    return visitor_error(msg);
  }
  loop:
  if (cond->boolean.val == true) {
    Scope* local_scope = init_scope();
    if (!scope->is_global) {
      local_scope->prev = scope;
    }
    AST* visited = visitor_visit(visitor, local_scope, node->while_block.compound);
    switch (visited->type) {
      case AST_RETURN_VAL:
        return visited;
      case AST_STOP:
        return get_ast_noop();
      default:
        break;
    }
    cond = visitor_visit(visitor, scope, node->while_block.cond);
    goto loop;
  }
  
  return get_ast_noop();
}

AST* visitor_visit_for(Visitor* visitor, Scope* scope, AST* node)
{
  Scope* for_scope = init_scope();
  if (!scope->is_global) {
    for_scope->prev = scope;
  }
  scope = for_scope;

  // visit first
  if (node->for_block.has_first) {
    visitor_visit(visitor, scope, node->for_block.first);
  }

  // visit second
  AST* cond = node->for_block.has_second ? visitor_visit(visitor, scope, node->for_block.second) : get_ast_true();
  if (cond->type != AST_BOOL) {
    char msg[128];
    sprintf(msg, "for condition body requires bool but got: '%s'", ast_name(cond->type));
    return visitor_error(msg);
  }

  loop:
  if (cond->boolean.val == true) {
    Scope* local_scope = init_scope();
    if (!scope->is_global) {
      local_scope->prev = scope;
    }
    AST* visited = visitor_visit(visitor, local_scope, node->for_block.compound);
    switch (visited->type) {
      case AST_RETURN_VAL:
        return visited;
      case AST_STOP:
        return get_ast_noop();
      default:
        break;
    }
    // visit third after loop executed
    if (node->for_block.has_third) {
      visitor_visit(visitor, scope, node->for_block.third);
    }
    // and then check second again
    cond = node->for_block.has_second ? visitor_visit(visitor, scope, node->for_block.second) : get_ast_true();
    goto loop;
  }
  
  return get_ast_noop();
}

AST* visitor_visit_return(Visitor* visitor, Scope* scope, AST* node)
{
  AST* ast = init_ast(AST_RETURN_VAL);
  if (node->return_expr.is_empty_return) {
    ast->return_val.val = get_ast_noop();
    return ast;
  }
  ast->return_val.val = visitor_visit(visitor, scope, node->return_expr.expr);
  return ast;
}

AST* visitor_visit_return_val(Visitor* visitor, Scope* scope, AST* node)
{
  return node->return_val.val;
}

AST* visitor_visit_skip(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_stop(Visitor* visitor, Scope* scope, AST* node)
{
  return node;
}

AST* visitor_visit_include(Visitor* visitor, Scope* scope, AST* node)
{
  for (int i = 0; i < visitor->module_size; i++) {
    if (strcmp(visitor->modules[i]->name, node->include.module_name) == 0) {
      char msg[96];
      sprintf(msg, "module '%s' has already been included", node->include.module_name);
      return visitor_error(msg);
    }
  }
  visitor->module_size++;
  visitor->modules = realloc(visitor->modules, visitor->module_size * sizeof(Module*));
  visitor->modules[visitor->module_size - 1] = init_module(node->include.module_name);

  return get_ast_noop();
}

AST* visitor_visit_module_function_call(Visitor* visitor, Scope* scope, AST* node)
{
  for (int i = 0; i < visitor->module_size; i++) {
    if (strcmp(visitor->modules[i]->name, node->module_function_call.module_name) == 0) {
      AST* f_call = node->module_function_call.func;
      AST** args = calloc(f_call->function_call.arg_size, sizeof(AST*));
      for (int i = 0; i < f_call->function_call.arg_size; i++) {
        args[i] = visitor_visit(visitor, scope, f_call->function_call.args[i]);
      }
      return module_function_call(visitor->modules[i], f_call->function_call.name, args, f_call->function_call.arg_size);
    }
  }
  char msg[128];
  sprintf(msg, "undeclared module: '%s'", node->module_function_call.module_name);
  return visitor_error(msg);
}
