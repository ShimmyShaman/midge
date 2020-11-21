/* core_source_loader.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core_definitions.h"
#include "mc_str.h"
#include "midge_error_handling.h"

#include "tinycc/libtccinterp.h"

static int _mcl_obtain_core_source_information(void **p_core_source_info)
{
  const int STRUCT = 11;
  const int FUNCTION = 12;
  const int ENUM = 13;
  struct core_object_info {
    int type;
    const char *identity;
    const char *filepath;
  };

  const int MAX_OBJECTS_COUNT = 50;
  struct core_object_info *objects =
      (struct core_object_info *)malloc(sizeof(struct core_object_info) * MAX_OBJECTS_COUNT);
  int objects_index = 0;

  // Set
  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "struct_id";
  objects[objects_index++].filepath = "src/core/core_definitions.h";

  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "node";
  objects[objects_index++].filepath = "src/core/core_definitions.h";

  objects[objects_index].type = 0;
  if (objects_index >= MAX_OBJECTS_COUNT) {
    MCerror(17, "Increase Array Size");
  }
  *p_core_source_info = (void *)objects;

  return 0;
}

static int _mcl_read_all_file_text(const char *filepath, char **contents)
{
  // Load the text from the core functions directory
  FILE *f = fopen(filepath, "rb");
  if (!f) {
    MCerror(44, "Could not open filepath:'%s'", filepath);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  *contents = (char *)malloc(fsize + 1);
  fread(*contents, sizeof(char), fsize, f);
  fclose(f);

  (*contents)[fsize] = '\0';

  return 0;
}

size_t _mcl_save_text_to_file(char *filepath, char *text)
{
  FILE *f = fopen(filepath, "w");
  if (f == NULL) {
    printf("problem opening file '%s'\n", filepath);
    return 0;
  }
  fseek(f, 0, SEEK_SET);

  int len = strlen(text);

  size_t written = fwrite(text, sizeof(char), len, f);
  printf("written %zu bytes to %s\n", written, filepath);
  fclose(f);

  return written;
}

static int _mcl_find_sequence_in_text_ignoring_empty_text(const char *text, const char *first, const char *second,
                                                          const char *third, int *index)
{
  for (int i = 0;; ++i) {

    if (text[i] == '\0') {
      break;
    }

    // First
    int s;
    for (s = 0; first; ++s) {
      if (first[s] == '\0') {
        break;
      }
      if (text[i + s] != first[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    // Second
    int t = s;
    for (s = 0; second; ++s) {
      if (second[s] == '\0') {
        break;
      }
      if (text[i + t + s] != second[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    // Third
    t = t + s;
    for (s = 0; third; ++s) {
      if (third[s] == '\0') {
        break;
      }
      if (text[i + t + s] != third[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    *index = i;
    return 0;
  }

  MCerror(60, "Could not find '%s'>'%s'>'%s'", first, second, third);
}

const char *_mcl_core_structs[] = {
    // midge_common.h
    "mc_str",

    // core_definitions.h
    "source_file_type",
    "source_definition_type",
    "node_type",
    "parameter_kind",
    "field_kind",
    "preprocessor_define_type",
    "struct_id",
    "source_definition",
    "mc_global_data",
    "mc_source_file_info",
    "function_info",
    "parameter_info",
    "field_declarator_info",
    "struct_info",
    "enum_member_info",
    "enumeration_info",
    "event_handler_array",
    "preprocess_define_info",
    "field_info",
    "field_info_list",
    "mc_node_list",
    "mc_node",

    "mc_syntax_node",
    "mc_syntax_node_type",
    "mc_token_type",
    "mc_syntax_node_list",
    "parsing_state",

    // mc_code_transcription.c
    "mct_transcription_scope_variable",
    "mct_transcription_scope",
    "mct_transcription_state",
    "mct_expression_type_info",
    "mct_function_transcription_options",
    "mct_function_variable_value",
    "mct_function_variable_value_list",
    "mct_function_variable_report_index",
    "mct_statement_transcription_info",

    // And everything here before -------------------------------------------------------------
    NULL,
};

const char *_mcl_ignore_functions[] = {
    "printf",
    "puts",
    "strcat",
    "strcmp",
    "strncmp",
    "free",
    "strcpy",
    "strncpy",
    "sprintf",
    "memcpy",
    "memmove",
    "register_midge_error_tag",
    "va_start",
    "va_end",
    "MCerror",
    "stat",
    "fseek",
    "fread",
    "fclose",
    "cprintf",
    "isalpha",
    "usleep",
    // "clint_declare",

    // And everything here before -------------------------------------------------------------
    NULL,
};

const char *_mcl_core_functions[] = {
    "tcci_add_string",

    // midge_common
    "init_mc_str",
    "init_mc_str_with_specific_capacity",
    "set_mc_str",
    "set_mc_strn",
    "release_mc_str",
    "append_char_to_mc_str",
    "append_to_mc_str",
    "append_to_mc_strn",
    "append_to_mc_strf",
    "insert_into_mc_str",
    "restrict_mc_str",

    // core_definitions
    "obtain_midge_global_root",
    "mc_throw_delayed_error",
    "read_file_text",
    "append_to_collection",
    "insert_in_collection",
    "remove_from_collection",
    "remove_ptr_from_collection",
    "find_function_info",
    "find_function_info_by_ptr",
    "find_struct_info",
    "find_enumeration_info",
    "find_enum_member_info",
    "release_field_declarator_info",
    "release_field_declarator_info_list",
    "release_field_info",
    "release_field_info_list",
    "release_parameter_info",
    "mcs_parse_array_initializer_assignment_expression",

    // c_parser_lexer
    "_mcs_print_syntax_node_ancestry",
    "mcs_add_syntax_node_to_parent",
    "get_mc_token_type_name",
    "get_mc_syntax_token_type_name",
    "_mcs_copy_syntax_node_to_text",
    "print_syntax_node",
    "print_parse_error",
    "_mcs_parse_token",
    "mcs_is_supernumerary_token",
    "mcs_construct_syntax_node",
    "mcs_peek_token_type_and_index",
    "mcs_parse_token",
    "mcs_peek_token_type",
    "mcs_parse_through_token",
    "mcs_parse_through_supernumerary_tokens",
    "mcs_parse_function_pointer_declarator",
    "mcs_parse_type_identifier",
    "mcs_parse_initializer_expression",
    "mcs_parse_dereference_sequence",
    "mcs_parse_field_declarator",
    "mcs_parse_field_declarators",
    "mcs_parse_field_declaration",
    "mcs_parse_extern_c_block",
    "mcs_parse_statement",
    "mcs_parse_expression_unary",
    "mcs_parse_root_statement",
    "mcs_parse_goto_statement",
    "mcs_parse_label_statement",
    "mcs_parse_pp_ifdef",
    "mcs_parse_pp_ifndef",

    // mc_code_transcription
    "mct_release_expression_type_info_fields",
    "mct_transcribe_field_declarators",
    "mct_transcribe_field_list",
    "mct_transcribe_statement",
    "_determine_type_of_expression_subsearch",
    "determine_type_of_expression",
    "mct_increment_scope_depth",
    "mct_decrement_scope_depth",
    "mct_add_scope_variable",
    "mct_transcribe_non_mc_invocation",
    "mct_transcribe_function_pointer_declarator",
    "mct_transcribe_function_pointer_declaration",
    // "mct_transcribe_mc_invocation_argument",
    "mct_syntax_descendants_contain_node_type",
    "mct_transcribe_fptr_invocation",
    "mct_transcribe_goto_statement",
    "mct_transcribe_label_statement",
    "mct_transcribe_variable_value_report",
    "mct_transcribe_struct_declaration",
    "mct_transcribe_enum_declaration",
    "mct_transcribe_function",
    "mct_transcribe_type_alias",
    "mct_transcribe_function_end",
    "mct_transcribe_indent",
    "mct_transcribe_text_with_indent",
    "mct_transcribe_file_root_children",
    "mct_transcribe_file_ast",
    "mcs_append_syntax_node_to_mc_str",

    // mc_source
    "attach_struct_info_to_owner",
    "attach_function_info_to_owner",
    "mcs_summarize_field_declarator_list",
    "mcs_summarize_type_field_list",
    "mcs_register_struct_declaration",
    "mcs_process_ast_root_children",
    "mcs_process_file_ast",
    "mcs_interpret_file",
    "mcs_register_function_declaration",
    "mcs_register_function_definition",
    "mcs_register_struct_declaration",
    "mcs_register_struct_definition",

    "parse_file_to_syntax_tree",
    "mc_init_source_file_info",
    "release_struct_id",
    "release_syntax_node",
    "mcs_parse_parameter_declaration",
    "mcs_parse_expression",
    "mcs_parse_expression_variable_access",
    "mcs_parse_parenthesized_expression",
    "mcs_parse_cast_expression",
    "_mcs_parse_expression",
    "mcs_parse_local_declaration",
    "mcs_parse_expression_beginning_with_bracket",
    "mcs_parse_expression_conditional",
    "mcs_parse_code_block",
    "mcs_parse_statement_list",
    "mcs_parse_if_statement",
    "mcs_parse_for_statement",
    "mcs_parse_switch_statement",
    "mcs_parse_while_statement",
    "mcs_parse_simple_statement",
    "mcs_parse_return_statement",
    "mcs_parse_function_pointer_declaration",
    "mcs_parse_expression_statement",
    "mcs_parse_local_declaration_statement",
    "mcs_parse_function_definition_header",
    "mcs_parse_type_alias_definition",
    "mcs_parse_type_declaration",
    "mcs_parse_enum_definition",
    "mcs_parse_function_definition",
    "mcs_parse_preprocessor_directive",
    "mcs_parse_struct_declaration_list",
    "mcs_copy_syntax_node_to_text",
    "mct_contains_mc_invoke",
    "mcs_append_syntax_node_to_mc_str",
    "mct_transcribe_expression",
    "mct_transcribe_type_identifier",
    "mct_transcribe_invocation",
    "mct_transcribe_declarator",
    "mct_transcribe_code_block",
    "mct_transcribe_if_statement",
    "mct_transcribe_statement_list",
    "mct_transcribe_for_statement",
    "mct_transcribe_while_statement",
    "mct_transcribe_switch_statement",
    "mct_transcribe_declaration_statement",
    "mct_transcribe_field",
    "mct_transcribe_mcerror",
    "mct_transcribe_va_list_statement",
    "mcs_parse_va_list_statement",
    "mcs_parse_va_start_statement",
    "mcs_parse_va_end_statement",
    "remove_ptr_from_collection",
    "initialize_parameter_info_from_syntax_node",
    "attach_struct_info_to_owner",
    "attach_enumeration_info_to_owner",
    "mct_transcribe_function_to_mc",
    "update_or_register_struct_info_from_syntax",
    "register_sub_type_syntax_to_field_info",
    "transcribe_struct_to_mc",
    "parse_definition_to_syntax_tree",

    // "METHOD_HERE",

    // And everything here before -------------------------------------------------------------
    "instantiate_all_definitions_from_file",
    NULL,
};

static int _mcl_print_parse_error(const char *const text, int index, const char *const function_name,
                                  const char *section_id)
{
  const int LEN = 84;
  const int FH = LEN / 2 - 2;
  const int SH = LEN - FH - 3 - 1;
  char buf[LEN];
  for (int i = 0; i < FH; ++i) {
    if (index - FH + i < 0)
      buf[i] = ' ';
    else
      buf[i] = text[index - FH + i];
  }
  buf[FH] = '|';
  if (text[index] == '\0') {
    buf[FH + 1] = '\\';
    buf[FH + 2] = '0';
    buf[FH + 3] = '|';
    for (int i = 1; i < SH; ++i) {
      buf[FH + 3 + i] = ' ';
    }
  }
  else {
    buf[FH + 1] = text[index];
    buf[FH + 2] = '|';
    char eof = text[index] == '\0';
    for (int i = 0; i < SH; ++i) {
      if (eof)
        buf[FH + 3 + i] = ' ';
      else {
        eof = text[index + 1 + i] == '\0';
        buf[FH + 3 + i] = text[index + 1 + i];
      }
    }
  }
  buf[LEN - 1] = '\0';

  printf("\n%s>%s#unhandled-char:'%s'\n", function_name, section_id, buf);

  return 0;
}

const char *_mcl_core_files[] = {
    "src/core/core_definitions.h",
    "src/core/c_parser_lexer.h",
    "src/core/mc_code_transcription.h",
    "src/core/core_definitions.c",
    "src/core/c_parser_lexer.c",
    "src/core/mc_code_transcription.c",
    // And everything here before -------------------------------------------------------------
    "src/core/mc_source.c",
    NULL,
};

#define MCL_CORE_SOURCE_FILE_COUNT 4
const char *_mcl_core_source_files[] = {
    // "dep/tinycc/lib/va_list.c", // TODO -- this
    // "src/midge_error_handling.c",
    "src/core/core_definitions.c",
    "src/core/c_parser_lexer.c",
    "src/core/mc_code_transcription.c",
    // And everything here before -------------------------------------------------------------
    "src/core/mc_source.c",
    NULL,
};

static int _mcl_determine_cached_file_name(const char *input, char **output)
{
  mc_str *str;
  MCcall(init_mc_str(&str));
  MCcall(set_mc_str(str, "bin/cached/_cmc_"));

  int fni = 0;
  fni += 11;
  for (int k = 0; k < 256; ++k) {
    if (input[k] == '/') {
      append_to_mc_str(str, "_");
    }
    else {
      if (input[k] == '\0') {
        break;
      }
      char cs[3];
      cs[0] = input[k];
      cs[1] = '\0';
      append_to_mc_str(str, cs);
    }
  }

  *output = str->text;
  release_mc_str(str, false);

  return 0;
}

// int _mcl_load_core_temp_source(TCCInterpState *tmp_itp)
// {
//   mc_str *src;
//   MCcall(init_mc_str(&src));

//   for (int a = 0; _mcl_core_source_files[a]; ++a) {
//     char *file_text;

//     char *cached_file_name;
//     _mcl_determine_cached_file_name(_mcl_core_source_files[a], &cached_file_name);

//     // Compare modified times and process new source or use cache
//     bool use_cached_file = false;
//     if (access(cached_file_name, F_OK) != -1) {

//       struct stat src_attrib;
//       stat(_mcl_core_source_files[a], &src_attrib);

//       struct stat cch_attrib;
//       stat(cached_file_name, &cch_attrib);

//       use_cached_file = (src_attrib.st_mtime < cch_attrib.st_mtime);
//     }

//     // use_cached_file = false;
//     if (use_cached_file) {
//       MCcall(_mcl_read_all_file_text(cached_file_name, &file_text));
//       MCcall(set_mc_str(src, file_text));
//       free(file_text);

//       printf("declaring file[cch]:'%s'\n", _mcl_core_source_files[a]);
//     }
//     else {
//       // Read & process source
//       MCcall(_mcl_read_all_file_text(_mcl_core_source_files[a], &file_text));
//       MCcall(set_mc_str(src, file_text));
//       free(file_text);

//       printf("declaring file[src]:'%s'\n", _mcl_core_source_files[a]);
//       MCcall(_mcl_format_core_file(src, a));
//     }

//     MCcall(tcci_add_string(tmp_itp, _mcl_core_source_files[a], src->text));

//     if (!use_cached_file) {
//       // Save the processed version to file
//       _mcl_save_text_to_file(cached_file_name, src->text);
//     }

//     free(cached_file_name);
//     // MCcall(clint_process("printf(\"%p\", &mc_core_v_init_mc_str);//mc_strtr *str;
//     mc_core_v_inimc_strstr(&str);}"));
//   }

//   release_mc_str(src, true);

//   return 0;
// }

static int mcl_load_core_source(TCCInterpState *tmp_itp)
{
  int (*mcs_interpret_file)(TCCInterpState *, const char *) = tcci_get_symbol(tmp_itp, "mcs_interpret_file");
  if (!mcs_interpret_file) {
    MCerror(1240, "Couldn't obtain mcs_interpret_file");
  }

  char buf[512];
  for (int i = 0; _mcl_core_files[i]; ++i) {
    int result = 0;

    MCcall(mcs_interpret_file(tmp_itp, _mcl_core_files[i]));

    if (result != 0) {
      return result;
    }
  }

  return 0;
}

int mcl_load_remaining_app_source(TCCInterpState *itp)
{
  // Used by the MCcall macro
  void (*register_midge_stack_invocation)(const char *, const char *, int, int *) =
      tcci_get_symbol(itp, "register_midge_stack_invocation");
  void (*register_midge_stack_return)(int) = tcci_get_symbol(itp, "register_midge_stack_return");

  {
    // const char *_mcl_external_dependency_source_files[] = {
    //     "src/temp/external_decl.h",

    //     // And everything here before -------------------------------------------------------------
    //     NULL,
    // };

    // char buf[1536];
    // for (int i = 0; _mcl_external_dependency_source_files[i]; ++i) {
    //   printf("registering external file:'%s'\n", _mcl_external_dependency_source_files[i]);
    //   int result = 0;
    //   sprintf(buf,
    //           "{\n"
    //           "  //printf(\"obtain_midge_global_root:%%p\\n\", obtain_midge_global_root);\n"
    //           "  mc_core_v_mc_global_data *global_data;\n"
    //           "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"
    //           ""
    //           "  void *mc_vargs[4];\n"
    //           "  mc_vargs[0] = &global_data->global_node;\n"
    //           "  const char *filepath = \"%s\";\n"
    //           "  mc_vargs[1] = &filepath;\n"
    //           "  void * p_null = NULL;\n"
    //           "  mc_vargs[2] = &p_null;\n"
    //           "  int return_value;\n"
    //           "  mc_vargs[3] = &return_value;\n"
    //           ""
    //           "  {\n"
    //           "    int midge_error_stack_index;\n"
    //           "    register_midge_stack_invocation(\"instantiate_all_definitions_from_file\",
    //           \"core_source_loader.c\", " "          1224, &midge_error_stack_index);\n" "    int result = 0;\n" "
    //           result = register_external_definitions_from_file(4, mc_vargs);\n" "
    //           register_midge_stack_return(midge_error_stack_index);\n"
    //           ""
    //           "    if (result) {\n"
    //           "      printf(\"--register_external_definitions_from_file #in - clint_process\\n\");\n"
    //           "      *(int *)(%p) = result;\n"
    //           "    }\n"
    //           "  }\n"
    //           "}",
    //           _mcl_external_dependency_source_files[i], &result);
    //   // MCcall(mcc_interpret_and_execute_single_use_code("mcl_load_remaining_app_source]register_external_file.c",
    //   buf));

    //   if (result != 0) {
    //     return result;
    //   }
    // }
  }

  register_midge_error_tag("mcl_load_remaining_app_source()");
  const char *_mcl_app_source_files[] = {
      // TODO -- this better, should maybe not have to declare headers first (preferably at all)
      // Headers required for app initialization
      "src/m_threads.h",
      "src/platform/mc_xcb.h",
      "src/render/render_common.h",
      "src/render/mc_vulkan.h",
      "src/render/mc_vk_utils.h",
      "src/render/render_thread.h",

      // Headers that will be declared just before app initialization
      "src/env/environment_definitions.h",
      "src/ui/ui_definitions.h",
      "src/control/mc_controller.h",
      "src/core/midge_app.h",

      // Source required for app initialization
      "src/m_threads.c",
      "src/env/util.c",
      "src/platform/mc_xcb.c",
      "src/render/render_common.c",
      "src/render/mc_vulkan.c",
      "src/render/mc_vk_utils.c",
      "src/render/render_thread.c",
      "src/core/app_modules.c",
      "src/ui/ui_functionality.c",
      "src/control/mc_controller.c",
      "src/core/midge_app.c",

      // And everything here before -------------------------------------------------------------
      NULL,
  };

  int (*mcs_interpret_file)(TCCInterpState *, const char *) = tcci_get_symbol(itp, "mcs_interpret_file");
  if (!mcs_interpret_file) {
    MCerror(1409, "Could not obtain mcs_interpret_file");
  }

  for (int i = 0; _mcl_app_source_files[i]; ++i) {

    MCcall(mcs_interpret_file(itp, _mcl_app_source_files[i]));
  }

  return 0;
}

struct foo {
  int a;
  char b[3];
  double e;
};

// void init_assert_tcc_error_TODO()
// {
//   // TODO make this work in tcc?
//   // Works
//   int t[] = {4, 2};

//   // Causes initializer overflow in init_assert() in tcc
//   struct foo m = {};
//   struct foo n = {};

//   struct foo f[] = {m, n};

//   return 0;
// }

/* Builds a loader on the passed in interpreter state, using that to load midge into another
     initialized interpreter state which is then returned in the pointer reference. Note: Cleanup
     of the passed in interpreter will be dealt with by this method. Only freeing/deletion of the
     returned interpreter state is required by the caller. */
