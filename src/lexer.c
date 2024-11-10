#include "inc/lexer.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Lexer* init_lexer(char* src)
{
  Lexer* lexer = calloc(1, sizeof(Lexer));

  lexer->src = src;
  lexer->src_size = strlen(src);
  lexer->i = 0;
  lexer->line = 1;
  lexer->tokens = (void*)0;
  lexer->token_size = 0;
  lexer->encountered_word = 0;
  lexer->prev_indent = 0;
  lexer->cur_indent = 0;

  return lexer;
}

static void lexer_error(Lexer* lexer, char* msg)
{
  printf("Lexer-> Error at line: %u, %s\n", lexer->line, msg);
  exit(1);
}

void lexer_collect_tokens(Lexer* lexer)
{
  while (!lexer_is_end(lexer)) {
    lexer_collect_token(lexer);
  }
  lexer_add_token(lexer, init_token(TOKEN_EOF, "\0", lexer->line));
}

void lexer_collect_token(Lexer* lexer)
{
  char c = lexer_advance(lexer);

  switch (c) {
    case '~':
      if (lexer_peek(lexer) == '~') {
        lexer_advance(lexer);
        lexer_skip_comment_block(lexer);
        break;
      }
      lexer_skip_comment_line(lexer);
      break;
    case ',':
      lexer_add_token(lexer, init_token(TOKEN_COMMA, ",", lexer->line));
      break;
    case '.':
      lexer_add_token(lexer, init_token(TOKEN_DOT, ".", lexer->line));
      break;
    case ';':
      lexer_add_token(lexer, init_token(TOKEN_SEMICOLON, ";", lexer->line));
      break;
    case '^':
      lexer_add_token(lexer, init_token(TOKEN_POW, "^", lexer->line));
      break;
    case '(':
      switch (lexer->tokens[lexer->token_size - 1]->type) {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_BOOL:
          lexer->tokens[lexer->token_size - 1]->type = TOKEN_ID;
          break;
        default:
          break;
      }
      lexer_add_token(lexer, init_token(TOKEN_LPAREN, "(", lexer->line));
      break;
    case ')':
      lexer_add_token(lexer, init_token(TOKEN_RPAREN, ")", lexer->line));
      break;
    case '{':
      lexer_add_token(lexer, init_token(TOKEN_LBRACE, "{", lexer->line));
      break;
    case '}':
      lexer_add_token(lexer, init_token(TOKEN_RBRACE, "}", lexer->line));
      break;
    case '\\':
      while ((c = lexer_advance(lexer)) != '\n') {
        if (c != ' ' && c != '\t') {
          if (c == '~') {
            lexer_skip_comment_line(lexer);
          } else {
            lexer_error(lexer, "nothing must come after '\\'");
          }
        }
      }
      lexer->line++;
      break;
    case '\n':
      if (!lexer->encountered_word) {
        lexer->cur_indent = 0;
      } else {
        lexer_add_token(lexer, init_token(TOKEN_NEWL, "\0", lexer->line)); 
      }
      lexer->line++;
      break;
    case '\t':
      if (!lexer->encountered_word) {
        lexer->cur_indent++;
      }
      break;
    case ' ':
      break;
    case '\"':
      lexer_get_string(lexer);
      break;
    case '+':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_PLUSEQ, "+=", lexer->line));
      }
      /*
      else if (lexer_peek(lexer) == '+') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_INCREMENT, "++", lexer->line));
      } */
      else {
        lexer_add_token(lexer, init_token(TOKEN_PLUS, "+", lexer->line));
      }
      break;
    case '-':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_MINUSEQ, "-=", lexer->line));
      }
      /*
      else if (lexer_peek(lexer) == '-') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_DECREMENT, "--", lexer->line));
      } */
      else {
        lexer_add_token(lexer, init_token(TOKEN_MINUS, "-", lexer->line));
      }
      break;
    case '*':
       if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_MULEQ, "*=", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_MUL, "*", lexer->line));
      }
      break;
    case '/':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_DIVEQ, "/=", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_DIV, "/", lexer->line));
      }
      break;
    case '%':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_MODEQ, "%=", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_MOD, "%", lexer->line));
      }
      break;
    case '=':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_EQ, "==", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_ASSIGN, "=", lexer->line));
      }
      break;
    case '!':
      if (lexer_peek(lexer) != '=') {
        lexer_error(lexer, "expected '=' after '!'");
      }
      lexer_advance(lexer);
      lexer_add_token(lexer, init_token(TOKEN_NE, "!=", lexer->line));
      break;
    case '>':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_GE, ">=", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_GT, ">", lexer->line));
      }
      break;
    case '<':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOKEN_LE, "<=", lexer->line));
      } else {
        lexer_add_token(lexer, init_token(TOKEN_LT, "<", lexer->line));
      }
      break;
    case '_':
      lexer_get_id(lexer);
      break;
    default:
      if (isalpha(c)) {
        lexer_get_id(lexer);
        break;
      } else if (isdigit(c)) {
        lexer_get_digit(lexer);
        break;
      } else {
        char msg[128]; sprintf(msg, "unrecognized character: '%c'", c);
        lexer_error(lexer, msg);
      }
      break;
  }
}

