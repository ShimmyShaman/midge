
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <unistd.h>

#include "midge_common.h"
#include "midge_error_handling.h"

#include "core/c_parser_lexer.h"

typedef struct parsing_state {
  char *code;
  int index;
  int line;
  int col;

  bool allow_imperfect_parse;

} parsing_state;

// extern "C" {
// int init_c_str(c_str **ptr);
// }

int mcs_parse_type_identifier(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int _mcs_parse_expression(parsing_state *ps, int allowable_precedence, mc_syntax_node *parent,
                          mc_syntax_node **additional_destination);
int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_statement_list(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_type_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_root_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_parameter_declaration(parsing_state *ps, bool allow_name_skip, mc_syntax_node *parent,
                                    mc_syntax_node **additional_destination);

const char *get_mc_token_type_name(mc_token_type type)
{
  switch (type) {
  case MC_TOKEN_NULL:
    return "MC_TOKEN_NULL";
  case MC_TOKEN_NULL_CHARACTER:
    return "MC_TOKEN_NULL_CHARACTER";
  case MC_TOKEN_PP_KEYWORD_DEFINE:
    return "MC_TOKEN_PP_KEYWORD_DEFINE";
  case MC_TOKEN_PP_KEYWORD_INCLUDE:
    return "MC_TOKEN_PP_KEYWORD_INCLUDE";
  case MC_TOKEN_PP_KEYWORD_IFNDEF:
    return "MC_TOKEN_PP_KEYWORD_IFNDEF";
  case MC_TOKEN_PP_KEYWORD_IFDEF:
    return "MC_TOKEN_PP_KEYWORD_IFDEF";
  case MC_TOKEN_PP_KEYWORD_ENDIF:
    return "MC_TOKEN_PP_KEYWORD_ENDIF";
  case MC_TOKEN_PP_IDENTIFIER:
    return "MC_TOKEN_PP_IDENTIFIER";
  case MC_TOKEN_STAR_CHARACTER:
    return "MC_TOKEN_STAR_CHARACTER";
  case MC_TOKEN_ESCAPE_CHARACTER:
    return "MC_TOKEN_ESCAPE_CHARACTER";
  case MC_TOKEN_MULTIPLY_OPERATOR:
    return "MC_TOKEN_MULTIPLY_OPERATOR";
  case MC_TOKEN_DEREFERENCE_OPERATOR:
    return "MC_TOKEN_DEREFERENCE_OPERATOR";
  case MC_TOKEN_IDENTIFIER:
    return "MC_TOKEN_IDENTIFIER";
  case MC_TOKEN_SQUARE_OPENING_BRACKET:
    return "MC_TOKEN_SQUARE_OPENING_BRACKET";
  case MC_TOKEN_SQUARE_CLOSING_BRACKET:
    return "MC_TOKEN_SQUARE_CLOSING_BRACKET";
  case MC_TOKEN_OPENING_BRACKET:
    return "MC_TOKEN_OPENING_BRACKET";
  case MC_TOKEN_CLOSING_BRACKET:
    return "MC_TOKEN_CLOSING_BRACKET";
  case MC_TOKEN_SEMI_COLON:
    return "MC_TOKEN_SEMI_COLON";
  case MC_TOKEN_COLON:
    return "MC_TOKEN_COLON";
  case MC_TOKEN_EQUALITY_OPERATOR:
    return "MC_TOKEN_EQUALITY_OPERATOR";
  case MC_TOKEN_INEQUALITY_OPERATOR:
    return "MC_TOKEN_INEQUALITY_OPERATOR";
  case MC_TOKEN_DECREMENT_OPERATOR:
    return "MC_TOKEN_DECREMENT_OPERATOR";
  case MC_TOKEN_INCREMENT_OPERATOR:
    return "MC_TOKEN_INCREMENT_OPERATOR";
  case MC_TOKEN_POINTER_OPERATOR:
    return "MC_TOKEN_POINTER_OPERATOR";
  case MC_TOKEN_ASSIGNMENT_OPERATOR:
    return "MC_TOKEN_ASSIGNMENT_OPERATOR";
  case MC_TOKEN_LOGICAL_NOT_OPERATOR:
    return "MC_TOKEN_LOGICAL_NOT_OPERATOR";
  case MC_TOKEN_SUBTRACT_OPERATOR:
    return "MC_TOKEN_SUBTRACT_OPERATOR";
  case MC_TOKEN_DIVIDE_OPERATOR:
    return "MC_TOKEN_DIVIDE_OPERATOR";
  case MC_TOKEN_PLUS_OPERATOR:
    return "MC_TOKEN_PLUS_OPERATOR";
  case MC_TOKEN_MODULO_OPERATOR:
    return "MC_TOKEN_MODULO_OPERATOR";
  case MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR:
    return "MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR";
  case MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR:
    return "MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR";
  case MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_BITWISE_NOT_OPERATOR:
    return "MC_TOKEN_BITWISE_NOT_OPERATOR";
  case MC_TOKEN_TERNARY_OPERATOR:
    return "MC_TOKEN_TERNARY_OPERATOR";
  case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_DIVIDE_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_DIVIDE_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_EXTERN_KEYWORD:
    return "MC_TOKEN_EXTERN_KEYWORD";
  case MC_TOKEN_TYPEDEF_KEYWORD:
    return "MC_TOKEN_TYPEDEF_KEYWORD";
  case MC_TOKEN_IF_KEYWORD:
    return "MC_TOKEN_IF_KEYWORD";
  case MC_TOKEN_ELSE_KEYWORD:
    return "MC_TOKEN_ELSE_KEYWORD";
  case MC_TOKEN_GOTO_KEYWORD:
    return "MC_TOKEN_GOTO_KEYWORD";
  case MC_TOKEN_WHILE_KEYWORD:
    return "MC_TOKEN_WHILE_KEYWORD";
  case MC_TOKEN_DO_KEYWORD:
    return "MC_TOKEN_DO_KEYWORD";
  case MC_TOKEN_FOR_KEYWORD:
    return "MC_TOKEN_FOR_KEYWORD";
  case MC_TOKEN_CONTINUE_KEYWORD:
    return "MC_TOKEN_CONTINUE_KEYWORD";
  case MC_TOKEN_BREAK_KEYWORD:
    return "MC_TOKEN_BREAK_KEYWORD";
  case MC_TOKEN_SWITCH_KEYWORD:
    return "MC_TOKEN_SWITCH_KEYWORD";
  case MC_TOKEN_RETURN_KEYWORD:
    return "MC_TOKEN_RETURN_KEYWORD";
  case MC_TOKEN_CONST_KEYWORD:
    return "MC_TOKEN_CONST_KEYWORD";
  case MC_TOKEN_STATIC_KEYWORD:
    return "MC_TOKEN_STATIC_KEYWORD";
  case MC_TOKEN_SIZEOF_KEYWORD:
    return "MC_TOKEN_SIZEOF_KEYWORD";
  case MC_TOKEN_OFFSETOF_KEYWORD:
    return "MC_TOKEN_OFFSETOF_KEYWORD";
  case MC_TOKEN_VA_ARG_WORD:
    return "MC_TOKEN_VA_ARG_WORD";
  case MC_TOKEN_VA_LIST_WORD:
    return "MC_TOKEN_VA_LIST_WORD";
  case MC_TOKEN_VA_START_WORD:
    return "MC_TOKEN_VA_START_WORD";
  case MC_TOKEN_VA_END_WORD:
    return "MC_TOKEN_VA_END_WORD";
  case MC_TOKEN_CURLY_OPENING_BRACKET:
    return "MC_TOKEN_CURLY_OPENING_BRACKET";
  case MC_TOKEN_CURLY_CLOSING_BRACKET:
    return "MC_TOKEN_CURLY_CLOSING_BRACKET";
  case MC_TOKEN_NEW_LINE:
    return "MC_TOKEN_NEW_LINE";
  case MC_TOKEN_SPACE_SEQUENCE:
    return "MC_TOKEN_SPACE_SEQUENCE";
  case MC_TOKEN_TAB_SEQUENCE:
    return "MC_TOKEN_TAB_SEQUENCE";
  case MC_TOKEN_LINE_COMMENT:
    return "MC_TOKEN_LINE_COMMENT";
  case MC_TOKEN_MULTI_LINE_COMMENT:
    return "MC_TOKEN_MULTI_LINE_COMMENT";
  case MC_TOKEN_DECIMAL_POINT:
    return "MC_TOKEN_DECIMAL_POINT";
  case MC_TOKEN_NUMERIC_LITERAL:
    return "MC_TOKEN_NUMERIC_LITERAL";
  case MC_TOKEN_STRING_LITERAL:
    return "MC_TOKEN_STRING_LITERAL";
  case MC_TOKEN_CHAR_LITERAL:
    return "MC_TOKEN_CHAR_LITERAL";
  case MC_TOKEN_COMMA:
    return "MC_TOKEN_COMMA";
  case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
    return "MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR";
  case MC_TOKEN_ARROW_OPENING_BRACKET:
    return "MC_TOKEN_ARROW_OPENING_BRACKET";
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
    return "MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR";
  case MC_TOKEN_ARROW_CLOSING_BRACKET:
    return "MC_TOKEN_ARROW_CLOSING_BRACKET";
  case MC_TOKEN_LOGICAL_AND_OPERATOR:
    return "MC_TOKEN_LOGICAL_AND_OPERATOR";
  case MC_TOKEN_BINARY_AND_ASSIGNMENT_OPERATOR:
    return "MC_TOKEN_BINARY_AND_ASSIGNMENT_OPERATOR";
  case MC_TOKEN_AMPERSAND_CHARACTER:
    return "MC_TOKEN_AMPERSAND_CHARACTER";
  case MC_TOKEN_LOGICAL_OR_OPERATOR:
    return "MC_TOKEN_LOGICAL_OR_OPERATOR";
  case MC_TOKEN_BINARY_OR_ASSIGNMENT_OPERATOR:
    return "MC_TOKEN_BINARY_OR_ASSIGNMENT_OPERATOR";
  case MC_TOKEN_BITWISE_OR_OPERATOR:
    return "MC_TOKEN_BITWISE_OR_OPERATOR";
  case MC_TOKEN_CASE_KEYWORD:
    return "MC_TOKEN_CASE_KEYWORD";
  case MC_TOKEN_DEFAULT_KEYWORD:
    return "MC_TOKEN_DEFAULT_KEYWORD";
  case MC_TOKEN_STRUCT_KEYWORD:
    return "MC_TOKEN_STRUCT_KEYWORD";
  case MC_TOKEN_UNION_KEYWORD:
    return "MC_TOKEN_UNION_KEYWORD";
  case MC_TOKEN_ENUM_KEYWORD:
    return "MC_TOKEN_ENUM_KEYWORD";
  case MC_TOKEN_VOID_KEYWORD:
    return "MC_TOKEN_VOID_KEYWORD";
  case MC_TOKEN_INT_KEYWORD:
    return "MC_TOKEN_INT_KEYWORD";
  case MC_TOKEN_CHAR_KEYWORD:
    return "MC_TOKEN_CHAR_KEYWORD";
  case MC_TOKEN_UNSIGNED_KEYWORD:
    return "MC_TOKEN_UNSIGNED_KEYWORD";
  case MC_TOKEN_SIGNED_KEYWORD:
    return "MC_TOKEN_SIGNED_KEYWORD";
  // case MC_TOKEN_BOOL_KEYWORD:
  //   return "MC_TOKEN_BOOL_KEYWORD";
  case MC_TOKEN_FLOAT_KEYWORD:
    return "MC_TOKEN_FLOAT_KEYWORD";
  case MC_TOKEN_LONG_KEYWORD:
    return "MC_TOKEN_LONG_KEYWORD";
  case MC_TOKEN_SHORT_KEYWORD:
    return "MC_TOKEN_SHORT_KEYWORD";
  default: {
    // TODO -- DEBUG -- this string is never free-d
    int cprintf_n = snprintf(NULL, 0, "TODO_ENCODE_THIS_TYPE_OR_UNSUPPORTED:%i", type);
    char *mc_temp_cstr = (char *)malloc(sizeof(char) * (cprintf_n + 1));
    sprintf(mc_temp_cstr, "TODO_ENCODE_THIS_TYPE_OR_UNSUPPORTED:%i", type);
    return (char *)mc_temp_cstr;
  }
  }
}

const char *get_mc_syntax_token_type_name(mc_syntax_node_type type)
{
  switch ((mc_syntax_node_type)type) {
  case MC_SYNTAX_FILE_ROOT:
    return "MC_SYNTAX_FILE_ROOT";
  case MC_SYNTAX_PP_DIRECTIVE_IFDEF:
    return "MC_SYNTAX_PP_DIRECTIVE_IFDEF";
  case MC_SYNTAX_PP_DIRECTIVE_IFNDEF:
    return "MC_SYNTAX_PP_DIRECTIVE_IFNDEF";
  case MC_SYNTAX_PP_DIRECTIVE_INCLUDE:
    return "MC_SYNTAX_PP_DIRECTIVE_INCLUDE";
  case MC_SYNTAX_PP_DIRECTIVE_DEFINE:
    return "MC_SYNTAX_PP_DIRECTIVE_DEFINE";
  case MC_SYNTAX_FUNCTION:
    return "MC_SYNTAX_FUNCTION";
  case MC_SYNTAX_EXTERN_C_BLOCK:
    return "MC_SYNTAX_EXTERN_C_BLOCK";
  case MC_SYNTAX_TYPE_ALIAS:
    return "MC_SYNTAX_TYPE_ALIAS";
  case MC_SYNTAX_STRUCT_DECL:
    return "MC_SYNTAX_STRUCT_DECL";
  case MC_SYNTAX_UNION_DECL:
    return "MC_SYNTAX_UNION_DECL";
  case MC_SYNTAX_ENUM_DECL:
    return "MC_SYNTAX_ENUM_DECL";
  case MC_SYNTAX_ENUM_DECL_MEMBER:
    return "MC_SYNTAX_ENUM_DECL_MEMBER";
  case MC_SYNTAX_NESTED_TYPE_DECLARATION:
    return "MC_SYNTAX_NESTED_TYPE_DECLARATION";
  case MC_SYNTAX_CODE_BLOCK:
    return "MC_SYNTAX_CODE_BLOCK";
  case MC_SYNTAX_STATEMENT_LIST:
    return "MC_SYNTAX_STATEMENT_LIST";
  case MC_SYNTAX_FOR_STATEMENT:
    return "MC_SYNTAX_FOR_STATEMENT";
  case MC_SYNTAX_WHILE_STATEMENT:
    return "MC_SYNTAX_WHILE_STATEMENT";
  case MC_SYNTAX_IF_STATEMENT:
    return "MC_SYNTAX_IF_STATEMENT";
  case MC_SYNTAX_SWITCH_STATEMENT:
    return "MC_SYNTAX_SWITCH_STATEMENT";
  case MC_SYNTAX_SWITCH_SECTION:
    return "MC_SYNTAX_SWITCH_SECTION";
  case MC_SYNTAX_SWITCH_CASE_LABEL:
    return "MC_SYNTAX_SWITCH_CASE_LABEL";
  case MC_SYNTAX_SWITCH_DEFAULT_LABEL:
    return "MC_SYNTAX_SWITCH_DEFAULT_LABEL";
  case MC_SYNTAX_CONTINUE_STATEMENT:
    return "MC_SYNTAX_CONTINUE_STATEMENT";
  case MC_SYNTAX_BREAK_STATEMENT:
    return "MC_SYNTAX_BREAK_STATEMENT";
  case MC_SYNTAX_EXPRESSION_STATEMENT:
    return "MC_SYNTAX_EXPRESSION_STATEMENT";
  case MC_SYNTAX_GOTO_STATEMENT:
    return "MC_SYNTAX_GOTO_STATEMENT";
  case MC_SYNTAX_LABEL_STATEMENT:
    return "MC_SYNTAX_LABEL_STATEMENT";
  case MC_SYNTAX_DECLARATION_STATEMENT:
    return "MC_SYNTAX_DECLARATION_STATEMENT";
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATION:
    return "MC_SYNTAX_LOCAL_VARIABLE_DECLARATION";
  case MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER:
    return "MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER";
  case MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER:
    return "MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER";
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR:
    return "MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR";
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR:
    return "MC_SYNTAX_FUNCTION_POINTER_DECLARATOR";
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATION:
    return "MC_SYNTAX_FUNCTION_POINTER_DECLARATION";
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION:
    return "MC_SYNTAX_PARENTHESIZED_EXPRESSION";
  case MC_SYNTAX_TYPE_INITIALIZER:
    return "MC_SYNTAX_TYPE_INITIALIZER";
  case MC_SYNTAX_INITIALIZER_EXPRESSION:
    return "MC_SYNTAX_INITIALIZER_EXPRESSION";
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION:
    return "MC_SYNTAX_ASSIGNMENT_EXPRESSION";
  case MC_SYNTAX_SIZEOF_EXPRESSION:
    return "MC_SYNTAX_SIZEOF_EXPRESSION";
  case MC_SYNTAX_OFFSETOF_EXPRESSION:
    return "MC_SYNTAX_OFFSETOF_EXPRESSION";
  case MC_SYNTAX_VA_ARG_EXPRESSION:
    return "MC_SYNTAX_VA_ARG_EXPRESSION";
  case MC_SYNTAX_VA_LIST_STATEMENT:
    return "MC_SYNTAX_VA_LIST_STATEMENT";
  case MC_SYNTAX_VA_START_STATEMENT:
    return "MC_SYNTAX_VA_START_STATEMENT";
  case MC_SYNTAX_VA_END_STATEMENT:
    return "MC_SYNTAX_VA_END_STATEMENT";
  case MC_SYNTAX_RETURN_STATEMENT:
    return "MC_SYNTAX_RETURN_STATEMENT";
  case MC_SYNTAX_INVOCATION:
    return "MC_SYNTAX_INVOCATION";
  case MC_SYNTAX_SUPERNUMERARY:
    return "MC_SYNTAX_SUPERNUMERARY";
  case MC_SYNTAX_INCLUDE_SYSTEM_HEADER_IDENTITY:
    return "MC_SYNTAX_INCLUDE_SYSTEM_HEADER_IDENTITY";
  case MC_SYNTAX_TYPE_IDENTIFIER:
    return "MC_SYNTAX_TYPE_IDENTIFIER";
  case MC_SYNTAX_DEREFERENCE_SEQUENCE:
    return "MC_SYNTAX_DEREFERENCE_SEQUENCE";
  case MC_SYNTAX_PARAMETER_DECLARATION:
    return "MC_SYNTAX_PARAMETER_DECLARATION";
  case MC_SYNTAX_FIELD_DECLARATION:
    return "MC_SYNTAX_FIELD_DECLARATION";
  case MC_SYNTAX_FIELD_DECLARATOR:
    return "MC_SYNTAX_FIELD_DECLARATOR";
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION:
    return "MC_SYNTAX_STRING_LITERAL_EXPRESSION";
  case MC_SYNTAX_CAST_EXPRESSION:
    return "MC_SYNTAX_CAST_EXPRESSION";
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION:
    return "MC_SYNTAX_PREPENDED_UNARY_EXPRESSION";
  case MC_SYNTAX_LOGICAL_EXPRESSION:
    return "MC_SYNTAX_LOGICAL_EXPRESSION";
  case MC_SYNTAX_BITWISE_EXPRESSION:
    return "MC_SYNTAX_BITWISE_EXPRESSION";
  case MC_SYNTAX_TERNARY_CONDITIONAL:
    return "MC_SYNTAX_TERNARY_CONDITIONAL";
  case MC_SYNTAX_RELATIONAL_EXPRESSION:
    return "MC_SYNTAX_RELATIONAL_EXPRESSION";
  case MC_SYNTAX_OPERATIONAL_EXPRESSION:
    return "MC_SYNTAX_OPERATIONAL_EXPRESSION";
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
    return "MC_SYNTAX_MEMBER_ACCESS_EXPRESSION";
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION:
    return "MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION";
  case MC_SYNTAX_FIXREMENT_EXPRESSION:
    return "MC_SYNTAX_FIXREMENT_EXPRESSION";
  case MC_SYNTAX_DEREFERENCE_EXPRESSION:
    return "MC_SYNTAX_DEREFERENCE_EXPRESSION";
  default:
    return get_mc_token_type_name((mc_token_type)type);
  }
}

int print_parse_error(char *const text, int index, const char *const function_name, const char *section_id)
{
  const int LEN = 168;
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

  printf("~~~~~~!!!!!!!!!!!!!!!!!!!~~~~~~\n%s>%s#unhandled-char:'%s'\n", function_name, section_id, buf);

  return 0;
}

int _mcs_print_syntax_node_ancestry(mc_syntax_node *syntax_node, int depth, int ancestry_count)
{

  if (syntax_node->parent) {
    _mcs_print_syntax_node_ancestry(syntax_node->parent, depth, ancestry_count + 1);
    printf(" |  | \n");
  }

  if (ancestry_count) {
    const char *type_name = get_mc_syntax_token_type_name(syntax_node->type);
    printf("|^ %s ^|", type_name);
  }

  return 0;
}

int print_syntax_node(mc_syntax_node *syntax_node, int depth)
{
  // printf("mpsyn-tree-0 %p >%i\n", syntax_node, depth);
  for (int i = 0; i < depth; ++i) {
    printf("|  ");
  }

  const char *type_name = get_mc_syntax_token_type_name(syntax_node->type);
  switch (syntax_node->type) {
  case MC_SYNTAX_INVOCATION: {
    function_info *func_info;
    char *function_name;
    mcs_copy_syntax_node_to_text(syntax_node->invocation.function_identity, &function_name);
    find_function_info(function_name, &func_info);
    free(function_name);
    printf("|--%s : %p", type_name, func_info);
  } break;
  default:
    printf("|--%s", type_name);
    break;
  }

  if ((int)syntax_node->type < (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    if (syntax_node->type == (mc_syntax_node_type)MC_TOKEN_NEW_LINE) {
      printf(":'\\n'\n");
    }
    else {
      printf(":'%s'\n", syntax_node->text);
    }
  }
  else {
    printf("\n");
    for (int i = 0; i < syntax_node->children->count; ++i) {
      // printf("mpst-2\n");
      if (syntax_node->children->items[i] == NULL) {
        // printf("mpst-3\n");
        // printf("NULL??\n");
        continue;
      }
      // printf("mpst-4\n");
      print_syntax_node(syntax_node->children->items[i], depth + 1);
    }
    // printf("mpst-5\n");
  }
  // printf("mpsyn-tree-exit >%i\n", depth);

  return 0;
}
#include <unistd.h>
int mcs_append_syntax_node_to_c_str(c_str *cstr, mc_syntax_node *syntax_node)
{
  // const char *type_name = get_mc_syntax_token_type_name(syntax_node->type);
  // register_midge_error_tag("mcs_append_syntax_node_to_c_str(%s)", type_name);

  if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    // printf("syntax_node:%p\n", syntax_node);
    // printf("cstr:%p\n", cstr);
    // printf("syntax_node->text:%p\n", syntax_node->text);
    // printf("syntax_node->text:%s\n", syntax_node->text);
    append_to_c_str(cstr, syntax_node->text);
    // printf("cstr->text:%s\n", cstr->text);
    // printf("syntax_node:%p\n", syntax_node);
    // usleep(10000);
    register_midge_error_tag("_mcs_copy_syntax_node_to_text(~t)");
    return 0;
  }

  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    mcs_append_syntax_node_to_c_str(cstr, child);
  }

  register_midge_error_tag("mcs_copy_syntax_node_to_text(~*)");
  return 0;
}

int mcs_copy_syntax_node_to_text(mc_syntax_node *syntax_node, char **output)
{
  c_str *cstr;
  init_c_str(&cstr);

  mcs_append_syntax_node_to_c_str(cstr, syntax_node);

  *output = cstr->text;
  release_c_str(cstr, false);

  return 0;
}

int mcs_add_syntax_node_to_parent(mc_syntax_node *parent, mc_syntax_node *child)
{
  if (parent) {
    append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count, child);
  }

  child->parent = parent;

  return 0;
}

