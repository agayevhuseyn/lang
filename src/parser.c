#include "inc/parser.h"
#include "inc/ast.h"
#include "inc/token.h"
#include <stdio.h>
#include <string.h>

Parser* init_parser(Lexer* lexer)
{
  Parser* parser = calloc(1, sizeof(Parser));

  parser->tokens = lexer->tokens;
  parser->token_size = lexer->token_size;
  parser->i = 0;

  return parser;
}

static AST* parser_error(Parser* parser, char* msg)
{
  printf("Parser-> Error at line: %u, %s\n", parser_peek(parser)->line, msg);
  exit(1);
  return get_ast_noop();
}

int parser_is_end(Parser* parser)
{
  return parser->i >= parser->token_size;
}

Token* parser_peek(Parser* parser)
{
  return parser->tokens[parser->i];
}

Token* parser_peek_offset(Parser* parser, int offset)
{
  int i = parser->i + offset;
  if (i >= parser->token_size) return parser->tokens[parser->token_size - 1];
  return parser->tokens[i];
}

Token* parser_advance(Parser* parser)
{ 
  return parser->tokens[parser->i++];
}

Token* parser_eat(Parser* parser, TokenType type)
{
  if (parser_peek(parser)->type == type)
    return parser_advance(parser);
  char msg[64]; sprintf(msg, "expected '%s' instead of '%s'", token_name(type), token_name(parser_peek(parser)->type));
  parser_error(parser, msg);
  return 0;
}

AST* parser_parse(Parser* parser)
{
  return parser_parse_statements(parser);
}

AST* parser_parse_statement(Parser* parser)
{
  switch (parser_peek(parser)->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_STRING:
    case TOKEN_BOOL:
      return parser_parse_variable_declaration(parser);
    case TOKEN_ID:
      return parser_parse_id(parser);
    case TOKEN_IF:
      return parser_parse_if(parser, false, false);
    case TOKEN_WHILE:
      return parser_parse_while(parser, false);
    case TOKEN_FOR:
      return parser_parse_for(parser, false);
    case TOKEN_FUNCTION:
      return parser_parse_function_declaration(parser);
    case TOKEN_EOF:
      return get_ast_noop();
    default: {
      char msg[64]; sprintf(msg, "unexpected token at parse statement: '%s'", token_name(parser_peek(parser)->type));
      parser_error(parser, msg);
      return get_ast_noop();
    }
  }
}

AST* parser_parse_statements(Parser* parser)
{
  AST* ast = init_ast(AST_COMPOUND);

  ast->compound.type = COMPOUND_ENTRY;
  ast->compound.statement_size = 1;
  ast->compound.statements = calloc(1, sizeof(AST*));
  ast->compound.statements[0] = parser_parse_statement(parser);

  while(!parser_is_end(parser) && parser_peek(parser)->type == TOKEN_NEWL) {
    parser_eat(parser, TOKEN_NEWL);

    ast->compound.statement_size++;
    ast->compound.statements = realloc(ast->compound.statements, ast->compound.statement_size * sizeof(AST*));
    ast->compound.statements[ast->compound.statement_size - 1] = parser_parse_statement(parser);
  }
  if (parser_peek(parser)->type != TOKEN_EOF) {
    char msg[64];
    sprintf(msg, "syntax error: encountered %s", token_name(parser_peek(parser)->type));
    return parser_error(parser, msg);
  }

  return ast;
}

