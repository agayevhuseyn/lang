#include <stdio.h>
#include "inc/ast.h"
#include "inc/io.h"
#include "inc/lexer.h"
#include "inc/parser.h"
#include "inc/visitor.h"

static void print_tokens(Token** tokens, size_t size)
{
  for (int i = 0; i < size; i++) {
    Token* t = tokens[i];
    printf("type: %s, value: %s, line: %u\n", token_name(t->type), t->value, t->line);
  }
}

static void print_ast(AST* root)
{
  switch (root->type) {
    case AST_UNARY:
      printf("%s, op: %s, value:\n\t", ast_name(root->type), token_name(root->unary.op));
      print_ast(root->unary.expr);
      break;
    case AST_INCLUDE:
      printf("%s, module name: %s\n", ast_name(root->type), root->include.module_name);
      break;
    case AST_MODULE_FUNCTION_CALL:
      printf("%s, module name: %s\n\t", ast_name(root->type), root->module_function_call.module_name);
      print_ast(root->module_function_call.func);
      break;
    case AST_RETURN:
      printf("%s", ast_name(root->type));
      if (!root->return_expr.is_empty_return) {
        printf("expr:\n");
        print_ast(root->return_expr.expr);
      }
      break;
    case AST_COMPOUND:
      printf("%s, type: %s, size: %lu\n", ast_name(root->type),
                                          compound_name(root->compound.type),
                                          root->compound.statement_size);
      for (int i = 0; i < root->compound.statement_size; i++) {
        print_ast(root->compound.statements[i]);
      }
      break;
    case AST_FUNCTION_DECLARATION:
      printf("%s, name: %s, arg size: %lu\n", ast_name(root->type),
                                              root->function_declaration.name,
                                              root->function_declaration.arg_size);
      for (int i = 0; i < root->function_declaration.arg_size; i++) {
        printf("arg %d: type: %s, name: %s\n", i,
               var_type_name(root->function_declaration.arg_types[i]),
               root->function_declaration.args[i]->variable.name);
      }
      print_ast(root->function_declaration.compound);
      break;
    case AST_IF:
      printf("%s, size: %lu\n", ast_name(root->type), root->if_block.compound->compound.statement_size);
      printf("\tcond: %s\n", ast_name(root->if_block.cond->type)); print_ast(root->if_block.cond);
      print_ast(root->if_block.compound);
      if (root->if_block.got_else) {
        print_ast(root->if_block.else_block);
      }
      break;
    case AST_WHILE:
      printf("%s, size: %lu\n", ast_name(root->type), root->while_block.compound->compound.statement_size);
      printf("\tcond: %s\n", ast_name(root->while_block.cond->type)); print_ast(root->while_block.cond);
      print_ast(root->while_block.compound);
      break;
    case AST_FOR:
      printf("%s, size: %lu\n", ast_name(root->type), root->for_block.compound->compound.statement_size);
      if (root->for_block.has_first) {
        printf("\tfirst: %s\n", ast_name(root->for_block.first->type)); print_ast(root->for_block.first);
      }
      if (root->for_block.has_second) {
        printf("\tsecond: %s\n", ast_name(root->for_block.second->type)); print_ast(root->for_block.second);
      }
      if (root->for_block.has_third) {
        printf("\tthird: %s\n", ast_name(root->for_block.third->type)); print_ast(root->for_block.third);
      }
      print_ast(root->for_block.compound);
      break;
    case AST_ELSE:
      printf("%s, size: %lu\n", ast_name(root->type), root->else_block.compound->compound.statement_size);
      print_ast(root->else_block.compound);
      break;
    case AST_INT:
      printf("%s, value: %d\n", ast_name(root->type), root->integer.val);
      break;
    case AST_FLOAT:
      printf("%s, value: %f\n", ast_name(root->type), root->floating.val);
      break;
    case AST_STRING:
      printf("%s, value: %s\n", ast_name(root->type), root->string.val);
      break;
    case AST_BOOL:
      printf("%s, value: %s\n", ast_name(root->type), root->boolean.val ? "true" : "false");
      break;
    case AST_BINARY:
      printf("%s, operator: %s\n", ast_name(root->type), token_name(root->binary.op));
      printf("%s left: ", token_name(root->binary.op)); print_ast(root->binary.left);
        printf("%s right: ", token_name(root->binary.op)); print_ast(root->binary.right);
      break;
    case AST_SKIP:
    case AST_STOP:
    case AST_TYPE_NOOP:
      printf("%s\n", ast_name(root->type));
      break;
    case AST_FUNCTION_CALL:
      printf("%s, name: %s, arg size: %lu\n", ast_name(root->type), root->function_call.name, root->function_call.arg_size);
      printf("args:\n");
      for (int i = 0; i < root->function_call.arg_size; i++) {
        printf("%d ", i); print_ast(root->function_call.args[i]);
      }
      break;
    case AST_VARIABLE_DECLARATION:
      printf("%s, size: %lu, type: %s\n",
             ast_name(root->type),
             root->variable_declaration.size,
             var_type_name(root->variable_declaration.type));
      for (int i = 0; i < root->variable_declaration.size; i++) {
        printf("%s varname: %s, value:\n\t", ast_name(root->type), root->variable_declaration.names[i]);
        print_ast(root->variable_declaration.values[i]);
      }
      break;
    case AST_VARIABLE:
      printf("%s, name: %s\n", ast_name(root->type), root->variable.name);
      break;
    case AST_VARIABLE_ASSIGN:
      printf("%s, name: %s, op: %s\n", ast_name(root->type), root->variable_assign.name, token_name(root->variable_assign.op));
      printf("%s value:\n\t", ast_name(root->type));
      print_ast(root->variable_assign.assign_val);
      break;
  }
}

int main(int argc, char** argv)
{
//  printf("%d\n", AST_TRUE->boolean.val);
  char* src = read_file(argv[1]);
  if (src == (void*)0) return -1;
  Lexer* lexer = init_lexer(src);
  lexer_collect_tokens(lexer);
//                print_tokens(lexer->tokens, lexer->token_size);

  Parser* parser = init_parser(lexer);
  AST* root = parser_parse(parser);
//  print_ast(root);

  Visitor* visitor = init_visitor(parser);
  visitor_visit(visitor, visitor->global_scope, root);
  return 0;
}
