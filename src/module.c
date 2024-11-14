#include "inc/module.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

static void module_error()
{
#ifdef _WIN32
  DWORD error = GetLastError();
  fprintf(stderr, "Module-> error: %lu\n", error);
#else
  fprintf(stderr, "Module-> error: %s\n", dlerror());
#endif
  exit(1);
}

Module* init_module(char* name)
{
  Module* module = calloc(1, sizeof(Module));

  /*
  module->name = name;
  char path[256];
  sprintf(path, "./%s.so", name);
  module->handle = dlopen(path, RTLD_LAZY);
  if (!module->handle) {
    module_error();
  }
  */
  module->name = name;
  char path[256];
#ifdef _WIN32  // For Windows
  sprintf(path, "./%s.dll", name);  // Use .dll for Windows
  module->handle = LoadLibrary(path);  // Use LoadLibrary for Windows
  if (!module->handle) {
    module_error();  // Handle error if loading fails
  }
#else
  sprintf(path, "./%s.so", name);  // Use .so for Linux
  module->handle = dlopen(path, RTLD_LAZY);  // Use dlopen for Linux
  if (!module->handle) {
    module_error();  // Handle error if loading fails
  }
#endif
  module->functions = (void*)0;
  module->function_names = (void*)0;
  module->function_size = 0;
  
  return module;
}

AST* module_function_call(Module* module, char* func_name, AST** args, size_t arg_size)
{
  for (int i = 0; i < module->function_size; i++) {
    char act_fn_name[strlen(func_name) + 2];
    sprintf(act_fn_name, "_%s", func_name);
    if (strcmp(module->function_names[i], act_fn_name) == 0) {
      return module->functions[i](args, arg_size);
    }
  }
  
  module->function_size++;
  module->function_names = realloc(module->function_names, module->function_size * sizeof(char*));
  module->functions = realloc(module->functions, module->function_size * sizeof(AST* (*) (AST** args, size_t arg_size)));

  char* act_fn_name = calloc(strlen(func_name) + 2, sizeof(char));
  sprintf(act_fn_name, "_%s", func_name);
  module->function_names[module->function_size - 1] = act_fn_name;

  // Load function from the module dynamically
#ifdef _WIN32  // For Windows
    FARPROC function = GetProcAddress(module->handle, act_fn_name);
    if (!function) {
      module_error();  // Handle error if function not found
    }
    AST* (*function_ptr)(AST**, size_t) = (AST* (*)(AST**, size_t))function;
#else  // For Linux
    AST* (*function_ptr)(AST**, size_t) = (AST* (*)(AST**, size_t))dlsym(module->handle, act_fn_name);
    if (!function_ptr) {
      module_error();  // Handle error if function not found
    }
#endif

  module->functions[module->function_size - 1] = function_ptr;
  return function_ptr(args, arg_size);
}
