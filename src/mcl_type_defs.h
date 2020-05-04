/* mcl_type_defs.h */

#ifndef MCL_TYPE_DEFS_H
#define MCL_TYPE_DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

// #include "cling/Interpreter/Interpreter.h"
// #include "cling/Interpreter/Transaction.h"

// static cling::Interpreter *clint;

#include "midge.h"

// std::vector<cling::Transaction *> transactions;
std::map<std::string, cling::Transaction *> definedTypes;
void define_structure(std::string typeName, std::string definition)
{
  std::map<std::string, cling::Transaction *>::iterator it = definedTypes.find(typeName);
  if (it != definedTypes.end())
  {
    std::cout << "type " << typeName << " already exists!" << std::endl;

    uint wastid = clint->getLatestTransaction()->getUniqueID();
    //std::cout << "current=" << interp->getLatestTransaction()->getUniqueID() << std::endl;
    //interp->getLatestTransaction()->dump();
    int iter = 0;
    while (clint->getLatestTransaction()->getUniqueID() > it->second->getUniqueID())
    {
      ++iter;
      clint->unload(1);
      //interp->getLatestTransaction()->dump();
    }

    if (clint->getLatestTransaction()->getUniqueID() != it->second->getUniqueID())
      throw 343;
    clint->unload(1);
  }

  cling::Transaction *transaction = nullptr;
  clint->declare(definition, &transaction);
  if (!transaction)
    throw 111;
  else
  {
    //std::cout << "Transaction:" << transaction->getUniqueID() << std::endl;
    //transaction->dump();
    // transactions.push_back(transaction);
    definedTypes[typeName] = transaction;
  }
}

typedef struct defined_function
{
  int version;
  const char *name;
  char **params_types;
  char **params_names;
  int params_count;
  const char *given_code;
} defined_function;

