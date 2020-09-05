/* midge.h

   Copyright 2013, Adam Rasburn, All rights reserved.
*/

#ifndef MIDGE_H
#define MIDGE_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (!indent && !strcmp(entry->d_name, "main")) {
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
    else {
      if (!strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".h") ||
          !strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".c") ||
          !strcmp(&entry->d_name[strlen(entry->d_name) - 4], ".cpp")) {
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

extern "C" {
int clint_process(const char *str) { return clint->process(str); }

int clint_declare(const char *str) { return clint->declare(str); }

void *clint_compile_function(const char *name, const char *str) { return clint->compileFunction(name, str); }

int clint_loadfile(const char *path) { return clint->loadFile(path); }

int clint_loadheader(const char *path) { return clint->loadHeader(path); }
}

int (*mc_dummy_function)(int, void **);
int mc_dummy_function_v1(int argsc, void **argsv) { return 0; }

// void mc_printf(char *str, void **a) { printf(str, *a); }

// void mc_invoke_function(const char *fn, void *arg)
// {
//   char buf[222];
//   sprintf(buf, "%s((int *)%p);", fn, arg);
//   clint_process(buf);
// }

// void mc_invoke_functionptr(void *fptr, void *arg)
// {
//   char buf[222];
//   sprintf(buf,
//           "void (*fptr)(int *) = (void (*)(int *))%p;\n"
//           "fptr((int *)%p);",
//           fptr, arg);
//   clint_process(buf);
// }

// void domctest()
// {
//   char buf[1202];

//   sprintf(buf, "")
//   int v0 = 5;
//   mc_invoke_function("brilly", &v0);

//   v0 = 77;
//   mc_invoke_functionptr((void *)&brilly, &v0);

//   int v0 = 5;
//   mc_invoke_function("printf", &v0);
// }

int _midge_run()
{
  // domctest();
  // return 0;
  // Pointer Size Check
  int sizeof_void_ptr = sizeof(void *);
  if (sizeof_void_ptr != sizeof(int *) || sizeof_void_ptr != sizeof(char *) ||
      sizeof_void_ptr != sizeof(unsigned int *) || sizeof_void_ptr != sizeof(const char *) ||
      sizeof_void_ptr != sizeof(void **) || sizeof_void_ptr != sizeof(mc_dummy_function) ||
      sizeof_void_ptr != sizeof(&mc_dummy_function_v1) || sizeof_void_ptr != sizeof(unsigned long)) {
    printf("pointer sizes aren't equal!!! This is the basis of midge. Exiting.\n");
    return -777;
  }

  // TODO -- integrate the ERROR STACK/TAG stuff with the rest of the midge code
  // TODO -- put this right at the start of the main app?
  struct timespec app_begin_time;
  clock_gettime(CLOCK_REALTIME, &app_begin_time);

  // clint->allowRedefinition();
  try {
    // TODO - temp to deal with how enums are handled
    clint->process("#pragma clang diagnostic ignored \"-Wswitch\"");
    clint->process("#pragma clang diagnostic ignored \"-Wtautological-constant-out-of-range-compare\"");

    // Include Paths
    // clint->AddIncludePath("/usr/include");
    clint->AddIncludePath("/home/jason/midge/dep/cglm/include");
    clint->AddIncludePath("/home/jason/midge/dep/stb");
    // clint->AddIncludePath("/home/jason/midge/dep/glslang");

    // Libraries
    loadLibrary("vulkan");
    loadLibrary("xcb");
    // loadLibrary("dep/glslang/bin/glslangValidator");

    // Load App source
    printf("<AppSourceLoading>\n");
    clint->loadFile("/home/jason/midge/src/midge_error_handling.h");
    clint->loadFile("/home/jason/midge/src/core/core_source_loader.c");

    // clint->declare("extern \"C\" {\n"
    //                "int (*midge_initialize_app)(int, void **);\n"
    //                "int (*midge_run_app)(int, void **);\n"
    //                "int (*midge_cleanup_app)(int, void **);\n"
    //                "}");
    clint->process("initialize_midge_error_handling(clint);");
    clint->process("{\n""unsigned int dummy_value;\n"
    "register_midge_thread_creation(&dummy_value, \"_midge_run\")
    int mc_res;
    char buf[512];
    sprintf(buf, "(*(int *)%p) = mcl_load_app_source();\n", &mc_res);
    clint->process(buf);
    if (mc_res) {
      printf("--_midge_run() |line~ :147 ERR:%i\n", mc_res);
      return mc_res;
    }
    clint->process("printf(\"</AppSourceLoading>\\n\\n\");");

    sprintf(buf,
            "int _midge_internal_run() {\n"
            "  void *mc_vargs[1];\n"
            "  void *mc_vargs_0 = (void *)%p;\n"
            "  mc_vargs[0] = &mc_vargs_0;\n"
            "  MCcall(midge_initialize_app(1, mc_vargs));\n"
            ""
            "  MCcall(midge_run_app(0, NULL));\n"
            ""
            "  MCcall(midge_cleanup_app(0, NULL));\n"
            ""
            "  return 0;\n"
            "}\n",
            &app_begin_time);
    clint->declare(buf);

    sprintf(buf, "(*(int *)%p) = _midge_internal_run();\n", &mc_res);
    clint->process(buf);
    if (mc_res) {
      printf("--_midge_run() |line~ :167 ERR:%i\n", mc_res);
      return mc_res;
    }

    // // loadSourceFiles("/home/jason/midge/src/", 0);
    // clint->loadFile("/home/jason/midge/src/midge_common.h");
    // clint->loadFile("/home/jason/midge/src/midge_common.c");
    // clint->loadFile("/home/jason/midge/src/m_threads.h");
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_core.h");
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.h");
    // clint->loadFile("/home/jason/midge/src/rendering/mvk_init_util.cpp");
    // clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.c");
    // clint->loadFile("/home/jason/midge/src/rendering/xcbwindow.h");
    // clint->loadFile("/home/jason/midge/src/rendering/renderer.h");
    // clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.h");
    // clint->loadFile("/home/jason/midge/src/rendering/renderer.cpp");
    // clint->loadFile("/home/jason/midge/src/rendering/vulkandebug.c");
    // // clint->loadFile("/home/jason/midge/src/mcl_type_defs.h");
    // clint->loadFile("/home/jason/midge/src/core/midge_core.h");
    // clint->loadFile("/home/jason/midge/src/midge_main.h");
    // clint->loadFile("/home/jason/midge/src/midge_main.c");

    // // Run App
    // clint->declare("void updateUI(mthread_info *p_render_thread) { int ms = 0; while(ms < 40000 &&"
    //                " !p_render_thread->has_concluded) { ++ms; usleep(1000); } }");

    // clint->process("mthread_info *rthr;");
    // // printf("process(begin)\n");
    // clint->process("begin_mthread(midge_render_thread, &rthr, (void *)&rthr);");
    // printf("process(updateUI)\n");
    // clint->process("updateUI(rthr);");
    // printf("process(end)\n");
    // clint->process("end_mthread(rthr);");
    // printf("\n! MIDGE COMPLETE !\n");

    /* Goal: is the ability to change a structure (which contains a resource which must be destroyed & initialized)
     * which is used in-a-loop in a seperate thread routine > then change the thread routine to make use of that
     * structure change
     */

    // clint->loadFile("/home/jason/midge/src/c_code_lexer.h");
    // clint->process("mc_main(0, NULL);");

    // clint->process("#include \"midge.h\"");
    // clint->process("redef();");
  }
  catch (const std::exception &e) {
    std::cerr << "midge.h] Caught Error:" << std::endl;
    std::cerr << e.what() << '\n';
  }
  clint->process("printf(\"</midge>\\n\");");
  return 0;
}
#endif // MIDGE_H