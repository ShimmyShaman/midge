/* midge.h */

#ifndef MIDGE_H
#define MIDGE_H

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
void defineStructure(std::string typeName, std::string definition)
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

void redef()
{
  // -- Redefinition
  // define structure
  defineStructure("shaver", "typedef struct shaver { float battery_life; } shaver;");

  // define method
  clint->declare("void *shaver_display_routine(void *vargp) {"
                 "  void **vargs = (void **)vargp;"
                 "  mthread_info *thr = *(mthread_info **)vargs[0];"
                 "  shaver *s = (shaver *)vargs[1];"
                 "  "
                 "  float last_measure = 120.f;"
                 "  while(!thr->should_exit) {"
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
                 "    usleep(50000);"
                 "    ms += 50;"
                 "    s->battery_life = 0.9999f * s->battery_life - 0.00007f * ms;"
                 "  }"
                 "  "
                 "  thr->has_concluded = true;"
                 "  return NULL;"
                 "}");

  // Begin thread
  clint->process("mthread_info *rthr, *uthr;");
  clint->process("shaver s_data = { .battery_life = 83.4f };");
  clint->process("void *args[2];");
  clint->process("args[1] = &s_data;");

  clint->process("args[0] = &rthr;");
  clint->process("begin_mthread(shaver_display_routine, &rthr, args);");
  clint->process("args[0] = &uthr;");
  clint->process("begin_mthread(shaver_update_routine, &uthr, args);");

  // redefine structure in main thread
  // clint->process("rthr->do_pause = true;");
  // defineStructure("shaver", "typedef struct shaver { float battery_life; float condition_multiplier; } shaver;");

  // end
  clint->process("while(!uthr->has_concluded) usleep(500);");
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