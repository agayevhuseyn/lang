#include "inc/token.h"

Token* init_token(TokenType type, char* value, unsigned line)
{
  Token* token = calloc(1, sizeof(Token));

  token->type = type;
  token->value = value;
  token->line = line;

  return token;
}

char* token_name(TokenType type)
{
  switch (type) {
    case TOKEN_INT: return "TOKEN_INT";
    case TOKEN_FLOAT: return "TOKEN_FLOAT";
    case TOKEN_STRING: return "TOKEN_STRING";
    case TOKEN_BOOL: return "TOKEN_BOOL";
    case TOKEN_OBJECT: return "TOKEN_OBJECT";
//  ----------------
    case TOKEN_INT_VAL: return "TOKEN_INT_VAL";
    case TOKEN_FLOAT_VAL: return "TOKEN_FLOAT_VAL";
    case TOKEN_STRING_VAL: return "TOKEN_STRING_VAL";
    case TOKEN_TRUE: return "TOKEN_TRUE";
    case TOKEN_FALSE: return "TOKEN_FALSE";
//  ----------------
    case TOKEN_PLUS: return "TOKEN_PLUS";
    case TOKEN_MINUS: return "TOKEN_MINUS";
    case TOKEN_MUL: return "TOKEN_MUL";
    case TOKEN_DIV: return "TOKEN_DIV";
    case TOKEN_MOD: return "TOKEN_MOD";
    case TOKEN_POW: return "TOKEN_POW";
//  ----------------
    case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
    case TOKEN_PLUSEQ: return "TOKEN_PLUSEQ";
    case TOKEN_MINUSEQ: return "TOKEN_MINUSEQ";
    case TOKEN_MULEQ: return "TOKEN_MULEQ";
    case TOKEN_DIVEQ: return "TOKEN_DIVEQ";
    case TOKEN_MODEQ: return "TOKEN_MODEQ";
//  ----------------
    case TOKEN_EQ: return "TOKEN_EQ";
    case TOKEN_NE: return "TOKEN_NE";
    case TOKEN_GT: return "TOKEN_GT";
    case TOKEN_GE: return "TOKEN_GE";
    case TOKEN_LT: return "TOKEN_LT";
    case TOKEN_LE: return "TOKEN_LE";
//  ----------------
    case TOKEN_FUNCTION: return "TOKEN_FUNCTION";
    case TOKEN_IF: return "TOKEN_IF";
    case TOKEN_ELSE: return "TOKEN_ELSE";
    case TOKEN_WHILE: return "TOKEN_WHILE";
    case TOKEN_FOR: return "TOKEN_FOR";
    case TOKEN_RETURN: return "TOKEN_RETURN";
    case TOKEN_SKIP: return "TOKEN_SKIP";
    case TOKEN_STOP: return "TOKEN_STOP";
//  ----------------
    case TOKEN_NEWL: return "TOKEN_NEWL";
    case TOKEN_INDENT: return "TOKEN_INDENT";
    case TOKEN_DEDENT: return "TOKEN_DEDENT";
    case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
//  ----------------
    case TOKEN_ID: return "TOKEN_ID";
    case TOKEN_LPAREN: return "TOKEN_LPAREN";
    case TOKEN_RPAREN: return "TOKEN_RPAREN";
    case TOKEN_COMMA: return "TOKEN_COMMA";
    case TOKEN_INCLUDE: return "TOKEN_INCLUDE";
    case TOKEN_DOT: return "TOKEN_DOT";
    case TOKEN_LBRACE: return "TOKEN_LBRACE";
    case TOKEN_RBRACE: return "TOKEN_RBRACE";
//  ----------------
    case TOKEN_AND: return "TOKEN_AND";
    case TOKEN_OR: return "TOKEN_OR";
    case TOKEN_NOT: return "TOKEN_NOT";
//  ----------------
    case TOKEN_INCREMENT: return "TOKEN_INCREMENT";
    case TOKEN_DECREMENT: return "TOKEN_DECREMENT";
//  ----------------
    case TOKEN_EOF: return "TOKEN_EOF";
//  ----------------
  }
}