AST* parser_parse_statement_in_block(Parser* parser, bool is_function, bool is_loop)
{
  switch (parser_peek(parser)->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_STRING:
    case TOKEN_BOOL:
      return parser_parse_variable_declaration(parser);
    case TOKEN_ID:
      return parser_parse_id(parser);
    case TOKEN_IF:
      return parser_parse_if(parser, is_function, is_loop);
    case TOKEN_WHILE:
      return parser_parse_while(parser, is_function);
    case TOKEN_FOR:
      return parser_parse_for(parser, is_function);
    case TOKEN_RETURN:
      if (is_function)
        return parser_parse_return(parser);
      {
        char msg[64]; sprintf(msg, "return not used in function");
        return parser_error(parser, msg);
      }
    case TOKEN_SKIP:
      if (is_loop)
        return parser_parse_skip(parser);
      {
        char msg[64]; sprintf(msg, "skip not used in loop");
        return parser_error(parser, msg);
      }
    case TOKEN_STOP:
      if (is_loop)
        return parser_parse_stop(parser);
      {
        char msg[64]; sprintf(msg, "stop not used in loop");
        return parser_error(parser, msg);
      }
    case TOKEN_DEDENT:
      return get_ast_noop();
    default: {
      char msg[64]; sprintf(msg, "unexpected token at parse statement in block: '%s'", token_name(parser_peek(parser)->type));
      return parser_error(parser, msg);
    }
  }
}

AST* parser_parse_statements_in_block(Parser* parser, bool is_function, bool is_loop)
{
  AST* ast = init_ast(AST_COMPOUND);

  ast->compound.statement_size = 1;
  ast->compound.statements = calloc(1, sizeof(AST*));
  ast->compound.statements[0] = parser_parse_statement_in_block(parser, is_function, is_loop);

  while(!parser_is_end(parser) && parser_peek(parser)->type == TOKEN_NEWL) {
    parser_eat(parser, TOKEN_NEWL);

    ast->compound.statement_size++;
    ast->compound.statements = realloc(ast->compound.statements, ast->compound.statement_size * sizeof(AST*));
    ast->compound.statements[ast->compound.statement_size - 1] = parser_parse_statement_in_block(parser, is_function, is_loop);
  }
  if (parser_peek(parser)->type != TOKEN_DEDENT) {
    char msg[64];
    sprintf(msg, "syntax error: encountered %s", token_name(parser_peek(parser)->type));
    return parser_error(parser, msg);
  }

  return ast;
}

AST* parser_parse_expr(Parser* parser)
{
  AST* left = parser_parse_equality(parser);

  TokenType t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOKEN_AND ||
        t == TOKEN_OR)) {
    
    AST* binary = init_ast(AST_BINARY);
    binary->binary.left = left;
    binary->binary.op = parser_advance(parser)->type;
    binary->binary.right = parser_parse_equality(parser);

    left = binary;

    t = parser_peek(parser)->type;
  }

  return left;
}

AST* parser_parse_equality(Parser* parser)
{
  AST* left = parser_parse_comparison(parser);

  TokenType t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOKEN_EQ ||
        t == TOKEN_NE)) {
    
    AST* binary = init_ast(AST_BINARY);
    binary->binary.left = left;
    binary->binary.op = parser_advance(parser)->type;
    binary->binary.right = parser_parse_comparison(parser);

    left = binary;

    t = parser_peek(parser)->type;
  }

  return left;
}

AST* parser_parse_comparison(Parser* parser)
{
  AST* left = parser_parse_term(parser);

  TokenType t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOKEN_GT ||
        t == TOKEN_GE ||
        t == TOKEN_LT ||
        t == TOKEN_LE)) {
    
    AST* binary = init_ast(AST_BINARY);
    binary->binary.left = left;
    binary->binary.op = parser_advance(parser)->type;
    binary->binary.right = parser_parse_term(parser);

    left = binary;

    t = parser_peek(parser)->type;
  }

  return left;
}

AST* parser_parse_term(Parser* parser)
{
  AST* left = parser_parse_factor(parser);

  TokenType t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOKEN_PLUS ||
        t == TOKEN_MINUS)) {
    
    AST* binary = init_ast(AST_BINARY);
    binary->binary.left = left;
    binary->binary.op = parser_advance(parser)->type;
    binary->binary.right = parser_parse_factor(parser);

    left = binary;

    t = parser_peek(parser)->type;
  }

  return left;
}

