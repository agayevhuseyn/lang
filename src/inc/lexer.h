#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
  char* src;
  size_t src_size;
  unsigned i;
  unsigned line;
  unsigned prev_indent, cur_indent;
  int encountered_word;
  Token** tokens;
  size_t token_size;
} Lexer;

Lexer* init_lexer(char* src);

void lexer_collect_tokens(Lexer* lexer);
void lexer_collect_token(Lexer* lexer);
char lexer_advance(Lexer* lexer);
char lexer_peek(Lexer* lexer);
char lexer_peek_offset(Lexer* lexer, int offset);
int lexer_is_end(Lexer* lexer);
void lexer_get_id(Lexer* lexer);
void lexer_get_digit(Lexer* lexer);
void lexer_get_string(Lexer* lexer);
void lexer_add_token(Lexer* lexer, Token* token);
void lexer_skip_comment_line(Lexer* lexer);
void lexer_skip_comment_block(Lexer* lexer);

#endif
