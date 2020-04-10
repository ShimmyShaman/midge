/* main.cpp */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "core_interpreter.h"
#include "midge_app.h"

using namespace std;

const int BUILD_NUMBER = 0;

const char *fileText =
    "[class:Global{"
    "[method:print:void:(){"
    "print(\"\nHello Universe\n\n\");"
    "}]"
    "}]"
    "[entry:run:void:(){"
    "instance(Global,global);"
    "invoke(print,midge);"
    "binvoke(printCrap,\"Shorty\")"
    "}]";

int findDefinitionPartEnd(const char *fileText, int *startIndex)
{
  int i = *startIndex;
  while (fileText[i] != '{' && fileText[i] != ':')
    ++i;

  return i;
}

int main(void)
{
  CoreInterpreter interpreter;
  MidgeApp *app = interpreter.interpret(fileText);

  int result = app->run();

  delete (app);
  return result;
}