int mcs_construct_syntax_node(parsing_state *ps, mc_syntax_node_type node_type, char *mc_token_primitive_text,
                              mc_syntax_node *parent, mc_syntax_node **result)
{
  const char *type_name = get_mc_syntax_token_type_name(node_type);
  register_midge_error_tag("mcs_construct_syntax_node(%s)", type_name);

  mc_syntax_node *syntax_node = (mc_syntax_node *)calloc(sizeof(mc_syntax_node), 1);
  syntax_node->type = node_type;
  syntax_node->begin.index = ps->index;
  syntax_node->begin.line = ps->line;
  syntax_node->begin.col = ps->col;

  if ((int)node_type >= (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    syntax_node->children = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->children->alloc = 0;
    syntax_node->children->count = 0;
  }
  else {
    syntax_node->text = mc_token_primitive_text;
  }

  switch (node_type) {
  case MC_SYNTAX_FILE_ROOT: {
    // Nothing
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_IFDEF: {
    syntax_node->preprocess_ifdef.identifier = NULL;
    syntax_node->preprocess_ifdef.groupopt = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->preprocess_ifdef.groupopt->alloc = 0;
    syntax_node->preprocess_ifdef.groupopt->count = 0;
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_IFNDEF: {
    syntax_node->preprocess_ifndef.identifier = NULL;
    syntax_node->preprocess_ifndef.groupopt = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->preprocess_ifndef.groupopt->alloc = 0;
    syntax_node->preprocess_ifndef.groupopt->count = 0;
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_INCLUDE: {
    // Temporary. TODO
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_DEFINE: {
    syntax_node->preprocess_define.statement_type = PREPROCESSOR_DEFINE_NULL;
    syntax_node->preprocess_define.identifier = NULL;
    syntax_node->preprocess_define.replacement_list = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->preprocess_define.replacement_list->alloc = 0;
    syntax_node->preprocess_define.replacement_list->count = 0;
  } break;
  case MC_SYNTAX_ENUM_DECL: {
    syntax_node->enumeration.name = NULL;
    syntax_node->enumeration.type_identity = NULL;
    syntax_node->enumeration.members = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->enumeration.members->alloc = 0;
    syntax_node->enumeration.members->count = 0;
  } break;
  case MC_SYNTAX_ENUM_DECL_MEMBER: {
    syntax_node->enum_member.identifier = NULL;
    syntax_node->enum_member.value_expression = NULL;
  } break;
  case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
    syntax_node->nested_type.declaration = NULL;
    syntax_node->nested_type.declarators = NULL;
  } break;
  case MC_SYNTAX_TYPE_ALIAS: {
    syntax_node->type_alias.type_descriptor = NULL;
    syntax_node->type_alias.alias = NULL;
  } break;
  case MC_SYNTAX_STRUCT_DECL: {
    syntax_node->struct_decl.type_name = NULL;
    syntax_node->struct_decl.fields = NULL;
  } break;
  case MC_SYNTAX_UNION_DECL: {
    syntax_node->union_decl.type_name = NULL;
    syntax_node->union_decl.fields = NULL;
  } break;
  case MC_SYNTAX_FUNCTION: {
    syntax_node->function.return_type_identifier = NULL;
    syntax_node->function.return_type_dereference = NULL;
    syntax_node->function.name = NULL;
    syntax_node->function.parameters = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->function.parameters->alloc = 0;
    syntax_node->function.parameters->count = 0;
  } break;
  case MC_SYNTAX_CODE_BLOCK: {
    syntax_node->code_block.statement_list = NULL;
  } break;
  case MC_SYNTAX_EXTERN_C_BLOCK: {
    syntax_node->extern_block.declarations = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->extern_block.declarations->alloc = 0;
    syntax_node->extern_block.declarations->count = 0;
  } break;
  case MC_SYNTAX_BREAK_STATEMENT:
  case MC_SYNTAX_CONTINUE_STATEMENT:
  case MC_SYNTAX_SWITCH_DEFAULT_LABEL: {
    // Nothing
  } break;
  case MC_SYNTAX_STATEMENT_LIST: {
    syntax_node->statement_list.statements = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->statement_list.statements->alloc = 0;
    syntax_node->statement_list.statements->count = 0;
  } break;
  case MC_SYNTAX_SWITCH_STATEMENT: {
    syntax_node->switch_statement.conditional = NULL;
    syntax_node->switch_statement.sections = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->switch_statement.sections->alloc = 0;
    syntax_node->switch_statement.sections->count = 0;
  } break;
  case MC_SYNTAX_SWITCH_SECTION: {
    syntax_node->switch_section.statement_list = NULL;
    syntax_node->switch_section.labels = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->switch_section.labels->alloc = 0;
    syntax_node->switch_section.labels->count = 0;
  } break;
  case MC_SYNTAX_SWITCH_CASE_LABEL: {
    syntax_node->switch_case_label.constant = NULL;
  } break;
  case MC_SYNTAX_FOR_STATEMENT: {
    syntax_node->for_statement.initialization = NULL;
    syntax_node->for_statement.conditional = NULL;
    syntax_node->for_statement.fix_expression = NULL;
    syntax_node->for_statement.loop_statement = NULL;
  } break;
  case MC_SYNTAX_WHILE_STATEMENT: {
    syntax_node->while_statement.conditional = NULL;
    syntax_node->while_statement.do_first = false;
    syntax_node->while_statement.do_statement = NULL;
  } break;
  case MC_SYNTAX_IF_STATEMENT: {
    syntax_node->if_statement.conditional = NULL;
    syntax_node->if_statement.do_statement = NULL;
    syntax_node->if_statement.else_continuance = NULL;
  } break;
  case MC_SYNTAX_EXPRESSION_STATEMENT: {
    syntax_node->expression_statement.expression = NULL;
  } break;
  case MC_SYNTAX_DECLARATION_STATEMENT: {
    syntax_node->declaration_statement.declaration = NULL;
  } break;
  case MC_SYNTAX_GOTO_STATEMENT: {
    syntax_node->goto_statement.label = NULL;
  } break;
  case MC_SYNTAX_LABEL_STATEMENT: {
    syntax_node->label_statement.label = NULL;
  } break;
  case MC_SYNTAX_RETURN_STATEMENT: {
    syntax_node->return_statement.expression = NULL;
  } break;
  case MC_SYNTAX_PARAMETER_DECLARATION: {
    syntax_node->parameter.type = PARAMETER_KIND_NULL;
    syntax_node->parameter.function_pointer = NULL;
    syntax_node->parameter.type_identifier = NULL;
    syntax_node->parameter.type_dereference = NULL;
    syntax_node->parameter.name = NULL;
  } break;
  case MC_SYNTAX_FIELD_DECLARATION: {
    syntax_node->field.type = FIELD_KIND_NULL;
    {
      // Redundant
      syntax_node->field.function_pointer = NULL;
      syntax_node->field.nested_struct = NULL;
      syntax_node->field.nested_union = NULL;
      syntax_node->field.type_identifier = NULL;
    }
    syntax_node->field.declarators = NULL;
  } break;
  case MC_SYNTAX_FIELD_DECLARATOR: {
    syntax_node->field_declarator.type_dereference = NULL;
    syntax_node->field_declarator.name = NULL;
    syntax_node->field_declarator.array_size = NULL;
    syntax_node->field_declarator.function_pointer = NULL;
  } break;
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR: {
    syntax_node->fptr_declarator.fp_dereference = NULL;
    syntax_node->fptr_declarator.name = NULL;
    syntax_node->fptr_declarator.parameters = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->fptr_declarator.parameters->alloc = 0;
    syntax_node->fptr_declarator.parameters->count = 0;
  } break;
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATION: {
    syntax_node->fptr_declaration.declarator = NULL;
    syntax_node->fptr_declaration.return_type_identifier = NULL;
    syntax_node->fptr_declaration.return_type_dereference = NULL;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATION: {
    syntax_node->local_variable_declaration.type_identifier = NULL;
    syntax_node->local_variable_declaration.declarators = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->local_variable_declaration.declarators->alloc = 0;
    syntax_node->local_variable_declaration.declarators->count = 0;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR: {
    syntax_node->local_variable_declarator.type_dereference = NULL;
    syntax_node->local_variable_declarator.initializer = NULL;
    syntax_node->local_variable_declarator.variable_name = NULL;
    syntax_node->local_variable_declarator.function_pointer = NULL;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER: {
    syntax_node->local_variable_array_initializer.size_expression = NULL;
    syntax_node->local_variable_array_initializer.assignment_expression = NULL;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER: {
    syntax_node->local_variable_assignment_initializer.value_expression = NULL;
  } break;
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION: {
    syntax_node->assignment_expression.variable = NULL;
    syntax_node->assignment_expression.assignment_operator = NULL;
    syntax_node->assignment_expression.value_expression = NULL;
  } break;
  // case MC_SYNTAX_ARITHMETIC_ASSIGNMENT_STATEMENT: {
  //   syntax_node->arithmetic_assignment.variable = NULL;
  //   syntax_node->arithmetic_assignment.assignment_operator = NULL;
  //   syntax_node->arithmetic_assignment.value_expression = NULL;
  // } break;
  case MC_SYNTAX_INVOCATION: {
    syntax_node->invocation.function_identity = NULL;
    syntax_node->invocation.arguments = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->invocation.arguments->alloc = 0;
    syntax_node->invocation.arguments->count = 0;
  } break;
  case MC_SYNTAX_TERNARY_CONDITIONAL: {
    syntax_node->ternary_conditional.condition = NULL;
    syntax_node->ternary_conditional.true_expression = NULL;
    syntax_node->ternary_conditional.false_expression = NULL;
  } break;
  case MC_SYNTAX_LOGICAL_EXPRESSION: {
    syntax_node->logical_expression.left = NULL;
    syntax_node->logical_expression.logical_operator = NULL;
    syntax_node->logical_expression.right = NULL;
  } break;
  case MC_SYNTAX_BITWISE_EXPRESSION: {
    syntax_node->bitwise_expression.left = NULL;
    syntax_node->bitwise_expression.bitwise_operator = NULL;
    syntax_node->bitwise_expression.right = NULL;
  } break;
  case MC_SYNTAX_RELATIONAL_EXPRESSION: {
    syntax_node->relational_expression.left = NULL;
    syntax_node->relational_expression.relational_operator = NULL;
    syntax_node->relational_expression.right = NULL;
  } break;
  case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
    syntax_node->operational_expression.left = NULL;
    syntax_node->operational_expression.operational_operator = NULL;
    syntax_node->operational_expression.right = NULL;
  } break;
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
    syntax_node->member_access_expression.primary = NULL;
    syntax_node->member_access_expression.access_operator = NULL;
    syntax_node->member_access_expression.identifier = NULL;
  } break;
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
    syntax_node->element_access_expression.primary = NULL;
    syntax_node->element_access_expression.access_expression = NULL;
  } break;
  case MC_SYNTAX_FIXREMENT_EXPRESSION: {
    syntax_node->fixrement_expression.primary = NULL;
    syntax_node->fixrement_expression.fix_operator = NULL;
  } break;
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
    syntax_node->fixrement_expression.primary = NULL;
    syntax_node->fixrement_expression.fix_operator = NULL;
  } break;
  case MC_SYNTAX_DEREFERENCE_EXPRESSION: {
    syntax_node->dereference_expression.deref_sequence = NULL;
    syntax_node->dereference_expression.unary_expression = NULL;
  } break;
  case MC_SYNTAX_CAST_EXPRESSION: {
    syntax_node->cast_expression.type_identifier = NULL;
    syntax_node->cast_expression.type_dereference = NULL;
    syntax_node->cast_expression.expression = NULL;
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    syntax_node->parenthesized_expression.expression = NULL;
  } break;
  case MC_SYNTAX_TYPE_INITIALIZER: {
    // Nothing for the moment
  } break;
  case MC_SYNTAX_INITIALIZER_EXPRESSION: {
    // Nothing for the moment
    syntax_node->initializer_expression.list = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->initializer_expression.list->alloc = 0;
    syntax_node->initializer_expression.list->count = 0;
  } break;
  case MC_SYNTAX_SIZEOF_EXPRESSION: {
    syntax_node->sizeof_expression.type_identifier = NULL;
    syntax_node->sizeof_expression.type_dereference = NULL;
  } break;
  case MC_SYNTAX_OFFSETOF_EXPRESSION: {
    syntax_node->offsetof_expression.type_identifier = NULL;
    syntax_node->offsetof_expression.type_dereference = NULL;
    syntax_node->offsetof_expression.field_identity = NULL;
  } break;
  case MC_SYNTAX_VA_ARG_EXPRESSION: {
    syntax_node->va_arg_expression.list_identity = NULL;
    syntax_node->va_arg_expression.type_identifier = NULL;
    syntax_node->va_arg_expression.type_dereference = NULL;
  } break;
  case MC_SYNTAX_VA_LIST_STATEMENT: {
    syntax_node->va_list_expression.list_identity = NULL;
  } break;
  case MC_SYNTAX_VA_START_STATEMENT: {
    // Nothing for the moment
  } break;
  case MC_SYNTAX_VA_END_STATEMENT: {
    // Nothing for the moment
  } break;
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION: {
    // Nothing for the moment
  } break;
  case MC_SYNTAX_TYPE_IDENTIFIER: {
    syntax_node->type_identifier.identifier = NULL;
    syntax_node->type_identifier.is_const = false;
    syntax_node->type_identifier.is_static = false;
    syntax_node->type_identifier.has_struct_prepend = false;
    syntax_node->type_identifier.is_signed = -1;
    syntax_node->type_identifier.size_modifiers = NULL;
  } break;
  case MC_SYNTAX_DEREFERENCE_SEQUENCE: {
    syntax_node->dereference_sequence.count = 0;
  } break;
  case MC_SYNTAX_INCLUDE_SYSTEM_HEADER_IDENTITY: {
    syntax_node->include_directive.is_system_header_search = false;
    syntax_node->include_directive.filepath = NULL;
  } break;
  case MC_SYNTAX_SUPERNUMERARY: {
    // TODO ?
  } break;
  default: {
    if ((int)node_type < (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
      // Token
      // -- Do nothing
      break;
    }
    MCerror(355, "Unsupported:%s %i", get_mc_syntax_token_type_name(node_type), node_type);
  }
  }

  if (parent) {
    mcs_add_syntax_node_to_parent(parent, syntax_node);
  }
  else {
    syntax_node->parent = NULL;
  }

  *result = syntax_node;
  return 0;
}

// TODO -- inline this method
int release_syntax_node_list(mc_syntax_node_list *list, bool release_items)
{
  if (!list)
    return 0;

  if (list->alloc) {
    for (int i = 0; release_items && i < list->count; ++i) {
      release_syntax_node(list->items[i]);
    }

    free(list->items);
  }

  free(list);

  return 0;
}

int release_syntax_node(mc_syntax_node *sn)
{
  if (!sn) {
    return 0;
  }

  // Tokens
  if ((int)sn->type < (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    if (sn->text) {
      free(sn->text);
    }
    return 0;
  }

  // Release all children
  release_syntax_node_list(sn->children, true);

  // Release all created lists (but not their items which were released with children above)
  switch (sn->type) {
  case MC_SYNTAX_FUNCTION: {
    release_syntax_node_list(sn->function.parameters, false);
  } break;
  case MC_SYNTAX_STRUCT_DECL: {
    release_syntax_node_list(sn->struct_decl.fields, false);
  } break;
  case MC_SYNTAX_UNION_DECL: {
    release_syntax_node_list(sn->union_decl.fields, false);
  } break;
  case MC_SYNTAX_ENUM_DECL: {
    release_syntax_node_list(sn->enumeration.members, false);
  } break;
  case MC_SYNTAX_INVOCATION: {
    release_syntax_node_list(sn->invocation.arguments, false);
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_DEFINE: {
    release_syntax_node_list(sn->preprocess_define.replacement_list, false);
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_IFDEF: {
    release_syntax_node_list(sn->preprocess_ifdef.groupopt, false);
  } break;
  case MC_SYNTAX_PP_DIRECTIVE_IFNDEF: {
    release_syntax_node_list(sn->preprocess_ifndef.groupopt, false);
  } break;
  case MC_SYNTAX_FIELD_DECLARATION: {
    release_syntax_node_list(sn->field.declarators, false);
  } break;
  case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
    release_syntax_node_list(sn->nested_type.declarators, false);
  } break;
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR: {
    release_syntax_node_list(sn->fptr_declarator.parameters, false);
  } break;
  case MC_SYNTAX_FILE_ROOT:
  case MC_SYNTAX_TYPE_IDENTIFIER:
  case MC_SYNTAX_FIELD_DECLARATOR:
  case MC_SYNTAX_DEREFERENCE_SEQUENCE:
  case MC_SYNTAX_TYPE_ALIAS:
  case MC_SYNTAX_PARAMETER_DECLARATION:
  case MC_SYNTAX_PP_DIRECTIVE_INCLUDE:
  case MC_SYNTAX_ENUM_DECL_MEMBER:
    break;
  default: {
    MCerror(6616, "release_sn(); Clear type:%s for proper release of item collections",
            get_mc_syntax_token_type_name(sn->type));
  }
  }

  // TODO this -- should be releasing syntax nodes when you parse anything
  return 0;
}

int _mcs_parse_token(char *code, int *index, mc_token_type *token_type, char **text)
{
  switch (code[*index]) {
  case '\0': {
    *token_type = MC_TOKEN_NULL_CHARACTER;
    if (text) {
      allocate_and_copy_cstr(*text, "");
    }
  } break;
  case '{': {
    *token_type = MC_TOKEN_CURLY_OPENING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "{");
    }
    ++*index;
  } break;
  case '}': {
    *token_type = MC_TOKEN_CURLY_CLOSING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "}");
    }
    ++*index;
  } break;
  case '(': {
    *token_type = MC_TOKEN_OPENING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "(");
    }
    ++*index;
  } break;
  case ')': {
    *token_type = MC_TOKEN_CLOSING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, ")");
    }
    ++*index;
  } break;
  case '[': {
    *token_type = MC_TOKEN_SQUARE_OPENING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "[");
    }
    ++*index;
  } break;
  case ']': {
    *token_type = MC_TOKEN_SQUARE_CLOSING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "]");
    }
    ++*index;
  } break;
  case '"': {
    *token_type = MC_TOKEN_STRING_LITERAL;

    bool escaped = false;
    bool loop = true;
    int s = *index;
    while (loop) {
      ++*index;
      switch (code[*index]) {
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
        ++*index;
        loop = false;
      } break;
      default: {
        escaped = false;
      } break;
      }
    }
    if (text) {
      allocate_and_copy_cstrn(*text, code + s, *index - s);
    }
  } break;
  case '\'': {
    *token_type = MC_TOKEN_CHAR_LITERAL;

    int s = *index;
    ++*index;
    if (code[*index] == '\\') {
      // Escape
      ++*index;
    }
    *index += 2;

    if (text) {
      allocate_and_copy_cstrn(*text, code + s, *index - s);
    }
  } break;
  case ';': {
    *token_type = MC_TOKEN_SEMI_COLON;
    if (text) {
      allocate_and_copy_cstr(*text, ";");
    }
    ++*index;
  } break;
  case ':': {
    *token_type = MC_TOKEN_COLON;
    if (text) {
      allocate_and_copy_cstr(*text, ":");
    }
    ++*index;
  } break;
  case '!': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_INEQUALITY_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "!=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_LOGICAL_NOT_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "!");
    }
    ++*index;
  } break;
  case '.': {
    *token_type = MC_TOKEN_DECIMAL_POINT;
    if (text) {
      allocate_and_copy_cstr(*text, ".");
    }
    ++*index;
  } break;
  case '*': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "*=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_STAR_CHARACTER;
    if (text) {
      allocate_and_copy_cstr(*text, "*");
    }
    ++*index;
  } break;
  case '~': {
    *token_type = MC_TOKEN_BITWISE_NOT_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "~");
    }
    ++*index;
  } break;
  case '-': {
    if (code[*index + 1] == '>') {
      *token_type = MC_TOKEN_POINTER_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "->");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '-') {
      *token_type = MC_TOKEN_DECREMENT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "--");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "-=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_SUBTRACT_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "-");
    }
    ++*index;
  } break;
  case '+': {
    if (code[*index + 1] == '+') {
      *token_type = MC_TOKEN_INCREMENT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "++");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "+=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_PLUS_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "+");
    }
    ++*index;
  } break;
  case '%': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_MODULO_AND_ASSIGN_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "%=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_MODULO_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "%");
    }
    ++*index;
  } break;
  case '&': {
    if (code[*index + 1] == '&') {
      *token_type = MC_TOKEN_LOGICAL_AND_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "&&");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_BINARY_AND_ASSIGNMENT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "&=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_AMPERSAND_CHARACTER;
    if (text) {
      allocate_and_copy_cstr(*text, "&");
    }
    ++*index;
  } break;
  case '|': {
    if (code[*index + 1] == '|') {
      *token_type = MC_TOKEN_LOGICAL_OR_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "||");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_BINARY_OR_ASSIGNMENT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "|=");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_BITWISE_OR_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "|");
    }
    ++*index;
  } break;
  case '\\': {
    *token_type = MC_TOKEN_ESCAPE_CHARACTER;
    if (text) {
      allocate_and_copy_cstr(*text, "\\");
    }
    ++*index;
  } break;
  case ',': {
    *token_type = MC_TOKEN_COMMA;
    if (text) {
      allocate_and_copy_cstr(*text, ",");
    }
    ++*index;
  } break;
  case '?': {
    *token_type = MC_TOKEN_TERNARY_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "?");
    }
    ++*index;
  } break;
  case '\n': {
    *token_type = MC_TOKEN_NEW_LINE;
    if (text) {
      allocate_and_copy_cstr(*text, "\n");
    }
    ++*index;
  } break;
  case ' ': {
    *token_type = MC_TOKEN_SPACE_SEQUENCE;
    int s = *index;
    while (code[*index] == ' ') {
      ++*index;
    }
    if (text) {
      allocate_and_copy_cstrn(*text, code + s, *index - s);
    }
  } break;
  case '/': {
    int s = *index;
    switch (code[s + 1]) {
    case '/': {
      *token_type = MC_TOKEN_LINE_COMMENT;
      while (1) {
        ++*index;
        if (code[*index] == '\0') {
          break;
        }
        if (code[*index] == '\n') {
          break;
        }
      }

      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
      }
      // printf("after the line:'%c'\n", code[*index]);

    } break;
    case '*': {
      *token_type = MC_TOKEN_MULTI_LINE_COMMENT;
      while (1) {
        ++*index;
        if (code[*index] == '\0') {
          // print_parse_error(code, *index, "_mcs_parse_token", "eof");
          MCerror(72, "Unexpected end-of-file\n");
        }
        if (code[*index] == '*' && code[*index + 1] == '/') {
          *index += 2;
          break;
        }
      }

      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
      }
      // printf("after the line:'%c'\n", code[*index]);

    } break;
    case '\0': {
      MCerror(732, "TODO");
    }
    default: {
      *token_type = MC_TOKEN_DIVIDE_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "/");
      }
      ++*index;
    } break;
    }
  } break;
  case '=': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_EQUALITY_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "==");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_ASSIGNMENT_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "=");
    }
    ++*index;
  } break;
  case '<': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "<=");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '<') {
      if (code[*index + 2] == '=') {
        *token_type = MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR;
        if (text) {
          allocate_and_copy_cstr(*text, "<<=");
        }
        *index += 3;
        break;
      }

      *token_type = MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, "<<");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_ARROW_OPENING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "<");
    }
    ++*index;
  } break;
  case '>': {
    if (code[*index + 1] == '=') {
      *token_type = MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, ">=");
      }
      *index += 2;
      break;
    }
    else if (code[*index + 1] == '>') {
      if (code[*index + 2] == '=') {
        *token_type = MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR;
        if (text) {
          allocate_and_copy_cstr(*text, ">>=");
        }
        *index += 3;
        break;
      }

      *token_type = MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR;
      if (text) {
        allocate_and_copy_cstr(*text, ">>");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_ARROW_CLOSING_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, ">");
    }
    ++*index;
  } break;
  case '#': {
    // Preprocessor Keyword
    int s = *index;
    ++*index;
    if (code[*index] == '#') {
      ++*index;
    }
    while (isalnum(code[*index]) || code[*index] == '_') {
      ++*index;
    }

    // Keywords
    int slen = *index - s;
    {
      // Keywords
      if (slen == 13 && !strncmp(code + s, "##__VA_ARGS__", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_VARIADIC_ARGS;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
      if (slen == 6 && !strncmp(code + s, "#ifdef", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_IFDEF;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
      if (slen == 7 && !strncmp(code + s, "#ifndef", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_IFNDEF;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
      if (slen == 6 && !strncmp(code + s, "#endif", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_ENDIF;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
      if (slen == 7 && !strncmp(code + s, "#define", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_DEFINE;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
      if (slen == 8 && !strncmp(code + s, "#include", slen)) {
        *token_type = MC_TOKEN_PP_KEYWORD_INCLUDE;
        if (text) {
          allocate_and_copy_cstrn(*text, code + s, slen);
        }
        break;
      }
    }

    *token_type = MC_TOKEN_PP_IDENTIFIER;
    if (text) {
      allocate_and_copy_cstrn(*text, code + s, slen);
    }

    // char *error_text;
    // if (!text) {
    //   allocate_and_copy_cstrn(error_text, code + s, slen);
    // }
    // else {
    //   error_text = *text;
    // }
    // print_parse_error(code, s, "_mcs_parse_token", "#");
    // MCerror(1273, "Unsupported preprocessor keyword:'%s'", error_text);
  } break;
  default: {
    // Identifier
    if (isalpha(code[*index]) || code[*index] == '_') {
      int s = *index;
      while (isalnum(code[*index]) || code[*index] == '_') {
        ++*index;
      }

      // Keywords
      int slen = *index - s;
      {
        // Keywords
        if (slen == 2 && !strncmp(code + s, "if", slen)) {
          *token_type = MC_TOKEN_IF_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "extern", slen)) {
          *token_type = MC_TOKEN_EXTERN_KEYWORD;

          if (strncmp(code + s + slen, " \"C\"", 4)) {
            print_parse_error(code, s + slen, "see-below", "");
            MCerror(1197, "TODO");
          }
          slen += 4;
          *index += 4;

          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 7 && !strncmp(code + s, "typedef", slen)) {
          *token_type = MC_TOKEN_TYPEDEF_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "else", slen)) {
          *token_type = MC_TOKEN_ELSE_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "goto", slen)) {
          *token_type = MC_TOKEN_GOTO_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 5 && !strncmp(code + s, "while", slen)) {
          *token_type = MC_TOKEN_WHILE_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "va_arg", slen)) {
          *token_type = MC_TOKEN_VA_ARG_WORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 7 && !strncmp(code + s, "va_list", slen)) {
          *token_type = MC_TOKEN_VA_LIST_WORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 8 && !strncmp(code + s, "va_start", slen)) {
          *token_type = MC_TOKEN_VA_START_WORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "va_end", slen)) {
          *token_type = MC_TOKEN_VA_END_WORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "sizeof", slen)) {
          *token_type = MC_TOKEN_SIZEOF_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 8 && !strncmp(code + s, "offsetof", slen)) {
          *token_type = MC_TOKEN_OFFSETOF_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 2 && !strncmp(code + s, "do", slen)) {
          *token_type = MC_TOKEN_DO_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 3 && !strncmp(code + s, "for", slen)) {
          *token_type = MC_TOKEN_FOR_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 8 && !strncmp(code + s, "continue", slen)) {
          *token_type = MC_TOKEN_CONTINUE_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 5 && !strncmp(code + s, "break", slen)) {
          *token_type = MC_TOKEN_BREAK_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "switch", slen)) {
          *token_type = MC_TOKEN_SWITCH_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "return", slen)) {
          *token_type = MC_TOKEN_RETURN_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 5 && !strncmp(code + s, "const", slen)) {
          *token_type = MC_TOKEN_CONST_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "static", slen)) {
          *token_type = MC_TOKEN_STATIC_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "case", slen)) {
          *token_type = MC_TOKEN_CASE_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 7 && !strncmp(code + s, "default", slen)) {
          *token_type = MC_TOKEN_DEFAULT_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "enum", slen)) {
          *token_type = MC_TOKEN_ENUM_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "struct", slen)) {
          *token_type = MC_TOKEN_STRUCT_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 5 && !strncmp(code + s, "union", slen)) {
          *token_type = MC_TOKEN_UNION_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "void", slen)) {
          *token_type = MC_TOKEN_VOID_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "char", slen)) {
          *token_type = MC_TOKEN_CHAR_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 3 && !strncmp(code + s, "int", slen)) {
          *token_type = MC_TOKEN_INT_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 8 && !strncmp(code + s, "unsigned", slen)) {
          *token_type = MC_TOKEN_UNSIGNED_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 6 && !strncmp(code + s, "signed", slen)) {
          *token_type = MC_TOKEN_SIGNED_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        // if (slen == 4 && !strncmp(code + s, "bool", slen)) {
        //   *token_type = MC_TOKEN_BOOL_KEYWORD;
        //   if (text) {
        //     allocate_and_copy_cstrn(*text, code + s, slen);
        //   }
        //   break;
        // }
        if (slen == 5 && !strncmp(code + s, "float", slen)) {
          *token_type = MC_TOKEN_FLOAT_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 4 && !strncmp(code + s, "long", slen)) {
          *token_type = MC_TOKEN_LONG_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
        if (slen == 5 && !strncmp(code + s, "short", slen)) {
          *token_type = MC_TOKEN_SHORT_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
      }

      // Plain Identifier
      *token_type = MC_TOKEN_IDENTIFIER;
      if (text) {
        allocate_and_copy_cstrn(*text, code + s, slen);
      }
      break;
    }

    // Number
    if (isdigit(code[*index])) {
      *token_type = MC_TOKEN_NUMERIC_LITERAL;
      int s = *index;
      bool prev_digit = true;
      bool loop = true;
      bool is_hex = false, is_binary = false, is_scientific = false;
      if (code[*index] == '0') {
        if (code[*index + 1] == 'x') {
          is_hex = true;
          ++*index;
        }
        else if (code[*index + 1] == 'b') {
          is_binary = true;
          ++*index;
        }
      }
      else if (code[*index] == '1') {
        if (code[*index + 1] == 'e') {
          is_scientific = true;
          ++*index;
          if (code[*index + 1] == '-') {
            ++*index;
          }

          if (!isdigit(code[*index + 1])) {
            MCerror(1630, "Expected Numeric Digit");
          }
        }
      }

      while (loop) {
        ++*index;
        if (is_binary) {
          if (code[*index] == '0' || code[*index] == '1')
            continue;
        }
        else if (is_hex) {
          if (isdigit(code[*index]))
            continue;
          switch (code[*index]) {
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
            continue;
          default:
            break;
          }
        }
        else if (isdigit(code[*index])) {
          prev_digit = true;
          continue;
        }

        // Other characters
        switch (code[*index]) {
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E':
          if (is_hex) {
            prev_digit = true;
            continue;
          }
          loop = false;
          break;
        case 'f':
        case 'F':
          if (is_hex) {
            prev_digit = true;
            continue;
          }
          ++*index;
          loop = false;
          break;
        case '.': {
          if (prev_digit && !is_hex && !is_binary) {
            prev_digit = false;
            continue;
          }
          print_parse_error(code, *index, "_mcs_parse_token", "");
          MCerror(173, "Invalid Numeric Literal Format");
        } break;
        // case 'F':
        // case 'f': {
        //   loop = false;
        //   ++*index;
        // } break;
        case 'u':
        case 'U': {
          switch (code[*index + 1]) {
          case 'l':
          case 'L':
            ++*index;
            break;
          case '\n':
          case ']':
          case ':':
          case ' ':
          case ',':
          case ')':
          case '}':
          case ';':
            break;
          default: {
            print_parse_error(code, *index, "_mcs_parse_token", "");
            MCerror(1817, "NotYetSupported Numeric Literal unsigned postfix character:'%c'", code[*index]);
          }
          }

          loop = false;
          ++*index;
        } break;

        // Indicates valid bread from reading numeric literal
        case '\n':
        case ']':
        case ':':
        case ' ':
        case ',':
        case ')':
        case '}':
        case ';': {
          loop = false;
        } break;
        default:
          print_parse_error(code, *index, "_mcs_parse_token", "");
          MCerror(191, "Invalid Numeric Literal character:'%c'", code[*index]);
        }
      }

      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
      }
      break;
    }

    print_parse_error(code, *index, "_mcs_parse_token", "");
    MCerror(101, "unknown token");
  }
  }

  return 0;
}

