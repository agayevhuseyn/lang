#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include <stdio.h>

typedef struct {
  char* name;
  void* handle;
  AST* (*(*functions)) (AST** args, size_t arg_size);
  char** function_names;
  size_t function_size;
} Module;

Module* init_module(char* name);
AST* module_function_call(Module* module, char* func_name, AST** args, size_t arg_size);

#endif