AST* parser_parse_factor(Parser* parser)
{
  AST* left = parser_parse_unary(parser);

  TokenType t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOKEN_MUL ||
        t == TOKEN_DIV ||
        t == TOKEN_MOD)) {
    
    AST* binary = init_ast(AST_BINARY);
    binary->binary.left = left;
    binary->binary.op = parser_advance(parser)->type;
    binary->binary.right = parser_parse_unary(parser);

    left = binary;

    t = parser_peek(parser)->type;
  }

  return left;
}

AST* parser_parse_unary(Parser* parser)
{
  switch (parser_peek(parser)->type) {
    case TOKEN_MINUS:
    case TOKEN_NOT: {
      AST* unary = init_ast(AST_UNARY);
      unary->unary.op = parser_advance(parser)->type;
      unary->unary.expr = parser_parse_primary(parser);
      return unary;
    }
    default:
      break;
  }

  return parser_parse_primary(parser);
}

AST* parser_parse_primary(Parser* parser)
{
  switch (parser_peek(parser)->type) {
    case TOKEN_STRING_VAL:
      return parser_parse_string(parser);
    case TOKEN_INT_VAL:
    case TOKEN_FLOAT_VAL:
      return parser_parse_digit(parser);
    case TOKEN_TRUE:
    case TOKEN_FALSE:
      return parser_parse_bool(parser);
    case TOKEN_ID:
      return parser_parse_variable(parser);
    case TOKEN_LPAREN:
      parser_advance(parser);
      {
        AST* ast = parser_parse_expr(parser);
        parser_eat(parser, TOKEN_RPAREN);
        return ast;
      }
    default: {
      error: {
        char msg[64]; sprintf(msg, "unexpected token at parse primary: '%s'", token_name(parser_peek(parser)->type));
        return parser_error(parser, msg);
      }
    }
  }
}

AST* parser_parse_digit(Parser* parser)
{
  AST* ast = calloc(1, sizeof(AST));

  switch (parser_peek(parser)->type) {
    case TOKEN_INT_VAL:
      ast->type = AST_INT;
      ast->integer.val = atoi(parser_advance(parser)->value);
      break;
    case TOKEN_FLOAT_VAL:
      ast->type = AST_FLOAT;
      ast->floating.val = atof(parser_advance(parser)->value);
      break;
    default: {
      char msg[64]; sprintf(msg, "unexpected token at parse digit: '%s'", token_name(parser_peek(parser)->type));
      parser_error(parser, msg);
    }
  }

  return ast;
}

AST* parser_parse_function_call(Parser* parser)
{
  AST* ast = init_ast(AST_FUNCTION_CALL);

  ast->function_call.name = parser_eat(parser, TOKEN_ID)->value;
  parser_eat(parser, TOKEN_LPAREN);

  ast->function_call.arg_size = 0;

  if (parser_peek(parser)->type != TOKEN_RPAREN) {
    ast->function_call.arg_size = 1;
    ast->function_call.args = calloc(1, sizeof(AST*));
    ast->function_call.args[0] = parser_parse_expr(parser);
  }

  while (!parser_is_end(parser) && parser_peek(parser)->type != TOKEN_RPAREN) {
    parser_eat(parser, TOKEN_COMMA);
    ast->function_call.arg_size++;
    ast->function_call.args = realloc(ast->function_call.args, ast->function_call.arg_size * sizeof(AST*));
    ast->function_call.args[ast->function_call.arg_size - 1] = parser_parse_expr(parser);
  }
  parser_eat(parser, TOKEN_RPAREN);

  return ast;
}

AST* parser_parse_string(Parser* parser)
{
  AST* ast = init_ast(AST_STRING);

  ast->string.val = parser_advance(parser)->value;

  return ast;
}

AST* parser_parse_id(Parser* parser)
{
  return parser_parse_variable(parser);
}