int mcs_is_supernumerary_token(mc_token_type token_type, bool *is_supernumerary_token)
{
  switch (token_type) {
  case MC_TOKEN_MULTI_LINE_COMMENT:
  case MC_TOKEN_LINE_COMMENT:
  case MC_TOKEN_SPACE_SEQUENCE:
  case MC_TOKEN_TAB_SEQUENCE:
  case MC_TOKEN_NEW_LINE: {
    *is_supernumerary_token = true;
  } break;
  default:
    *is_supernumerary_token = false;
    break;
  }

  // printf("issupernum: %s = %s\n", get_mc_token_type_name(token_type), (*is_supernumerary_token) ? "is!" : "is
  // not!");

  return 0;
}

int mcs_peek_token_type(parsing_state *ps, bool include_supernumerary_tokens, int tokens_ahead,
                        mc_token_type *token_type)
{
  int index = ps->index;
  ++tokens_ahead;
  while (tokens_ahead) {
    _mcs_parse_token(ps->code, &index, token_type, NULL);

    if (!include_supernumerary_tokens) {
      bool is_supernumerary_token;
      mcs_is_supernumerary_token(*token_type, &is_supernumerary_token);
      if (is_supernumerary_token) {
        ++tokens_ahead;
      }
    }

    --tokens_ahead;
  }
  return 0;
}

int mcs_peek_token_type_and_index(parsing_state *ps, bool include_supernumerary_tokens, int tokens_ahead,
                                  mc_token_type *token_type, int *token_end_index)
{
  *token_end_index = ps->index;
  ++tokens_ahead;
  while (tokens_ahead) {
    _mcs_parse_token(ps->code, token_end_index, token_type, NULL);

    if (!include_supernumerary_tokens) {
      bool is_supernumerary_token;
      mcs_is_supernumerary_token(*token_type, &is_supernumerary_token);
      if (is_supernumerary_token) {
        ++tokens_ahead;
      }
    }

    --tokens_ahead;
  }
  return 0;
}

int mcs_parse_token(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **token_result)
{
  mc_token_type type;
  char *text;
  _mcs_parse_token(ps->code, &ps->index, &type, &text);
  // printf("-%s-\n", text);

  mcs_construct_syntax_node(ps, (mc_syntax_node_type)type, text, parent, token_result);

  // append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
  //                             *token_result);

  // printf("parse:'%s'\n", text);

  // Adjust parsing state line & column
  switch (type) {
  case MC_TOKEN_TAB_SEQUENCE: {
    MCerror(72, "ERR-unhandled-token:%s:%s", get_mc_token_type_name(type), text);
  } break;
  case MC_TOKEN_NEW_LINE: {
    ++ps->line;
    ps->col = 0;
  } break;
  default:
    ps->col += strlen(text);
    break;
  }

  return 0;
}

int mcs_parse_through_token(parsing_state *ps, mc_syntax_node *parent, mc_token_type expected_token,
                            mc_syntax_node **additional_ptr)
{
  mc_syntax_node *token;
  mcs_parse_token(ps, parent, &token);

  mc_token_type was_type = (mc_token_type)token->type;

  if (expected_token != MC_TOKEN_NULL && was_type != expected_token) {
    print_parse_error(ps->code, ps->index - strlen(token->text), "mcs_parse_through_token", "");
    MCerror(878, "Unexpected token. Expected:%s Was:%s", get_mc_token_type_name(expected_token),
            get_mc_token_type_name(was_type));
  }

  if (additional_ptr) {
    *additional_ptr = token;
  }

  return 0;
}

