/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tcc.h"

// typedef struct mc_compiler_index {
//   unsigned int dyn_statement_invoke_uid;

//   TCCState *tcc_state;

// } mc_compiler_index;

// int tcc_process_statement_list(mc_compiler_index *mci, const char *statements)
// {
//   struct timespec debug_start_time, debug_end_time;
//   clock_gettime(CLOCK_REALTIME, &debug_start_time);

//   TCCState *s = tcc_new();
//   if (!s) {
//     fprintf(stderr, "Could not create tcc state\n");
//     exit(1);
//   }

//   char temp_func_name[32];
//   strcpy(temp_func_name, "mc_tempstfc");
//   sprintf(temp_func_name + strlen(temp_func_name), "%u", mci->dyn_statement_invoke_uid++);

//   const char *temp_func_return_type = "int ";
//   const char *rest_of_temp_func = "(void) {\n";
//   const char *postamble = "\nreturn 0;\n}";
//   char buf[strlen(temp_func_return_type) + strlen(temp_func_name) + strlen(rest_of_temp_func) + strlen(statements) +
//            strlen(postamble) + 1];
//   strcpy(buf, "int ");
//   strcat(buf, temp_func_name);
//   strcat(buf, rest_of_temp_func);
//   strcat(buf, statements);
//   strcat(buf, postamble);

//   /* MUST BE CALLED before any compilation */
//   tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

//   puts("compiling...");
//   puts(buf);
//   if (tcc_compile_string(s, buf) == -1) {
//     puts("error-compiling");
//     usleep(1000000);
//     return 1;
//   }

//   /* as a test, we add symbols that the compiled program can use.
//      You may also open a dll with tcc_add_dll() and use symbols from that */
//   // puts("adding symbols...");

//   /* relocate the code */
//   //   puts("relocating...\n\n");
//   if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
//     puts("Error relocating symbols");
//     return 1;
//   }

//   /* get entry symbol */
//   puts("getting symbol...");
//   int (*func)(void) = tcc_get_symbol(s, temp_func_name);
//   if (!func) {
//     printf("Could not find temporary statement function: '%s'\n", temp_func_name);
//     return 1;
//   }

//   // /* run the code */
//   int mc_res = func();
//   if (mc_res) {
//     puts("Error occured executing temporary statements");
//   }

//   tcc_delete(s);

//   clock_gettime(CLOCK_REALTIME, &debug_end_time);
//   printf("compilation took %.2f ms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
//                                            1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
//   return 0;
// }

// int tcc_declare_function(TCCState *s, const char *function, const char *function_name)
// {
//   struct timespec debug_start_time, debug_end_time;
//   clock_gettime(CLOCK_REALTIME, &debug_start_time);

//   puts("compiling...");
//   // puts(buf);
//   if (tcc_compile_string(s, function) == -1) {
//     puts("error-compiling");
//     usleep(1000000);
//     return 1;
//   }

//   /* as a test, we add symbols that the compiled program can use.
//      You may also open a dll with tcc_add_dll() and use symbols from that */
//   // puts("adding symbols...");

//   /* relocate the code */
//   puts("relocating...\n\n");
//   if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
//     puts("Error relocating symbols");
//     return 1;
//   }

//   /* get entry symbol */
//   // puts("getting symbol...");
//   int (*func)(void) = tcc_get_symbol(s, function_name);
//   if (!func) {
//     printf("Could not find temporary statement function: '%s'\n", function_name);
//     return 1;
//   }

//   // /* run the code */
//   int mc_res = func();
//   if (mc_res) {
//     puts("Error occured executing temporary statements");
//   }

//   clock_gettime(CLOCK_REALTIME, &debug_end_time);
//   printf("compilation took %.2f ms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
//                                            1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
//   return 0;
// }

// void tcc_original()
// {

//   TCCState *s = tcc_new();
//   if (!s) {
//     fprintf(stderr, "Could not create tcc state\n");
//     exit(1);
//   }

//   /* MUST BE CALLED before any compilation */
//   tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

