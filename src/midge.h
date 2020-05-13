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

extern "C"
{
  int clint_process(const char *str)
  {
    return clint->process(str);
  }

  int clint_declare(const char *str)
  {
    return clint->declare(str);
  }

  int clint_loadfile(const char *path)
  {
    return clint->loadFile(path);
  }

  int clint_loadheader(const char *path)
  {
    return clint->loadHeader(path);
  }
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
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_core.h");
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.h");
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.cpp");
    // clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.c");
    // clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.h");
    // clint->loadFile("/home/jason/midge/src/rendering/renderer.h");
    // clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.h");
    // clint->loadFile("/home/jason/midge/src/rendering/renderer.cpp");
    // clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.c");
    // clint->loadFile("/home/jason/midge/src/mcl_type_defs.h");
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

    // clint->loadFile("/home/jason/midge/src/c_code_lexer.h");
    clint->loadFile("/home/jason/midge/src/midge_core.c");
    clint->process("mc_main(0, NULL);");

    // clint->process("#include \"midge.h\"");
    // clint->process("redef();");
  }
  catch (const std::exception &e)
  {
    std::cerr << "midge.h] Caught Error:" << std::endl;
    std::cerr << e.what() << '\n';
  }
  clint->process("printf(\"</midge>\\n\");");
}
#endif // MIDGE_H