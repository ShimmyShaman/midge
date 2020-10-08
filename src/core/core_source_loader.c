/* core_source_loader.c */

#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct _csl_c_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} _csl_c_str;

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true ((unsigned char)0xff)
#endif
#ifndef false
#define false ((unsigned char)0)
#endif

#define MC_ASSERT(condition, message)                        \
  if (!(condition)) {                                        \
    printf("MC-ASSERT ERR[" #condition "] :" #message "\n"); \
    return -37373;                                           \
  }

// TEMP will have to do function defines sooner or later
#define VK_CHECK(res, func_name)                               \
  if (res) {                                                   \
    printf("VK-ERR[%i] :%i --" func_name "\n", res, __LINE__); \
    return res;                                                \
  }
#define VK_ASSERT(condition, message)                        \
  if (!(condition)) {                                        \
    printf("VK-ASSERT ERR[" #condition "] :" #message "\n"); \
    return -11111;                                           \
  }
// int callit_nonmc()
// {
//   printf("!!callit-NON-mc!!\n");
//   return 0;
// }

#define MCcall(function)                                                                   \
  {                                                                                        \
    int mc_error_stack_index;                                                              \
    register_midge_stack_invocation(#function, __FILE__, __LINE__, &mc_error_stack_index); \
    int mc_res = function;                                                                 \
    if (mc_res) {                                                                          \
      printf("--" #function "line:%i:ERR:%i\n", __LINE__, mc_res);                         \
      return mc_res;                                                                       \
    }                                                                                      \
    register_midge_stack_return(mc_error_stack_index);                                     \
  }

#define MCvacall(function)                                                                 \
  {                                                                                        \
    int mc_error_stack_index;                                                              \
    register_midge_stack_invocation(#function, __FILE__, __LINE__, &mc_error_stack_index); \
    int mc_res = function;                                                                 \
    if (mc_res) {                                                                          \
      printf("-- line:%d varg-function-call:%i\n", __LINE__, mc_res);                      \
      return mc_res;                                                                       \
    }                                                                                      \
    register_midge_stack_return(mc_error_stack_index);                                     \
  }

void *__mch_thread_entry(void *state)
{
  unsigned int mc_error_thread_index;
  int base_error_stack_index;
  register_midge_thread_creation(&mc_error_thread_index, "__mch_thread_entry", "core_source_loader.c", 54,
                                 &base_error_stack_index);

  void **state_args = (void **)state;
  int (*mc_routine)(int, void **) = *(int (**)(int, void **))state_args[0];
  void *wrapped_state = state_args[1];

  void *mcf_vargs[2];
  mcf_vargs[0] = &wrapped_state;
  void *routine_result;
  mcf_vargs[1] = &routine_result;

  // printf("mc_routine ptr:%p\n", mc_routine);
  // printf("mc_routine deref ptr:%p\n", *mc_routine);
  {
    int mc_error_stack_index;
    register_midge_stack_invocation("unknown-thread-start-function", __FILE__, __LINE__ + 1, &mc_error_stack_index);
    int mc_res = mc_routine(2, mcf_vargs);
    if (mc_res) {
      printf("--unknown-thread-start-function: line:%i: ERR:%i\n", __LINE__ - 2, mc_res);
      return NULL;
    }
    register_midge_stack_return(mc_error_stack_index);
  }
  // printf("routine called\n");

  register_midge_thread_conclusion(mc_error_thread_index);
  // return routine_result;
  return NULL;
}

int init__csl_c_str(_csl_c_str **ptr)
{
  (*ptr) = (_csl_c_str *)malloc(sizeof(_csl_c_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int append_to__csl_c_str(_csl_c_str *cstr, const char *text)
{
  int len = strlen(text);
  if (cstr->len + len + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + len + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atcs-a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
  // printf("atcs-b text:'%s'\n", text);
  // memcpy(cstr->text + cstr->len, text, sizeof(char) * len);
  strcpy(cstr->text + cstr->len, text);
  // printf("atcs-c cstrtext:'%s' len:%u end:'%c'\n", cstr->text + cstr->len - 2, cstr->len, cstr->text[cstr->len]);
  cstr->len += len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int set__csl_c_str(_csl_c_str *cstr, const char *src)
{
  cstr->len = 0;
  cstr->text[0] = '\0';
  append_to__csl_c_str(cstr, src);

  return 0;
}

int release__csl_c_str(_csl_c_str *ptr, bool free_char_string_also)
{
  if (ptr->alloc > 0 && free_char_string_also && ptr->text) {
    free(ptr->text);
  }

  free(ptr);

  return 0;
}

int insert_into__csl_c_str(_csl_c_str *cstr, const char *text, int index)
{
  if (index > cstr->len) {
    MCerror(8169, "TODO");
  }

  int n = strlen(text);
  if (cstr->len + n + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + n + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    if (index) {
      memcpy(newptr, cstr->text, sizeof(char) * index);
    }
    memcpy(newptr + index, text, sizeof(char) * n);
    if (cstr->len - index) {
      memcpy(newptr + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
    }
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    cstr->len += n;
    cstr->text[cstr->len] = '\0';
    // printf("atc-8\n");
    return 0;
  }

  memmove(cstr->text + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
  memcpy(cstr->text + index, text, sizeof(char) * n);
  cstr->len += n;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int remove_from__csl_c_str(_csl_c_str *cstr, int start_index, int len)
{
  if (start_index > cstr->len || len == 0)
    return 0;

  if (start_index + len == cstr->len) {
    cstr->len = start_index;
    return 0;
  }

  int a;
  for (a = 0; start_index + len + a < cstr->len; ++a) {
    cstr->text[start_index + a] = cstr->text[start_index + len + a];
  }
  cstr->len -= len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int _mcl_obtain_core_source_information(void **p_core_source_info)
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

int _mcl_read_all_file_text(const char *filepath, char **contents)
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

int _mcl_find_sequence_in_text_ignoring_empty_text(const char *text, const char *first, const char *second,
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
    "c_str",

    // core_definitions.h
    "source_file_type",
    "source_definition_type",
    "node_type",
    "parameter_kind",
    "field_kind",
    "preprocessor_define_type",
    "struct_id",
    "source_definition",
    "global_root_data",
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
    "mct_transcription_state",
    "mct_expression_type_info",

    // And everything here before -------------------------------------------------------------
    NULL,
};

const char *_mcl_ignore_functions[] = {
    "printf",
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
    "allocate_and_copy_cstr",
    "allocate_and_copy_cstrn",
    // "clint_declare",

    // And everything here before -------------------------------------------------------------
    NULL,
};

const char *_mcl_core_functions[] = {
    "do_the_first_thing",
    "do_the_second_thing",
    "do_the_third_thing",
    "do_many_things",

    "clint_process",
    "clint_declare",

    // midge_common
    "init_c_str",
    "init_c_str_with_specific_capacity",
    "set_c_str",
    "set_c_strn",
    "release_c_str",
    "append_char_to_c_str",
    "append_to_c_str",
    "append_to_c_strn",
    "append_to_c_strf",
    "insert_into_c_str",
    "restrict_c_str",

    // core_definitions
    "obtain_midge_global_root",
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
    "_copy_syntax_node_to_text",
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
    "mct_transcribe_mc_invocation_argument",
    "mct_syntax_descendants_contain_node_type",
    // "get_keyword_const_text_name",
    "mct_transcribe_fptr_invocation",
    "mct_transcribe_goto_statement",
    "mct_transcribe_label_statement",

    // mc_source
    "summarize_field_declarator_list",
    "summarize_type_field_list",
    "transcribe_enumeration_to_mc",
    "instantiate_define_statement",
    "instantiate_struct_definition_from_ast",
    "instantiate_enum_definition_from_ast",
    "update_or_register_function_info_from_syntax",
    "attach_function_info_to_owner",
    "instantiate_function_definition_from_ast",
    "instantiate_ast_children",
    "instantiate_definition",
    "update_or_register_enum_info_from_syntax",
    "register_external_definitions_from_file",
    "register_external_enum_declaration",
    "register_external_declarations_from_syntax_children",
    "mcl_determine_cached_file_name",
    "attempt_instantiate_all_definitions_from_cached_file",
    "instantiate_definitions_from_cached_file",

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
    "copy_syntax_node_to_text",
    "mct_append_indent_to_c_str",
    "mct_contains_mc_invoke",
    "mct_append_to_c_str",
    "mct_append_node_text_to_c_str",
    "mct_transcribe_expression",
    "mct_transcribe_type_identifier",
    "mct_transcribe_mc_invocation",
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
    "transcribe_function_to_mc",
    "update_or_register_struct_info_from_syntax",
    "register_sub_type_syntax_to_field_info",
    "transcribe_struct_to_mc",
    "parse_definition_to_syntax_tree",

    // "METHOD_HERE",

    // And everything here before -------------------------------------------------------------
    "instantiate_all_definitions_from_file",
    NULL,
};

int mc_core_v_clint_declare(char *str) { return clint_declare(str); }
int mc_core_v_clint_process(char *str) { return clint_process(str); }

int _mcl_print_parse_error(const char *const text, int index, const char *const function_name, const char *section_id)
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

const char *_mcl_core_source_files[] = {
    "src/midge_common.h",
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

const char *_mcl_core_defines[] = {
    "MIDGE_COMMON_H",
    "CORE_DEFINITIONS_H",
    "C_PARSER_LEXER_H",
    "MC_CODE_TRANSCRIPTION_H",
    // And everything here before -------------------------------------------------------------
    NULL,
};

int _mcl_format_core_file(_csl_c_str *src, int source_file_index)
{
  // Replace Core Defines
  for (int i = 0; i < src->len; ++i) {
    for (int a = 0; _mcl_core_defines[a]; ++a) {
      if (!strncmp(src->text + i, _mcl_core_defines[a], strlen(_mcl_core_defines[a]))) {
        insert_into__csl_c_str(src, "MC_CORE_DEF_", i);
        i += 13;
        break;
      }
    }
  }

  // Search for more structs/enums
  for (int i = 0; i < src->len; ++i) {
    if (!strncmp(src->text + i, "typedef struct ", 15)) {
      int preamble_offset = i + 15;
      bool found = false;
      for (int o = 0; _mcl_core_structs[o]; ++o) {
        int n = strlen(_mcl_core_structs[o]);
        if (!strncmp(src->text + preamble_offset, _mcl_core_structs[o], n)) {
          found = true;
          break;
        }
      }
      if (!found) {
        char *type_name;
        for (int j = preamble_offset;; ++j) {
          if (!isalpha(src->text[j]) && src->text[j] != '_') {
            int len = j - (preamble_offset);
            type_name = (char *)malloc(sizeof(char) * (len + 1));
            strncpy(type_name, src->text + preamble_offset, len);
            type_name[len] = '\0';
            break;
          }
        }
        printf("core-type-to-add:\n\"%s\",\n\n\n", type_name);
        free(type_name);
        return -6;
      }
    }
    if (!strncmp(src->text + i, "typedef enum ", 14)) {
      int preamble_offset = i + 14;
      bool found = false;
      for (int o = 0; _mcl_core_structs[o]; ++o) {
        int n = strlen(_mcl_core_structs[o]);
        if (!strncmp(src->text + preamble_offset, _mcl_core_structs[o], n)) {
          found = true;
          break;
        }
      }
      if (!found) {
        char *type_name;
        for (int j = preamble_offset;; ++j) {
          if (!isalpha(src->text[j]) && src->text[j] != '_') {
            int len = j - (preamble_offset);
            type_name = (char *)malloc(sizeof(char) * (len + 1));
            strncpy(type_name, src->text + preamble_offset, len);
            type_name[len] = '\0';
            break;
          }
        }
        printf("core-type-to-add:\n\"%s\",\n\n\n", type_name);
        free(type_name);
        return -7;
      }
    }
  }

  // No longer accept MCcall / MCvacall
  for (int i = 0; i < src->len; ++i) {
    if (!strncmp(src->text + i, "MCcall", 6) || !strncmp(src->text + i, "MCvacall", 8)) {
      MCerror(332, "'%s' contains a MCcall/MCvacall", _mcl_core_source_files[source_file_index]);
    }
  }

  // Functions
  for (int i = 0; i < src->len; ++i) {
    // Remove Includes
    if (src->text[i] == '#') {
      if (!strncmp(src->text + i, "#include \"", 10)) {

        int s = i, e = i;
        while (src->text[e] != '\n') {
          ++e;
        }

        // printf("removing ...'");
        // for (int a = 0; a < e - s; ++a)
        //   printf("%c", src->text[s + a]);
        // printf("'");
        // printf("-'%i' chars\n", e - s);

        // Delete all includes
        MCcall(remove_from__csl_c_str(src, s, e - s));
        --i;
        // if (!strcmp(_mcl_core_source_files[a], "src/midge_common.h"))
        //   printf("def:\n%s||\n", src->text);
        //   return 0;
        continue;
      }
    }
  }

  for (int i = 0; i < src->len; ++i) {
    // Exceptions
    switch (src->text[i - 1]) {
    case '_':
    case '"':
      continue;
    default: {
      if (isalpha(src->text[i - 1]))
        continue;
      break;
    }
    }

    // Check against each object identity
    for (int o = 0; _mcl_core_structs[o]; ++o) {
      int n = strlen(_mcl_core_structs[o]);
      if (!strncmp(src->text + i, _mcl_core_structs[o], n)) {
        switch (src->text[i + n]) {
        case '_':
        case '"':
          continue;
        default: {
          if (isalpha(src->text[i + n]))
            continue;
          break;
        }
        }

        // Insert
        MCcall(insert_into__csl_c_str(src, "mc_core_v_", i));
        i += 10 + n - 1;
      }
    }
  }

  // Error-Handling
  for (int i = 0; i < src->len; ++i) {
    if (src->text[i] == '(') {
      // Is it a function that can be "MCcalled"
      // It has to be a direct function call (a-z,_, no dereferences)
      // It has to be connected to the previous line by only spaces
      bool valid = true;
      bool in_identity = true;
      int func_start_index = -1;
      for (int a = i - 1;; --a) {
        if (a < 0) {
          valid = false;
          break;
        }
        if (in_identity) {
          if (isalnum(src->text[a]) || src->text[a] == '_') {
            continue;
          }
          if (i - a == 1) {
            valid = false;
            break;
          }
          in_identity = false;
          func_start_index = a + 1;
        }
        if (src->text[a] == ' ')
          continue;
        if (src->text[a] != '\n') {
          valid = false;
        }
        break;
      }
      //
      if (valid) {
        valid = false;
        for (int a = 0;; ++a) {
          if (!_mcl_core_functions[a])
            break;
          if (!strncmp(src->text + func_start_index, _mcl_core_functions[a], strlen(_mcl_core_functions[a]))) {
            valid = true;
            break;
          }
        }
        if (!valid) {
          // if (!strncmp(src->text + func_start_index, "clint_process", 13) ||
          //     !strncmp(src->text + func_start_index, "clint_declare", 13)) {
          //   valid = true;
          // }
          // else
          {
            char fname[128];
            int f;
            for (f = 0; f < 128 & func_start_index + f < i; ++f)
              fname[f] = src->text[func_start_index + f];
            fname[f] = '\0';
            bool specified = false;
            for (int f = 0; _mcl_ignore_functions[f]; ++f) {
              if (!strcmp(_mcl_ignore_functions[f], fname)) {
                specified = true;
                break;
              }
            }
            if (!specified) {
              printf("i:%i\n", i);
              printf("core-function-to-add:\n\"%s\",\n\n\n", fname);
              // MCerror(465, "core_source_loader: unspecified ignore function:'%s'\n", fname);
              return -5;
            }
          }
        }
      }
      if (valid) {
        int function_end_index = i + 1;
        int bracket_count = 1;
        while (bracket_count) {
          switch (src->text[function_end_index]) {
          case '"': {
            bool escaped = false;
            bool loop = true;
            while (loop) {
              ++function_end_index;
              switch (src->text[function_end_index]) {
              case '\\': {
                escaped = !escaped;
              } break;
              case '\0': {
                MCerror(92, "unexpected eof");
              }
              case '"': {
                if (escaped) {
                  escaped = false;
                  break;
                }
                loop = false;
              } break;
              default: {
                escaped = false;
              } break;
              }
            }
          } break;
          case '\0': {
            _mcl_print_parse_error(src->text, i, "began here", "");
            MCerror(469, "CheckIt");
          }
          case '(':
            ++bracket_count;
            break;
          case ')': {
            --bracket_count;
            break;
          }
          default:
            break;
          }
          function_end_index++;
        }
        if (src->text[function_end_index] == ';') {

          const char *before_template =
              "{int mc_error_stack_index; "
              "register_midge_stack_invocation(\"%s\", \"%s\", __LINE__, &mc_error_stack_index); "
              "int mc_res = mc_core_v_";

          const char *after_template = " if (mc_res) { "
                                       "printf(\"--%s |line :%%i :ERR:%%i\\n\", __LINE__, mc_res); "
                                       "return mc_res; "
                                       "} "
                                       "register_midge_stack_return(mc_error_stack_index);}";

          const int FNBUF_SIZE = 164;
          char fnbuf[FNBUF_SIZE];
          for (int b = 0; b < 164; ++b) {
            char c = src->text[func_start_index /*+ 10 strlen("mc_core_v_")*/ + b];
            if (c == '(') {
              fnbuf[b] = '\0';
              break;
            }
            fnbuf[b] = c;
          }
          char befbuf[384];
          sprintf(befbuf, before_template, fnbuf, _mcl_core_source_files[source_file_index]);

          char aftbuf[384];
          // int d = 0;
          // bool in_literal = false;
          // for (int b = 0; d < FNBUF_SIZE - 4; ++b) {
          //   char c = src->text[func_start_index + 10 /*strlen("mc_core_v_")*/ + b];
          //   if (!in_literal && c == ';') {
          //     fnbuf[d] = '\0';
          //     break;
          //   }
          //   else if (d > FNBUF_SIZE - 4) {
          //     if (in_literal)
          //       fnbuf[d++] = '"';
          //     fnbuf[d] = '\0';
          //     break;
          //   }
          //   if (c == '"') {
          //     fnbuf[d++] = '\\';
          //     in_literal = !in_literal;
          //   }
          //   else if (c == '%') {
          //     fnbuf[d++] = '%';
          //   }
          //   else if (c == '\\') {
          //     fnbuf[d++] = '\\';
          //   }
          //   fnbuf[d++] = c;
          // }
          sprintf(aftbuf, after_template, fnbuf);

          int len = strlen(befbuf) + strlen(aftbuf) + function_end_index + 1 - func_start_index;

          // Turn it into a temporary MCcall
          MCcall(insert_into__csl_c_str(src, aftbuf, function_end_index + 1));
          MCcall(insert_into__csl_c_str(src, befbuf, func_start_index));
          i += strlen(befbuf) + strlen(aftbuf);
          // if (len > 490) {
          //   MCcall(insert_into__csl_c_str(src, "\n", function_end_index + 1));
          //   ++i;
          // }

          // printf("#########################################################\n"
          //        "src:\n%s||\n",
          //        src->text);
        }
      }
    }
  }

  for (int i = 0; i < src->len; ++i) {
    // Exceptions
    switch (src->text[i - 1]) {
    case '_':
    case '"':
      continue;
    default: {
      if (isalpha(src->text[i - 1]))
        continue;
      break;
    }
    }

    for (int o = 0; _mcl_core_functions[o]; ++o) {
      int n = strlen(_mcl_core_functions[o]);
      if (!strncmp(src->text + i, _mcl_core_functions[o], n)) {
        // Exceptions
        switch (src->text[i + n]) {
        case '_':
        case '"':
          continue;
        default: {
          if (isalpha(src->text[i + n]))
            continue;
          break;
        }
        }

        // Insert
        MCcall(insert_into__csl_c_str(src, "mc_core_v_", i));
        i += 10 + n - 1;
      }
    }
  }

  return 0;
}

int _mcl_determine_cached_file_name(const char *input, char **output)
{
  _csl_c_str *str;
  MCcall(init__csl_c_str(&str));
  MCcall(set__csl_c_str(str, "bin/cached/_cmc_"));

  int fni = 0;
  fni += 11;
  for (int k = 0; k < 256; ++k) {
    if (input[k] == '/') {
      append_to__csl_c_str(str, "_");
    }
    else {
      if (input[k] == '\0') {
        break;
      }
      char cs[3];
      cs[0] = input[k];
      cs[1] = '\0';
      append_to__csl_c_str(str, cs);
    }
  }

  *output = str->text;
  release__csl_c_str(str, false);

  return 0;
}

int _mcl_load_core_temp_source()
{
  _csl_c_str *src;
  MCcall(init__csl_c_str(&src));

  // set__csl_c_str(src, "abcdefghijklmnopqrstuvwxyz");
  // printf("%s\n", src->text);
  // MCcall(remove_from__csl_c_str(src, 3, 13));
  // printf("%s\n", src->text);
  // return 0;

  for (int a = 0; _mcl_core_source_files[a]; ++a) {

    char *file_text;

    char *cached_file_name;
    _mcl_determine_cached_file_name(_mcl_core_source_files[a], &cached_file_name);

    // Compare modified times and process new source or use cache
    bool use_cached_file = false;
    if (access(cached_file_name, F_OK) != -1) {

      struct stat src_attrib;
      stat(_mcl_core_source_files[a], &src_attrib);

      struct stat cch_attrib;
      stat(cached_file_name, &cch_attrib);

      use_cached_file = (src_attrib.st_mtime < cch_attrib.st_mtime);
    }

    //  use_cached_file = false;
    if (use_cached_file) {
      MCcall(_mcl_read_all_file_text(cached_file_name, &file_text));
      MCcall(set__csl_c_str(src, file_text));
      free(file_text);

      printf("declaring file[cch]:'%s'\n", _mcl_core_source_files[a]);
    }
    else {
      // Read & process source
      MCcall(_mcl_read_all_file_text(_mcl_core_source_files[a], &file_text));
      MCcall(set__csl_c_str(src, file_text));
      free(file_text);

      printf("declaring file[src]:'%s'\n", _mcl_core_source_files[a]);
      MCcall(_mcl_format_core_file(src, a));
    }

    // if (!strcmp(_mcl_core_source_files[a], "src/midge_common.h")) {
    //   printf("def:\n%s||\n", src->text);
    //   return 3;
    // }
    MCcall(clint_declare(src->text));

    if (!use_cached_file) {
      // Save the processed version to file
      _mcl_save_text_to_file(cached_file_name, src->text);
    }

    free(cached_file_name);
    // MCcall(clint_process("printf(\"%p\", &mc_core_v_init_c_str);//{c_str *str; mc_core_v_init_c_str(&str);}"));
  }

  release__csl_c_str(src, true);

  return 0;
}

int _mcl_init_core_data()
{
  char buf[3072];
  void *p_global_root;
  sprintf(
      buf,
      "{"
      "  mc_core_v_mc_node *global = (mc_core_v_mc_node *)calloc(sizeof(mc_core_v_mc_node), 1);"
      "  global->type = NODE_TYPE_GLOBAL_ROOT;"
      "  allocate_and_copy_cstr(global->name, \"global\");"
      "  global->parent = NULL;"
      "  global->children = (mc_core_v_mc_node_list *)malloc(sizeof(mc_core_v_mc_node_list));"
      "  global->children->count = 0;"
      "  global->children->alloc = 0;"
      "  global->children->items = NULL;"
      ""
      "  mc_core_v_global_root_data *global_root_data = (mc_core_v_global_root_data *)"
      "                                                 malloc(sizeof(mc_core_v_global_root_data ));"
      "  global->data = global_root_data;"
      "  global_root_data->global_node = global;"
      ""
      "  global_root_data->exit_requested = false;"
      ""
      // "  global_root_data->children = (mc_core_v_mc_node_list *)malloc(sizeof(mc_core_v_mc_node_list));"
      // "  global_root_data->children->alloc = 0;"
      // "  global_root_data->children->count = 0;"
      ""
      "  global_root_data->source_files.alloc = 100;"
      "  global_root_data->source_files.count = 0;"
      "  global_root_data->source_files.items = (mc_core_v_mc_source_file_info **)"
      "                       calloc(sizeof(mc_core_v_mc_source_file_info *), global_root_data->source_files.alloc);"
      ""
      "  global_root_data->functions.alloc = 100;"
      "  global_root_data->functions.count = 0;"
      "  global_root_data->functions.items = (mc_core_v_function_info **)calloc(sizeof(mc_core_v_function_info *),"
      "                                         global_root_data->functions.alloc);"
      ""
      "  global_root_data->function_declarations.alloc = 100;"
      "  global_root_data->function_declarations.count = 0;"
      "  global_root_data->function_declarations.items = (mc_core_v_function_info **)"
      "                   calloc(sizeof(mc_core_v_function_info *), global_root_data->function_declarations.alloc);"
      ""
      "  global_root_data->structs.alloc = 30;"
      "  global_root_data->structs.count = 0;"
      "  global_root_data->structs.items = (mc_core_v_struct_info **)calloc(sizeof(mc_core_v_struct_info *),"
      "                                       global_root_data->structs.alloc);"
      ""
      "  global_root_data->enumerations.alloc = 20;"
      "  global_root_data->enumerations.count = 0;"
      "  global_root_data->enumerations.items = (mc_core_v_enumeration_info **)"
      "                       calloc(sizeof(mc_core_v_enumeration_info *), global_root_data->enumerations.alloc);"
      ""
      "  global_root_data->preprocess_defines.alloc = 20;"
      "  global_root_data->preprocess_defines.count = 0;"
      "  global_root_data->preprocess_defines.items = (mc_core_v_preprocess_define_info **)"
      "                       calloc(sizeof(mc_core_v_preprocess_define_info *), "
      "                       global_root_data->preprocess_defines.alloc);"
      ""
      "  global_root_data->event_handlers.alloc = 0;"
      "  global_root_data->event_handlers.count = 0;"
      ""
      "  *(void **)(%p) = (void *)global_root_data;"
      "}",
      &p_global_root);
  MCcall(clint_process(buf));

  sprintf(buf,
          "int mc_core_v_obtain_midge_global_root(mc_core_v_global_root_data **root_data) {\n"
          "  *root_data = (mc_core_v_global_root_data *)(%p);\n"
          "  return 0;\n"
          "}\n",
          p_global_root);
  MCcall(clint_declare(buf));

  return 0;
}

int _mcl_load_core_mc_source()
{
  char buf[512];
  for (int i = 0; _mcl_core_source_files[i]; ++i) {
    int result = 0;
    sprintf(buf,
            "{\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"

            "  int result = mc_core_v_instantiate_all_definitions_from_file(global_data->global_node, (char *)\"%s\","
            "                 NULL);\n"
            "  if (result) {\n"
            "    printf(\"--mc_core_v_instantiate_all_definitions_from_file #in - clint_process\\n\");\n"
            "    *(int *)(%p) = result;\n"
            "  }\n"
            "}",
            _mcl_core_source_files[i], &result);

    MCcall(clint_process(buf));

    if (result != 0) {
      return result;
    }

    // // DEBUG
    // // DEBUG
    // MCcall(clint_process("void *vv[1]; do_many_things(1, vv);"));

    // if (result != 0) {
    //   return result;
    // }
    // // DEBUG
    // // DEBUG
  }

  // obtain_midge_global_root function
  {
    void *p_global_data;
    sprintf(buf,
            "{\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"
            "  *(void **)%p = (void *)global_data;\n"
            "}",
            &p_global_data);
    MCcall(clint_process(buf));

    char fbuf[148];
    sprintf(fbuf,
            "int obtain_midge_global_root(global_root_data **root_data) {\n"
            "  *root_data = (global_root_data *)(%p);\n"
            "  return 0;\n"
            "}\n",
            p_global_data);

    int result = 0;
    sprintf(buf,
            "{\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"

            "  int result = mc_core_v_instantiate_definition(global_data->global_node, (char *)%p,"
            "                 NULL, NULL, NULL);\n"
            "  if (result) {\n"
            "    printf(\"--mc_core_v_instantiate_definition #in - clint_process\\n\");\n"
            "    *(int *)(%p) = result;\n"
            "  }\n"
            "}",
            fbuf, &result);
    MCcall(clint_process(buf));

    if (result != 0) {
      return result;
    }
  }

  return 0;
}

int _mcl_load_app_mc_source()
{
  const char *_mcl_external_dependency_source_files[] = {
      "src/temp/external_decl.h",

      // And everything here before -------------------------------------------------------------
      NULL,
  };

  char buf[1536];
  for (int i = 0; _mcl_external_dependency_source_files[i]; ++i) {
    printf("registering external file:'%s'\n", _mcl_external_dependency_source_files[i]);
    int result = 0;
    sprintf(buf,
            "{\n"
            "  //printf(\"obtain_midge_global_root:%%p\\n\", obtain_midge_global_root);\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"
            ""
            "  void *mc_vargs[4];\n"
            "  mc_vargs[0] = &global_data->global_node;\n"
            "  const char *filepath = \"%s\";\n"
            "  mc_vargs[1] = &filepath;\n"
            "  void * p_null = NULL;\n"
            "  mc_vargs[2] = &p_null;\n"
            "  int return_value;\n"
            "  mc_vargs[3] = &return_value;\n"
            ""
            "  {\n"
            "    int midge_error_stack_index;\n"
            "    register_midge_stack_invocation(\"instantiate_all_definitions_from_file\", \"core_source_loader.c\", "
            "          1224, &midge_error_stack_index);\n"
            "    int result = 0;\n"
            "    result = register_external_definitions_from_file(4, mc_vargs);\n"
            "    register_midge_stack_return(midge_error_stack_index);\n"
            ""
            "    if (result) {\n"
            "      printf(\"--register_external_definitions_from_file #in - clint_process\\n\");\n"
            "      *(int *)(%p) = result;\n"
            "    }\n"
            "  }\n"
            "}",
            _mcl_external_dependency_source_files[i], &result);
    MCcall(clint_process(buf));

    if (result != 0) {
      return result;
    }
  }

  register_midge_error_tag("_mcl_load_app_mc_source()");
  const char *_mcl_app_source_files[] = {
      // Headers required for app initialization
      "src/m_threads.h",
      "src/platform/mc_xcb.h",
      "src/render/render_common.h",
      "src/render/mc_vulkan.h",
      "src/render/mc_vk_utils.h",
      "src/render/render_thread.h",
      "src/core/midge_app.h",

      // Headers that will be declared just before app initialization
      "src/env/environment_definitions.h",
      "src/ui/ui_definitions.h",
      "src/control/mc_controller.h",

      // Source required for app initialization
      "src/platform/mc_xcb.c",
      "src/render/mc_vulkan.c",
      "src/render/mc_vk_utils.c",
      "src/render/render_thread.c",
      "src/core/app_modules.c",
      "src/core/midge_app.c",

      // And everything here before -------------------------------------------------------------
      NULL,
  };

  for (int i = 0; _mcl_app_source_files[i]; ++i) {
    int result = 0;
    sprintf(buf,
            "{\n"
            "  //printf(\"obtain_midge_global_root:%%p\\n\", obtain_midge_global_root);\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"
            ""
            "  void *mc_vargs[4];\n"
            "  mc_vargs[0] = &global_data->global_node;\n"
            "  const char *filepath = \"%s\";\n"
            "  mc_vargs[1] = &filepath;\n"
            "  void * p_null = NULL;\n"
            "  mc_vargs[2] = &p_null;\n"
            "  int return_value;\n"
            "  mc_vargs[3] = &return_value;\n"
            ""
            "  {\n"
            "    int midge_error_stack_index;\n"
            "    register_midge_stack_invocation(\"instantiate_all_definitions_from_file\", \"core_source_loader.c\", "
            "          1224, &midge_error_stack_index);\n"
            "    int result = 0;\n"
            "    result = instantiate_all_definitions_from_file(4, mc_vargs);\n"
            "    register_midge_stack_return(midge_error_stack_index);\n"
            ""
            "    if (result) {\n"
            "      printf(\"--instantiate_all_definitions_from_file #in - clint_process\\n\");\n"
            "      *(int *)(%p) = result;\n"
            "    }\n"
            "  }\n"
            "}",
            _mcl_app_source_files[i], &result);
    MCcall(clint_process(buf));

    if (result != 0) {
      return result;
    }
  }

  return 0;
}

int mcl_load_app_source()
{
  void *p_core_source_info = NULL;
  // MCcall(_mcl_obtain_core_source_information(&p_core_source_info));
  printf("[_mcl_load_core_temp_source]\n");
  MCcall(_mcl_load_core_temp_source());

  printf("[_mcl_init_core_data]\n");
  MCcall(_mcl_init_core_data());

  printf("[_mcl_load_core_mc_source]\n");
  MCcall(_mcl_load_core_mc_source());

  printf("[_mcl_load_app_mc_source]\n");
  MCcall(_mcl_load_app_mc_source());

  printf("[_mcl_load_source:COMPLETE]\n");

  return 0;
}