int mcs_parse_through_supernumerary_tokens(parsing_state *ps, mc_syntax_node *parent)
{
  while (1) {
    mc_token_type token_type;
    mcs_peek_token_type(ps, true, 0, &token_type);
    // printf("sps->index:%i peek_type:%s\n", ps->index, get_mc_token_type_name(token_type));
    bool is_supernumerary_token;
    mcs_is_supernumerary_token(token_type, &is_supernumerary_token);

    if (!is_supernumerary_token) {
      break;
    }

    // printf("sps->index:%i\n", ps->index);
    mcs_parse_through_token(ps, parent, token_type, NULL);
  }

  // printf("sps->index:%i\n", ps->index);
  return 0;
}

int mcs_parse_dereference_sequence(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_dereference_sequence()");
  mc_syntax_node *sequence;
  mcs_construct_syntax_node(ps, MC_SYNTAX_DEREFERENCE_SEQUENCE, NULL, parent, &sequence);
  if (additional_destination) {
    *additional_destination = sequence;
  }

  while (1) {
    mcs_parse_through_token(ps, sequence, MC_TOKEN_STAR_CHARACTER, NULL);
    ++sequence->dereference_sequence.count;

    mc_token_type token0;
    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_CONST_KEYWORD) {
      mcs_parse_through_token(ps, sequence, MC_TOKEN_CONST_KEYWORD, NULL);
      mcs_peek_token_type(ps, false, 0, &token0);
    }
    if (token0 != MC_TOKEN_STAR_CHARACTER) {
      break;
    }

    mcs_parse_through_supernumerary_tokens(ps, sequence);
  }

  return 0;
}

int mcs_parse_function_pointer_declarator(parsing_state *ps, mc_syntax_node *parent, bool allow_name_skip,
                                          mc_syntax_node **additional_destination)
{
  mc_syntax_node *fp_decl;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FUNCTION_POINTER_DECLARATOR, NULL, parent, &fp_decl);
  if (additional_destination) {
    *additional_destination = fp_decl;
  }

  mcs_parse_through_token(ps, fp_decl, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, fp_decl);

  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_STAR_CHARACTER) {
    mcs_parse_dereference_sequence(ps, fp_decl, &fp_decl->fptr_declarator.fp_dereference);
    mcs_parse_through_supernumerary_tokens(ps, fp_decl);
  }
  else {
    fp_decl->fptr_declarator.fp_dereference = NULL;
  }

  mcs_peek_token_type(ps, false, 0, &token_type);
  if (!allow_name_skip || token_type == MC_TOKEN_IDENTIFIER) {
    mcs_parse_through_token(ps, fp_decl, MC_TOKEN_IDENTIFIER, &fp_decl->fptr_declarator.name);
    mcs_parse_through_supernumerary_tokens(ps, fp_decl);
  }

  mcs_parse_through_token(ps, fp_decl, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, fp_decl);

  mcs_parse_through_token(ps, fp_decl, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, fp_decl);

  mc_syntax_node_list *parameter_list = fp_decl->fptr_declarator.parameters;

  while (1) {
    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_CLOSING_BRACKET) {
      break;
    }
    else if (parameter_list->count) {
      mcs_parse_through_token(ps, fp_decl, MC_TOKEN_COMMA, NULL);
      mcs_parse_through_supernumerary_tokens(ps, fp_decl);
    }

    mc_syntax_node *parameter_decl;
    mcs_parse_parameter_declaration(ps, true, fp_decl, &parameter_decl);
    append_to_collection((void ***)&parameter_list->items, &parameter_list->alloc, &parameter_list->count,
                         parameter_decl);
  }
  mcs_parse_through_token(ps, fp_decl, MC_TOKEN_CLOSING_BRACKET, NULL);

  return 0;
}

int mcs_parse_function_pointer_declaration(parsing_state *ps, mc_syntax_node *parent, bool allow_name_skip,
                                           mc_syntax_node **additional_destination)
{
  mc_syntax_node *fp_decl;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FUNCTION_POINTER_DECLARATION, NULL, parent, &fp_decl);
  if (additional_destination) {
    *additional_destination = fp_decl;
  }

  mcs_parse_type_identifier(ps, fp_decl, &fp_decl->fptr_declaration.return_type_identifier);
  mcs_parse_through_supernumerary_tokens(ps, fp_decl);

  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    mcs_parse_dereference_sequence(ps, fp_decl, &fp_decl->fptr_declaration.return_type_dereference);
    mcs_parse_through_supernumerary_tokens(ps, fp_decl);
  }
  else {
    fp_decl->fptr_declaration.return_type_dereference = NULL;
  }

  mcs_parse_function_pointer_declarator(ps, fp_decl, allow_name_skip, &fp_decl->fptr_declaration.declarator);

  return 0;
}

int mcs_parse_parameter_declaration(parsing_state *ps, bool allow_name_skip, mc_syntax_node *parent,
                                    mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_parameter_declaration()");
  mc_syntax_node *parameter_decl;
  mcs_construct_syntax_node(ps, MC_SYNTAX_PARAMETER_DECLARATION, NULL, parent, &parameter_decl);
  if (additional_destination) {
    *additional_destination = parameter_decl;
  }

  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_DECIMAL_POINT) {
    parameter_decl->parameter.type = PARAMETER_KIND_VARIABLE_ARGS;

    mcs_parse_through_token(ps, parameter_decl, MC_TOKEN_DECIMAL_POINT, NULL);
    mcs_parse_through_token(ps, parameter_decl, MC_TOKEN_DECIMAL_POINT, NULL);
    mcs_parse_through_token(ps, parameter_decl, MC_TOKEN_DECIMAL_POINT, NULL);
  }
  else {

    mcs_parse_type_identifier(ps, parameter_decl, &parameter_decl->parameter.type_identifier);
    mcs_parse_through_supernumerary_tokens(ps, parameter_decl);

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, parameter_decl, &parameter_decl->parameter.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, parameter_decl);
    }
    else {
      parameter_decl->parameter.type_dereference = NULL;
    }

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_OPENING_BRACKET) {
      parameter_decl->parameter.type = PARAMETER_KIND_FUNCTION_POINTER;

      mcs_parse_function_pointer_declarator(ps, parameter_decl, true, &parameter_decl->parameter.function_pointer);
    }
    else {
      parameter_decl->parameter.type = PARAMETER_KIND_STANDARD;
      // Standard 'Type (*) Name' parameter

      bool skip_name = false;
      if (allow_name_skip) {
        mc_token_type token_type;
        mcs_peek_token_type(ps, false, 0, &token_type);
        if (token_type != MC_TOKEN_IDENTIFIER) {
          skip_name = true;
        }
      }
      if (!skip_name) {
        mcs_parse_through_token(ps, parameter_decl, MC_TOKEN_IDENTIFIER, &parameter_decl->parameter.name);
      }
    }
  }

  register_midge_error_tag("mcs_parse_parameter_declaration(~)");
  return 0;
}

int mcs_parse_type_identifier(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *type_root;
  mcs_construct_syntax_node(ps, MC_SYNTAX_TYPE_IDENTIFIER, NULL, NULL, &type_root);
  type_root->type_identifier.is_const = false;
  type_root->type_identifier.has_struct_prepend = false;
  type_root->type_identifier.is_signed = -1;

  // Initial const
  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_CONST_KEYWORD) {
    type_root->type_identifier.is_const = true;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_CONST_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }

  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_STATIC_KEYWORD) {
    type_root->type_identifier.is_static = true;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_STATIC_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }

  // Signing
  if (token_type == MC_TOKEN_UNSIGNED_KEYWORD) {
    type_root->type_identifier.is_signed = 0;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_UNSIGNED_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }
  else if (token_type == MC_TOKEN_SIGNED_KEYWORD) {
    type_root->type_identifier.is_signed = 1;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_SIGNED_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }

  // Struct
  if (token_type == MC_TOKEN_STRUCT_KEYWORD) {
    type_root->type_identifier.has_struct_prepend = true;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_STRUCT_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type != MC_TOKEN_IDENTIFIER) {
      MCerror(1507, "FORMAT ERROR: expected struct identifier");
    }
  }

  switch (token_type) {
  case MC_TOKEN_IDENTIFIER: {
    if (type_root->type_identifier.is_signed != -1) {
      print_parse_error(ps->code, ps->index, "mcs_parse_type_identifier", "");
      MCerror(2266, "modifier cannot be pre-applied to custom struct");
    }
  } break;
  case MC_TOKEN_VOID_KEYWORD: {
    if (type_root->type_identifier.is_signed != -1) {
      MCerror(1014, "modifier cannot be pre-applied to void");
    }
  } break;
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_SHORT_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  // case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD: {
    // Valid
    break;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(6808, "MCS:>:ERR-token:%s", get_mc_token_type_name(token_type));
  }
  }

  mcs_parse_through_token(ps, type_root, token_type, &type_root->type_identifier.identifier);

  register_midge_error_tag("mcs_parse_type_identifier()-4");

  // Trailing const
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_CONST_KEYWORD) {
    type_root->type_identifier.is_const = true;
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_parse_through_token(ps, type_root, MC_TOKEN_CONST_KEYWORD, NULL);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }

  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_STATIC_KEYWORD) {
    type_root->type_identifier.is_static = true;
    mcs_parse_through_token(ps, type_root, MC_TOKEN_STATIC_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_root);
    mcs_peek_token_type(ps, false, 0, &token_type);
  }

  mcs_add_syntax_node_to_parent(parent, type_root);
  if (additional_destination) {
    *additional_destination = type_root;
  }

  register_midge_error_tag("mcs_parse_type_identifier(~)");
  return 0;
}

int mcs_parse_expression_variable_access(parsing_state *ps, mc_syntax_node *parent,
                                         mc_syntax_node **additional_destination)
{
  mc_token_type token0, token1;
  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 != MC_TOKEN_IDENTIFIER) {
    MCerror(857, "TODO:%s", get_mc_token_type_name(token0));
  }

  bool is_member_access = false;
  mcs_peek_token_type(ps, false, 1, &token1);
  switch (token1) {
  case MC_TOKEN_DECIMAL_POINT:
  case MC_TOKEN_POINTER_OPERATOR: {
    is_member_access = true;
  } break;
  case MC_TOKEN_SEMI_COLON:
  case MC_TOKEN_COMMA:
  case MC_TOKEN_OPENING_BRACKET:
  case MC_TOKEN_CLOSING_BRACKET:
  case MC_TOKEN_SQUARE_CLOSING_BRACKET:
  case MC_TOKEN_ASSIGNMENT_OPERATOR:
  case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
  case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
  case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_ARROW_OPENING_BRACKET:
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_ARROW_CLOSING_BRACKET:
  case MC_TOKEN_EQUALITY_OPERATOR:
  case MC_TOKEN_INEQUALITY_OPERATOR: {
    // End-of-line
    // Expected tokens that come at the end of a unary expression
    mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination);
  } break;
  case MC_TOKEN_SQUARE_OPENING_BRACKET: {
    // Element access
    mc_syntax_node *element_access;
    mcs_construct_syntax_node(ps, MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION, NULL, parent, &element_access);
    if (additional_destination) {
      *additional_destination = element_access;
    }

    mcs_parse_through_token(ps, element_access, MC_TOKEN_IDENTIFIER,
                            &element_access->element_access_expression.primary);

    mcs_parse_through_supernumerary_tokens(ps, element_access);
    mcs_parse_through_token(ps, element_access, MC_TOKEN_SQUARE_OPENING_BRACKET, NULL);
    mcs_parse_expression(ps, element_access, &element_access->element_access_expression.access_expression);
    mcs_parse_through_supernumerary_tokens(ps, element_access);
    mcs_parse_through_token(ps, element_access, MC_TOKEN_SQUARE_CLOSING_BRACKET, NULL);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(872, "MCS:>Unsupported-token:%s", get_mc_token_type_name(token1));
  }
  }

  if (is_member_access) {
    // Member-access
    mc_syntax_node *member_access;
    mcs_construct_syntax_node(ps, MC_SYNTAX_MEMBER_ACCESS_EXPRESSION, NULL, parent, &member_access);
    if (additional_destination) {
      *additional_destination = member_access;
    }

    mcs_parse_through_token(ps, member_access, MC_TOKEN_IDENTIFIER, &member_access->member_access_expression.primary);

    mcs_parse_through_supernumerary_tokens(ps, member_access);

    mcs_parse_through_token(ps, member_access, token1, &member_access->member_access_expression.access_operator);

    mcs_parse_expression_variable_access(ps, member_access, &member_access->member_access_expression.identifier);
  }
  else {
    // Element-access
  }

  return 0;
}

// int mcs_parse_function_definition_call(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node
// **additional_destination)
// {
//   // printf("mcs_parse_function_definition_call()\n");
//   mc_syntax_node *statement;
//   mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION_STATEMENT, NULL, parent, &statement);
//   if (additional_destination) {
//     *additional_destination = statement;
//   }

//   mcs_parse_expression(ps, statement, ))

//   mcs_parse_expression_variable_access(ps, statement, &statement->invocation.function_identity;
//   mcs_parse_through_supernumerary_tokens(ps, statement);

//   mc_function_info

//   mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
//   mcs_parse_through_supernumerary_tokens(ps, statement);

//   while (1) {
//     mc_token_type token0;
//     mcs_peek_token_type(ps, false, 0, &token0);

//     if (token0 == MC_TOKEN_CLOSING_BRACKET) {
//       break;
//     }

//     if (statement->invocation.arguments->count) {
//       mcs_parse_through_token(ps, statement, MC_TOKEN_COMMA, NULL);
//       mcs_parse_through_supernumerary_tokens(ps, statement);
//     }

//     mc_syntax_node *argument;
//     mcs_parse_expression(ps, statement, &argument);
//     mcs_parse_through_supernumerary_tokens(ps, statement);

//     append_to_collection((void ***)&statement->invocation.arguments->items,
//                                 &statement->invocation.arguments->alloc, &statement->invocation.arguments->count,
//                                 argument);
//   }
//   mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);

//   // printf("mcs_parse_function_definition_call(ret)\n");
//   return 0;
// }

int mcs_parse_cast_expression(parsing_state *ps, mc_syntax_node *parent, bool is_function_pointer_declaration,
                              mc_syntax_node **additional_destination)
{
  const int CASE_PRECEDENCE = 3;
  mc_syntax_node *cast_expression;
  mcs_construct_syntax_node(ps, MC_SYNTAX_CAST_EXPRESSION, NULL, parent, &cast_expression);
  if (additional_destination) {
    *additional_destination = cast_expression;
  }

  mcs_parse_through_token(ps, cast_expression, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, cast_expression);

  // bool is_function_pointer_declaration;
  mc_token_type token_type;
  // int debug_index;
  // mcs_peek_token_type(ps, false, 0, &token_type);
  // switch (token_type) {
  // case MC_TOKEN_UNION_KEYWORD:
  // case MC_TOKEN_STRUCT_KEYWORD: {
  //   is_function_pointer_declaration = false;
  // } break;
  // case MC_TOKEN_CONST_KEYWORD:
  // case MC_TOKEN_SIGNED_KEYWORD:
  // case MC_TOKEN_UNSIGNED_KEYWORD:
  // case MC_TOKEN_LONG_KEYWORD: {
  //   mcs_peek_token_type_and_index(ps, false, 1, &token_type, &debug_index);
  //   switch (token_type) {
  //   default:
  //     // print_syntax_node(cast_expression, 0);
  //     print_parse_error(ps->code, debug_index, "mcs_parse_cast_expression", "is_fptr");
  //     MCerror(2427, "TODO:%s", get_mc_token_type_name(token_type));
  //     break;
  //   }
  // } break;
  // case MC_TOKEN_CHAR_KEYWORD:
  // case MC_TOKEN_IDENTIFIER: {
  //   mcs_peek_token_type_and_index(ps, false, 1, &token_type, &debug_index);
  //   switch (token_type) {
  //   case MC_TOKEN_STAR_CHARACTER: {
  //     int peek_ahead = 1;
  //     do {
  //       ++peek_ahead;
  //       mcs_peek_token_type_and_index(ps, false, peek_ahead, &token_type, &debug_index);
  //     } while (token_type == MC_TOKEN_STAR_CHARACTER);

  //     switch (token_type) {
  //     case MC_TOKEN_CLOSING_BRACKET: {
  //       is_function_pointer_declaration = false;
  //     } break;
  //     default:
  //       // print_syntax_node(cast_expression, 0);
  //       print_parse_error(ps->code, debug_index, "mcs_parse_cast_expression", "is_fptr");
  //       MCerror(2446, "TODO:%s", get_mc_token_type_name(token_type));
  //       break;
  //     }
  //   } break;
  //   default:
  //     // print_syntax_node(cast_expression, 0);
  //     print_parse_error(ps->code, debug_index, "mcs_parse_cast_expression", "is_fptr");
  //     MCerror(2437, "TODO:%s", get_mc_token_type_name(token_type));
  //     break;
  //   }
  // } break;
  // default:
  //   // print_syntax_node(cast_expression, 0);
  //   print_parse_error(ps->code, ps->index, "mcs_parse_cast_expression", "is_fptr");
  //   MCerror(2236, "TODO:%s", get_mc_token_type_name(token_type));
  //   break;
  // }
  // {
  //   int peek_ahead = 0;
  //   int stage = 0;
  //   bool unsure = true;
  //   while (unsure) {
  //     mcs_peek_token_type(ps, false, peek_ahead, &token_type);
  //     switch (token_type) {
  //     case MC_TOKEN_UNION_KEYWORD:
  //     case MC_TOKEN_STRUCT_KEYWORD: {
  //       unsure = false;
  //     } break;
  //     case MC_TOKEN_CONST_KEYWORD:
  //       break;
  //     case MC_TOKEN_INT_KEYWORD:
  //     case MC_TOKEN_CHAR_KEYWORD:
  //     case MC_TOKEN_LONG_KEYWORD:
  //     case MC_TOKEN_SHORT_KEYWORD:
  //     case MC_TOKEN_FLOAT_KEYWORD:
  //     case MC_TOKEN_VOID_KEYWORD:
  //     case MC_TOKEN_SIGNED_KEYWORD:
  //     case MC_TOKEN_UNSIGNED_KEYWORD:
  //     case MC_TOKEN_IDENTIFIER: {
  //       if (stage) {
  //         // print_syntax_node(cast_expression, 0);
  //         print_parse_error(ps->code, ps->index, "mcs_parse_cast_expression", "is_fptr");
  //         MCerror(2236, "TODO");
  //       }
  //       stage = 1;
  //     } break;
  //     case MC_TOKEN_STAR_CHARACTER: {
  //       if (!stage) {
  //         MCerror(2247, "unexpected");
  //       }
  //       stage = 2;
  //     } break;
  //     case MC_TOKEN_CLOSING_BRACKET: {
  //       unsure = false;
  //     } break;
  //     case MC_TOKEN_OPENING_BRACKET: {
  //       unsure = false;
  //       is_function_pointer_declaration = true;
  //     } break;
  //     default:
  //       MCerror(2244, "Unsupported:%s", get_mc_token_type_name(token_type));
  //     }
  //     ++peek_ahead;
  //   }
  // }

  if (is_function_pointer_declaration) {
    mcs_parse_function_pointer_declaration(ps, cast_expression, true,
                                           &cast_expression->cast_expression.type_identifier);
  }
  else {
    mcs_parse_type_identifier(ps, cast_expression, &cast_expression->cast_expression.type_identifier);
    mcs_parse_through_supernumerary_tokens(ps, cast_expression);

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, cast_expression, &cast_expression->cast_expression.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, cast_expression);
    }
    else {
      cast_expression->cast_expression.type_dereference = NULL;
    }
  }

  mcs_parse_through_token(ps, cast_expression, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, cast_expression);
  _mcs_parse_expression(ps, CASE_PRECEDENCE, cast_expression, &cast_expression->cast_expression.expression);

  return 0;
}

