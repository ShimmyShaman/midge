/* main.cpp */

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "cling/Interpreter/Interpreter.h"

using namespace std;

cling::Interpreter *clint;

int main(int argc, const char *const *argv)
{

  // char buffer[200];
  // getcwd(buffer, 200);
  const char *LLVMDIR = "/home/jason/cling/inst";
  clint = new cling::Interpreter(argc, argv, LLVMDIR);

  // clint->loadFile("/home/jason/midge/src/main/remove_mc_mcva_calls.c");
  // clint->process("remove_all_MCcalls();");
  // return 0;

  clint->AddIncludePath("/home/jason/midge/src");
  clint->AddIncludePath("/home/jason/cling/inst/include");

  clint->loadFile("/home/jason/midge/src/midge.h");
  char buf[512];
  sprintf(buf, "clint = (cling::Interpreter *)%p;", (void *)clint);
  clint->process(buf);

  int result;
  sprintf(buf, "*(int *)(%p) = _midge_run();", &result);
  clint->process(buf);

  delete (clint);

  // IGNORE_MIDGE_ERROR_REPORT = true;

  usleep(1000000);
  return result;
}