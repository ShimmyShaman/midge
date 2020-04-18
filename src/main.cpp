/* main.cpp */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <time.h>
#include <vector>

#include "midge_app.h"
#include "core_bindings.h"

#include "glfw/glfw_bindings.h"

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Value.h"
#include "cling/Utils/Casting.h"

using namespace std;

int main(int argc, const char *const *argv)
{
  // Create the Interpreter. LLVMDIR is provided as -D during compilation.
  const char *LLVMDIR = "/home/daniel/cling/obj";
  cling::Interpreter interp(argc, argv, LLVMDIR);
  
  // Declare a function to the interpreter. Make it extern "C" to remove
  // mangling from the game.
  interp.declare("extern \"C\" int plutification(int siss, int sat) "
                 "{ return siss * sat; }");
  void* addr = interp.getAddressOfGlobal("plutification");
  using func_t = int(int, int);
  func_t* pFunc = cling::utils::VoidToFunctionPtr<func_t*>(addr);
  std::cout << "7 * 8 = " << pFunc(7, 8) << endl;

  /*CoreBindings::bindFunctions();
  GLFWBindings::bindFunctions();

  MidgeApp midgeApp;

  exit(midgeApp.run());*/
}