int mcs_parse_parenthesized_expression(parsing_state *ps, mc_syntax_node *parent,
                                       mc_syntax_node **additional_destination)
{
  // const int CASE_PRECEDENCE = 17;
  mc_syntax_node *parenthesized_expression;
  mcs_construct_syntax_node(ps, MC_SYNTAX_PARENTHESIZED_EXPRESSION, NULL, parent, &parenthesized_expression);
  if (additional_destination) {
    *additional_destination = parenthesized_expression;
  }

  mcs_parse_through_token(ps, parenthesized_expression, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, parenthesized_expression);

  mcs_parse_expression(ps, parenthesized_expression, &parenthesized_expression->parenthesized_expression.expression);
  mcs_parse_through_supernumerary_tokens(ps, parenthesized_expression);

  mcs_parse_through_token(ps, parenthesized_expression, MC_TOKEN_CLOSING_BRACKET, NULL);

  return 0;
}

int mcs_parse_initializer_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *initializer_expression;
  mcs_construct_syntax_node(ps, MC_SYNTAX_INITIALIZER_EXPRESSION, NULL, parent, &initializer_expression);
  if (additional_destination) {
    *additional_destination = initializer_expression;
  }

  mcs_parse_through_token(ps, initializer_expression, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, initializer_expression);

  while (1) {
    mc_token_type token_type;
    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }

    if (initializer_expression->initializer_expression.list->count > 0) {
      mcs_parse_through_token(ps, initializer_expression, MC_TOKEN_COMMA, NULL);
      mcs_parse_through_supernumerary_tokens(ps, initializer_expression);
    }
    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }

    mc_syntax_node *item;
    mcs_parse_expression(ps, initializer_expression, &item);
    mcs_parse_through_supernumerary_tokens(ps, initializer_expression);

    append_to_collection((void ***)&initializer_expression->initializer_expression.list->items,
                         &initializer_expression->initializer_expression.list->alloc,
                         &initializer_expression->initializer_expression.list->count, item);
  }

  mcs_parse_through_token(ps, initializer_expression, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);

  return 0;
}

int mcs_parse_expression_beginning_with_bracket(parsing_state *ps, mc_syntax_node *parent,
                                                mc_syntax_node **additional_destination)
{
  // Determine expression type
  // If cast expression, determine if function pointer declaration
  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type != MC_TOKEN_OPENING_BRACKET) {
    MCerror(1527, "Flow format");
  }

  mcs_peek_token_type(ps, false, 1, &token_type);
  switch (token_type) {
  case MC_TOKEN_AMPERSAND_CHARACTER:
  case MC_TOKEN_OPENING_BRACKET:
  case MC_TOKEN_LOGICAL_NOT_OPERATOR:
  case MC_TOKEN_STAR_CHARACTER:
  case MC_TOKEN_NUMERIC_LITERAL:
  case MC_TOKEN_CHAR_LITERAL: {
    mcs_parse_parenthesized_expression(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  case MC_TOKEN_STRUCT_KEYWORD:
  case MC_TOKEN_CONST_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_SHORT_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_VOID_KEYWORD:
  case MC_TOKEN_SIGNED_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD: {
    int peek_ahead = 1;

    bool is_modifier = true;
    while (is_modifier) {
      switch (token_type) {
      case MC_TOKEN_CONST_KEYWORD:
      case MC_TOKEN_UNSIGNED_KEYWORD:
      case MC_TOKEN_SIGNED_KEYWORD: {
        ++peek_ahead;
        mcs_peek_token_type(ps, false, peek_ahead, &token_type);
        continue;
      }
      case MC_TOKEN_INT_KEYWORD:
      case MC_TOKEN_CHAR_KEYWORD:
      case MC_TOKEN_LONG_KEYWORD:
      case MC_TOKEN_FLOAT_KEYWORD:
      case MC_TOKEN_VOID_KEYWORD:
        ++peek_ahead;
        break;
      default:
        print_parse_error(ps->code, ps->index, "parse_bracket", "is_fptr");
        MCerror(2648, "TODO:%s", get_mc_token_type_name(token_type));
      }

      // While-is-modifier
      break;
    }

    mcs_peek_token_type(ps, false, peek_ahead, &token_type);
    switch (token_type) {
    case MC_TOKEN_STAR_CHARACTER: {
      do {
        ++peek_ahead;
        mcs_peek_token_type(ps, false, peek_ahead, &token_type);
      } while (token_type == MC_TOKEN_STAR_CHARACTER);

      switch (token_type) {
      case MC_TOKEN_CLOSING_BRACKET: {
        mcs_parse_cast_expression(ps, parent, false, additional_destination);
      } break;
      default:
        // print_syntax_node(cast_expression, 0);
        print_parse_error(ps->code, ps->index, "mcs_parse_cast_expression", "is_fptr");
        MCerror(2652, "TODO:%s", get_mc_token_type_name(token_type));
        break;
      }
    } break;
    case MC_TOKEN_CLOSING_BRACKET:
      mcs_parse_cast_expression(ps, parent, false, additional_destination);
      break;
    case MC_TOKEN_OPENING_BRACKET: {
      // Could be a method or function pointer search further
      mcs_peek_token_type(ps, false, peek_ahead + 1, &token_type);
      switch (token_type) {
      case MC_TOKEN_STAR_CHARACTER: {
        do {
          ++peek_ahead;
          mcs_peek_token_type(ps, false, peek_ahead, &token_type);
        } while (token_type == MC_TOKEN_STAR_CHARACTER);

        switch (token_type) {
        case MC_TOKEN_CLOSING_BRACKET: {
          mcs_parse_cast_expression(ps, parent, true, additional_destination);
        } break;
        default:
          // print_syntax_node(cast_expression, 0);
          print_parse_error(ps->code, ps->index, "mcs_parse_cast_expression", "is_fptr");
          MCerror(2701, "TODO:\"type [*](*\">%s(%i)", get_mc_token_type_name(token_type), peek_ahead);
          break;
        }
      } break;
      default:
        print_parse_error(ps->code, ps->index, "parse_bracket", "is_fptr");
        MCerror(2707, "TODO:\"type [*](\">%s(%i)", get_mc_token_type_name(token_type), peek_ahead + 1);
        break;
      }
    } break;
    default:
      // print_syntax_node(cast_expression, 0);
      print_parse_error(ps->code, ps->index, "parse_bracket", "is_fptr");
      MCerror(2663, "TODO:%s(%i)", get_mc_token_type_name(token_type), peek_ahead);
      break;
    }
  } break;
  case MC_TOKEN_IDENTIFIER: {
    mcs_peek_token_type(ps, false, 2, &token_type);
    switch (token_type) {
    case MC_TOKEN_CLOSING_BRACKET: {
      // See whats after
      mcs_peek_token_type(ps, false, 3, &token_type);
      switch (token_type) {
      case MC_TOKEN_NUMERIC_LITERAL:
      case MC_TOKEN_OPENING_BRACKET:
      case MC_TOKEN_CURLY_OPENING_BRACKET:
      case MC_TOKEN_IDENTIFIER: {
        // Cast
        mcs_parse_cast_expression(ps, parent, false, additional_destination);
      } break;
      case MC_TOKEN_CLOSING_BRACKET: // TODO -- is ((int)) something a cast still
      case MC_TOKEN_SEMI_COLON: {
        mcs_parse_parenthesized_expression(ps, parent, additional_destination);
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(1648, "MCS:MC_TOKEN_IDENTIFIER>CLOSING_BRACKET>Unsupported-token:%s",
                get_mc_token_type_name(token_type));
      }
      }
    } break;
    case MC_TOKEN_OPENING_BRACKET:
    case MC_TOKEN_SQUARE_OPENING_BRACKET:
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT:
    case MC_TOKEN_PLUS_OPERATOR:
    case MC_TOKEN_SUBTRACT_OPERATOR:
    case MC_TOKEN_MODULO_OPERATOR:
    case MC_TOKEN_DIVIDE_OPERATOR:
    case MC_TOKEN_AMPERSAND_CHARACTER:
    case MC_TOKEN_EQUALITY_OPERATOR:
    case MC_TOKEN_INEQUALITY_OPERATOR:
    case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR:
    case MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR:
    case MC_TOKEN_LOGICAL_AND_OPERATOR:
    case MC_TOKEN_LOGICAL_OR_OPERATOR:
    case MC_TOKEN_BITWISE_OR_OPERATOR:
    case MC_TOKEN_BITWISE_XOR_OPERATOR:
    case MC_TOKEN_ASSIGNMENT_OPERATOR:
    case MC_TOKEN_ARROW_OPENING_BRACKET:
    case MC_TOKEN_ARROW_CLOSING_BRACKET: {
      mcs_parse_parenthesized_expression(ps, parent, additional_destination);
    } break;
    case MC_TOKEN_STAR_CHARACTER: {
      int peek_ahead = 2;
      do {
        ++peek_ahead;
        mcs_peek_token_type(ps, false, peek_ahead, &token_type);
      } while (token_type == MC_TOKEN_STAR_CHARACTER);
      switch (token_type) {
      case MC_TOKEN_CLOSING_BRACKET: {
        // Cast
        mcs_parse_cast_expression(ps, parent, false, additional_destination);
        // printf("cast-expression:\n");
        // print_syntax_node(*additional_destination, 0);
      } break;
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_NUMERIC_LITERAL: {
        mcs_parse_parenthesized_expression(ps, parent, additional_destination);
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(1556, "MCS:MC_TOKEN_IDENTIFIER>*STAR-SEQUENCE*>Unsupported-token:%s",
                get_mc_token_type_name(token_type));
      }
      }
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(1563, "MCS:MC_TOKEN_IDENTIFIER>NotYetSupported-token:%s", get_mc_token_type_name(token_type));
    }
    }
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1569, "MCS:Unsupported-token:%s", get_mc_token_type_name(token_type));
  }
  }

  return 0;
}

int mcs_parse_local_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *local_declaration;
  mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_DECLARATION, NULL, parent, &local_declaration);
  if (additional_destination) {
    *additional_destination = local_declaration;
  }

  // Type

  mcs_parse_type_identifier(ps, local_declaration, &local_declaration->local_variable_declaration.type_identifier);
  mcs_parse_through_supernumerary_tokens(ps, local_declaration);

  // Declarators
  while (1) {
    mc_token_type token0;
    mcs_peek_token_type(ps, false, 0, &token0);

    // Comma
    if (local_declaration->local_variable_declaration.declarators->count) {
      mcs_parse_through_token(ps, local_declaration, MC_TOKEN_COMMA, NULL);
      mcs_parse_through_supernumerary_tokens(ps, local_declaration);
      mcs_peek_token_type(ps, false, 0, &token0);
    }

    // Expectation
    // if (token0 != MC_TOKEN_IDENTIFIER && token0 != MC_TOKEN_STAR_CHARACTER) {
    //   print_parse_error(ps->code, ps->index, "see-below", "");
    //   MCerror(2462, "Expected Local Variable Declarator. was:%s", get_mc_token_type_name(token0));
    // }

    mc_syntax_node *declarator;
    mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR, NULL, local_declaration, &declarator);
    append_to_collection((void ***)&local_declaration->local_variable_declaration.declarators->items,
                         &local_declaration->local_variable_declaration.declarators->alloc,
                         &local_declaration->local_variable_declaration.declarators->count, declarator);

    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, declarator, &declarator->local_variable_declarator.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, declarator);
    }
    else {
      declarator->local_variable_declarator.type_dereference = NULL;
    }

    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_OPENING_BRACKET) {
      mcs_parse_function_pointer_declarator(ps, declarator, false,
                                            &declarator->local_variable_declarator.function_pointer);
      declarator->local_variable_declarator.variable_name = NULL;
    }
    else {
      mcs_parse_through_token(ps, declarator, MC_TOKEN_IDENTIFIER,
                              &declarator->local_variable_declarator.variable_name);
      declarator->local_variable_declarator.function_pointer = NULL;
    }

    mcs_peek_token_type(ps, false, 0, &token0);
    bool end_loop = false;
    switch (token0) {
    case MC_TOKEN_SEMI_COLON: {
      end_loop = true;
    } break;
    case MC_TOKEN_COMMA: {
      // Do Nothing more
    } break;
    case MC_TOKEN_ASSIGNMENT_OPERATOR: {
      mcs_parse_through_supernumerary_tokens(ps, declarator);

      mc_syntax_node *assignment_initializer;
      mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER, NULL, declarator,
                                &assignment_initializer);
      declarator->local_variable_declarator.initializer = assignment_initializer;

      mcs_parse_through_token(ps, assignment_initializer, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL);
      mcs_parse_through_supernumerary_tokens(ps, assignment_initializer);

      mcs_parse_expression(ps, assignment_initializer,
                           &assignment_initializer->local_variable_assignment_initializer.value_expression);
    } break;
    case MC_TOKEN_SQUARE_OPENING_BRACKET: {
      mcs_parse_through_supernumerary_tokens(ps, declarator);

      mc_syntax_node *array_initializer;
      mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER, NULL, declarator, &array_initializer);
      declarator->local_variable_declarator.initializer = array_initializer;

      // [  size  ]
      mcs_parse_through_token(ps, array_initializer, MC_TOKEN_SQUARE_OPENING_BRACKET, NULL);
      mcs_parse_through_supernumerary_tokens(ps, array_initializer);

      mcs_peek_token_type(ps, false, 0, &token0);
      bool unknown_size_array_declaration = (token0 == MC_TOKEN_SQUARE_CLOSING_BRACKET);
      if (!unknown_size_array_declaration) {
        mcs_parse_expression(ps, array_initializer,
                             &array_initializer->local_variable_array_initializer.size_expression);
        mcs_parse_through_supernumerary_tokens(ps, array_initializer);
      }
      else
        array_initializer->local_variable_array_initializer.size_expression = NULL;

      mcs_parse_through_token(ps, array_initializer, MC_TOKEN_SQUARE_CLOSING_BRACKET, NULL);

      mcs_peek_token_type(ps, false, 0, &token0);
      switch (token0) {
      case MC_TOKEN_SEMI_COLON:
      case MC_TOKEN_COMMA:
        break;
      case MC_TOKEN_ASSIGNMENT_OPERATOR: {
        mcs_parse_through_supernumerary_tokens(ps, array_initializer);
        mcs_parse_through_token(ps, array_initializer, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL);
        mcs_parse_through_supernumerary_tokens(ps, array_initializer);

        mcs_parse_initializer_expression(ps, array_initializer,
                                         &array_initializer->local_variable_array_initializer.assignment_expression);
      } break;
      default:
        MCerror(2603, "Token Unsupported:%s", get_mc_token_type_name(token0));
      }
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(2487, "MCS:ERR-token:%s", get_mc_token_type_name(token0));
    }
    }

    if (!end_loop) {
      mcs_peek_token_type(ps, false, 0, &token0);

      if (token0 != MC_TOKEN_SEMI_COLON) {
        continue;
      }
    }

    break;
  }

  return 0;
}

