/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "tinycc/libtccinterp.h"

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
//   // if (tcci_add_string(ds, "my_code_file.c", 7 /* line_offset */, struct_a)) {
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

//   int *f = malloc(sizeof(int) * 8);
//   printf("before:%p\n", f);

//   const char *program_a =
//       // "#include <stdlib.h>\n"
//       // "#include <stdio.h>\n"
//       "struct y;\n"
//       "struct x { struct y *p; /* ... */ };\n"
//       "struct y { struct x *q; /* ... */ };\n"
//       "void alpha() { struct x d; struct y e; d.p = &e; }\n";

//   if (tcci_add_string(ds, "alpha.c" /* line_offset */, program_a)) {
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

//   int *t = malloc(sizeof(int) * 8);
//   printf("after:%p\n", t);
//   // const char *program_b = "void beta() { alpha();\n puts(\"then beta\");\n }\n";
//   // if (tcci_add_string(ds, "beta.c" /* line_offset */, program_b)) {
//   //   puts("error interpreting beta");
//   //   usleep(100000);
//   //   exit(1);
//   // }

//   // void (*beta)(void) = tcci_get_symbol(ds, "beta");
//   // if (!beta) {
//   //   puts("could not find symbol beta");
//   //   usleep(100000);
//   //   exit(1);
//   // }
//   // puts("#####Calling beta()#####");
//   // beta();
//   // puts("#########################");

//   // puts("#####Redefining alpha()#####");
//   // const char *program_a_2 = "void alpha() { const char *cobeta = \"alpha\";\n printf(\"%s \", cobeta); }\n";
//   // if (tcci_add_string(ds, "alpha.c" /* line_offset */, program_a_2)) {
//   //   puts("error interpreting alpha 2");
//   //   usleep(100000);
//   //   exit(1);
//   // }
//   // puts("#########################");

//   // puts("##Calling beta() again###");
//   // beta();
//   // puts("#########################");
//   tcci_delete(ds);

//   usleep(100000);

//   printf("\n<libtcc_text-exit>\n");
//   return 0;
// }

// ############################################################################################################
// ############################################################################################################
// ############################################################################################################
// ############################################################################################################
// ############################################################################################################
// ############################################################################################################
// ############################################################################################################
// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <map>
// #include <stdarg.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string>
#include <time.h>
// #include <unistd.h>
// #include <vector>

static TCCInterpState *__mc_itp;

// int mcc_set_pp_define(const char *identifier, const char *value) { return tcci_set_pp_define(__mc_itp, identifier,
// value); }

int mcc_interpret_and_execute_single_use_code(const char *filename, const char *comma_seperated_includes,
                                              const char *contents)
{
  printf("\n...interpreting single-use '%s'\n", filename);
  return tcci_execute_single_use_code(__mc_itp, filename, comma_seperated_includes, contents);
}

int mcc_interpret_file_contents(const char *filename, const char *contents)
{
  printf("\n...interpreting file-str '%s'\n", filename);
  return tcci_add_string(__mc_itp, filename, contents);
}

int mcc_interpret_file_on_disk(const char *filepath)
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

  printf("\n...interpreting file-dsk '%s'\n", filepath);
  return tcci_add_string(__mc_itp, filepath, contents);
}

int mcc_interpret_files_in_block(const char **files, int nb_files)
{
  printf("\n...interpreting file-blk:\n");
  for (int i = 0; i < nb_files; ++i) {
    printf("--'%s'\n", files[i]);
  }
  return tcci_add_files(__mc_itp, files, nb_files);
}

void *mcc_set_global_symbol(const char *symbol_name, void *ptr) { return tcci_add_symbol(__mc_itp, symbol_name, ptr); }

void *mcc_get_global_symbol(const char *symbol_name) { return tcci_get_symbol(__mc_itp, symbol_name); }

#define MCcall(func_call)                          \
  res = func_call;                                 \
  if (res) {                                       \
    printf("\nCompilation Failed. res=%i\n", res); \
    goto do_exit;                                  \
  }

