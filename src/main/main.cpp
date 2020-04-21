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

cling::Interpreter *clint;

#include "opengl/gl_loop.h"
int main(int argc, const char *const *argv)
{
    runTriangle(nullptr, 0);

    /*char buffer[200];
    getcwd(buffer, 200);
    cout << "cwd:" << std::string(buffer) << endl;

    const char *LLVMDIR = "/home/daniel/cling/obj";
    clint = new cling::Interpreter(argc, argv, LLVMDIR);

    clint->AddIncludePath("/home/daniel/midge/src");
    clint->AddIncludePath("/home/daniel/cling/src/include");
    clint->AddIncludePath("/home/daniel/cling/src/tools/cling/include");
    clint->AddIncludePath("/home/daniel/cling/src/tools/clang/include");
    clint->AddIncludePath("/home/daniel/cling/obj/include");

    cling::Interpreter::CompilationResult result;
    clint->AddIncludePath("/usr/local/include/");
    clint->AddIncludePath("/usr/local/lib64/");
    // std::cout << clint->lookupFileOrLibrary("glad.o") << std::endl;
    std::cout << clint->lookupFileOrLibrary("glfw") << std::endl;
    result = clint->loadLibrary(clint->lookupFileOrLibrary("glfw"));
    //result = clint->loadLibrary("libglfw3.a");
    std::cout << "glfw3:" << result << std::endl;
    // clint->AddIncludePath("/home/daniel/midge/dep/");
    result = clint->loadLibrary("/home/daniel/midge/dep/glad.o");
    std::cout << "dep/glad.o:" << result << std::endl;
    result = clint->loadLibrary("X11");
    std::cout << "X11:" << result << std::endl;
    result = clint->loadLibrary("GL");
    std::cout << "GL:" << result << std::endl;

    //interp->declare("cling::Interpreter *clint;");
    clint->loadFile("midge.h");
    clint->process("clint = (cling::Interpreter *)" + std::to_string((long)clint) + ";");

    clint->process("run()");

    delete (clint);*/
}