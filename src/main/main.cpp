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

  clint->AddIncludePath("/home/jason/midge/src");
  clint->AddIncludePath("/home/jason/cling/inst/include");

  clint->loadFile("/home/jason/midge/src/midge.h");
  char buf[512];
  sprintf(buf, "clint = (cling::Interpreter *)%p;", (void *)clint);
  clint->process(buf);

  clint->process("_midge_run()");

  delete (clint);

  // IGNORE_MIDGE_ERROR_REPORT = true;

  usleep(100000);
}