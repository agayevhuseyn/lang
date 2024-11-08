#include "inc/module.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

static void module_error()
{
  fprintf(stderr, "Module-> error: %s\n", dlerror());
  exit(1);
}

Module* init_module(char* name)
{
  Module* module = calloc(1, sizeof(Module));

  module->name = name;
  char path[32];
  sprintf(path, "./%s.so", name);
  module->handle = dlopen(path, RTLD_LAZY);
  if (!module->handle) {
    module_error();
  }
  module->functions = (void*)0;
  module->function_names = (void*)0;
  module->function_size = 0;
  
  return module;
}

AST* module_function_call(Module* module, char* func_name, AST** args, size_t arg_size)
{
  for (int i = 0; i < module->function_size; i++) {
    if (strcmp(module->function_names[i], func_name) == 0) {
      return module->functions[i](args, arg_size);
    }
  }
  
  module->function_size++;
  module->function_names = realloc(module->function_names, module->function_size * sizeof(char*));
  module->functions = realloc(module->functions, module->function_size * sizeof(AST* (*) (AST** args, size_t arg_size)));

  module->function_names[module->function_size - 1] = func_name;
  AST* (*function) (AST**, size_t) = dlsym(module->handle, func_name);
  if (!function) {
    module_error();
  }
  module->functions[module->function_size - 1] = function;
  return function(args, arg_size);
}