int mcl_load_app_source(TCCInterpState *itp, TCCInterpState **mc_interp, int *mc_interp_error_thread_index)
{
  // mcl_exp();
  // exit(8);

  // Begin error handling for the temp source loading
  void (*init_error_handling)(void) = tcci_get_symbol(itp, "initialize_midge_error_handling");
  init_error_handling();

  unsigned int temp_source_error_thread_index;
  int temp_source_error_stack_index;
  void (*register_midge_thread_creation)(unsigned int *, const char *, const char *, int, int *) =
      tcci_get_symbol(itp, "register_midge_thread_creation");
  {
    register_midge_thread_creation(&temp_source_error_thread_index, "mcl_load_app_source", "core_source_loader.c", 1463,
                                   &temp_source_error_stack_index);
  }

  // Used by the MCcall macro
  void (*register_midge_stack_invocation)(const char *, const char *, int, int *) =
      tcci_get_symbol(itp, "register_midge_stack_invocation");
  void (*register_midge_stack_return)(int) = tcci_get_symbol(itp, "register_midge_stack_return");

  // Initialize the new interpreter
  TCCInterpState *midge_itp;
  {
    midge_itp = tcci_new();

    // Add Include Paths & tcc symbols
    MCcall(tcci_add_include_path(midge_itp, "/home/jason/midge/src"));
    MCcall(tcci_add_include_path(midge_itp, "/home/jason/midge/dep"));

    tcci_set_symbol(midge_itp, "tcci_add_include_path", &tcci_add_include_path);
    tcci_set_symbol(midge_itp, "tcci_add_library", &tcci_add_library);
    tcci_set_symbol(midge_itp, "tcci_add_files", &tcci_add_files);
    tcci_set_symbol(midge_itp, "tcci_add_string", &tcci_add_string);
    tcci_set_symbol(midge_itp, "tcci_define_symbol", &tcci_define_symbol);
    tcci_set_symbol(midge_itp, "tcci_undefine_symbol", &tcci_undefine_symbol);
    tcci_set_symbol(midge_itp, "tcci_set_symbol", &tcci_set_symbol);
    tcci_set_symbol(midge_itp, "tcci_get_symbol", &tcci_get_symbol);
    tcci_set_symbol(midge_itp, "tcci_new", &tcci_new);
    tcci_set_symbol(midge_itp, "tcci_delete", &tcci_delete);

    // Allow obtaining of the midge interpreter from both interpreter states
    // char buf[128];
    // sprintf(buf,
    //         "#include \"tinycc/libtccinterp.h\"\n"
    //         "TCCInterpState *mc_obtain_interpreter() {\n"
    //         "  return (TCCInterpState *)%p;\n"
    //         "}",
    //         midge_itp);
    // MCcall(tcci_add_string(itp, "obtain_interpreter.c", buf));
    // MCcall(tcci_add_string(midge_itp, "obtain_interpreter.c", buf));

    const char *initial_compile_list[] = {
        "dep/tinycc/lib/va_list.c", // TODO -- this
        "src/midge_error_handling.c",
        "src/core/init_global_root.c",
        "src/mc_str.c",
    };
    MCcall(tcci_add_files(midge_itp, initial_compile_list, sizeof(initial_compile_list) / sizeof(const char *)));

    // Initialize midge global data and allow it to be obtained from temp source interpreter
    int (*init_mc_global_data)(TCCInterpState *) = tcci_get_symbol(midge_itp, "init_mc_global_data");
    MCcall(init_mc_global_data(midge_itp));

    // printf("mcl_load_app_source, addr:%p\n", mcl_load_app_source);

    // mc_global_data *data;
    // int (*obtain_midge_global_root)(mc_global_data **) = tcci_get_symbol(itp, "obtain_midge_global_root");
    // obtain_midge_global_root(&data);
    // printf("obtain_midge_global_root(before):%p %p\n", obtain_midge_global_root, data);

    int (*mdg_obtain_midge_global_root)(mc_global_data **) = tcci_get_symbol(midge_itp, "obtain_midge_global_root");
    tcci_set_symbol(itp, "obtain_midge_global_root", mdg_obtain_midge_global_root);

    // mdg_obtain_midge_global_root(&data);
    // printf("mdg_obtain_midge_global_root(after):%p %p\n", mdg_obtain_midge_global_root, data);

    // obtain_midge_global_root(&data);
    // printf("obtain_midge_global_root(after):%p %p\n", obtain_midge_global_root, data);

    tcci_add_library(midge_itp, "xcb");
    tcci_add_library(midge_itp, "vulkan");
    tcci_define_symbol(midge_itp, "VK_USE_PLATFORM_XCB_KHR", NULL);
  }

  // Load the rest of the temporary source
  puts("[_mcl_load_core_temp_source]");
  // MCcall(_mcl_load_core_temp_source(itp));
  MCcall(tcci_add_files(itp, _mcl_core_source_files, MCL_CORE_SOURCE_FILE_COUNT));

  // Begin loading into the midge interpreter state using the preload interpreter state
  puts("[mcl_load_core_source]");
  MCcall(mcl_load_core_source(itp));

  // Replace the temporary interpreter with the app version
  // -- Conclude the temporary interpreter
  void (*register_midge_thread_conclusion)(unsigned int) = tcci_get_symbol(itp, "register_midge_thread_conclusion");
  register_midge_thread_conclusion(temp_source_error_thread_index);

  // -- Switch error handling to be monitored by the mc loaded source interpreter
  init_error_handling = tcci_get_symbol(midge_itp, "initialize_midge_error_handling");
  init_error_handling();
  register_midge_thread_creation = tcci_get_symbol(midge_itp, "register_midge_thread_creation");

  register_midge_stack_invocation = tcci_get_symbol(midge_itp, "register_midge_stack_invocation");
  register_midge_stack_return = tcci_get_symbol(midge_itp, "register_midge_stack_return");

  {
    // Resume thread error handling
    int dummy_int;
    register_midge_thread_creation(mc_interp_error_thread_index, "mcl_load_app_source", "core_source_loader.c", 1555,
                                   &dummy_int);
  }

  // Load the remainder of the application source files with the new interpreter
  printf("[mcl_load_remaining_app_source]\n");
  MCcall(mcl_load_remaining_app_source(midge_itp));

  printf("[_mcl_load_source:COMPLETE]\n");
  *mc_interp = midge_itp;
  return 0;
}