std::map<std::string, defined_function *> defined_functions;
void define_function(const char *return_type, const char *name, const char *params, const char *block)
{
  if (strcmp(return_type, "void"))
  {
    printf("Only allowed void returning functions atm.");
    return;
  }

  const char *TAB = "  ";
  char decl[16384];
  std::map<std::string, defined_function *>::iterator it = defined_functions.find(name);
  defined_function *df;
  if (it == defined_functions.end())
  {
    // Declare Function Pointer
    strcpy(decl, "static void (*");
    strcat(decl, name);
    strcat(decl, ")(void **);");
    clint->declare(decl);
    printf("%s\n", decl);

    df = (defined_function *)malloc(sizeof df);
    df->version = 0;
    df->params_names = NULL;
    df->params_types = NULL;
    defined_functions[name] = df;
  }
  else
    df = it->second;

  // Set version and function name postfix
  int version = ++df->version;
  df->name = name;
  df->given_code = block;

  if (df->params_names)
  {
    free(df->params_names);
    free(df->params_types);
    for (int i = 0; i < df->params_count; ++i)
    {
      free(df->params_names[i]);
      free(df->params_types[i]);
    }
  }
  int allocated_params = 20;
  df->params_names = (char **)malloc(sizeof(char *) * allocated_params);
  df->params_types = (char **)malloc(sizeof(char *) * allocated_params);
  df->params_count = 0;

  char verstr[7];
  strcpy(verstr, "_v");
  sprintf(verstr + 2, "%i", version);

  // Form the function declaration
  // -- header
  strcpy(decl, "void ");
  strcat(decl, name);
  strcat(decl, verstr);
  strcat(decl, "(void **p_vargs) {\n");

  // -- params
  int n = strlen(params);
  int s = 0, t = 0;
  if (n > 0)
  {
    int u = 0, p = 0;
    for (int i = 0; i <= n; ++i)
    {
      if (t == s)
      {
        if (params[i] == ' ' || params[i] == '&' || params[i] == '*')
        {
          int mod = 0;
          while (params[i + 1] == '&' || params[i + 1] == '*')
          {
            mod = 1;
            ++i;
          }
          t = i + mod;
          u = mod ? t : (i + 1);
        }
      }
      else
      {
        if (params[i] == ',' || params[i] == '\0')
        {
          // Set
          if (df->params_count > allocated_params)
          {
            allocated_params = allocated_params + 4 + allocated_params / 2;
            df->params_types = (char **)realloc(df->params_types, sizeof(char *) * allocated_params);
            df->params_names = (char **)realloc(df->params_names, sizeof(char *) * allocated_params);
          }
          df->params_types[df->params_count] = (char *)malloc(sizeof(char) * (t - s));
          strncpy(df->params_types[df->params_count], params + s, t - s);
          df->params_types[df->params_count][t - s] = '\0';
          df->params_names[df->params_count] = (char *)malloc(sizeof(char) * (i - u));
          strncpy(df->params_names[df->params_count], params + u, i - u);
          df->params_names[df->params_count][i - u] = '\0';
          printf("param_type:'%s' param_name:'%s'\n", df->params_types[df->params_count], df->params_names[df->params_count]);
          ++df->params_count;

          // Add to declaration
          strcat(decl, TAB);
          strncat(decl, params + s, i - s);
          strcat(decl, " = (");
          strncat(decl, params + s, t - s);
          strcat(decl, ")p_vargs[");
          sprintf(decl + strlen(decl), "%i", p);
          strcat(decl, "];\n");

          // Reset
          ++p;
          s = t = i + 1;
        }
      }
    }
    strcat(decl, "\n");
  }
  if (allocated_params != df->params_count)
  {
    df->params_types = (char **)realloc(df->params_types, sizeof(char *) * df->params_count);
    df->params_names = (char **)realloc(df->params_names, sizeof(char *) * df->params_count);
  }

  // -- code-block
  n = strlen(block);
  s = 0;
  t = 0;
  int varyd = 0;
  for (int i = 0; i <= n; ++i)
  {
    if (block[i] == ' ' || block[i] == '\n' || block[i] == '\t')
      continue;
    if (t == s)
      t = i;
    if (block[i] == ';' || block[i] == '\0')
    {
      strncat(decl, block + s, i - s + 1);
      s = t = i + 1;
    }

    if (block[i] != '(')
      continue;

    char call_name[256];
    strncpy(call_name, block + t, i - t);
    call_name[i - t] = '\0';
    it = defined_functions.find(call_name);
    if (it == defined_functions.end())
      continue;

    defined_function *sf = it->second;
    char nstname[10];
    printf("found-call:(%i:%i):%s\n", t, i - t, call_name);
    {
      // Declare the array of void pointers
      char nst[100];
      char nstnum[7];
      const char *initial = "void *_mdge_";
      strcpy(nst, initial);
      sprintf(nstnum, "%i", varyd);
      strcat(nst, nstnum);

      // -- sidebar : set name of array for other pointers to access
      strcpy(nstname, initial);
      strcat(nstname, nstnum);

      strcat(nst, "[");
      memset(nstnum, '\0', 7);
      sprintf(nstnum, "%i", sf->params_count);
      strcat(nst, nstnum);
      strcat(nst, "];\n");
      strcat(decl, nst);
    }
    for (int j = 0; j < sf->params_count; ++j)
    {
      char nst[2048];
      char nstnum[7];
      strcpy(nst, nstname);
      strcat(nst, "[");
      sprintf(nstnum, "%i", j);
      strcat(nst, nstnum);
      strcat(nst, "] = (void *)");
    }
    ++varyd;
  }
  strcat(decl, "}\n");

  // Declare function
  printf("decl:%s\n", decl);
  clint->declare(decl);

  // Set pointer to function
  strcpy(decl, name);
  strcat(decl, " = &");
  strcpy(decl, name);
  strcat(decl, verstr);
  strcat(decl, ";");
  clint->process(decl);
}

void code(const char *cstr)
{
  clint->process(cstr);
}

typedef struct structure_definition
{
  const char *name;
  cling::Transaction transaction;

} structure_definition;

void load_mc_file(const char *filepath, bool error_on_redefinition)
{
  FILE *fp = fopen(filepath, "r");
}

void redef()
{
  // -- Redefinition
  // load_mc_file("/home/jason/midge/src/mc_exp.mc", true);

  const char *mc_add_to_num = "func add_to_num(int *v, int e) { *v += 4 * e; }";
  clint->declare("void add_to_num (void **vargs) {"
                 "  int **v = (int **)vargs[0];\n"
                 "  int *e = (int *)vargs[1];\n"
                 "  \n"
                 "  **v += 4 * *e;\n }");

  const char *mc_puffy = "func puffy() {"
                         "  int v = 3;"
                         "  add_to_num(&v, 2);"
                         "  printf(\"out:%i\\n\", v);\n"
                         "}";
  clint->declare("void puffy() {"
                 "  int *v = (int *)malloc(sizeof (int));\n"
                 "  *v = 3;\n"
                 "\n"
                 "  void *vargs[2];\n"
                 "  vargs[0] = (void *)&v;\n"
                 "  int mcliteral_0 = 2;"
                 "  vargs[1] = (void *)&mcliteral_0;\n"
                 "  add_to_num(vargs);"
                 "\n"
                 "  printf(\"out:%i\\n\", *v);\n"
                 "\n"
                 "  free(v); }");

  clint->process("puffy()");
  // define_function("void", "add_to_num", "int *v", "  *v += 4;\n");
  // define_function("void", "puffy", "",
  //                 "  int v = 3;\n"
  //                 "  add_to_num(&v);\n"
  //                 "  printf(\"out:%i\\n\", v);\n");
  // code("int v = 3;");
  // code("add_to_num(&v);");
  // define_method("add_to_num", "void add_to_num(int *v) { *v += 19; }");
  // code("add_to_num(&v);");

  // code("printf(\"Initial >v=%i\\n\", v);");

  // define structure
  // define_structure("shaver", "typedef struct shaver { float battery_life; } shaver;");

  // define_method("do_stuff", "void do_stuff(shaver *s) { s->battery_life -= 7; }");

  // code("shaver s;");
  // code("s.battery_life = 100;");
  // code("do_stuff(&s);");
  // code("printf(\"Before>s.battery_life=%.2f\\n\", s.battery_life);");

  // define_structure("shaver", "typedef struct shaver { float battery_life; float condition_multiplier; } shaver;");
  // code("printf(\"After >s.battery_life=%i\\n\", s.battery_life);");
}

