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

int main(int argc, const char *const *argv)
{
  char buffer[200];
  getcwd(buffer, 200);
  const char *LLVMDIR = "/home/jason/cling/inst";
  clint = new cling::Interpreter(argc, argv, LLVMDIR);

  clint->AddIncludePath("/home/jason/midge/src");
  clint->AddIncludePath("/home/jason/cling/inst/include");

  clint->loadFile("/home/jason/midge/src/midge.h");
  clint->process("clint = (cling::Interpreter *)" + std::to_string((long)clint) + ";");

  clint->process("run()");

  delete (clint);
}