//   // puts("compiling...alpha");
//   // const char *program_a =
//   // // "struct fruit { int a, b, c; };\n"
//   // // "typedef struct fruit fruit_t;\n"
//   // // "typedef struct apple { fruit f; float radius; } apple_t;\n"
//   // "const char *APPLE_NAME = \"Apple\"\n;"
//   // "const char *PEACH_NAME = \"Peach\"\n;"
//   // "void apple() { puts(APPLE_NAME); }\n"
//   // "void alpha() { apple(); puts(PEACH_NAME); }\n";
//   const char *program_a = "void alpha() { const char *cobeta = \"alpha\";\n puts(cobeta); }\n";
//   if (tcc_compile_string(s, program_a) == -1) {
//     puts("error-compiling");
//     usleep(1000000);
//     exit(1);
//   }
//   tcc_print_stats(s, 1);

//   /* relocate the code */
//   // puts("relocating...");
//   if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
//     puts("Error relocating symbols");
//     exit(1);
//   }

//   /* get entry symbol */
//   // puts("getting symbol 'alpha'...\n");
//   void (*alpha)(void) = tcc_get_symbol(s, "alpha");
//   if (!alpha) {
//     puts("Could not find symbol 'alpha'");
//     exit(1);
//   }
//   alpha();

//   // puts("COMPILING...beta");
//   // const char *program_b = "void beta() { alpha(); puts(\"beta\"); }";
//   // if (tcc_compile_string(s, program_b) == -1) {
//   //   puts("error-compiling");
//   //   usleep(1000000);
//   //   exit(1);
//   // }
//   // tcc_print_stats(s, 1);

//   // /* get entry symbol */
//   // // puts("getting symbol 'alpha'...\n");
//   // alpha = tcc_get_symbol(s, "alpha");
//   // if (!alpha) {
//   //   puts("Could not find symbol 'alpha'");
//   //   exit(1);
//   // }
//   // alpha();

//   // /* relocate the code */
//   // puts("relocating after beta...\n\n");
//   // if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
//   //   puts("Error relocating symbols");
//   //   exit(1);
//   // }
//   // alpha();

//   // // /* as a test, we add symbols that the compiled program can use.
//   // //    You may also open a dll with tcc_add_dll() and use symbols from that */
//   // // puts("adding symbols...");
//   // // tcc_add_symbol(s, "add", add);
//   // // tcc_add_symbol(s, "hello", hello);

//   // /* get entry symbol */
//   // // puts("getting symbol 'beta'...\n");
//   // void (*func)(void) = tcc_get_symbol(s, "beta");
//   // if (!func) {
//   //   puts("Could not find symbol 'beta'");
//   //   exit(1);
//   // }
//   // func();

//   // tcc_delete(s);

//   // // mc_compiler_index mci;
//   // // mci.dyn_statement_invoke_uid = 0;
//   // //   mci.tcc_state = s;

//   // // for (int i = 0; i < 20; ++i) {
//   // //   char fnbuf[32];
//   // //   sprintf(fnbuf, "dooditty_%i", i);

//   // //   char buf[512];
//   // //   buf[0] = '\0';

//   // //   if (i > 0) {
//   // //     sprintf(buf, "extern int dooditty_%i(void);\n\n", i - 1);
//   // //   }

//   // //   sprintf(buf + strlen(buf), "int %s(void) {\nint a = %i;\nprintf(\"a:%%i\", a);\n", fnbuf, i);
//   // //   if (i > 0) {
//   // //     sprintf(buf + strlen(buf), "a += dooditty_%i();\nprintf(\"  atot:%%i\", a);\n", i - 1);
//   // //   }
//   // //   sprintf(buf + strlen(buf), "printf(\"\\n\");\nreturn 0;\n}");

//   // //   if (tcc_declare_function(s, buf, fnbuf)) {
//   // //     return 1;
//   // //   }
//   // //   // if (tcc_process_statement_list(&mci, buf))//"int a = 18;\nputs(\"18\");"))//printf(\"a=%i +22=%i\\n\",
//   a, a
//   // +
//   // //   // 22);"))
//   // //   //   return 1;
//   // // }
//   // //   if (tcc_process_statement_list(&mci, "int a = 19;\nputs(\"19\");"))//printf(\"a=%i +24=%i\\n\", a, a +
//   24);"))
//   // //     return 1;

//   // /* delete the state */
//   // // func(32);
// }

// int main(int argc, char **argv)
// {
//   int i;

//   TCCInterpState *ds;
//   ds = tcci_new();

