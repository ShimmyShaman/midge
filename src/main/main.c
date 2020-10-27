/* main.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "tcc.h"

/* this function is called by the generated code */
int add(int a, int b)
{
    return a + b;
}

/* this strinc is referenced by the generated code */
const char hello[] = "Hello World!";

char my_program[] =
    // "#include <tcclib.h>\n" /* include the "Simple libc header for TCC" */
    // "#include \"../tinycc/lib/libtcc1.c\"\n"
    // "#include \"../tinycc/include/stdarg.h\"\n"
    "extern int add(int a, int b);\n"
    "extern const char hello[];\n"
    "static int r = 0;"
    "int fib(int n)\n"
    "{\n"
    "    ++r;\n"
    "    if (n <= 2)\n"
    "        return 1;\n"
    "    else\n"
    "        return fib(n-1) + fib(n-2);\n"
    "}\n"
    "\n"
    "int foo(int n)\n"
    "{\n"
    "    printf(\"%s\\n\", hello);\n"
    "    printf(\"fib(%d) = %d :: %i\\n\", n, fib(n), r);\n"
    "    printf(\"add(%d, %d) = %d\\n\", n, 2 * n, add(n, 2 * n));\n"
    "    return 0;\n"
    "}\n";

int tcc_process_statement_list(TCCState *s, const char *statements)
{
    const char *preamble = "void dooditty(void) {\n";
    const char *postamble = "}";
    char buf[strlen(preamble) + strlen(statements) + strlen(postamble) + 1];
    strcpy(buf, preamble);
    strcat(buf, statements);
    strcat(buf, postamble);

    puts("compiling...");
    if (tcc_compile_string(s, buf) == -1)
    {
        puts("error-compiling");
        usleep(1000000);
        return 1;
    }

    /* as a test, we add symbols that the compiled program can use.
       You may also open a dll with tcc_add_dll() and use symbols from that */
    // puts("adding symbols...");

    /* relocate the code */
    puts("relocating...");
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
    {
        puts("Error relocating symbols");
        return 1;
    }

    /* get entry symbol */
    puts("getting symbol 'dooditty'...");
    void (*func)(void) = tcc_get_symbol(s, "dooditty");
    if (!func)
    {
        puts("Could not find symbol 'dooditty'");
        return 1;
    }

    // /* run the code */
    func();

    return 0;
}

int main(int argc, char **argv)
{
    TCCState *s;
    sizeof(TCCState);
    int i;
    int (*func)(int);

    s = tcc_new();
    if (!s)
    {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }

    // s->verbose = 2;

    //     // /* if tcclib.h and libtcc1.a are not installed, where can we find them */
    //     // for (i = 1; i < argc; ++i) {
    //     //     char *a = argv[i];
    //     //     if (a[0] == '-') {
    //     //         if (a[1] == 'B')
    //     //             tcc_set_lib_path(s, a+2);
    //     //         else if (a[1] == 'I')
    //     //             tcc_add_include_path(s, a+2);
    //     //         else if (a[1] == 'L')
    //     //             tcc_add_library_path(s, a+2);
    //     //     }
    //     // }

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

  struct timespec debug_start_time, debug_end_time;
      clock_gettime(CLOCK_REALTIME, &debug_start_time);
    puts("compiling...");
    if (tcc_compile_string(s, my_program) == -1)
    {
        puts("error-compiling");
        usleep(1000000);
        return 1;
    }

    /* as a test, we add symbols that the compiled program can use.
       You may also open a dll with tcc_add_dll() and use symbols from that */
    puts("adding symbols...");
    tcc_add_symbol(s, "add", add);
    tcc_add_symbol(s, "hello", hello);

    /* relocate the code */
    puts("relocating...");
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
    {
        puts("Error relocating symbols");
        return 1;
    }

    /* get entry symbol */
    puts("getting symbol 'foo'...");
    func = tcc_get_symbol(s, "foo");
    if (!func)
    {
        puts("Could not find symbol 'foo'");
        return 1;
    }


    // /* run the code */
      clock_gettime(CLOCK_REALTIME, &debug_end_time);
      printf("compilation took %.2f ms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
                                                      1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));


    // tcc_process_statement_list(s, "int a = 18;\nprintf(\"a=%i +22=%i\\n\", a, a + 22);");

    /* delete the state */
    tcc_delete(s);
    func(32);

    printf("\n<libtcc_text-exit>\n");
    return 0;
}

// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <map>
// #include <stdarg.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string>
// #include <time.h>
// #include <unistd.h>
// #include <vector>

// #include "cling/Interpreter/Interpreter.h"

// using namespace std;

// cling::Interpreter *clint;

// int main(int argc, const char *const *argv)
// {

//   // char buffer[200];
//   // getcwd(buffer, 200);
//   const char *LLVMDIR = "/home/jason/cling/inst";
//   clint = new cling::Interpreter(argc, argv, LLVMDIR);

//   // clint->loadFile("/home/jason/midge/src/main/remove_mc_mcva_calls.c");
//   // clint->process("remove_all_MCcalls();");
//   // return 0;

//   clint->AddIncludePath("/home/jason/midge/src");
//   clint->AddIncludePath("/home/jason/cling/inst/include");

//   clint->loadFile("/home/jason/midge/src/midge.h");
//   char buf[512];
//   sprintf(buf, "clint = (cling::Interpreter *)%p;", (void *)clint);
//   clint->process(buf);

//   int result;
//   sprintf(buf, "*(int *)(%p) = _midge_run();", &result);
//   clint->process(buf);

//   delete (clint);

//   // IGNORE_MIDGE_ERROR_REPORT = true;

//   usleep(1000000);
//   return result;
// }