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
#include <stdio.h>

#include "cling/Interpreter/Interpreter.h"

using namespace std;

cling::Interpreter *clint;

int main(int argc, const char *const *argv)
{
    char buffer[200];
    getcwd(buffer, 200);
    cout << "cwd:" << std::string(buffer) << endl;

    const char *LLVMDIR = "/home/jason/cling/inst";
    cout << "string" << endl;
    clint = new cling::Interpreter(argc, argv, LLVMDIR);

    cout << "rope" << endl;
    clint->process("838 + 181");
    return 1;

    clint->AddIncludePath("/home/jason/midge/src");
    // clint->AddIncludePath("/home/jason/cling/src/include");
    // clint->AddIncludePath("/home/jason/cling/src/tools/cling/include");
    // clint->AddIncludePath("/home/jason/cling/src/tools/clang/include");
    clint->AddIncludePath("/home/jason/cling/inst/include");

    // cling::Interpreter::CompilationResult result;
    // clint->AddIncludePath("/usr/local/include/");
    // clint->AddIncludePath("/usr/local/lib64/");
    // // std::cout << clint->lookupFileOrLibrary("glad.o") << std::endl;
    // std::cout << clint->lookupFileOrLibrary("glfw") << std::endl;
    // result = clint->loadLibrary(clint->lookupFileOrLibrary("glfw"));
    // //result = clint->loadLibrary("libglfw3.a");
    // std::cout << "glfw3:" << result << std::endl;
    // // clint->AddIncludePath("/home/jason/midge/dep/");
    // result = clint->loadLibrary("/home/jason/midge/dep/glad.o");
    // std::cout << "dep/glad.o:" << result << std::endl;
    // result = clint->loadLibrary("X11");
    // std::cout << "X11:" << result << std::endl;
    // result = clint->loadLibrary("GL");
    // std::cout << "GL:" << result << std::endl;

    //interp->declare("cling::Interpreter *clint;");
    clint->loadFile("midge.h");
    clint->process("clint = (cling::Interpreter *)" + std::to_string((long)clint) + ";");

    clint->process("run()");

    delete (clint);
}