//   // const char *struct_a = "typedef struct pummel { int target; void *sorty; } pummel;";
//   // if (tcc_interpret_file(ds, "my_code_file.c", 7 /* line_offset */, struct_a)) {
//   //   puts("error interpreting pummel");
//   //   exit(1);
//   // }

//   // tcc_original();
//   puts("\n####################\n\n");

//   // const char *program_a = "const char *global_str = \"pre#alpha\";\n"
//   //                         "static int global_phi = 88;\n"
//   //                         "struct omega { int a; char *b; };\n"
//   //                         "typedef struct omega omega;\n"
//   //                         "static void prealpha() { puts(global_str); }\n"
//   //                         "void alpha() { prealpha();\n puts(\"alpha\"); }\n";

//   const char *program_a = "void alpha() { const char *cobeta = \"alpha\";\n puts(cobeta); }\n";
//   if (tcc_interpret_file(ds, "alpha.c" /* line_offset */, program_a)) {
//     puts("error interpreting alpha");
//     usleep(100000);
//     exit(1);
//   }

//   void (*alpha)(void) = tcci_get_symbol(ds, "alpha");
//   if (!alpha) {
//     puts("could not find symbol alpha");
//     usleep(100000);
//     exit(1);
//   }
//   puts("#####Calling alpha()#####");
//   alpha();
//   puts("#########################");

//   const char *program_b = "void beta() { alpha();\n puts(\"then beta\");\n }\n";
//   if (tcc_interpret_file(ds, "beta.c" /* line_offset */, program_b)) {
//     puts("error interpreting beta");
//     usleep(100000);
//     exit(1);
//   }

//   void (*beta)(void) = tcci_get_symbol(ds, "beta");
//   if (!beta) {
//     puts("could not find symbol beta");
//     usleep(100000);
//     exit(1);
//   }
//   puts("#####Calling beta()#####");
//   beta();
//   puts("#########################");

//   puts("#####Redefining alpha()#####");
//   const char *program_a_2 = "void alpha() { const char *cobeta = \"alpha\";\n printf(\"%s \", cobeta); }\n";
//   if (tcc_interpret_file(ds, "alpha.c" /* line_offset */, program_a_2)) {
//     puts("error interpreting alpha 2");
//     usleep(100000);
//     exit(1);
//   }
//   puts("#########################");

//   puts("##Calling beta() again###");
//   beta();
//   puts("#########################");
//   tcci_delete(ds);

//   usleep(100000);

//   printf("\n<libtcc_text-exit>\n");
//   return 0;
// }

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

static TCCInterpState *__mc_itp;

int mcc_interpret_file(const char *filepath)
{
  // Read the file
  FILE *f = fopen(filepath, "rb");
  if (!f) {
    printf("ERR[44] Could not open filepath:'%s'\n", filepath);
    return 44;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *contents = (char *)malloc(fsize + 1);
  fread(contents, sizeof(char), fsize, f);
  fclose(f);
  contents[fsize] = '\0';

  printf("...interpreting file '%s'\n", filepath);
  return tcc_interpret_file(__mc_itp, filepath, contents);
}

int main(int argc, const char *const *argv)
{
  int res;

  __mc_itp = tcci_new();

  tcci_add_symbol(__mc_itp, "mcc_interpret_file", &mcc_interpret_file);

  res = mcc_interpret_file("/home/jason/midge/src/midge.h");
  if (res) {
    printf("\nCompilation Failed. res=%i\n", res);
    goto do_exit;
  }

  int (*_midge_run)() = tcci_get_symbol(__mc_itp, "_midge_run");
  res = _midge_run();

  // clint->loadFile("/home/jason/midge/src/main/remove_mc_mcva_calls.c");
  // clint->process("remove_all_MCcalls();");
  // return 0;

  // clint->AddIncludePath("/home/jason/midge/src");
  // clint->AddIncludePath("/home/jason/cling/inst/include");

  // clint->loadFile("/home/jason/midge/src/midge.h");
  // char buf[512];
  // sprintf(buf, "clint = (cling::Interpreter *)%p;", (void *)clint);
  // clint->process(buf);

  // int result;
  // sprintf(buf, "*(int *)(%p) = _midge_run();", &result);
  // clint->process(buf);

  // IGNORE_MIDGE_ERROR_REPORT = true;

do_exit:
  usleep(1000000);
  tcci_delete(__mc_itp);
  exit(res);
}