AST* parser_parse_variable_declaration(Parser* parser)
{
  AST* ast = init_ast(AST_VARIABLE_DECLARATION);
  VariableType var_type;
  switch (parser_advance(parser)->type) {
    case TOKEN_INT:
      var_type = VAR_INT;
      break;
    case TOKEN_FLOAT:
      var_type = VAR_FLOAT;
      break;
    case TOKEN_STRING:
      var_type = VAR_STRING;
      break;
    case TOKEN_BOOL:
      var_type = VAR_BOOL;
      break;
    default:
      return get_ast_noop();
  }

  ast->variable_declaration.type = var_type;

  loop: {
    char* name = parser_eat(parser, TOKEN_ID)->value;
    bool is_defined = false;

    if (parser_peek(parser)->type == TOKEN_ASSIGN) {
      parser_eat(parser, TOKEN_ASSIGN);
      is_defined = true;
    }

    AST* value = is_defined ? parser_parse_expr(parser) : (void*)0;

    ast->variable_declaration.size++;

    ast->variable_declaration.names = realloc(ast->variable_declaration.names, ast->variable_declaration.size * sizeof(AST*));
    ast->variable_declaration.values = realloc(ast->variable_declaration.values, ast->variable_declaration.size * sizeof(char*));
    ast->variable_declaration.is_defined = realloc(ast->variable_declaration.is_defined, ast->variable_declaration.size * sizeof(bool));

    ast->variable_declaration.names[ast->variable_declaration.size - 1] = name;
    ast->variable_declaration.values[ast->variable_declaration.size - 1] = value;
    ast->variable_declaration.is_defined[ast->variable_declaration.size - 1] = is_defined;

    if (parser_peek(parser)->type == TOKEN_COMMA) {
      parser_eat(parser, TOKEN_COMMA);
      goto loop;
    }
  }

  return ast;
}

AST* parser_parse_variable(Parser* parser)
{
  if (parser_peek_offset(parser, 1)->type == TOKEN_LPAREN) {
    return parser_parse_function_call(parser);
  }

  char* name = parser_eat(parser, TOKEN_ID)->value;
/*
  if (parser_peek(parser)->type == TOKEN_ASSIGN) {
    parser_advance(parser);
    AST* ast = init_ast(AST_VARIABLE_ASSIGN);
    ast->variable_assign.name = name;
    ast->variable_assign.assign_val = parser_parse_expr(parser);
    return ast;
  }
*/
  switch (parser_peek(parser)->type) {
    case TOKEN_ASSIGN:
    case TOKEN_PLUSEQ:
    case TOKEN_MINUSEQ:
    case TOKEN_MULEQ:
    case TOKEN_DIVEQ:
    case TOKEN_MODEQ: {
      AST* ast = init_ast(AST_VARIABLE_ASSIGN);
      ast->variable_assign.name = name;
      ast->variable_assign.op = parser_advance(parser)->type;
      ast->variable_assign.assign_val = parser_parse_expr(parser);
      return ast;
    }
    default:
      break;
  }
  AST* ast = init_ast(AST_VARIABLE);
  ast->variable.name = name;
  return ast;
}

AST* parser_parse_bool(Parser* parser)
{
  switch (parser_peek(parser)->type) {
    case TOKEN_TRUE:
      parser_advance(parser);
      return get_ast_true();
    case TOKEN_FALSE:
      parser_advance(parser);
      return get_ast_false();
    default:
      return get_ast_noop();
  }
}

AST* parser_parse_if(Parser* parser, bool in_function, bool in_loop)
{
  parser_eat(parser, TOKEN_IF);

  AST* ast = init_ast(AST_IF);
  ast->if_block.cond = parser_parse_expr(parser);
  parser_eat(parser, TOKEN_NEWL);
  parser_eat(parser, TOKEN_INDENT);
  ast->if_block.compound = parser_parse_statements_in_block(parser, in_function, in_loop);
  ast->if_block.compound->compound.type = COMPOUND_IF;
  parser_eat(parser, TOKEN_DEDENT);
// else
  if (parser_peek_offset(parser, 1)->type == TOKEN_ELSE) {
    parser_eat(parser, TOKEN_NEWL);
    parser_eat(parser, TOKEN_ELSE);
    if (parser_peek(parser)->type == TOKEN_IF) {
      ast->if_block.else_block = parser_parse_if(parser, in_function, in_loop);
    }
    else {
      parser_eat(parser, TOKEN_NEWL);
      parser_eat(parser, TOKEN_INDENT);
      AST* ast_else = init_ast(AST_ELSE);
      ast_else->else_block.compound = parser_parse_statements_in_block(parser, in_function, in_loop);
      ast_else->else_block.compound->compound.type = COMPOUND_IF;
      parser_eat(parser, TOKEN_DEDENT);

      ast->if_block.else_block = ast_else;
    }
    ast->if_block.got_else = true;
  }

  return ast;
}

