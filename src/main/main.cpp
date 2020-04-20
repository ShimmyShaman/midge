/* main.cpp */

#include <stdlib.h>
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

cling::Interpreter *interp;

int main(int argc, const char *const *argv)
{
    char buffer[200];
    getcwd(buffer, 200);
    cout << "cwd:" << std::string(buffer) << endl;

    const char *LLVMDIR = "/home/daniel/cling/obj";
    interp = new cling::Interpreter(argc, argv, LLVMDIR);

    interp->AddIncludePath("/home/daniel/midge/src");
    interp->AddIncludePath("/home/daniel/cling/src/include");
    interp->AddIncludePath("/home/daniel/cling/src/tools/cling/include");
    interp->AddIncludePath("/home/daniel/cling/src/tools/clang/include");
    interp->AddIncludePath("/home/daniel/cling/obj/include");

    interp->declare("cling::Interpreter *clint;");
    interp->process("clint = (cling::Interpreter *)" + std::to_string((long)interp) + ";");

    interp->loadFile("midge.h");
    interp->process("run()");

    delete (interp);
}