void doexp()
{
  // mcc_set_pp_define("IPPY", "83");
  // mcc_interpret_and_execute_single_use_code("sippy.c", "<stdio.h>", "printf(\"IPPY:%i\\n\", IPPY);");
  const char *code = "#include <stdio.h>\n"
                     "#define MCerror(error_code, error_message, ...)                                \\\n"
                     "printf(\"\\n\\nERR[%i]: \" error_message \"\\n\", error_code, ##__VA_ARGS__);  \\\n"
                     "return error_code;\n"
                     "\n"
                     "int doit() {\n"
                     "  MCerror(8472, \"ERR[%i] Dummy message\", 482);\n"
                     "}";

  mcc_interpret_file_contents("dummy_doit.c", code);

  int (*doit)() = mcc_get_global_symbol("doit");
  doit();
}

int main(int argc, const char *const *argv)
{
  int res = 0;
  struct timespec app_begin_time;

  // Begin
  clock_gettime(CLOCK_REALTIME, &app_begin_time);

  // Initialize the interpreter
  __mc_itp = tcci_new();

  // Add Include Paths
  MCcall(tcci_add_include_path(__mc_itp, "/home/jason/midge/src"));
  MCcall(tcci_add_include_path(__mc_itp, "/home/jason/midge/dep"));

  tcci_add_symbol(__mc_itp, "tcci_add_include_path", &tcci_add_include_path);
  tcci_add_symbol(__mc_itp, "tcci_add_files", &tcci_add_files);
  tcci_add_symbol(__mc_itp, "tcci_add_symbol", &tcci_add_symbol);
  tcci_add_symbol(__mc_itp, "tcci_new", &tcci_new);
  tcci_add_symbol(__mc_itp, "tcci_delete", &tcci_delete);

  tcci_add_symbol(__mc_itp, "mcc_interpret_and_execute_single_use_code", &mcc_interpret_and_execute_single_use_code);
  tcci_add_symbol(__mc_itp, "mcc_interpret_file_contents", &mcc_interpret_file_contents);
  tcci_add_symbol(__mc_itp, "mcc_interpret_file_on_disk", &mcc_interpret_file_on_disk);
  tcci_add_symbol(__mc_itp, "mcc_interpret_files_in_block", &mcc_interpret_files_in_block);
  tcci_add_symbol(__mc_itp, "mcc_set_global_symbol", &mcc_set_global_symbol);
  tcci_add_symbol(__mc_itp, "mcc_get_global_symbol", &mcc_get_global_symbol);

  // doexp();
  // exit(0);

#define USE_CORE_LOADER_FOR_DEBUGGING
#ifdef USE_CORE_LOADER_FOR_DEBUGGING
  const char *initial_compile_list[] = {
      "dep/tinycc/lib/va_list.c", // TODO -- this
      "src/midge_error_handling.c",
      // And everything here before -------------------------------------------------------------
      "src/core/core_source_loader.c",
      "src/core/init_global_root.c",
  };
  MCcall(mcc_interpret_files_in_block(initial_compile_list, 3));

  void (*init_error_handling)(void) = tcci_get_symbol(__mc_itp, "initialize_midge_error_handling");
  init_error_handling();

  void (*register_midge_thread_creation)(unsigned int *, const char *, const char *, int, int *) =
      tcci_get_symbol(__mc_itp, "register_midge_thread_creation");
  {
    unsigned int dummy_uint;
    int dummy_int;
    register_midge_thread_creation(&dummy_uint, "main", "main.c", 406, &dummy_int);
  }

  void *obtain_midge_global_root = tcci_get_symbol(__mc_itp, "obtain_midge_global_root");

  void (*mcl_load_app_source)(void *) = tcci_get_symbol(__mc_itp, "mcl_load_app_source");
  mcl_load_app_source(obtain_midge_global_root);
#else // For performance?

  const char *initial_compile_list[] = {
      "dep/tinycc/lib/va_list.c", // TODO -- this
      "src/midge_error_handling.c",
      "src/midge_common.c",
      "src/core/core_definitions.c",
      "src/core/c_parser_lexer.c",
      "src/core/mc_code_transcription.c",
      // And everything here before -------------------------------------------------------------
      "src/core/mc_source.c",
  };

  // usleep(100000);
  //   // Reload it from within midge
  //   // int instantiate_all_definitions_from_file(mc_node *definitions_owner, char *filepath, mc_source_file_info
  //   **source_file) int (*instantiate_all_definitions_from_file)(void *, char *, void **) =
  //   mcc_get_global_symbol("instantiate_all_definitions_from_file"); res = instantiate_all_definitions_from_file(NULL,
  //   "src/midge_common.c", NULL);

  //   printf("final res:%i\n", res);

#endif
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