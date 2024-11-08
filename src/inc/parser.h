#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
  Token** tokens;
  size_t token_size;
  unsigned i;

  AST** function_declarations;
  size_t function_size;
} Parser;

Parser* init_parser(Lexer* lexer);

int parser_is_end(Parser* parser);
Token* parser_peek(Parser* parser); Token* parser_peek_offset(Parser* parser, int offset);
Token* parser_advance(Parser* parser);
Token* parser_eat(Parser* parser, TokenType type);
AST* parser_parse(Parser* parser);
AST* parser_parse_statement(Parser* parser);
AST* parser_parse_statements(Parser* parser);
AST* parser_parse_statement_in_block(Parser* parser, bool is_function, bool is_loop);
AST* parser_parse_statements_in_block(Parser* parser, bool is_function, bool is_loop);

AST* parser_parse_expr(Parser* parser);
AST* parser_parse_equality(Parser* parser);
AST* parser_parse_comparison(Parser* parser);
AST* parser_parse_term(Parser* parser);
AST* parser_parse_factor(Parser* parser);
AST* parser_parse_unary(Parser* parser);
AST* parser_parse_primary(Parser* parser);

AST* parser_parse_digit(Parser* parser);
AST* parser_parse_function_call(Parser* parser);
AST* parser_parse_string(Parser* parser);
AST* parser_parse_id(Parser* parser);
AST* parser_parse_variable_declaration(Parser* parser);
AST* parser_parse_variable(Parser* parser);
AST* parser_parse_bool(Parser* parser);
AST* parser_parse_if(Parser* parser, bool in_function, bool in_loop);
AST* parser_parse_while(Parser* parser, bool in_function);
AST* parser_parse_for(Parser* parser, bool in_function);
AST* parser_parse_function_declaration(Parser* parser);
AST* parser_parse_return(Parser* parser);
AST* parser_parse_skip(Parser* parser);
AST* parser_parse_stop(Parser* parser);
AST* parser_parse_include(Parser* parser);
AST* parser_parse_module_function_call(Parser* parser);

#endif
