/* main.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <time.h>
#include <vector>
#include <unistd.h>

#include "cling/Interpreter/Interpreter.h"

using namespace std;

cling::Interpreter *clint;

// void func_a()
// {
// #define BIG "huge"
//   printf("%s\n", BIG);
// }

// void func_b()
// {
// #undef BIG
// #define BIG "not so much"
//   printf("%s\n", BIG);
// }

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
  // clint->process("clint = (cling::Interpreter *)" + std::to_string((long)clint) + ";");
  clint->process(buf);

  clint->process("run()");

  delete (clint);

  usleep(100000);
}