AST* parser_parse_while(Parser* parser, bool in_function)
{
  parser_eat(parser, TOKEN_WHILE);

  AST* ast = init_ast(AST_WHILE);
  ast->while_block.cond = parser_parse_expr(parser);
  parser_eat(parser, TOKEN_NEWL);
  parser_eat(parser, TOKEN_INDENT);
  ast->while_block.compound = parser_parse_statements_in_block(parser, in_function, true);
  ast->while_block.compound->compound.type = COMPOUND_WHILE;
  parser_eat(parser, TOKEN_DEDENT);

  return ast;
}

AST* parser_parse_for(Parser* parser, bool in_function)
{
  parser_eat(parser, TOKEN_FOR);

  AST* ast = init_ast(AST_FOR);

  // first
  if (parser_peek(parser)->type == TOKEN_SEMICOLON) {
    ast->for_block.has_first = false;
    parser_eat(parser, TOKEN_SEMICOLON);
  } else {
    ast->for_block.has_first = true;
    switch (parser_peek(parser)->type) {
      case TOKEN_INT:
      case TOKEN_FLOAT:
      case TOKEN_STRING:
      case TOKEN_BOOL:
        ast->for_block.first = parser_parse_variable_declaration(parser);
        break;
      default:
        ast->for_block.first = parser_parse_expr(parser);
        break;
    }
    parser_eat(parser, TOKEN_SEMICOLON);
  }

  // second
  if (parser_peek(parser)->type == TOKEN_SEMICOLON) {
    ast->for_block.has_second = false;
    parser_eat(parser, TOKEN_SEMICOLON);
  } else {
    ast->for_block.has_second = true;
    ast->for_block.second = parser_parse_expr(parser);
    parser_eat(parser, TOKEN_SEMICOLON);
  }

  // third
  if (parser_peek(parser)->type == TOKEN_NEWL) {
    ast->for_block.has_third = false;
    goto empty_third;
  } else {
    ast->for_block.has_third = true;
    ast->for_block.third = parser_parse_expr(parser);
  }

  empty_third:
  parser_eat(parser, TOKEN_NEWL);
  parser_eat(parser, TOKEN_INDENT);
  ast->for_block.compound = parser_parse_statements_in_block(parser, in_function, true);
  ast->for_block.compound->compound.type = COMPOUND_FOR;
  parser_eat(parser, TOKEN_DEDENT);

  return ast;
}