char lexer_advance(Lexer* lexer)
{
  return lexer->src[lexer->i++];
}

char lexer_peek(Lexer* lexer)
{
  return lexer->src[lexer->i];
}

char lexer_peek_offset(Lexer* lexer, int offset)
{
  int i = lexer->i + offset;
  return i >= lexer->src_size ? '\0' : lexer->src[i];
}

int lexer_is_end(Lexer* lexer)
{
  return lexer->i >= lexer->src_size; 
}

void lexer_get_id(Lexer* lexer)
{
  int len = 1;
  char* s = calloc(len + 1, sizeof(char));
  s[len - 1] = lexer_peek_offset(lexer, -1);
  s[len] = '\0';

  char c;
  while ((isalnum(c = lexer_peek(lexer)) || c == '_') && !lexer_is_end(lexer)) {
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';
    lexer_advance(lexer);
  }

  if (strcmp(s, "int") == 0) lexer_add_token(lexer, init_token(TOKEN_INT, s, lexer->line));
  else if (strcmp(s, "float") == 0) lexer_add_token(lexer, init_token(TOKEN_FLOAT, s, lexer->line));
  else if (strcmp(s, "string") == 0) lexer_add_token(lexer, init_token(TOKEN_STRING, s, lexer->line));
  else if (strcmp(s, "bool") == 0) lexer_add_token(lexer, init_token(TOKEN_BOOL, s, lexer->line));
  else if (strcmp(s, "object") == 0) lexer_add_token(lexer, init_token(TOKEN_OBJECT, s, lexer->line));
  else if (strcmp(s, "true") == 0) lexer_add_token(lexer, init_token(TOKEN_TRUE, s, lexer->line));
  else if (strcmp(s, "false") == 0) lexer_add_token(lexer, init_token(TOKEN_FALSE, s, lexer->line));
  else if (strcmp(s, "function") == 0) lexer_add_token(lexer, init_token(TOKEN_FUNCTION, s, lexer->line));
  else if (strcmp(s, "if") == 0) lexer_add_token(lexer, init_token(TOKEN_IF, s, lexer->line));
  else if (strcmp(s, "else") == 0) lexer_add_token(lexer, init_token(TOKEN_ELSE, s, lexer->line));
  else if (strcmp(s, "while") == 0) lexer_add_token(lexer, init_token(TOKEN_WHILE, s, lexer->line));
  else if (strcmp(s, "for") == 0) lexer_add_token(lexer, init_token(TOKEN_FOR, s, lexer->line));
  else if (strcmp(s, "return") == 0) lexer_add_token(lexer, init_token(TOKEN_RETURN, s, lexer->line));
  else if (strcmp(s, "skip") == 0) lexer_add_token(lexer, init_token(TOKEN_SKIP, s, lexer->line));
  else if (strcmp(s, "stop") == 0) lexer_add_token(lexer, init_token(TOKEN_STOP, s, lexer->line));
  else if (strcmp(s, "and") == 0) lexer_add_token(lexer, init_token(TOKEN_AND, s, lexer->line));
  else if (strcmp(s, "or") == 0) lexer_add_token(lexer, init_token(TOKEN_OR, s, lexer->line));
  else if (strcmp(s, "not") == 0) lexer_add_token(lexer, init_token(TOKEN_NOT, s, lexer->line));
  else if (strcmp(s, "include") == 0) lexer_add_token(lexer, init_token(TOKEN_INCLUDE, s, lexer->line));
  else lexer_add_token(lexer, init_token(TOKEN_ID, s, lexer->line));
}

