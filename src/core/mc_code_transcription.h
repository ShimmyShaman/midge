/* mc_code_transcription.h */

#ifndef MC_CODE_TRANSCRIPTION_H
#define MC_CODE_TRANSCRIPTION_H

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

typedef struct mct_expression_type_info {
  // If is fptr - then type_identifier will be the return - type identifier
  char *type_name;
  // If is fptr - then deref count will be return type deref count
  unsigned int deref_count;
  bool is_array;
  bool is_fptr;
} mct_expression_type_info;

typedef struct mct_function_transcription_options {
  // Setting to true enables midge to keep track of the call stack (which reports on illegal memory access and such)
  bool report_function_entry_exit_to_stack;
  // NOT USED
  bool report_simple_args_to_error_stack;
  // Tags on function entry/exit - another form of 'stack tracing' if you also use tags to track where in the function
  // code gets to before error
  bool tag_on_function_entry, tag_on_function_exit;

  // If set, enables variable value reporting every time a variable value is set (or used?). NULL otherwise.
  mct_function_variable_report_index *report_variable_values;

} mct_function_transcription_options;

int mct_transcribe_file_ast(mc_syntax_node *file_root, mct_function_transcription_options *options, char **generated);

/*
 * Transcribes an isolated code block ast and appends it to the given mc_str.
 */
int mct_transcribe_isolated_code_block(mc_syntax_node *code_block_ast, const char *function_name,
                                       mct_function_transcription_options *options, mc_str *str);
// int mct_transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast,
//                                   mct_function_transcription_options *options, char **mc_transcription);
// int transcribe_enumeration_to_mc(enumeration_info *enum_info, mc_syntax_node *enumeration_ast, char
// **mc_transcription); int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *struct_ast, char
// **mc_transcription);

#endif // MC_CODE_TRANSCRIPTION_H