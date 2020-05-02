/* midge.h */

#ifndef MIDGE_H
#define MIDGE_H

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

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"

static cling::Interpreter *clint;

void loadSourceFiles(const char *root_dir, int indent)
{
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(root_dir)))
    return;

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_DIR)
    {
      if (!indent && !strcmp(entry->d_name, "main"))
      {
        printf("IgnoreDir: %*s[%s]\n", indent, "", entry->d_name);
        continue;
      }
      char path[1024];
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      snprintf(path, sizeof(path), "%s/%s", root_dir, entry->d_name);
      printf("Directory: %*s[%s]\n", indent, "", entry->d_name);
      loadSourceFiles(path, indent + 2);
    }
    else
    {
      if (!strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".h") || !strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".c") || !strcmp(&entry->d_name[strlen(entry->d_name) - 4], ".cpp"))
      {
        char path[1024];
        char *filePath = strcpy(path, root_dir);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);
        clint->loadFile(path);
        printf("LoadedSrc: \"%*s- %s\"\n", indent, "", entry->d_name);
        continue;
      }
      printf("IgnoreSrc: %*s- %s\n", indent, "", entry->d_name);
    }
  }
  closedir(dir);
}

void loadLibrary(const char *name)
{
  cling::Interpreter::CompilationResult result = clint->loadLibrary(name);
  if (result == cling::Interpreter::kSuccess)
    return;

  std::string lookedUpAddress = clint->lookupFileOrLibrary(name);
  printf("lookup: %s\n", lookedUpAddress.c_str());
  result = clint->loadLibrary(lookedUpAddress);
  if (result == cling::Interpreter::kSuccess)
    return;

  printf("Failure! %i = loadLibrary(\"%s\")\n", result, name);

  exit(-1);
}

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

std::map<std::string, int> defined_functions;
void define_function(const char *return_type, const char *name, const char *params, const char *block)
{
  if (strcmp(return_type, "void"))
  {
    printf("Only allowed void returning functions atm.");
    return;
  }

  const char *TAB = "  ";
  char decl[16384];
  std::map<std::string, int>::iterator it = defined_functions.find(name);
  if (it == defined_functions.end())
  {
    // Declare Function Pointer
    strcpy(decl, "static void (*");
    strcat(decl, name);
    strcat(decl, ")(void **);");
    clint->declare(decl);
    printf("%s\n", decl);
    defined_functions[name] = 0;
  }

  // Set version and function name postfix
  int version = defined_functions[name] + 1;
  defined_functions[name] = version;
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
        if (params[i] == ' ')
        {
          int mod = 0;
          while (params[i + 1] == '&' || params[i + 1] == '*')
          {
            mod = 1;
            ++i;
          }
          t = i + mod;
        }
      }
      else
      {
        if (params[i] == ',' || params[i] == '\0')
        {
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

  // -- code-block
  n = strlen(block);
  s = 0;
  t = 0;
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

    if (block[i] == '(' && i > 0)
    {
      char call_name[256];
      strncpy(call_name, block + t, i - t);
      call_name[i - t] = '\0';
      it = defined_functions.find(call_name);
      if (it == defined_functions.end())
        continue;

      printf("found-call:(%i:%i):%s\n", t, i - t, call_name);
      // TODO
      printf("TODO-TODO-TODO-TODO-TODO-TODO\n")
    }
  }
  // strcat(decl, block);
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

void redef()
{
  // -- Redefinition
  define_function("void", "add_to_num", "int *v", "  *v += 4;\n");
  define_function("void", "puffy", "",
                  "  int v = 3;\n"
                  "  add_to_num(&v);\n"
                  "  printf(\"out:%i\\n\", v);\n");
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

void run()
{
  try
  {
    // Include Paths
    // clint->AddIncludePath("/usr/include");
    clint->AddIncludePath("/home/jason/midge/dep/glm");
    // clint->AddIncludePath("/home/jason/midge/dep/glslang");

    // Libraries
    loadLibrary("vulkan");
    loadLibrary("xcb");
    // loadLibrary("dep/glslang/bin/glslangValidator");

    // Load App source
    printf("<AppSourceLoading>\n");
    // loadSourceFiles("/home/jason/midge/src", 0);
    clint->loadFile("/home/jason/midge/src/rendering/mvk_core.h");
    clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.h");
    clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.cpp");
    clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.c");
    clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.h");
    clint->loadFile("/home/jason/midge/src/rendering/renderer.h");
    clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.h");
    clint->loadFile("/home/jason/midge/src/rendering/renderer.cpp");
    clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.c");
    printf("</AppSourceLoading>\n\n");

    // // Run App
    // clint->declare("void updateUI(mthread_info *p_render_thread) { int ms = 0; while(ms < 40000 && !p_render_thread->has_concluded) { ++ms; usleep(1000); } }");

    // clint->process("mthread_info *rthr;");
    // // printf("process(begin)\n");
    // clint->process("begin_mthread(midge_render_thread, &rthr);");
    // // printf("process(updateUI)\n");
    // clint->process("updateUI(rthr);");
    // // printf("process(end)\n");
    // clint->process("end_mthread(rthr);");
    // printf("\n! MIDGE COMPLETE !\n");

    /* Goal: is the ability to change a structure (which contains a resource which must be destroyed & initialized) which is used in-a-loop
     *       in a seperate thread routine > then change the thread routine to make use of that structure change
     */

    redef();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}
#endif // MIDGE_H