void lexer_get_digit(Lexer* lexer)
{
  int len = 1;
  char* s = calloc(len + 1, sizeof(char));
  s[len - 1] = lexer_peek_offset(lexer, -1);
  s[len] = '\0';

  TokenType type = TOKEN_INT_VAL;
  char c;
  while (isdigit(c = lexer_peek(lexer)) && !lexer_is_end(lexer)) {
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';
    lexer_advance(lexer);
  }
  if (c == '.') {
    type = TOKEN_FLOAT_VAL;
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';
    lexer_advance(lexer);
    if (!isdigit(lexer_peek(lexer))) {
      lexer_error(lexer, "expected digit after '.'");
    }
    while (isdigit(c = lexer_peek(lexer)) && !lexer_is_end(lexer)) {
      len++;
      s = realloc(s, (len + 1) * sizeof(char));
      s[len - 1] = c;
      s[len] = '\0';
      lexer_advance(lexer);
    }
  }

  lexer_add_token(lexer, init_token(type, s, lexer->line));
}

void lexer_get_string(Lexer* lexer)
{
  int len = 0;
  char* s = (void*)0;

  char c;
  while ((c = lexer_peek(lexer)) != '\n' && c != '\"' && !lexer_is_end(lexer)) {
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';
    lexer_advance(lexer);
  }

  if (len == 0) {
    s = "\0";
  }
  if (lexer_peek(lexer) != '\"') {
    char msg[128];
    sprintf(msg, "unterminated string '%s'", s);
    lexer_error(lexer, msg);
  }
  lexer_advance(lexer); // for '\"'

  lexer_add_token(lexer, init_token(TOKEN_STRING_VAL, s, lexer->line));
}

void lexer_add_token(Lexer* lexer, Token* token)
{
  if (!lexer->encountered_word && token->type != TOKEN_NEWL && token->type != TOKEN_INDENT && token->type != TOKEN_DEDENT) {
    if (lexer->cur_indent > lexer->prev_indent) {
      int times = lexer->cur_indent - lexer->prev_indent;
      for (int i = 0; i < times; i++) {
        lexer_add_token(lexer, init_token(TOKEN_INDENT, "\0", lexer->line));
      }
    } else if (lexer->cur_indent < lexer->prev_indent) {
      int times = lexer->prev_indent - lexer->cur_indent;
      for (int i = 0; i < times; i++) {
        lexer_add_token(lexer, init_token(TOKEN_DEDENT, "\0", lexer->line));
        lexer_add_token(lexer, init_token(TOKEN_NEWL, "\0", lexer->line));
      }
    }
    lexer->encountered_word = 1;
  } 
  else if (lexer->encountered_word && token->type == TOKEN_NEWL) {
    lexer->encountered_word = 0;
    lexer->prev_indent = lexer->cur_indent;
    lexer->cur_indent = 0;
  }

  lexer->token_size++;
  lexer->tokens = realloc(lexer->tokens, lexer->token_size * sizeof(Token*));
  lexer->tokens[lexer->token_size - 1] = token;
}

void lexer_skip_comment_line(Lexer* lexer)
{
  while (!lexer_is_end(lexer) && lexer_peek(lexer) != '\n') {
    lexer_advance(lexer);
  }
}

void lexer_skip_comment_block(Lexer* lexer)
{
  while (!lexer_is_end(lexer)) {
    if (lexer_peek(lexer) == '~' && lexer_peek_offset(lexer, 1) == '~') {
      lexer_advance(lexer);
      lexer_advance(lexer);
      break;
    }
    if (lexer_advance(lexer) == '\n') {
      lexer->line++;
    }
  }
  if (lexer_is_end(lexer)) {
    lexer_error(lexer, "unclosed comment block (~~)");
  }
}
