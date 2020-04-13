/* main.cpp */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "midge_app.h"
#include "core_bindings.h"

using namespace std;

int main(void)
{
  CoreBindings::bindFunctions();

  MidgeApp midgeApp;

  exit(midgeApp.run());
}