int _mcs_parse_expression(parsing_state *ps, int allowable_precedence, mc_syntax_node *parent,
                          mc_syntax_node **additional_destination)
{
  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token_after;
    mcs_peek_token_type(ps, false, 1, &token_after);
    switch (token_after) {
    // case MC_TOKEN_CLOSING_BRACKET:
    // case MC_TOKEN_OPENING_BRACKET:
    // case MC_TOKEN_ASSIGNMENT_OPERATOR:
    // case MC_TOKEN_SQUARE_OPENING_BRACKET:
    // case MC_TOKEN_POINTER_OPERATOR:
    // case MC_TOKEN_COMMA:
    // case MC_TOKEN_ARROW_CLOSING_BRACKET:
    // case MC_TOKEN_LOGICAL_AND_OPERATOR:
    // case MC_TOKEN_SEMI_COLON:
    // case MC_TOKEN_PLUS_OPERATOR:
    //   break;
    case MC_TOKEN_IDENTIFIER: {
      // Declaration expression
      mcs_parse_local_declaration(ps, parent, additional_destination);

      mcs_peek_token_type(ps, false, 0, &token0);
      if (token0 != MC_TOKEN_SEMI_COLON) {
        MCerror(2632, "TODO");
      }

      return 0;
    }
    default: {
      // print_parse_error(ps->code, ps->index, "_mcs_parse_expression", "see-below");
      // MCerror(2622, "Unknown-Token-Path:IDENTIFIER>%s", get_mc_token_type_name(token_after));
    } break;
    }
  } break;
  case MC_TOKEN_UNSIGNED_KEYWORD:
  case MC_TOKEN_INT_KEYWORD: {
    // Declaration expression
    mcs_parse_local_declaration(ps, parent, additional_destination);

    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 != MC_TOKEN_SEMI_COLON) {
      MCerror(1753, "TODO");
    }

    return 0;
  }
  default:
    break;
  }

  mc_syntax_node *left;

  // Determine left
  switch (token0) {
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_CHAR_LITERAL:
  case MC_TOKEN_NUMERIC_LITERAL: {
    mcs_parse_through_token(ps, NULL, token0, &left);
  } break;
  case MC_TOKEN_OPENING_BRACKET: {
    // printf("mpe-open[2]\n");
    mcs_parse_expression_beginning_with_bracket(ps, NULL, &left);
  } break;
  case MC_TOKEN_STRING_LITERAL: {
    mcs_construct_syntax_node(ps, MC_SYNTAX_STRING_LITERAL_EXPRESSION, NULL, NULL, &left);

    mcs_parse_through_token(ps, left, MC_TOKEN_STRING_LITERAL, NULL);

    while (1) {
      mcs_peek_token_type(ps, false, 0, &token0);
      if (token0 != MC_TOKEN_STRING_LITERAL) {
        break;
      }

      mcs_parse_through_supernumerary_tokens(ps, left);
      mcs_parse_through_token(ps, left, MC_TOKEN_STRING_LITERAL, NULL);
    }
  } break;
  case MC_TOKEN_STAR_CHARACTER: {
    const int CASE_PRECEDENCE = 3;

    mc_token_type token1;
    mcs_peek_token_type(ps, false, 1, &token1);
    // if (token1 != MC_TOKEN_IDENTIFIER) {
    //   MCerror(1979, "TODO ? %s", get_mc_syntax_token_type_name((mc_syntax_node_type)token1));
    // }

    mcs_construct_syntax_node(ps, MC_SYNTAX_DEREFERENCE_EXPRESSION, NULL, NULL, &left);

    mcs_parse_dereference_sequence(ps, left, &left->dereference_expression.deref_sequence);

    mcs_parse_through_supernumerary_tokens(ps, left);

    _mcs_parse_expression(ps, CASE_PRECEDENCE, left, &left->dereference_expression.unary_expression);
  } break;
  case MC_TOKEN_CURLY_OPENING_BRACKET: {
    // Initializer
    mcs_parse_initializer_expression(ps, parent, additional_destination);
    return 0;
  }
  case MC_TOKEN_DECREMENT_OPERATOR:
  case MC_TOKEN_INCREMENT_OPERATOR: {
    mc_syntax_node *expression;
    mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, NULL, parent, &expression);
    if (additional_destination) {
      *additional_destination = expression;
    }
    expression->fixrement_expression.is_postfix = false;

    mcs_parse_through_token(ps, expression, token0, &expression->fixrement_expression.fix_operator);
    mcs_parse_through_supernumerary_tokens(ps, expression);
    _mcs_parse_expression(ps, 3, expression, &expression->fixrement_expression.primary);
    return 0;
  }
  case MC_TOKEN_BITWISE_NOT_OPERATOR:
  case MC_TOKEN_LOGICAL_NOT_OPERATOR:
  case MC_TOKEN_SUBTRACT_OPERATOR:
  case MC_TOKEN_PLUS_OPERATOR:
  case MC_TOKEN_AMPERSAND_CHARACTER: {
    const int CASE_PRECEDENCE = 3;
    mcs_construct_syntax_node(ps, MC_SYNTAX_PREPENDED_UNARY_EXPRESSION, NULL, NULL, &left);

    mcs_parse_through_token(ps, left, token0, &left->prepended_unary.prepend_operator);
    mcs_parse_through_supernumerary_tokens(ps, left);
    _mcs_parse_expression(ps, CASE_PRECEDENCE, left, &left->prepended_unary.unary_expression);

  } break;
  case MC_TOKEN_SIZEOF_KEYWORD: {
    // Cast
    const int CASE_PRECEDENCE = 3;
    mcs_construct_syntax_node(ps, MC_SYNTAX_SIZEOF_EXPRESSION, NULL, NULL, &left);
    if (additional_destination) {
      *additional_destination = left;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_SIZEOF_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_type_identifier(ps, left, &left->sizeof_expression.type_identifier);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, left, &left->sizeof_expression.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, left);
    }
    else {
      left->sizeof_expression.type_dereference = NULL;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_CLOSING_BRACKET, NULL);

    // printf("sizeof: %i\n", left->children->count);
    // print_syntax_node(left, 2);
  } break;
  case MC_TOKEN_OFFSETOF_KEYWORD: {
    // Cast
    const int CASE_PRECEDENCE = 3;
    mcs_construct_syntax_node(ps, MC_SYNTAX_OFFSETOF_EXPRESSION, NULL, NULL, &left);
    if (additional_destination) {
      *additional_destination = left;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_OFFSETOF_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_type_identifier(ps, left, &left->offsetof_expression.type_identifier);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, left, &left->offsetof_expression.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, left);
    }
    else {
      left->offsetof_expression.type_dereference = NULL;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_COMMA, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_IDENTIFIER, &left->offsetof_expression.field_identity);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_CLOSING_BRACKET, NULL);
  } break;
  case MC_TOKEN_VA_ARG_WORD: {
    // Cast
    const int CASE_PRECEDENCE = 2;
    mcs_construct_syntax_node(ps, MC_SYNTAX_VA_ARG_EXPRESSION, NULL, NULL, &left);
    if (additional_destination) {
      *additional_destination = left;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_VA_ARG_WORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_IDENTIFIER, &left->va_arg_expression.list_identity);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_through_token(ps, left, MC_TOKEN_COMMA, NULL);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mcs_parse_type_identifier(ps, left, &left->va_arg_expression.type_identifier);
    mcs_parse_through_supernumerary_tokens(ps, left);

    mc_token_type token1;
    mcs_peek_token_type(ps, false, 0, &token1);
    if (token1 == MC_TOKEN_STAR_CHARACTER) {
      mcs_parse_dereference_sequence(ps, left, &left->va_arg_expression.type_dereference);
      mcs_parse_through_supernumerary_tokens(ps, left);
    }
    else {
      left->va_arg_expression.type_dereference = NULL;
    }

    mcs_parse_through_token(ps, left, MC_TOKEN_CLOSING_BRACKET, NULL);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1798, "MCS:Unsupported-token:%s allowable_precedence:%i", get_mc_token_type_name(token0),
            allowable_precedence);
  }
  }

  // Middle / Postfix
  while (1) {
    mcs_peek_token_type(ps, false, 0, &token0);
    switch (token0) {
    case MC_TOKEN_DECREMENT_OPERATOR:
    case MC_TOKEN_INCREMENT_OPERATOR: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, NULL, NULL, &expression);
      expression->fixrement_expression.is_postfix = true;

      mcs_add_syntax_node_to_parent(expression, left);
      expression->fixrement_expression.primary = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->fixrement_expression.fix_operator);

      left = expression;
    } break;
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence < CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_MEMBER_ACCESS_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->member_access_expression.primary = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->member_access_expression.access_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->member_access_expression.identifier);

      left = expression;
    } break;
    case MC_TOKEN_SQUARE_OPENING_BRACKET: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->element_access_expression.primary = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, NULL);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_expression(ps, expression, &expression->element_access_expression.access_expression);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, MC_TOKEN_SQUARE_CLOSING_BRACKET, NULL);

      left = expression;
    } break;
    case MC_TOKEN_OPENING_BRACKET: {
      // printf("mpe-open[2]\n");
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_INVOCATION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->invocation.function_identity = left;
      mcs_parse_through_supernumerary_tokens(ps, expression);

      mcs_parse_through_token(ps, expression, MC_TOKEN_OPENING_BRACKET, NULL);
      mcs_parse_through_supernumerary_tokens(ps, expression);

      while (1) {
        mcs_peek_token_type(ps, false, 0, &token0);

        if (token0 == MC_TOKEN_CLOSING_BRACKET) {
          break;
        }

        if (expression->invocation.arguments->count) {
          mcs_parse_through_token(ps, expression, MC_TOKEN_COMMA, NULL);
          mcs_parse_through_supernumerary_tokens(ps, expression);
        }

        mc_syntax_node *argument;
        mcs_parse_expression(ps, expression, &argument);
        mcs_parse_through_supernumerary_tokens(ps, expression);

        append_to_collection((void ***)&expression->invocation.arguments->items,
                             &expression->invocation.arguments->alloc, &expression->invocation.arguments->count,
                             argument);
      }
      mcs_parse_through_token(ps, expression, MC_TOKEN_CLOSING_BRACKET, NULL);

      left = expression;
    } break;
    case MC_TOKEN_MODULO_OPERATOR:
    case MC_TOKEN_STAR_CHARACTER:
    case MC_TOKEN_DIVIDE_OPERATOR: {
      const int CASE_PRECEDENCE = 5;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_OPERATIONAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->operational_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->operational_expression.operational_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->operational_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_AMPERSAND_CHARACTER:
    case MC_TOKEN_BITWISE_OR_OPERATOR: {
      int case_precedence;
      switch (token0) {
      case MC_TOKEN_AMPERSAND_CHARACTER: // TODO -- convert this to name appropriate
        case_precedence = 11;
        break;
      case MC_TOKEN_BITWISE_XOR_OPERATOR:
        case_precedence = 12;
        break;
      case MC_TOKEN_BITWISE_OR_OPERATOR:
        case_precedence = 13;
        break;
      default:
        MCerror(2903, "Unsupported %s", get_mc_token_type_name(token0));
      }
      if (allowable_precedence <= case_precedence) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_BITWISE_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->bitwise_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->bitwise_expression.bitwise_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, case_precedence, expression, &expression->bitwise_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_SUBTRACT_OPERATOR:
    case MC_TOKEN_PLUS_OPERATOR: {
      const int CASE_PRECEDENCE = 6;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_OPERATIONAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->operational_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->operational_expression.operational_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->operational_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR:
    case MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR: {
      const int CASE_PRECEDENCE = 7;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_BITWISE_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->bitwise_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->bitwise_expression.bitwise_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->bitwise_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_ARROW_OPENING_BRACKET:
    case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_ARROW_CLOSING_BRACKET:
    case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR: {
      const int CASE_PRECEDENCE = 9;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_RELATIONAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->relational_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->relational_expression.relational_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->relational_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_LOGICAL_AND_OPERATOR: {
      const int CASE_PRECEDENCE = 14;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_LOGICAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->logical_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->logical_expression.logical_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->logical_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_LOGICAL_OR_OPERATOR: {
      const int CASE_PRECEDENCE = 15;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // printf("||:allowable_precedence:%i\n", allowable_precedence);
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_LOGICAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->logical_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->logical_expression.logical_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->logical_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_EQUALITY_OPERATOR:
    case MC_TOKEN_INEQUALITY_OPERATOR: {
      const int CASE_PRECEDENCE = 10;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_LOGICAL_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->logical_expression.left = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->logical_expression.logical_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->logical_expression.right);

      left = expression;
    } break;
    case MC_TOKEN_BINARY_AND_ASSIGNMENT_OPERATOR:
    case MC_TOKEN_BINARY_OR_ASSIGNMENT_OPERATOR:
    case MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_DIVIDE_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_MODULO_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_ASSIGNMENT_OPERATOR: {
      const int CASE_PRECEDENCE = 16;
      if (allowable_precedence < CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_ASSIGNMENT_EXPRESSION, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->assignment_expression.variable = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, token0, &expression->assignment_expression.assignment_operator);

      mcs_parse_through_supernumerary_tokens(ps, expression);

      _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->assignment_expression.value_expression);

      left = expression;
    } break;
    case MC_TOKEN_TERNARY_OPERATOR: {
      const int CASE_PRECEDENCE = 16;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        mcs_add_syntax_node_to_parent(parent, left);
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }
      mc_syntax_node *expression = NULL;
      mcs_construct_syntax_node(ps, MC_SYNTAX_TERNARY_CONDITIONAL, NULL, NULL, &expression);

      mcs_add_syntax_node_to_parent(expression, left);
      expression->ternary_conditional.condition = left;

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, MC_TOKEN_TERNARY_OPERATOR, NULL);
      mcs_parse_through_supernumerary_tokens(ps, expression);

      _mcs_parse_expression(ps, 17, expression, &expression->ternary_conditional.true_expression);

      mcs_parse_through_supernumerary_tokens(ps, expression);
      mcs_parse_through_token(ps, expression, MC_TOKEN_COLON, NULL);
      mcs_parse_through_supernumerary_tokens(ps, expression);

      _mcs_parse_expression(ps, 16, expression, &expression->ternary_conditional.false_expression);

      left = expression;
    } break;
    case MC_TOKEN_SEMI_COLON:
    case MC_TOKEN_COLON:
    case MC_TOKEN_COMMA:
    case MC_TOKEN_CLOSING_BRACKET:
    case MC_TOKEN_CURLY_CLOSING_BRACKET:
    case MC_TOKEN_SQUARE_CLOSING_BRACKET: {
      // Left is it
      mcs_add_syntax_node_to_parent(parent, left);
      if (additional_destination) {
        *additional_destination = left;
      }
      return 0;
    }
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(1813, "MCS:Unsupported-token:%s allowable_precedence:%i", get_mc_token_type_name(token0),
              allowable_precedence);
    }
    }
  }
  MCerror(1789, "Invalid Flow");
}

int mcs_parse_expression_conditional(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  _mcs_parse_expression(ps, 16, parent, additional_destination);

  // printf("Conditional-Expression!:\n");
  // print_syntax_node(*additional_destination, 1);
  return 0;
}

int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  _mcs_parse_expression(ps, 17, parent, additional_destination);

  // printf("Expression!:\n");
  if (!*additional_destination) {
    MCerror(2258, "expression was null");
  }
  // print_syntax_node(*additional_destination, 1);
  // _mcs_obtain_precedence_information(ps, 0, -1, &lpo_token_type, &lpo_peek_loc, &lpo_peek_loc2);

  // mc_syntax_node *expression;
  // mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION, parent, &expression);
  // if (additional_destination) {
  //   *additional_destination = expression;
  // }

  // mcs_parse_expression_conditional(ps, parent, additional_destination);

  return 0;
}

int mcs_parse_return_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *return_statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_RETURN_STATEMENT, NULL, parent, &return_statement);
  if (additional_destination) {
    *additional_destination = return_statement;
  }

  mcs_parse_through_token(ps, return_statement, MC_TOKEN_RETURN_KEYWORD, NULL);

  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 != MC_TOKEN_SEMI_COLON) {
    mcs_parse_through_supernumerary_tokens(ps, return_statement);
    mcs_parse_expression(ps, return_statement, &return_statement->return_statement.expression);
  }
  else {
    return_statement->return_statement.expression = NULL;
  }

  mcs_parse_through_supernumerary_tokens(ps, return_statement);
  mcs_parse_through_token(ps, return_statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_local_declaration_statement(parsing_state *ps, mc_syntax_node *parent,
                                          mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_DECLARATION_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_local_declaration(ps, statement, &statement->declaration_statement.declaration);

  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  // printf("local_declaration:\n");
  // print_syntax_node(local_declaration, 1);

  return 0;
}

int mcs_parse_goto_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_GOTO_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_GOTO_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, &statement->goto_statement.label);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_label_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_LABEL_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, &statement->label_statement.label);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_COLON, NULL);

  return 0;
}

int mcs_parse_for_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FOR_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_FOR_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  // Initialization
  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_UNSIGNED_KEYWORD:
  case MC_TOKEN_INT_KEYWORD: {
    mcs_parse_expression(ps, statement, &statement->for_statement.initialization);
  } break;
  case MC_TOKEN_SEMI_COLON: {
    statement->for_statement.initialization = NULL;
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(873, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  // Conditional
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_LOGICAL_NOT_OPERATOR:
  case MC_TOKEN_IDENTIFIER: {
    mcs_parse_expression_conditional(ps, statement, &statement->for_statement.conditional);
  } break;
  case MC_TOKEN_SEMI_COLON: {
    statement->for_statement.conditional = NULL;
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(882, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  // UpdateFix Expression
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_CLOSING_BRACKET: {
  } break;
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_INCREMENT_OPERATOR:
  case MC_TOKEN_DECREMENT_OPERATOR: {
    mcs_parse_expression(ps, statement, &statement->for_statement.fix_expression);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(891, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_peek_token_type(ps, false, 0, &token0);
  // if (token0 != MC_TOKEN_CURLY_OPENING_BRACKET) {
  // print_parse_error(ps->code, ps->index, "mcs_parse_for_statement", "");
  mcs_parse_statement(ps, statement, &statement->for_statement.loop_statement);
  // print_syntax_node(statement->for_statement.loop_statement, 0);
  // }
  // else {
  //   mcs_parse_code_block(ps, statement, &statement->for_statement.loop_statement);
  // }

  return 0;
}

int mcs_parse_while_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_WHILE_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 == MC_TOKEN_DO_KEYWORD) {
    // do { } while (condition);
    statement->while_statement.do_first = true;

    mcs_parse_through_token(ps, statement, MC_TOKEN_DO_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);

    mcs_peek_token_type(ps, false, 0, &token0);
    // if (token0 != MC_TOKEN_CURLY_OPENING_BRACKET) {
    mcs_parse_statement(ps, statement, &statement->while_statement.do_statement);
    //   MCerror(3119, "TODO single statement");
    // }
    // mcs_parse_code_block(ps, statement, &statement->while_statement.do_statement);
    mcs_parse_through_supernumerary_tokens(ps, statement);

    mcs_parse_through_token(ps, statement, MC_TOKEN_WHILE_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);

    mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);
    mcs_parse_expression_conditional(ps, statement, &statement->while_statement.conditional);
    mcs_parse_through_supernumerary_tokens(ps, statement);
    mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);

    mcs_parse_through_supernumerary_tokens(ps, statement);
    mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);
    return 0;
  }

  // while (condition) { }
  mcs_parse_through_token(ps, statement, MC_TOKEN_WHILE_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_expression_conditional(ps, statement, &statement->while_statement.conditional);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);

  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_statement(ps, statement, &statement->while_statement.do_statement);

  return 0;
}

int mcs_parse_switch_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *switch_statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_STATEMENT, NULL, parent, &switch_statement);
  if (additional_destination) {
    *additional_destination = switch_statement;
  }

  // printf("mpss-0\n");
  mcs_parse_through_token(ps, switch_statement, MC_TOKEN_SWITCH_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, switch_statement);
  mcs_parse_through_token(ps, switch_statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, switch_statement);
  mcs_parse_expression_conditional(ps, switch_statement, &switch_statement->switch_statement.conditional);
  mcs_parse_through_supernumerary_tokens(ps, switch_statement);
  mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, switch_statement);

  // printf("mpss-1\n");
  mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, switch_statement);

  mc_token_type token0;
  mc_syntax_node *switch_section = NULL;
  while (1) {
    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }
    // printf("mpss-2\n");

    if (switch_section == NULL) {
      // Construct section
      mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_SECTION, NULL, switch_statement, &switch_section);
    }

    if (token0 == MC_TOKEN_CASE_KEYWORD) {
      // printf("mpss-3\n");
      mc_syntax_node *case_label;
      mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_CASE_LABEL, NULL, switch_section, &case_label);
      mcs_parse_through_token(ps, case_label, MC_TOKEN_CASE_KEYWORD, NULL);
      mcs_parse_through_supernumerary_tokens(ps, case_label);

      // printf("mpss-3a\n");
      mcs_peek_token_type(ps, false, 0, &token0);
      switch (token0) {
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_CHAR_LITERAL:
      case MC_TOKEN_NUMERIC_LITERAL: {
        mcs_parse_through_token(ps, case_label, token0, &case_label->switch_case_label.constant);
        mcs_parse_through_supernumerary_tokens(ps, case_label);
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(2694, "MCS:>CASE_CONSTANT:ERR-token:%s", get_mc_token_type_name(token0));
      }
      }

      // printf("mpss-3b\n");
      mcs_parse_through_token(ps, case_label, MC_TOKEN_COLON, NULL);

      append_to_collection((void ***)&switch_section->switch_section.labels->items,
                           &switch_section->switch_section.labels->alloc, &switch_section->switch_section.labels->count,
                           case_label);
      // printf("mpss-3c\n");
    }
    else if (token0 == MC_TOKEN_DEFAULT_KEYWORD) {
      // printf("mpss-4\n");
      mc_syntax_node *default_label;
      mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_DEFAULT_LABEL, NULL, switch_section, &default_label);
      mcs_parse_through_token(ps, default_label, MC_TOKEN_DEFAULT_KEYWORD, NULL);
      mcs_parse_through_supernumerary_tokens(ps, default_label);

      mcs_parse_through_token(ps, default_label, MC_TOKEN_COLON, NULL);

      append_to_collection((void ***)&switch_section->switch_section.labels->items,
                           &switch_section->switch_section.labels->alloc, &switch_section->switch_section.labels->count,
                           default_label);
    }
    else {
      // printf("mpss-5\n");
      mcs_parse_statement_list(ps, switch_section, &switch_section->switch_section.statement_list);
      // printf("~switch_section\n");

      append_to_collection((void ***)&switch_statement->switch_statement.sections->items,
                           &switch_statement->switch_statement.sections->alloc,
                           &switch_statement->switch_statement.sections->count, switch_section);
      switch_section = NULL;
      // printf("~switch_section2\n");
    }

    // printf("mpss-6\n");
    if (switch_section) {
      mcs_parse_through_supernumerary_tokens(ps, switch_section);
    }
    else {
      mcs_parse_through_supernumerary_tokens(ps, switch_statement);
    }
    // printf("mpss-7\n");
  }

  mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);
  // printf("mpss-8\n");

  return 0;
}

int mcs_parse_if_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_IF_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_IF_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_expression_conditional(ps, statement, &statement->if_statement.conditional);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);

  mcs_parse_statement(ps, statement, &statement->if_statement.do_statement);

  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 == MC_TOKEN_ELSE_KEYWORD) {
    mcs_parse_through_supernumerary_tokens(ps, statement);
    mcs_parse_through_token(ps, statement, MC_TOKEN_ELSE_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);

    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_IF_KEYWORD) {
      mcs_parse_if_statement(ps, statement, &statement->if_statement.else_continuance);
    }
    else
      mcs_parse_statement(ps, statement, &statement->if_statement.else_continuance);
  }
  else {
    statement->if_statement.else_continuance = NULL;
  }

  return 0;
}

