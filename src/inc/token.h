#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

typedef enum {

  TOKEN_INT, TOKEN_FLOAT, TOKEN_STRING, TOKEN_BOOL, TOKEN_OBJECT, // data type
  TOKEN_INT_VAL, TOKEN_FLOAT_VAL, TOKEN_STRING_VAL, TOKEN_TRUE, TOKEN_FALSE, // value
  TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV, TOKEN_MOD, TOKEN_POW, // operator
  TOKEN_ASSIGN, TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_MULEQ, TOKEN_DIVEQ, TOKEN_MODEQ, // assignment operator
  TOKEN_EQ, TOKEN_NE, TOKEN_GT, TOKEN_GE, TOKEN_LT, TOKEN_LE, // comparison
  TOKEN_FUNCTION, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, TOKEN_RETURN, TOKEN_SKIP, TOKEN_STOP, // block
  TOKEN_NEWL, TOKEN_INDENT, TOKEN_DEDENT, TOKEN_SEMICOLON, // statement
  TOKEN_ID, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_COMMA, TOKEN_INCLUDE, TOKEN_DOT, TOKEN_LBRACE, TOKEN_RBRACE, // misc
  TOKEN_AND, TOKEN_OR, TOKEN_NOT, // bool
  TOKEN_INCREMENT, TOKEN_DECREMENT, // unary

  TOKEN_EOF
} _TokenType;

typedef struct {
  _TokenType type;
  char* value;
  unsigned line;
} Token;

Token* init_token(_TokenType type, char* value, unsigned line);
char* token_name(_TokenType type);

#endif
