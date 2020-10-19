/* mc_code_transcription.h */

#ifndef MC_CODE_TRANSCRIPTION_H
#define MC_CODE_TRANSCRIPTION_H

#include "midge_common.h"

#include "core/c_parser_lexer.h"

typedef struct mct_function_variable_value {
  char *name;
  int line, col;
  void *ptr;
} mct_function_variable_value;

typedef struct mct_function_variable_value_list {
  unsigned int call_index;

  struct {
    unsigned int capacity, count;
    mct_function_variable_value **items;
  } value_reports;
} mct_function_variable_value_list;

typedef struct mct_function_variable_report_index {
  unsigned int call_uid_counter;

  // unsigned int call_invocation_limit;
  void *tag_data;

  struct {
    unsigned int capacity, count;
    mct_function_variable_value_list **items;
  } calls;

  // int report_variable_value(mct_function_variable_report_index *report_index, unsigned int call_uid, const char
  // *type_identifier, const char *variable_name, int line, int col, void *p_value)
  void *report_variable_value_delegate;

  // int end_report_variable_values(mct_function_variable_report_index *report_index, unsigned int call_uid)
  void *end_report_variable_values_delegate;

} mct_function_variable_report_index;

typedef struct mct_function_transcription_options {

  bool report_invocations_to_error_stack;
  bool report_simple_args_to_error_stack;
  bool check_mc_functions_not_null;
  bool tag_on_function_entry, tag_on_function_exit;

  mct_function_variable_report_index *report_variable_values;

} mct_function_transcription_options;

extern "C" {
int mct_transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast,
                                  mct_function_transcription_options *options, char **mc_transcription);
int transcribe_enumeration_to_mc(enumeration_info *enum_info, mc_syntax_node *enumeration_ast, char **mc_transcription);
int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *struct_ast, char **mc_transcription);
}

#endif // MC_CODE_TRANSCRIPTION_H