void redef2()
{
  // -- Redefinition
  // define structure
  define_structure("shaver", "typedef struct shaver { float battery_life; } shaver;");

  // define method
  clint->declare("void *shaver_display_routine(void *vargp) {"
                 "  void **vargs = (void **)vargp;"
                 "  mthread_info *thr = *(mthread_info **)vargs[0];"
                 "  shaver *s = (shaver *)vargs[1];"
                 "  "
                 "  float last_measure = 120.f;"
                 "  while(!thr->should_exit) {"
                 "    if(thr->should_pause && hold_mthread(thr))"
                 "      break;"
                 "    if(last_measure - s->battery_life > 1.f) {"
                 "      last_measure = s->battery_life;"
                 "      printf(\"battery-life:%.2f\\n\", s->battery_life);"
                 "    }"
                 "    usleep(2000);"
                 "  }"
                 "  "
                 "  thr->has_concluded = true;"
                 "  return NULL;"
                 "}");
  clint->declare("void *shaver_update_routine(void *vargp) {"
                 "  void **vargs = (void **)vargp;"
                 "  mthread_info *thr = *(mthread_info **)vargs[0];"
                 "  shaver *s = (shaver *)vargs[1];"
                 "  "
                 "  int ms = 0;"
                 "  while(!thr->should_exit && ms < 10000) {"
                 "    if(thr->should_pause && hold_mthread(thr))"
                 "      break;"
                 "    usleep(50000);"
                 "    ms += 50;"
                 "    s->battery_life = 0.9999f * s->battery_life - 0.00007f * ms;"
                 "  }"
                 "  "
                 "  thr->has_concluded = true;"
                 "  return NULL;"
                 "}");

  // Begin
  clint->process("mthread_info *rthr, *uthr;");
  clint->process("shaver s_data = { .battery_life = 83.4f };");
  clint->process("void *args[2];");
  clint->process("args[1] = &s_data;");

  clint->process("args[0] = &rthr;");
  clint->process("begin_mthread(shaver_display_routine, &rthr, args);");
  clint->process("args[0] = &uthr;");
  clint->process("begin_mthread(shaver_update_routine, &uthr, args);");

  // Pause
  clint->process("int iterations = 0;");
  clint->process("while(!uthr->has_concluded && iterations < 1000) { usleep(4000); ++iterations; }");
  clint->process("printf(\"pausing...\\n\");");
  clint->process("pause_mthread(rthr, false);");
  clint->process("pause_mthread(uthr, false);");
  clint->process("while(!rthr->has_paused || !uthr->has_paused) usleep(1);");
  clint->process("printf(\"paused for 3 seconds.\\n\");");

  // Redefine
  // redefine structure in main thread
  define_structure("shaver", "typedef struct shaver { float battery_life; float condition_multiplier; } shaver;");

  // Resume
  clint->process("iterations = 0;");
  clint->process("while(!uthr->has_concluded && iterations < 1000) { usleep(3000); ++iterations; }");
  clint->process("printf(\"resuming...\");");
  clint->process("unpause_mthread(rthr, false);");
  clint->process("unpause_mthread(uthr, false);");
  clint->process("printf(\"resumed!\\n\");");

  // End
  clint->process("while(!uthr->has_concluded) usleep(1);");
  clint->process("printf(\"ending...\\n\");");
  clint->process("end_mthread(rthr);");
  clint->process("end_mthread(uthr);");
  clint->process("printf(\"success!\\n\");");
}

#endif // MCL_TYPE_DEFS_H