AST* parser_parse_function_declaration(Parser* parser)
{
  AST* ast = init_ast(AST_FUNCTION_DECLARATION);

  parser_eat(parser, TOKEN_FUNCTION);
  
  switch (parser_peek(parser)->type) {
    case TOKEN_INT:
      ast->function_declaration.return_type = VAR_INT;
      break;
    case TOKEN_FLOAT:
      ast->function_declaration.return_type = VAR_FLOAT;
      break;
    case TOKEN_STRING:
      ast->function_declaration.return_type = VAR_STRING;
      break;
    case TOKEN_BOOL:
      ast->function_declaration.return_type = VAR_BOOL;
      break;
    default:
      goto no_type;
  }

  ast->function_declaration.has_return = true;
  parser_advance(parser);

  no_type:

  ast->function_declaration.name = parser_eat(parser, TOKEN_ID)->value;
  for (int i = 0; i < parser->function_size; i++) {
    if (strcmp(ast->function_declaration.name, parser->function_declarations[i]->function_declaration.name) == 0) {
      char msg[128];
      sprintf(msg,
              "function '%s' has already been declared",
              ast->function_declaration.name);
      parser_error(parser, msg);
    }
  }

  parser_eat(parser, TOKEN_LPAREN);

  if (parser_peek(parser)->type != TOKEN_RPAREN) {
    ast->function_declaration.arg_size = 1;
    ast->function_declaration.args = calloc(1, sizeof(AST*));
    ast->function_declaration.arg_types = calloc(1, sizeof(VariableType));

    VariableType type;
    switch (parser_advance(parser)->type) {
      case TOKEN_INT:
        type = VAR_INT;
        break;
      case TOKEN_FLOAT:
        type = VAR_FLOAT;
        break;
      case TOKEN_STRING:
        type = VAR_STRING;
        break;
      case TOKEN_BOOL:
        type = VAR_BOOL;
        break;
      default: {
        char msg[64];
        sprintf(msg,
                "unexpected token at parse function declaration arguments: '%s'",
                token_name(parser_peek_offset(parser, -1)->type));
        parser_error(parser, msg);
      }
    }

    AST* arg = init_ast(AST_VARIABLE);
    arg->variable.name = parser_eat(parser, TOKEN_ID)->value;
    ast->function_declaration.args[0] = arg;
    ast->function_declaration.arg_types[0] = type;
  }

  while (!parser_is_end(parser) && parser_peek(parser)->type != TOKEN_RPAREN) {
    parser_eat(parser, TOKEN_COMMA);

    ast->function_declaration.arg_size++;
    ast->function_declaration.args = realloc(ast->function_declaration.args,
                                            ast->function_declaration.arg_size * sizeof(AST*));
    ast->function_declaration.arg_types = realloc(ast->function_declaration.arg_types,
                                                 ast->function_declaration.arg_size * sizeof(VariableType));

    VariableType type;
    switch (parser_advance(parser)->type) {
      case TOKEN_INT:
        type = VAR_INT;
        break;
      case TOKEN_FLOAT:
        type = VAR_FLOAT;
        break;
      case TOKEN_STRING:
        type = VAR_STRING;
        break;
      case TOKEN_BOOL:
        type = VAR_BOOL;
        break;
      default: {
        char msg[64];
        sprintf(msg,
                "unexpected token at parse function declaration arguments: '%s'",
                token_name(parser_peek_offset(parser, -1)->type));
        parser_error(parser, msg);
      }
    }

    AST* arg = init_ast(AST_VARIABLE);
    arg->variable.name = parser_eat(parser, TOKEN_ID)->value;
    ast->function_declaration.args[ast->function_declaration.arg_size - 1] = arg;
    ast->function_declaration.arg_types[ast->function_declaration.arg_size - 1] = type;
  }

  parser_eat(parser, TOKEN_RPAREN); 
  parser_eat(parser, TOKEN_NEWL);
  parser_eat(parser, TOKEN_INDENT);
  ast->function_declaration.compound = parser_parse_statements_in_block(parser, true, false);
  ast->function_declaration.compound->compound.type = COMPOUND_FUNCTION;
  parser_eat(parser, TOKEN_DEDENT);
  
  parser->function_size++;
  parser->function_declarations = realloc(parser->function_declarations, parser->function_size * sizeof(AST*));
  parser->function_declarations[parser->function_size - 1] = ast;

  return get_ast_noop();
}

AST* parser_parse_return(Parser* parser)
{
  AST* ast = init_ast(AST_RETURN);
  parser_eat(parser, TOKEN_RETURN);
  
  ast->return_expr.is_empty_return = true;
  if (parser_peek(parser)->type != TOKEN_NEWL) {
    ast->return_expr.is_empty_return = false;
    ast->return_expr.expr = parser_parse_expr(parser);
  }

  return ast;
}

AST* parser_parse_skip(Parser* parser)
{
  parser_eat(parser, TOKEN_SKIP);
  return init_ast(AST_SKIP);
}

AST* parser_parse_stop(Parser* parser)
{
  parser_eat(parser, TOKEN_STOP);
  return init_ast(AST_STOP);
}