int mcs_parse_va_list_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_VA_LIST_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_VA_LIST_WORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, &statement->va_list_expression.list_identity);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_va_start_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_VA_START_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_VA_START_WORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_COMMA, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_va_end_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_VA_END_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_VA_END_WORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, statement);
  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_expression_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION_STATEMENT, NULL, parent, &statement);
  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_expression(ps, statement, &statement->expression_statement.expression);
  mcs_parse_through_supernumerary_tokens(ps, statement);

  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_simple_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_CONTINUE_KEYWORD: {
    mcs_construct_syntax_node(ps, MC_SYNTAX_CONTINUE_STATEMENT, NULL, parent, &statement);

    mcs_parse_through_token(ps, statement, MC_TOKEN_CONTINUE_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);
  } break;
  case MC_TOKEN_BREAK_KEYWORD: {
    mcs_construct_syntax_node(ps, MC_SYNTAX_BREAK_STATEMENT, NULL, parent, &statement);

    mcs_parse_through_token(ps, statement, MC_TOKEN_BREAK_KEYWORD, NULL);
    mcs_parse_through_supernumerary_tokens(ps, statement);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(2613, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  if (additional_destination) {
    *additional_destination = statement;
  }

  mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_token_type token0;
  mcs_peek_token_type(ps, false, 0, &token0);
  switch (token0) {
  case MC_TOKEN_CURLY_OPENING_BRACKET: {
    mcs_parse_code_block(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_FOR_KEYWORD: {
    mcs_parse_for_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_IF_KEYWORD: {
    mcs_parse_if_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_SWITCH_KEYWORD: {
    mcs_parse_switch_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_DO_KEYWORD:
  case MC_TOKEN_WHILE_KEYWORD: {
    mcs_parse_while_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_BREAK_KEYWORD:
  case MC_TOKEN_CONTINUE_KEYWORD: {
    mcs_parse_simple_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_RETURN_KEYWORD: {
    mcs_parse_return_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_OPENING_BRACKET:
  case MC_TOKEN_STAR_CHARACTER:
  case MC_TOKEN_INCREMENT_OPERATOR:
  case MC_TOKEN_DECREMENT_OPERATOR: {
    mcs_parse_expression_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_VA_LIST_WORD: {
    mcs_parse_va_list_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_VA_START_WORD: {
    mcs_parse_va_start_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_VA_END_WORD: {
    mcs_parse_va_end_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_STRUCT_KEYWORD:
  case MC_TOKEN_CONST_KEYWORD:
  case MC_TOKEN_STATIC_KEYWORD:
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  // case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_SHORT_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_VOID_KEYWORD:
  case MC_TOKEN_SIGNED_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD: {
    // Some Sort of Declarative Statement
    mcs_parse_local_declaration_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_GOTO_KEYWORD: {
    mcs_parse_goto_statement(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token1;
    mcs_peek_token_type(ps, false, 1, &token1);
    // printf("token1:%s\n", get_mc_syntax_token_type_name((mc_syntax_node_type)token1));
    switch (token1) {
    case MC_TOKEN_IDENTIFIER:
    case MC_TOKEN_STAR_CHARACTER: {
      // Some Sort of Declarative Statement
      mcs_parse_local_declaration_statement(ps, parent, additional_destination);
    } break;
    case MC_TOKEN_COLON: {
      mcs_parse_label_statement(ps, parent, additional_destination);
    } break;
    case MC_TOKEN_OPENING_BRACKET:
    case MC_TOKEN_SQUARE_OPENING_BRACKET:
    case MC_TOKEN_ASSIGNMENT_OPERATOR:
    case MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR:
    case MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR:
    case MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_DIVIDE_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_INCREMENT_OPERATOR:
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT: {
      // Some sort of member-access statement
      mcs_parse_expression_statement(ps, parent, additional_destination);
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(1579, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
    }
    }
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1584, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  return 0;
}

int mcs_parse_statement_list(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement_list_node;
  mcs_construct_syntax_node(ps, MC_SYNTAX_STATEMENT_LIST, NULL, parent, &statement_list_node);
  if (additional_destination) {
    *additional_destination = statement_list_node;
  }

  bool loop = true;
  while (loop) {
    // printf("ps->index:%i\n", ps->index);

    mc_token_type token0;
    mcs_peek_token_type(ps, false, 0, &token0);

    switch (token0) {
    case MC_TOKEN_CURLY_CLOSING_BRACKET:
    case MC_TOKEN_CASE_KEYWORD:
    case MC_TOKEN_DEFAULT_KEYWORD:
      loop = false;
      break;
    default:
      break;
    }
    if (!loop) {
      break;
    }

    mcs_parse_through_supernumerary_tokens(ps, statement_list_node);

    mc_syntax_node *statement;
    mcs_parse_statement(ps, statement_list_node, &statement);

    // printf("token0:%s statement:%p\n", get_mc_syntax_token_type_name((mc_syntax_node_type)token0), statement);
    append_to_collection((void ***)&statement_list_node->statement_list.statements->items,
                         &statement_list_node->statement_list.statements->alloc,
                         &statement_list_node->statement_list.statements->count, statement);
  }

  // printf("~mcs_parse_statement_list()\n");
  return 0;
}

int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_code_block()");
  mc_syntax_node *code_block;
  mcs_construct_syntax_node(ps, MC_SYNTAX_CODE_BLOCK, NULL, parent, &code_block);
  if (additional_destination) {
    *additional_destination = code_block;
  }

  // printf("ps->index:%i\n", ps->index);
  mcs_parse_through_token(ps, code_block, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
  // print_parse_error(ps->code, ps->index - 1, "parse-code-block", "opening");
  mcs_parse_through_supernumerary_tokens(ps, code_block);

  mcs_parse_statement_list(ps, code_block, &code_block->code_block.statement_list);
  mcs_parse_through_supernumerary_tokens(ps, code_block);

  mcs_parse_through_token(ps, code_block, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);
  // print_parse_error(ps->code, ps->index - 1, "parse-code-block", "closing");

  // printf("~mcs_parse_code_block()\n");
  return 0;
}

int mcs_parse_function_definition_header(parsing_state *ps, mc_syntax_node *function)
{
  if (function->type != MC_SYNTAX_FUNCTION) {
    MCerror(3184, "Argument Error");
  }

  mc_token_type token0;
  mcs_parse_type_identifier(ps, function, &function->function.return_type_identifier);

  register_midge_error_tag("parse_mc_to_syntax_tree()-2");
  // print_syntax_node(function, 0);
  mcs_parse_through_supernumerary_tokens(ps, function);

  // print_syntax_node(function, 0);

  mcs_peek_token_type(ps, false, 0, &token0);
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    mcs_parse_dereference_sequence(ps, function, &function->function.return_type_dereference);
    mcs_parse_through_supernumerary_tokens(ps, function);
  }
  else {
    function->function.return_type_dereference = NULL;
  }

  // print_syntax_node(function, 0);

  mcs_parse_through_token(ps, function, MC_TOKEN_IDENTIFIER, &function->function.name);
  mcs_parse_through_supernumerary_tokens(ps, function);

  mcs_parse_through_token(ps, function, MC_TOKEN_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, function);

  while (1) {
    mcs_peek_token_type(ps, false, 0, &token0);
    if (token0 == MC_TOKEN_CLOSING_BRACKET) {
      break;
    }

    // Comma
    if (function->function.parameters->count) {
      mcs_parse_through_token(ps, function, MC_TOKEN_COMMA, NULL);
      mcs_parse_through_supernumerary_tokens(ps, function);
    }

    // Parse the parameter
    mc_syntax_node *parameter_decl;
    mcs_parse_parameter_declaration(ps, false, function, &parameter_decl);
    mcs_parse_through_supernumerary_tokens(ps, function);
    // printf("parameter_decl:%p\n", parameter_decl);
    // print_syntax_node(parameter_decl, 0);
    append_to_collection((void ***)&function->function.parameters->items, &function->function.parameters->alloc,
                         &function->function.parameters->count, parameter_decl);
  }

  mcs_parse_through_token(ps, function, MC_TOKEN_CLOSING_BRACKET, NULL);

  return 0;
}

int mcs_parse_pp_ifndef(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_token_type token_type;

  mc_syntax_node *ifndef_directive;
  mcs_construct_syntax_node(ps, MC_SYNTAX_PP_DIRECTIVE_IFNDEF, NULL, parent, &ifndef_directive);
  if (additional_destination) {
    *additional_destination = ifndef_directive;
  }

  mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_PP_KEYWORD_IFNDEF, NULL);

  mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
  mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_IDENTIFIER, &ifndef_directive->preprocess_ifndef.identifier);
  mcs_peek_token_type(ps, true, 0, &token_type);
  if (token_type == MC_TOKEN_SPACE_SEQUENCE)
    mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
  mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_NEW_LINE, NULL);

  while (1) {
    mcs_peek_token_type(ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_NULL_CHARACTER) {
      MCerror(3831, "TODO");
    }
    if (token_type == MC_TOKEN_PP_KEYWORD_ENDIF) {
      break;
    }
    mc_syntax_node *group_option;
    mcs_parse_root_statement(ps, ifndef_directive, &group_option);

    append_to_collection((void ***)&ifndef_directive->preprocess_ifndef.groupopt->items,
                         &ifndef_directive->preprocess_ifndef.groupopt->alloc,
                         &ifndef_directive->preprocess_ifndef.groupopt->count, group_option);

    mcs_parse_through_supernumerary_tokens(ps, ifndef_directive);
  }

  mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_PP_KEYWORD_ENDIF, NULL);
  while (1) {
    mcs_peek_token_type(ps, true, 0, &token_type);
    switch (token_type) {
    case MC_TOKEN_SPACE_SEQUENCE:
    case MC_TOKEN_LINE_COMMENT:
    case MC_TOKEN_MULTI_LINE_COMMENT:
      mcs_parse_through_token(ps, ifndef_directive, token_type, NULL);
      continue;
    case MC_TOKEN_NEW_LINE:
      mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_NEW_LINE, NULL);
      break;
    case MC_TOKEN_NULL_CHARACTER:
      break;
    default:
      MCerror(3857, "TODO:%s", get_mc_token_type_name(token_type));
    }
    break;
  }

  return 0;
}

int mcs_parse_pp_ifdef(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_token_type token_type;

  mc_syntax_node *ifdef_directive;
  mcs_construct_syntax_node(ps, MC_SYNTAX_PP_DIRECTIVE_IFDEF, NULL, parent, &ifdef_directive);
  if (additional_destination) {
    *additional_destination = ifdef_directive;
  }

  mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_PP_KEYWORD_IFDEF, NULL);

  mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
  mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_IDENTIFIER, &ifdef_directive->preprocess_ifndef.identifier);
  mcs_peek_token_type(ps, true, 0, &token_type);
  if (token_type == MC_TOKEN_SPACE_SEQUENCE)
    mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
  mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_NEW_LINE, NULL);

  while (1) {
    mcs_peek_token_type(ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_NULL_CHARACTER) {
      MCerror(3831, "TODO");
    }
    if (token_type == MC_TOKEN_PP_KEYWORD_ENDIF) {
      break;
    }
    mc_syntax_node *group_option;
    mcs_parse_root_statement(ps, ifdef_directive, &group_option);

    append_to_collection((void ***)&ifdef_directive->preprocess_ifndef.groupopt->items,
                         &ifdef_directive->preprocess_ifndef.groupopt->alloc,
                         &ifdef_directive->preprocess_ifndef.groupopt->count, group_option);

    mcs_parse_through_supernumerary_tokens(ps, ifdef_directive);
  }

  mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_PP_KEYWORD_ENDIF, NULL);
  while (1) {
    mcs_peek_token_type(ps, true, 0, &token_type);
    switch (token_type) {
    case MC_TOKEN_SPACE_SEQUENCE:
    case MC_TOKEN_LINE_COMMENT:
    case MC_TOKEN_MULTI_LINE_COMMENT:
      mcs_parse_through_token(ps, ifdef_directive, token_type, NULL);
      continue;
    case MC_TOKEN_NEW_LINE:
      mcs_parse_through_token(ps, ifdef_directive, MC_TOKEN_NEW_LINE, NULL);
      break;
    case MC_TOKEN_NULL_CHARACTER:
      break;
    default:
      MCerror(3857, "TODO:%s", get_mc_token_type_name(token_type));
    }
    break;
  }

  return 0;
}

int mcs_parse_preprocessor_directive(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_preprocessor_directive()");

  mc_token_type token_type;
  mcs_peek_token_type(ps, true, 0, &token_type);
  // printf("token_read:%s\n", get_mc_token_type_name(token_type));
  switch (token_type) {
  case MC_TOKEN_PP_KEYWORD_ENDIF: {
    mcs_parse_through_token(ps, parent, MC_TOKEN_PP_KEYWORD_ENDIF, additional_destination);
  } break;
  case MC_TOKEN_PP_KEYWORD_IFDEF: {
    mcs_parse_pp_ifdef(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_PP_KEYWORD_IFNDEF: {
    mcs_parse_pp_ifndef(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_PP_KEYWORD_INCLUDE: {
    mc_syntax_node *include_directive;
    mcs_construct_syntax_node(ps, MC_SYNTAX_PP_DIRECTIVE_INCLUDE, NULL, parent, &include_directive);
    if (additional_destination) {
      *additional_destination = include_directive;
    }

    // Temporary -- just parse the rest of the line
    mcs_parse_through_token(ps, include_directive, MC_TOKEN_PP_KEYWORD_INCLUDE, NULL);
    while (1) {
      mcs_peek_token_type(ps, true, 0, &token_type);
      if (token_type == MC_TOKEN_NEW_LINE) {
        mcs_parse_through_token(ps, include_directive, token_type, NULL);
        break;
      }
      if (token_type == MC_TOKEN_NULL_CHARACTER)
        break;

      mcs_parse_through_token(ps, include_directive, token_type, NULL);
    }
  } break;
  case MC_TOKEN_PP_KEYWORD_DEFINE: {
    mc_syntax_node *define_directive;
    mcs_construct_syntax_node(ps, MC_SYNTAX_PP_DIRECTIVE_DEFINE, NULL, parent, &define_directive);
    if (additional_destination) {
      *additional_destination = define_directive;
    }

    mcs_parse_through_token(ps, define_directive, MC_TOKEN_PP_KEYWORD_DEFINE, NULL);
    mcs_parse_through_token(ps, define_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
    mcs_parse_through_token(ps, define_directive, MC_TOKEN_IDENTIFIER, &define_directive->preprocess_define.identifier);
    mcs_peek_token_type(ps, true, 0, &token_type);

    if (token_type == MC_TOKEN_OPENING_BRACKET) {
      define_directive->preprocess_define.statement_type = PREPROCESSOR_DEFINE_FUNCTION_LIKE;

      mcs_parse_through_token(ps, define_directive, MC_TOKEN_OPENING_BRACKET, NULL);

      // Temporary -- Just parent the rest of the define statement in the children. do no processing TODO
      bool escape_token_previous = false;
      while (1) {
        bool break_loop = false;
        // if (ps->code[ps->index] == '#') {

        //   mcs_parse_through_token(ps, define_directive, MC_TOKEN_PP_IDENTIFIER, NULL);
        //   continue;
        // }

        mcs_peek_token_type(ps, true, 0, &token_type);
        // printf("%s\n", get_mc_token_type_name(token_type));
        switch (token_type) {
        case MC_TOKEN_ESCAPE_CHARACTER: {
          escape_token_previous = !escape_token_previous;
          mcs_parse_through_token(ps, define_directive, token_type, NULL);
        } break;
        case MC_TOKEN_NEW_LINE: {
          if (!escape_token_previous)
            break_loop = true;
          escape_token_previous = false;

          mcs_parse_through_token(ps, define_directive, token_type, NULL);
        } break;
        default: {
          mcs_parse_through_token(ps, define_directive, token_type, NULL);
          escape_token_previous = false;
        }
        }
        if (break_loop)
          break;
      }
    }
    else {
      if (token_type == MC_TOKEN_SPACE_SEQUENCE) {
        mcs_peek_token_type(ps, true, 1, &token_type);
      }
      if (token_type == MC_TOKEN_NEW_LINE) {
        define_directive->preprocess_define.statement_type = PREPROCESSOR_DEFINE_REMOVAL;
        break;
      }

      define_directive->preprocess_define.statement_type = PREPROCESSOR_DEFINE_REPLACEMENT;

      mcs_parse_through_token(ps, define_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);

      // Temporary -- Just parent the rest of the define statement in the children. do no processing TODO
      bool escape_token_previous = false;
      while (1) {
        if (!escape_token_previous) {
          mcs_peek_token_type(ps, true, 0, &token_type);
          if (token_type == MC_TOKEN_NEW_LINE) {
            break;
          }
        }

        mc_syntax_node *token;
        mcs_peek_token_type(ps, true, 0, &token_type);
        mcs_parse_through_token(ps, define_directive, token_type, &token);
        append_to_collection((void ***)&define_directive->preprocess_define.replacement_list->items,
                             &define_directive->preprocess_define.replacement_list->alloc,
                             &define_directive->preprocess_define.replacement_list->count, token);

        if (token_type == MC_TOKEN_ESCAPE_CHARACTER) {
          escape_token_previous = !escape_token_previous;
        }
        else {
          escape_token_previous = false;
        }

        // printf("#########\nreplace_count:%i\n######\n",
        // define_directive->preprocess_define.replacement_list->count);
      }
    }
    // if (token_type == MC_TOKEN_SPACE_SEQUENCE) {
    //   mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_SPACE_SEQUENCE, NULL);
    //   mcs_peek_token_type(ps, true, 0, &token_type);
    //   if (token_type == MC_TOKEN_IDENTIFIER) {
    //     mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_IDENTIFIER, NULL);
    //   }
    // }
    // mcs_parse_through_token(ps, ifndef_directive, MC_TOKEN_NEW_LINE, NULL);
  } break;
  default: {
    // mcs_parse_through_token(ps, preprocessor_directive, token_type, NULL);
    // print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(3713, "parse_file_root:Unsupported-Token:%s", get_mc_token_type_name(token_type));
  } break;
  }

  register_midge_error_tag("~mcs_parse_preprocessor_directive()");
  return 0;
}

int mcs_parse_enum_definition(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *enum_definition;
  mcs_construct_syntax_node(ps, MC_SYNTAX_ENUM_DECL, NULL, parent, &enum_definition);
  if (additional_destination) {
    *additional_destination = enum_definition;
  }

  mcs_parse_through_token(ps, enum_definition, MC_TOKEN_ENUM_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, enum_definition);

  mcs_parse_through_token(ps, enum_definition, MC_TOKEN_IDENTIFIER, &enum_definition->enumeration.name);
  mcs_parse_through_supernumerary_tokens(ps, enum_definition);

  mcs_parse_through_token(ps, enum_definition, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
  mcs_parse_through_supernumerary_tokens(ps, enum_definition);

  while (1) {
    mc_token_type token_type;
    mcs_peek_token_type(ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }
    if (enum_definition->enumeration.members->count > 0) {
      mcs_parse_through_token(ps, enum_definition, MC_TOKEN_COMMA, NULL);
      mcs_parse_through_supernumerary_tokens(ps, enum_definition);
      mcs_peek_token_type(ps, true, 0, &token_type);
      if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET) {
        break;
      }
    }

    mc_syntax_node *enum_member;
    mcs_construct_syntax_node(ps, MC_SYNTAX_ENUM_DECL_MEMBER, NULL, enum_definition, &enum_member);

    mcs_parse_through_token(ps, enum_member, MC_TOKEN_IDENTIFIER, &enum_member->enum_member.identifier);

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_ASSIGNMENT_OPERATOR) {
      mcs_parse_through_supernumerary_tokens(ps, enum_member);
      mcs_parse_through_token(ps, enum_member, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL);
      mcs_parse_through_supernumerary_tokens(ps, enum_member);

      mcs_parse_expression(ps, enum_member, &enum_member->enum_member.value_expression);
      // mcs_peek_token_type(ps, false, 0, &token_type);
      // switch (token_type) {
      // case MC_TOKEN_IDENTIFIER:
      // case MC_TOKEN_NUMERIC_LITERAL: {
      //   mcs_parse_through_token(ps, enum_member, token_type, &enum_member->enum_member.value_expression);
      // } break;
      // default:
      //   MCerror(3777, "TODO :%s", get_mc_token_type_name(token_type));
      // }
    }

    append_to_collection((void ***)&enum_definition->enumeration.members->items,
                         &enum_definition->enumeration.members->alloc, &enum_definition->enumeration.members->count,
                         enum_member);
    mcs_parse_through_supernumerary_tokens(ps, enum_definition);
  }

  mcs_parse_through_token(ps, enum_definition, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);

  return 0;
}

int mcs_parse_field_declarator(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_field_declaration()");
  mc_syntax_node *declarator;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FIELD_DECLARATOR, NULL, parent, &declarator);
  if (additional_destination) {
    *additional_destination = declarator;
  }

  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_STAR_CHARACTER) {
    mcs_parse_dereference_sequence(ps, declarator, &declarator->field_declarator.type_dereference);
    mcs_parse_through_supernumerary_tokens(ps, declarator);
  }
  else {
    declarator->field_declarator.type_dereference = NULL;
  }

  mcs_peek_token_type(ps, true, 0, &token_type);
  if (token_type == MC_TOKEN_OPENING_BRACKET) {
    declarator->field_declarator.name = NULL;
    declarator->field_declarator.array_size = NULL;

    //   // It's a function pointer!
    mcs_parse_function_pointer_declarator(ps, declarator, true, &declarator->field_declarator.function_pointer);
  }
  else if (token_type == MC_TOKEN_IDENTIFIER) {
    declarator->field_declarator.function_pointer = NULL;

    mcs_parse_through_token(ps, declarator, MC_TOKEN_IDENTIFIER, &declarator->field_declarator.name);

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_SQUARE_OPENING_BRACKET) {
      // Is an array declaration
      mcs_parse_through_token(ps, declarator, MC_TOKEN_SQUARE_OPENING_BRACKET, NULL);
      mcs_parse_through_supernumerary_tokens(ps, declarator);

      mcs_peek_token_type(ps, true, 0, &token_type);
      if (token_type != MC_TOKEN_NUMERIC_LITERAL && token_type != MC_TOKEN_IDENTIFIER) {
        MCerror(3994, "Unexpected token: Expected numeric expression");
      }

      mcs_parse_through_token(ps, declarator, token_type, &declarator->field_declarator.array_size);
      mcs_parse_through_supernumerary_tokens(ps, declarator);
      mcs_parse_through_token(ps, declarator, MC_TOKEN_SQUARE_CLOSING_BRACKET, NULL);
    }
    else {
      declarator->field_declarator.array_size = NULL;
    }
  }

  return 0;
}

int mcs_parse_field_declarators(parsing_state *ps, mc_syntax_node *field_decl, mc_syntax_node_list *declarators_list)
{

  while (1) {
    mc_syntax_node *declarator;
    mcs_parse_field_declarator(ps, field_decl, &declarator);

    append_to_collection((void ***)&declarators_list->items, &declarators_list->alloc, &declarators_list->count,
                         declarator);
    mc_token_type token_type;
    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      break;
    }

    mcs_parse_through_supernumerary_tokens(ps, field_decl);
    mcs_parse_through_token(ps, field_decl, MC_TOKEN_COMMA, NULL);
    mcs_parse_through_supernumerary_tokens(ps, field_decl);
  }

  return 0;
}

int mcs_parse_field_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_field_declaration()");
  mc_syntax_node *field_decl;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FIELD_DECLARATION, NULL, parent, &field_decl);
  if (additional_destination) {
    *additional_destination = field_decl;
  }

  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);

  mc_syntax_node *type_identity;
  mcs_parse_type_identifier(ps, field_decl, &type_identity);
  mcs_parse_through_supernumerary_tokens(ps, field_decl);

  field_decl->field.type = FIELD_KIND_STANDARD;
  field_decl->field.type_identifier = type_identity;

  field_decl->field.declarators = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
  field_decl->field.declarators->alloc = 0;
  field_decl->field.declarators->count = 0;

  mcs_parse_field_declarators(ps, field_decl, field_decl->field.declarators);
  // }

  register_midge_error_tag("mcs_parse_field_declaration(~)");
  return 0;
}

int mcs_parse_struct_declaration_list(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node_list **list_destination)
{
  mc_syntax_node_list *fields = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
  *list_destination = fields;
  fields->alloc = 0;
  fields->count = 0;
  fields->items = NULL;

  mc_token_type token_type;
  while (1) {
    mcs_peek_token_type(ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }

    switch (token_type) {
    case MC_TOKEN_CONST_KEYWORD:
    case MC_TOKEN_IDENTIFIER:
    case MC_TOKEN_INT_KEYWORD:
    case MC_TOKEN_CHAR_KEYWORD:
    case MC_TOKEN_LONG_KEYWORD:
    case MC_TOKEN_SHORT_KEYWORD:
    case MC_TOKEN_FLOAT_KEYWORD:
    case MC_TOKEN_VOID_KEYWORD:
    case MC_TOKEN_SIGNED_KEYWORD:
    case MC_TOKEN_UNSIGNED_KEYWORD: {
      mc_syntax_node *field_declaration;
      mcs_parse_field_declaration(ps, parent, &field_declaration);

      append_to_collection((void ***)&fields->items, &fields->alloc, &fields->count, field_declaration);

      mcs_parse_through_supernumerary_tokens(ps, parent);
      mcs_parse_through_token(ps, parent, MC_TOKEN_SEMI_COLON, NULL);
    } break;
    case MC_TOKEN_UNION_KEYWORD:
    case MC_TOKEN_STRUCT_KEYWORD: {
      // Determine if a specified field type or nested definition
      mc_token_type token1;
      mcs_peek_token_type(ps, false, 1, &token1);
      bool type_specifier;
      switch (token1) {
      case MC_TOKEN_IDENTIFIER: {
        mc_token_type token2;
        mcs_peek_token_type(ps, false, 1, &token2);
        switch (token2) {
        case MC_TOKEN_CURLY_OPENING_BRACKET: {
          type_specifier = false;
        } break;
        case MC_TOKEN_IDENTIFIER: {
          type_specifier = true;
        } break;
        default:
          print_parse_error(ps->code, ps->index, "mcs_parse_struct_declaration_list", "");
          MCerror(4602, "NotYetSupported:IDENTIFIER>%s", get_mc_token_type_name(token2));
        }
      } break;
      case MC_TOKEN_CURLY_OPENING_BRACKET: {
        type_specifier = false;
      } break;
      default:
        print_parse_error(ps->code, ps->index, "mcs_parse_struct_declaration_list", "");
        MCerror(4607, "NotYetSupported:%s", get_mc_token_type_name(token1));
      }

      // Parse appropriately
      if (type_specifier) {
        mc_syntax_node *field_declaration;
        mcs_parse_field_declaration(ps, parent, &field_declaration);

        append_to_collection((void ***)&fields->items, &fields->alloc, &fields->count, field_declaration);

        mcs_parse_through_supernumerary_tokens(ps, parent);
        mcs_parse_through_token(ps, parent, MC_TOKEN_SEMI_COLON, NULL);
      }
      else {
        mc_syntax_node *nested_declaration;
        mcs_construct_syntax_node(ps, MC_SYNTAX_NESTED_TYPE_DECLARATION, NULL, parent, &nested_declaration);
        append_to_collection((void ***)&fields->items, &fields->alloc, &fields->count, nested_declaration);

        mcs_parse_type_declaration(ps, nested_declaration, &nested_declaration->nested_type.declaration);
        mcs_parse_through_supernumerary_tokens(ps, parent);

        mcs_peek_token_type(ps, false, 0, &token_type);
        if (token_type == MC_TOKEN_IDENTIFIER || token_type == MC_TOKEN_STAR_CHARACTER) {

          nested_declaration->nested_type.declarators = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
          nested_declaration->nested_type.declarators->alloc = 0;
          nested_declaration->nested_type.declarators->count = 0;

          mcs_parse_field_declarators(ps, nested_declaration, nested_declaration->nested_type.declarators);
        }
        else {
          nested_declaration->nested_type.declarators = NULL;
        }

        mcs_parse_through_supernumerary_tokens(ps, parent);
        mcs_parse_through_token(ps, parent, MC_TOKEN_SEMI_COLON, NULL);
      }
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(3361, ":Unsupported-Token:%s", get_mc_token_type_name(token_type));
    }
    }
    mcs_parse_through_supernumerary_tokens(ps, parent);
  }

  return 0;
}

int mcs_parse_type_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_type_declaration()");

  mc_token_type token_type;
  mcs_peek_token_type(ps, true, 0, &token_type);

  mc_syntax_node *type_definition = NULL;
  bool is_struct = false;
  if (token_type == MC_TOKEN_STRUCT_KEYWORD) {
    mcs_construct_syntax_node(ps, MC_SYNTAX_STRUCT_DECL, NULL, parent, &type_definition);
    is_struct = true;
  }
  else if (token_type == MC_TOKEN_UNION_KEYWORD) {
    mcs_construct_syntax_node(ps, MC_SYNTAX_UNION_DECL, NULL, parent, &type_definition);
  }
  else {
    MCerror(3889, "expected union or struct keyword");
  }
  if (additional_destination) {
    *additional_destination = type_definition;
  }

  mcs_parse_through_token(ps, type_definition, token_type, NULL);
  mcs_parse_through_supernumerary_tokens(ps, type_definition);

  mcs_peek_token_type(ps, true, 0, &token_type);
  bool name_declared = false;
  if (token_type == MC_TOKEN_IDENTIFIER) {
    name_declared = true;
    if (is_struct) {
      mcs_parse_through_token(ps, type_definition, MC_TOKEN_IDENTIFIER, &type_definition->struct_decl.type_name);
    }
    else {
      mcs_parse_through_token(ps, type_definition, MC_TOKEN_IDENTIFIER, &type_definition->union_decl.type_name);
    }
    mcs_parse_through_supernumerary_tokens(ps, type_definition);
  }

  mcs_peek_token_type(ps, true, 0, &token_type);
  if (token_type == MC_TOKEN_CURLY_OPENING_BRACKET) {
    mcs_parse_through_token(ps, type_definition, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_definition);

    if (is_struct) {
      mcs_parse_struct_declaration_list(ps, type_definition, &type_definition->struct_decl.fields);
    }
    else {
      mcs_parse_struct_declaration_list(ps, type_definition, &type_definition->union_decl.fields);
    }

    mcs_parse_through_token(ps, type_definition, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, type_definition);
  }
  else {
    if (!name_declared) {
      MCerror(3949, "expected struct declaration list");
    }

    if (is_struct) {
      type_definition->struct_decl.fields = NULL;
    }
    else {
      type_definition->union_decl.fields = NULL;
    }
  }

  register_midge_error_tag("mcs_parse_type_declaration(~)");
  return 0;
}

int mcs_parse_type_alias_definition(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_function_definition()");

  mc_syntax_node *type_alias_definition;
  mcs_construct_syntax_node(ps, MC_SYNTAX_TYPE_ALIAS, NULL, parent, &type_alias_definition);
  if (additional_destination) {
    *additional_destination = type_alias_definition;
  }

  mcs_parse_through_token(ps, type_alias_definition, MC_TOKEN_TYPEDEF_KEYWORD, NULL);
  mcs_parse_through_supernumerary_tokens(ps, type_alias_definition);

  mc_token_type token_type;
  mcs_peek_token_type(ps, true, 0, &token_type);
  switch (token_type) {
  case MC_TOKEN_UNION_KEYWORD:
  case MC_TOKEN_STRUCT_KEYWORD: {
    mcs_peek_token_type(ps, false, 1, &token_type);
    switch (token_type) {
    case MC_TOKEN_CURLY_OPENING_BRACKET: {
      // Requires type alias definition
      mcs_parse_type_declaration(ps, type_alias_definition, &type_alias_definition->type_alias.type_descriptor);
    } break;
    case MC_TOKEN_IDENTIFIER: {
      mcs_peek_token_type(ps, false, 2, &token_type);
      switch (token_type) {
      case MC_TOKEN_CURLY_OPENING_BRACKET: {
        // Type Definition & Struct definition
        mcs_parse_type_declaration(ps, type_alias_definition, &type_alias_definition->type_alias.type_descriptor);
      } break;
      case MC_TOKEN_IDENTIFIER: {
        // Type Definition only
        mcs_parse_type_declaration(ps, type_alias_definition, &type_alias_definition->type_alias.type_descriptor);
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(3984, "TODO:%s", get_mc_token_type_name(token_type));
      }
      }
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(3990, "TODO:STRUCT/UNION:%s", get_mc_token_type_name(token_type));
    }
    }
  } break;
  case MC_TOKEN_ENUM_KEYWORD: {
    mcs_parse_enum_definition(ps, type_alias_definition, &type_alias_definition->type_alias.type_descriptor);
  } break;
  case MC_TOKEN_VOID_KEYWORD:
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_SHORT_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_SIGNED_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD:
  case MC_TOKEN_CONST_KEYWORD:
  case MC_TOKEN_IDENTIFIER: {
    // Determine type of definition
    mcs_peek_token_type(ps, false, 1, &token_type);
    switch (token_type) {
    case MC_TOKEN_OPENING_BRACKET: {
      // Function Pointer
      mcs_parse_function_pointer_declaration(ps, type_alias_definition, false,
                                             &type_alias_definition->type_alias.type_descriptor);

      type_alias_definition->type_alias.alias =
          type_alias_definition->type_alias.type_descriptor->fptr_declaration.declarator->fptr_declarator.name;

      mcs_parse_through_supernumerary_tokens(ps, type_alias_definition);
      mcs_parse_through_token(ps, type_alias_definition, MC_TOKEN_SEMI_COLON, NULL);

      // Break from below
      return 0;
    }
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(4956, "TODO:STRUCT/UNION:%s", get_mc_token_type_name(token_type));
    }
    }
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(4962, "TODO:%s", get_mc_token_type_name(token_type));
  }
  }

  mcs_parse_through_supernumerary_tokens(ps, type_alias_definition);
  mcs_parse_through_token(ps, type_alias_definition, MC_TOKEN_IDENTIFIER, &type_alias_definition->type_alias.alias);

  mcs_parse_through_supernumerary_tokens(ps, type_alias_definition);
  mcs_parse_through_token(ps, type_alias_definition, MC_TOKEN_SEMI_COLON, NULL);

  return 0;
}

int mcs_parse_function_definition(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_function_definition()");

  mc_syntax_node *function;
  mcs_construct_syntax_node(ps, MC_SYNTAX_FUNCTION, NULL, parent, &function);
  if (additional_destination) {
    *additional_destination = function;
  }

  // print_syntax_node(function, 0);
  mcs_parse_function_definition_header(ps, function);
  mcs_parse_through_supernumerary_tokens(ps, function);

  mc_token_type token_type;
  mcs_peek_token_type(ps, true, 0, &token_type);
  if (token_type == MC_TOKEN_CURLY_OPENING_BRACKET) {
    // TODO -- memory isn't cleared if this fails, and if this fails it is handled higher up. so memory is never
    // cleared
    mcs_parse_code_block(ps, function, &function->function.code_block);
  }
  else {
    function->function.code_block = NULL;
    mcs_parse_through_token(ps, function, MC_TOKEN_SEMI_COLON, NULL);
  }

  // print_syntax_node(function, 0);

  register_midge_error_tag("mcs_parse_function_definition(~)");
  return 0;
}

int mcs_parse_extern_c_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *extern_block;
  mcs_construct_syntax_node(ps, MC_SYNTAX_EXTERN_C_BLOCK, NULL, parent, &extern_block);
  if (additional_destination) {
    *additional_destination = extern_block;
  }

  mcs_parse_through_token(ps, extern_block, MC_TOKEN_EXTERN_KEYWORD, NULL);

  mc_token_type token_type;
  mcs_peek_token_type(ps, false, 0, &token_type);
  if (token_type == MC_TOKEN_CURLY_OPENING_BRACKET) {
    mcs_parse_through_supernumerary_tokens(ps, extern_block);
    mcs_parse_through_token(ps, extern_block, MC_TOKEN_CURLY_OPENING_BRACKET, NULL);
    mcs_parse_through_supernumerary_tokens(ps, extern_block);

    while (1) {
      mcs_peek_token_type(ps, false, 0, &token_type);
      if (token_type == MC_TOKEN_CURLY_CLOSING_BRACKET)
        break;

      mc_syntax_node *declaration;
      mcs_parse_function_definition(ps, extern_block, &declaration);
      mcs_parse_through_supernumerary_tokens(ps, extern_block);

      append_to_collection((void ***)&extern_block->extern_block.declarations->items,
                           &extern_block->extern_block.declarations->alloc,
                           &extern_block->extern_block.declarations->count, declaration);
    }
    mcs_parse_through_token(ps, extern_block, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL);
  }

  return 0;
}

int mcs_parse_root_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_token_type token_type;
  mcs_peek_token_type(ps, true, 0, &token_type);
  switch (token_type) {
  // case MC_TOKEN_MULTI_LINE_COMMENT: { // TODO
  //   mcs_parse_through_token(ps, function, token_type, &element_documentation);

  // } break;
  case MC_TOKEN_PP_KEYWORD_ENDIF:
  case MC_TOKEN_PP_KEYWORD_DEFINE:
  case MC_TOKEN_PP_KEYWORD_INCLUDE:
  case MC_TOKEN_PP_KEYWORD_IFDEF:
  case MC_TOKEN_PP_KEYWORD_IFNDEF: {
    mcs_parse_preprocessor_directive(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_TYPEDEF_KEYWORD: {
    mcs_parse_type_alias_definition(ps, parent, additional_destination);
  } break;
  case MC_TOKEN_UNION_KEYWORD:
  case MC_TOKEN_STRUCT_KEYWORD: {
    mc_syntax_node *declaration;
    mcs_parse_type_declaration(ps, parent, &declaration);
    if (additional_destination) {
      *additional_destination = declaration;
    }

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      mcs_parse_through_supernumerary_tokens(ps, declaration);
      mcs_parse_through_token(ps, declaration, MC_TOKEN_SEMI_COLON, NULL);
    }
  } break;
  case MC_TOKEN_ENUM_KEYWORD: {
    mc_syntax_node *declaration;
    mcs_parse_enum_definition(ps, parent, &declaration);

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      mcs_parse_through_supernumerary_tokens(ps, declaration);
      mcs_parse_through_token(ps, declaration, MC_TOKEN_SEMI_COLON, NULL);
    }
  } break;
  case MC_TOKEN_EXTERN_KEYWORD: {
    mc_syntax_node *declaration;
    mcs_parse_extern_c_block(ps, parent, &declaration);
    if (additional_destination) {
      *additional_destination = declaration;
    }

    mcs_peek_token_type(ps, false, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      mcs_parse_through_supernumerary_tokens(ps, declaration);
      mcs_parse_through_token(ps, declaration, MC_TOKEN_SEMI_COLON, NULL);
    }
  } break;
  case MC_TOKEN_VOID_KEYWORD:
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_SHORT_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_SIGNED_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD:
  case MC_TOKEN_CONST_KEYWORD:
  case MC_TOKEN_STATIC_KEYWORD:
  case MC_TOKEN_IDENTIFIER: {
    mcs_parse_function_definition(ps, parent, additional_destination);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(3716, "parse_file_root:Unsupported-Token:%s", get_mc_token_type_name(token_type));
  }
  }

  return 0;
}

int parse_definition_to_syntax_tree(char *code, mc_syntax_node **ast)
{
  parsing_state ps;
  ps.code = code;
  ps.allow_imperfect_parse = false;
  ps.index = 0;
  ps.line = 0;
  ps.col = 0;

  // mc_syntax_node *element_documentation = NULL;
  // case MC_TOKEN_MULTI_LINE_COMMENT: { // TODO
  //   mcs_parse_through_token(ps, function, token_type, &element_documentation);

  // } break;

  mc_token_type token_type;
  while (ps.code[ps.index] != '\0') {

    mcs_parse_root_statement(&ps, NULL, ast);

    mcs_parse_through_supernumerary_tokens(&ps, *ast);
    mcs_peek_token_type(&ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      mcs_parse_through_token(&ps, *ast, MC_TOKEN_SEMI_COLON, NULL);
      mcs_parse_through_supernumerary_tokens(&ps, *ast);
    }
  }

  // printf("parse_definition_to_syntax_tree\n");
  // print_syntax_node(*ast, 0);

  return 0;
}

int parse_file_to_syntax_tree(char *code, mc_syntax_node **file_ast)
{
  parsing_state ps;
  ps.code = code;
  ps.allow_imperfect_parse = false;
  ps.index = 0;
  ps.line = 0;
  ps.col = 0;

  mcs_construct_syntax_node(&ps, MC_SYNTAX_FILE_ROOT, NULL, NULL, file_ast);
  mcs_parse_through_supernumerary_tokens(&ps, *file_ast);

  mc_syntax_node *element_documentation = NULL;

  while (ps.code[ps.index] != '\0') {

    mcs_parse_root_statement(&ps, *file_ast, NULL);

    mcs_parse_through_supernumerary_tokens(&ps, *file_ast);
    mc_token_type token_type;
    mcs_peek_token_type(&ps, true, 0, &token_type);
    if (token_type == MC_TOKEN_SEMI_COLON) {
      mcs_parse_through_token(&ps, *file_ast, MC_TOKEN_SEMI_COLON, NULL);
      mcs_parse_through_supernumerary_tokens(&ps, *file_ast);
    }
  }

  return 0;
}