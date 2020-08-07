#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

typedef struct parsing_state {
  char *code;
  int index;
  int line;
  int col;

  bool allow_imperfect_parse;

} parsing_state;

int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_statement_list(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int _mcs_parse_expression(parsing_state *ps, int allowable_precedence, mc_syntax_node *parent,
                          mc_syntax_node **additional_destination);

const char *get_mc_token_type_name(mc_token_type type)
{
  switch (type) {
  case MC_TOKEN_NULL:
    return "MC_TOKEN_NULL";
  case MC_TOKEN_STAR_CHARACTER:
    return "MC_TOKEN_STAR_CHARACTER";
  case MC_TOKEN_MULTIPLY_OPERATOR:
    return "MC_TOKEN_MULTIPLY_OPERATOR";
  case MC_TOKEN_DEREFERENCE_OPERATOR:
    return "MC_TOKEN_DEREFERENCE_OPERATOR";
  case MC_TOKEN_IDENTIFIER:
    return "MC_TOKEN_IDENTIFIER";
  case MC_TOKEN_SQUARE_OPEN_BRACKET:
    return "MC_TOKEN_SQUARE_OPEN_BRACKET";
  case MC_TOKEN_SQUARE_CLOSE_BRACKET:
    return "MC_TOKEN_SQUARE_CLOSE_BRACKET";
  case MC_TOKEN_OPEN_BRACKET:
    return "MC_TOKEN_OPEN_BRACKET";
  case MC_TOKEN_CLOSING_BRACKET:
    return "MC_TOKEN_CLOSING_BRACKET";
  case MC_TOKEN_SEMI_COLON:
    return "MC_TOKEN_SEMI_COLON";
  case MC_TOKEN_COLON:
    return "MC_TOKEN_COLON";
  case MC_TOKEN_EQUALITY_OPERATOR:
    return "MC_TOKEN_EQUALITY_OPERATOR";
  case MC_TOKEN_DECREMENT_OPERATOR:
    return "MC_TOKEN_DECREMENT_OPERATOR";
  case MC_TOKEN_INCREMENT_OPERATOR:
    return "MC_TOKEN_INCREMENT_OPERATOR";
  case MC_TOKEN_POINTER_OPERATOR:
    return "MC_TOKEN_POINTER_OPERATOR";
  case MC_TOKEN_ASSIGNMENT_OPERATOR:
    return "MC_TOKEN_ASSIGNMENT_OPERATOR";
  case MC_TOKEN_NOT_OPERATOR:
    return "MC_TOKEN_NOT_OPERATOR";
  case MC_TOKEN_SUBTRACT_OPERATOR:
    return "MC_TOKEN_SUBTRACT_OPERATOR";
  case MC_TOKEN_DIVIDE_OPERATOR:
    return "MC_TOKEN_DIVIDE_OPERATOR";
  case MC_TOKEN_MODULO_OPERATOR:
    return "MC_TOKEN_MODULO_OPERATOR";
  case MC_TOKEN_PLUS_OPERATOR:
    return "MC_TOKEN_PLUS_OPERATOR";
  case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
    return "MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR";
  case MC_TOKEN_IF_KEYWORD:
    return "MC_TOKEN_IF_KEYWORD";
  case MC_TOKEN_ELSE_KEYWORD:
    return "MC_TOKEN_ELSE_KEYWORD";
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
  case MC_TOKEN_SIZEOF_KEYWORD:
    return "MC_TOKEN_SIZEOF_KEYWORD";
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
  case MC_TOKEN_LESS_THAN_OPERATOR:
    return "MC_TOKEN_LESS_THAN_OPERATOR";
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
    return "MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR";
  case MC_TOKEN_MORE_THAN_OPERATOR:
    return "MC_TOKEN_MORE_THAN_OPERATOR";
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
  case MC_TOKEN_BINARY_OR_OPERATOR:
    return "MC_TOKEN_BINARY_OR_OPERATOR";
  case MC_TOKEN_CASE_KEYWORD:
    return "MC_TOKEN_CASE_KEYWORD";
  case MC_TOKEN_DEFAULT_KEYWORD:
    return "MC_TOKEN_DEFAULT_KEYWORD";
  case MC_TOKEN_STRUCT_KEYWORD:
    return "MC_TOKEN_STRUCT_KEYWORD";
  case MC_TOKEN_VOID_KEYWORD:
    return "MC_TOKEN_VOID_KEYWORD";
  case MC_TOKEN_INT_KEYWORD:
    return "MC_TOKEN_INT_KEYWORD";
  case MC_TOKEN_CHAR_KEYWORD:
    return "MC_TOKEN_CHAR_KEYWORD";
  case MC_TOKEN_UNSIGNED_KEYWORD:
    return "MC_TOKEN_UNSIGNED_KEYWORD";
  case MC_TOKEN_BOOL_KEYWORD:
    return "MC_TOKEN_BOOL_KEYWORD";
  case MC_TOKEN_FLOAT_KEYWORD:
    return "MC_TOKEN_FLOAT_KEYWORD";
  case MC_TOKEN_LONG_KEYWORD:
    return "MC_TOKEN_LONG_KEYWORD";
  default: {
    // TODO -- DEBUG -- this string is never free-d
    char *new_string;
    cprintf(new_string, "TODO_ENCODE_THIS_TYPE_OR_UNSUPPORTED:%i", type);
    return (const char *)new_string;
  }
  }
}

const char *get_mc_syntax_token_type_name(mc_syntax_node_type type)
{
  switch ((mc_syntax_node_type)type) {
  case MC_SYNTAX_FUNCTION:
    return "MC_SYNTAX_FUNCTION";
  case MC_SYNTAX_BLOCK:
    return "MC_SYNTAX_BLOCK";
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
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION:
    return "MC_SYNTAX_PARENTHESIZED_EXPRESSION";
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION:
    return "MC_SYNTAX_ASSIGNMENT_EXPRESSION";
  case MC_SYNTAX_SIZEOF_EXPRESSION:
    return "MC_SYNTAX_SIZEOF_EXPRESSION";
  case MC_SYNTAX_RETURN_STATEMENT:
    return "MC_SYNTAX_RETURN_STATEMENT";
  case MC_SYNTAX_INVOCATION:
    return "MC_SYNTAX_INVOCATION";
  case MC_SYNTAX_SUPERNUMERARY:
    return "MC_SYNTAX_SUPERNUMERARY";
  case MC_SYNTAX_DEREFERENCE_SEQUENCE:
    return "MC_SYNTAX_DEREFERENCE_SEQUENCE";
  case MC_SYNTAX_PARAMETER_DECLARATION:
    return "MC_SYNTAX_PARAMETER_DECLARATION";
  case MC_SYNTAX_MODIFIED_TYPE:
    return "MC_SYNTAX_MODIFIED_TYPE";
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION:
    return "MC_SYNTAX_STRING_LITERAL_EXPRESSION";
  case MC_SYNTAX_CAST_EXPRESSION:
    return "MC_SYNTAX_CAST_EXPRESSION";
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION:
    return "MC_SYNTAX_PREPENDED_UNARY_EXPRESSION";
  case MC_SYNTAX_CONDITIONAL_EXPRESSION:
    return "MC_SYNTAX_CONDITIONAL_EXPRESSION";
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

int _mcs_print_syntax_node_ancestry(mc_syntax_node *syntax_node, int depth, int ancestry_count)
{

  if (syntax_node->parent) {
    MCcall(_mcs_print_syntax_node_ancestry(syntax_node->parent, depth, ancestry_count + 1));
    printf(" |  | \n");
  }

  if (ancestry_count) {
    printf("|^ %s ^|", get_mc_syntax_token_type_name(syntax_node->type));
  }

  return 0;
}

int print_syntax_node(mc_syntax_node *syntax_node, int depth)
{
  // printf("mpsyn-tree-0 %p >%i\n", syntax_node, depth);
  for (int i = 0; i < depth; ++i) {
    printf("|  ");
  }
  printf("|--%s", get_mc_syntax_token_type_name(syntax_node->type));
  // printf("mpst-0\n");

  // printf("mpsyn-tree-1\n");
  if ((int)syntax_node->type < (int)MC_TOKEN_STANDARD_MAX_VALUE) {
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
      MCcall(print_syntax_node(syntax_node->children->items[i], depth + 1));
    }
    // printf("mpst-5\n");
  }
  // printf("mpsyn-tree-exit >%i\n", depth);

  return 0;
}

int _copy_syntax_node_to_text_v1(c_str *cstr, mc_syntax_node *syntax_node)
{
  if ((mc_token_type)syntax_node->type < MC_TOKEN_STANDARD_MAX_VALUE) {
    MCcall(append_to_c_str(cstr, syntax_node->text));
    return 0;
  }

  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    MCcall(_copy_syntax_node_to_text_v1(cstr, child));
  }
  // switch (syntax_node->type) {
  // case MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR: {
  //   for (int a = 0; a < syntax_node->children->count; ++a) {
  //     mc_syntax_node *child = syntax_node->children->items[a];

  //     MCcall(copy_syntax_node_to_text_v1(cstr, child));
  //   }
  // } break;
  // default: {
  //   MCerror(290, "MCS_copy_syntax_node_to_text:>Unsupported-token:%s",
  //           get_mc_syntax_token_type_name(syntax_node->type));
  // }
  // }

  return 0;
}

int copy_syntax_node_to_text_v1(mc_syntax_node *syntax_node, char **output)
{
  if ((mc_token_type)syntax_node->type < MC_TOKEN_STANDARD_MAX_VALUE) {
    allocate_and_copy_cstr(*output, syntax_node->text);
    return 0;
  }

  c_str *cstr;
  MCcall(init_c_str(&cstr));

  MCcall(_copy_syntax_node_to_text_v1(cstr, syntax_node));

  *output = cstr->text;
  release_c_str(cstr, false);

  return 0;
}

int mcs_add_syntax_node_to_parent(mc_syntax_node *parent, mc_syntax_node *child)
{
  if (parent) {
    MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                                child));
  }

  child->parent = parent;

  return 0;
}

int mcs_construct_syntax_node(parsing_state *ps, mc_syntax_node_type node_type, char *mc_token_primitive_text,
                              mc_syntax_node *parent, mc_syntax_node **result)
{
  register_midge_error_tag("mcs_construct_syntax_node(%s)", get_mc_syntax_token_type_name(node_type));

  mc_syntax_node *syntax_node = (mc_syntax_node *)calloc(sizeof(mc_syntax_node), 1);
  syntax_node->type = node_type;
  syntax_node->begin.index = ps->index;
  syntax_node->begin.line = ps->line;
  syntax_node->begin.col = ps->col;

  if ((int)node_type >= (int)MC_TOKEN_STANDARD_MAX_VALUE) {
    syntax_node->children = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->children->alloc = 0;
    syntax_node->children->count = 0;
  }
  else {
    syntax_node->text = mc_token_primitive_text;
  }

  switch (node_type) {
  case MC_SYNTAX_FUNCTION: {
    syntax_node->function.return_type_identifier = NULL;
    syntax_node->function.return_type_dereference = NULL;
    syntax_node->function.return_mc_type = NULL;
    syntax_node->function.name = NULL;
    syntax_node->function.parameters = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->function.parameters->alloc = 0;
    syntax_node->function.parameters->count = 0;
  } break;
  case MC_SYNTAX_BLOCK: {
    syntax_node->block_node.statement_list = NULL;
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
    syntax_node->for_statement.code_block = NULL;
  } break;
  case MC_SYNTAX_WHILE_STATEMENT: {
    syntax_node->while_statement.conditional = NULL;
    syntax_node->while_statement.do_first = false;
    syntax_node->while_statement.code_block = NULL;
  } break;
  case MC_SYNTAX_IF_STATEMENT: {
    syntax_node->if_statement.conditional = NULL;
    syntax_node->if_statement.code_block = NULL;
    syntax_node->if_statement.else_continuance = NULL;
  } break;
  case MC_SYNTAX_EXPRESSION_STATEMENT: {
    syntax_node->expression_statement.expression = NULL;
  } break;
  case MC_SYNTAX_DECLARATION_STATEMENT: {
    syntax_node->declaration_statement.declaration = NULL;
  } break;
  case MC_SYNTAX_RETURN_STATEMENT: {
    syntax_node->return_statement.expression = NULL;
  } break;
  case MC_SYNTAX_PARAMETER_DECLARATION: {
    syntax_node->parameter.type_identifier = NULL;
    syntax_node->parameter.mc_type = NULL;
    syntax_node->parameter.type_dereference = NULL;
    syntax_node->parameter.name = NULL;
  } break;
  case MC_SYNTAX_MODIFIED_TYPE: {
    syntax_node->modified_type.type_modifier = NULL;
    syntax_node->modified_type.type_identifier = NULL;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATION: {
    syntax_node->local_variable_declaration.type_identifier = NULL;
    syntax_node->local_variable_declaration.mc_type = NULL;
    syntax_node->local_variable_declaration.declarators = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->local_variable_declaration.declarators->alloc = 0;
    syntax_node->local_variable_declaration.declarators->count = 0;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR: {
    syntax_node->local_variable_declarator.type_dereference = NULL;
    syntax_node->local_variable_declarator.initializer = NULL;
    syntax_node->local_variable_declarator.variable_name = NULL;
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER: {
    syntax_node->local_variable_array_initializer.size_expression = NULL;
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
    syntax_node->invocation.mc_function_info = NULL;
    syntax_node->invocation.arguments = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->invocation.arguments->alloc = 0;
    syntax_node->invocation.arguments->count = 0;
  } break;
  case MC_SYNTAX_CONDITIONAL_EXPRESSION: {
    syntax_node->conditional_expression.left = NULL;
    syntax_node->conditional_expression.conditional_operator = NULL;
    syntax_node->conditional_expression.right = NULL;
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
    syntax_node->cast_expression.mc_type = NULL;
    syntax_node->cast_expression.expression = NULL;
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    syntax_node->cast_expression.expression = NULL;
  } break;
  case MC_SYNTAX_SIZEOF_EXPRESSION: {
    syntax_node->sizeof_expression.type_identifier = NULL;
    syntax_node->sizeof_expression.type_dereference = NULL;
    syntax_node->sizeof_expression.mc_type = NULL;
  } break;
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION: {
    // Nothing for the moment
  } break;
  case MC_SYNTAX_DEREFERENCE_SEQUENCE: {
    syntax_node->dereference_sequence.count = 0;
  } break;
  case MC_SYNTAX_SUPERNUMERARY: {
    // TODO ?
  } break;
  default: {
    if ((int)node_type < (int)MC_TOKEN_STANDARD_MAX_VALUE) {
      // Token
      // -- Do nothing
      break;
    }
    MCerror(355, "Unsupported:%s %i", get_mc_syntax_token_type_name(node_type), node_type);
  }
  }

  if (parent) {
    MCcall(mcs_add_syntax_node_to_parent(parent, syntax_node));
  }
  else {
    syntax_node->parent = NULL;
  }

  *result = syntax_node;
  return 0;
}

void release_syntax_node(mc_syntax_node *syntax_node)
{
  if (!syntax_node) {
    return;
  }

  printf("release-%s\n", get_mc_syntax_token_type_name(syntax_node->type));
  if ((int)syntax_node->type >= (int)MC_TOKEN_STANDARD_MAX_VALUE) {
    if (syntax_node->children) {
      if (syntax_node->children->alloc) {
        printf("child_count-%i\n", syntax_node->children->count);
        for (int i = 0; i < syntax_node->children->count; ++i) {

          release_syntax_node(syntax_node->children->items[i]);
        }

        free(syntax_node->children->items);
      }

      free(syntax_node->children);
    }
  }
  else {
    if (syntax_node->text) {
      free(syntax_node->text);
    }
  }

  switch (syntax_node->type) {
  case MC_SYNTAX_FUNCTION: {
    if (syntax_node->function.parameters) {
      if (syntax_node->function.parameters->alloc) {
        free(syntax_node->function.parameters->items);
      }

      free(syntax_node->function.parameters);
    }
  } break;
  case MC_SYNTAX_INVOCATION: {
    if (syntax_node->invocation.arguments) {
      if (syntax_node->invocation.arguments->alloc) {
        free(syntax_node->invocation.arguments->items);
      }

      free(syntax_node->invocation.arguments);
    }
  } break;
  default: {
    break;
  }
  }

  return 0;
}

int _mcs_parse_token(char *code, int *index, mc_token_type *token_type, char **text)
{
  switch (code[*index]) {
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
    *token_type = MC_TOKEN_OPEN_BRACKET;
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
    *token_type = MC_TOKEN_SQUARE_OPEN_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "[");
    }
    ++*index;
  } break;
  case ']': {
    *token_type = MC_TOKEN_SQUARE_CLOSE_BRACKET;
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

    *token_type = MC_TOKEN_NOT_OPERATOR;
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
    *token_type = MC_TOKEN_STAR_CHARACTER;
    if (text) {
      allocate_and_copy_cstr(*text, "*");
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

    *token_type = MC_TOKEN_BINARY_OR_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "|");
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
          // print_parse_error(code, *index, "_mcs_parse_token", "eof");
          MCerror(72, "Unexpected end-of-file\n");
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
      MCerror(729, "TODO");
    }
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

    *token_type = MC_TOKEN_LESS_THAN_OPERATOR;
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

    *token_type = MC_TOKEN_MORE_THAN_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, ">");
    }
    ++*index;
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
        if (slen == 4 && !strncmp(code + s, "else", slen)) {
          *token_type = MC_TOKEN_ELSE_KEYWORD;
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
        if (slen == 6 && !strncmp(code + s, "sizeof", slen)) {
          *token_type = MC_TOKEN_SIZEOF_KEYWORD;
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
        if (slen == 6 && !strncmp(code + s, "struct", slen)) {
          *token_type = MC_TOKEN_STRUCT_KEYWORD;
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
        if (slen == 4 && !strncmp(code + s, "bool", slen)) {
          *token_type = MC_TOKEN_BOOL_KEYWORD;
          if (text) {
            allocate_and_copy_cstrn(*text, code + s, slen);
          }
          break;
        }
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
      while (loop) {
        ++*index;
        if (isdigit(code[*index])) {
          prev_digit = true;
          continue;
        }

        // Other characters
        switch (code[*index]) {
        case '.': {
          if (!prev_digit) {
            prev_digit = false;
            continue;
          }
          MCerror(173, "Invalid Numeric Literal Format");
        }
        case '\n':
        case ']':
        case ' ':
        case ',':
        case ')':
        case ';': {
          loop = false;
        } break;
        default:
          MCerror(173, "Invalid Numeric Literal character:'%c'", code[*index]);
        }
      }

      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
      }
      break;
    }

    MCcall(print_parse_error(code, *index, "_mcs_parse_token", ""));
    MCerror(101, "");
  }
  }

  return 0;
}

int mcs_is_supernumerary_token(mc_token_type token_type, bool *is_supernumerary_token)
{
  switch (token_type) {
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
    MCcall(_mcs_parse_token(ps->code, &index, token_type, NULL));

    if (!include_supernumerary_tokens) {
      bool is_supernumerary_token;
      MCcall(mcs_is_supernumerary_token(*token_type, &is_supernumerary_token));
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
    MCcall(_mcs_parse_token(ps->code, token_end_index, token_type, NULL));

    if (!include_supernumerary_tokens) {
      bool is_supernumerary_token;
      MCcall(mcs_is_supernumerary_token(*token_type, &is_supernumerary_token));
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
  MCcall(_mcs_parse_token(ps->code, &ps->index, &type, &text));
  // printf("-%s-\n", text);

  MCcall(mcs_construct_syntax_node(ps, (mc_syntax_node_type)type, text, parent, token_result));

  // MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
  //                             *token_result));

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
  MCcall(mcs_parse_token(ps, parent, &token));

  mc_token_type was_type = (mc_token_type)token->type;

  if (expected_token != MC_TOKEN_NULL && was_type != expected_token) {
    MCcall(print_parse_error(ps->code, ps->index - strlen(token->text), "mcs_parse_through_token", ""));
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
    MCcall(mcs_peek_token_type(ps, true, 0, &token_type));
    // printf("sps->index:%i peek_type:%s\n", ps->index, get_mc_token_type_name(token_type));
    bool is_supernumerary_token;
    MCcall(mcs_is_supernumerary_token(token_type, &is_supernumerary_token));

    if (!is_supernumerary_token) {
      break;
    }

    // printf("sps->index:%i\n", ps->index);
    MCcall(mcs_parse_through_token(ps, parent, token_type, NULL));
  }

  // printf("sps->index:%i\n", ps->index);
  return 0;
}

int mcs_parse_type_identifier(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **type_identifier,
                              struct_info **mc_type)
{
  register_midge_error_tag("mcs_parse_type_identifier()");
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  mc_token_type modifier_type = MC_TOKEN_NULL;
  bool const_applied = false;
  mc_token_type token_type;
  MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
  if (token_type == MC_TOKEN_UNSIGNED_KEYWORD) {
    modifier_type = MC_TOKEN_UNSIGNED_KEYWORD;
    MCcall(mcs_peek_token_type(ps, false, 1, &token_type));
  }

  switch (token_type) {
  case MC_TOKEN_IDENTIFIER: {
    if (modifier_type != MC_TOKEN_NULL) {
      MCerror(1009, "modifier cannot be pre-applied to identifier");
    }
  } break;
  case MC_TOKEN_VOID_KEYWORD: {
    if (modifier_type != MC_TOKEN_NULL) {
      MCerror(1014, "modifier cannot be pre-applied to void");
    }
  } break;
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD: {
    // Valid
    break;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(808, "MCS:>:ERR-token:%s", get_mc_token_type_name(token_type));
  }
  }

  if (modifier_type != MC_TOKEN_NULL) {
    mc_syntax_node *modified_type;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_MODIFIED_TYPE, NULL, parent, &modified_type));
    if (type_identifier) {
      *type_identifier = modified_type;
    }

    MCcall(mcs_parse_through_token(ps, modified_type, modifier_type, &modified_type->modified_type.type_modifier));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, modified_type));
    MCcall(mcs_parse_through_token(ps, modified_type, token_type, &modified_type->modified_type.type_identifier));
    *mc_type = NULL;

    return 0;
  }

  MCcall(mcs_parse_through_token(ps, parent, token_type, type_identifier));

  register_midge_error_tag("mcs_parse_type_identifier()-4");
  // Convert to the appropriate type
  if (token_type == MC_TOKEN_IDENTIFIER) {
    void *vargs[3];
    vargs[0] = &command_hub->nodespace;
    vargs[1] = &(*type_identifier)->text;
    vargs[2] = &(*mc_type);
    find_struct_info(3, vargs);
    // printf("mcs: find_struct_info(%s)=='%s'\n", (*type_identifier)->text,
    //        (*mc_type) == NULL ? "(null)" : (*mc_type)->declared_mc_name);
  }
  else {
    *mc_type = NULL;
  }

  register_midge_error_tag("mcs_parse_type_identifier(~)");
  return 0;
}

int mcs_parse_dereference_sequence(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_dereference_sequence()");
  mc_syntax_node *sequence;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_DEREFERENCE_SEQUENCE, NULL, parent, &sequence));
  if (additional_destination) {
    *additional_destination = sequence;
  }

  while (1) {
    MCcall(mcs_parse_through_token(ps, sequence, MC_TOKEN_STAR_CHARACTER, NULL));
    ++sequence->dereference_sequence.count;

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 != MC_TOKEN_STAR_CHARACTER) {
      break;
    }

    MCcall(mcs_parse_through_supernumerary_tokens(ps, sequence));
  }

  return 0;
}

int mcs_parse_expression_variable_access(parsing_state *ps, mc_syntax_node *parent,
                                         mc_syntax_node **additional_destination)
{
  mc_token_type token0, token1;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_IDENTIFIER) {
    MCerror(857, "TODO:%s", get_mc_token_type_name(token0));
  }

  bool is_member_access = false;
  MCcall(mcs_peek_token_type(ps, false, 1, &token1));
  switch (token1) {
  case MC_TOKEN_DECIMAL_POINT:
  case MC_TOKEN_POINTER_OPERATOR: {
    is_member_access = true;
  } break;
  case MC_TOKEN_SEMI_COLON:
  case MC_TOKEN_COMMA:
  case MC_TOKEN_OPEN_BRACKET:
  case MC_TOKEN_CLOSING_BRACKET:
  case MC_TOKEN_SQUARE_CLOSE_BRACKET:
  case MC_TOKEN_ASSIGNMENT_OPERATOR:
  case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
  case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
  case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_LESS_THAN_OPERATOR:
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_MORE_THAN_OPERATOR:
  case MC_TOKEN_EQUALITY_OPERATOR:
  case MC_TOKEN_INEQUALITY_OPERATOR: {
    // End-of-line
    // Expected tokens that come at the end of a unary expression
    MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
  } break;
  case MC_TOKEN_SQUARE_OPEN_BRACKET: {
    // Element access
    mc_syntax_node *element_access;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION, NULL, parent, &element_access));
    if (additional_destination) {
      *additional_destination = element_access;
    }

    MCcall(mcs_parse_through_token(ps, element_access, MC_TOKEN_IDENTIFIER,
                                   &element_access->element_access_expression.primary));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, element_access));
    MCcall(mcs_parse_through_token(ps, element_access, MC_TOKEN_SQUARE_OPEN_BRACKET, NULL));
    MCcall(mcs_parse_expression(ps, element_access, &element_access->element_access_expression.access_expression));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, element_access));
    MCcall(mcs_parse_through_token(ps, element_access, MC_TOKEN_SQUARE_CLOSE_BRACKET, NULL));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(872, "MCS:>Unsupported-token:%s", get_mc_token_type_name(token1));
  }
  }

  if (is_member_access) {
    // Member-access
    mc_syntax_node *member_access;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_MEMBER_ACCESS_EXPRESSION, NULL, parent, &member_access));
    if (additional_destination) {
      *additional_destination = member_access;
    }

    MCcall(mcs_parse_through_token(ps, member_access, MC_TOKEN_IDENTIFIER,
                                   &member_access->member_access_expression.primary));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, member_access));
    MCcall(
        mcs_parse_through_token(ps, member_access, token1, &member_access->member_access_expression.access_operator));
    MCcall(
        mcs_parse_expression_variable_access(ps, member_access, &member_access->member_access_expression.identifier));
  }
  else {
    // Element-access
  }

  return 0;
}

// int mcs_parse_function_call(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
// {
//   // printf("mcs_parse_function_call()\n");
//   mc_syntax_node *statement;
//   MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION_STATEMENT, NULL, parent, &statement));
//   if (additional_destination) {
//     *additional_destination = statement;
//   }

//   MCcall(mcs_parse_expression(ps, statement, ))

//   MCcall(mcs_parse_expression_variable_access(ps, statement, &statement->invocation.function_identity));
//   MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

//   mc_function_info

//   MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
//   MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

//   while (1) {
//     mc_token_type token0;
//     MCcall(mcs_peek_token_type(ps, false, 0, &token0));

//     if (token0 == MC_TOKEN_CLOSING_BRACKET) {
//       break;
//     }

//     if (statement->invocation.arguments->count) {
//       MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_COMMA, NULL));
//       MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
//     }

//     mc_syntax_node *argument;
//     MCcall(mcs_parse_expression(ps, statement, &argument));
//     MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

//     MCcall(append_to_collection((void ***)&statement->invocation.arguments->items,
//                                 &statement->invocation.arguments->alloc, &statement->invocation.arguments->count,
//                                 argument));
//   }
//   MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));

//   // printf("mcs_parse_function_call(ret)\n");
//   return 0;
// }

int mcs_parse_cast_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  const int CASE_PRECEDENCE = 3;
  mc_syntax_node *cast_expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CAST_EXPRESSION, NULL, parent, &cast_expression));
  if (additional_destination) {
    *additional_destination = cast_expression;
  }

  MCcall(mcs_parse_through_token(ps, cast_expression, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));

  MCcall(mcs_parse_type_identifier(ps, cast_expression, &cast_expression->cast_expression.type_identifier,
                                   &cast_expression->cast_expression.mc_type));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    MCcall(mcs_parse_dereference_sequence(ps, cast_expression, &cast_expression->cast_expression.type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));
  }
  else {
    cast_expression->cast_expression.type_dereference = NULL;
  }

  MCcall(mcs_parse_through_token(ps, cast_expression, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));
  MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, cast_expression, &cast_expression->cast_expression.expression));

  return 0;
}

int mcs_parse_parenthesized_expression(parsing_state *ps, mc_syntax_node *parent,
                                       mc_syntax_node **additional_destination)
{
  // const int CASE_PRECEDENCE = 17;
  mc_syntax_node *parenthesized_expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PARENTHESIZED_EXPRESSION, NULL, parent, &parenthesized_expression));
  if (additional_destination) {
    *additional_destination = parenthesized_expression;
  }

  MCcall(mcs_parse_through_token(ps, parenthesized_expression, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, parenthesized_expression));

  MCcall(mcs_parse_expression(ps, parenthesized_expression,
                              &parenthesized_expression->parenthesized_expression.expression));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, parenthesized_expression));

  MCcall(mcs_parse_through_token(ps, parenthesized_expression, MC_TOKEN_CLOSING_BRACKET, NULL));

  return 0;
}

int mcs_parse_expression_beginning_with_bracket(parsing_state *ps, mc_syntax_node *parent,
                                                mc_syntax_node **additional_destination)
{
  // Determine expression type
  mc_token_type token_type;
  MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
  if (token_type != MC_TOKEN_OPEN_BRACKET) {
    MCerror(1527, "Flow format");
  }

  MCcall(mcs_peek_token_type(ps, false, 1, &token_type));
  switch (token_type) {
  case MC_TOKEN_STAR_CHARACTER:
  case MC_TOKEN_NUMERIC_LITERAL:
  case MC_TOKEN_CHAR_LITERAL: {
    MCcall(mcs_parse_parenthesized_expression(ps, parent, additional_destination));
  } break;
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_VOID_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD: {
    // Cast
    MCcall(mcs_parse_cast_expression(ps, parent, additional_destination));
  } break;
  case MC_TOKEN_IDENTIFIER: {
    MCcall(mcs_peek_token_type(ps, false, 2, &token_type));
    switch (token_type) {
    case MC_TOKEN_CLOSING_BRACKET: {
      // See whats after
      MCcall(mcs_peek_token_type(ps, false, 3, &token_type));
      switch (token_type) {
      case MC_TOKEN_IDENTIFIER: {
        // Cast
        MCcall(mcs_parse_cast_expression(ps, parent, additional_destination));
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(1648, "MCS:MC_TOKEN_IDENTIFIER>CLOSING_BRACKET>Unsupported-token:%s",
                get_mc_token_type_name(token_type));
      }
      }
    } break;
    case MC_TOKEN_OPEN_BRACKET:
    case MC_TOKEN_SQUARE_OPEN_BRACKET:
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT:
    case MC_TOKEN_PLUS_OPERATOR:
    case MC_TOKEN_SUBTRACT_OPERATOR:
    case MC_TOKEN_MODULO_OPERATOR:
    case MC_TOKEN_DIVIDE_OPERATOR: {
      MCcall(mcs_parse_parenthesized_expression(ps, parent, additional_destination));
    } break;
    case MC_TOKEN_STAR_CHARACTER: {
      int peek_ahead = 2 + 1;
      do {
        MCcall(mcs_peek_token_type(ps, false, peek_ahead++, &token_type));
      } while (token_type == MC_TOKEN_STAR_CHARACTER);
      switch (token_type) {
      case MC_TOKEN_CLOSING_BRACKET: {
        // Cast
        MCcall(mcs_parse_cast_expression(ps, parent, additional_destination));
        // printf("cast-expression:\n");
        // MCcall(print_syntax_node(*additional_destination, 0));
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
      MCerror(1563, "MCS:MC_TOKEN_IDENTIFIER>Unsupported-token:%s", get_mc_token_type_name(token_type));
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
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_DECLARATION, NULL, parent, &local_declaration));
  if (additional_destination) {
    *additional_destination = local_declaration;
  }

  // Type
  MCcall(mcs_parse_type_identifier(ps, local_declaration,
                                   &local_declaration->local_variable_declaration.type_identifier,
                                   &local_declaration->local_variable_declaration.mc_type));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration));

  // Declarators
  while (1) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));

    // Comma
    if (local_declaration->local_variable_declaration.declarators->count) {
      MCcall(mcs_parse_through_token(ps, local_declaration, MC_TOKEN_COMMA, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration));
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    }

    // Expectation
    if (token0 != MC_TOKEN_IDENTIFIER && token0 != MC_TOKEN_STAR_CHARACTER) {
      MCerror(2462, "Expected Local Variable Declarator. was:%s", get_mc_token_type_name(token0));
    }

    mc_syntax_node *declarator;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR, NULL, local_declaration, &declarator));
    MCcall(append_to_collection((void ***)&local_declaration->local_variable_declaration.declarators->items,
                                &local_declaration->local_variable_declaration.declarators->alloc,
                                &local_declaration->local_variable_declaration.declarators->count, declarator));

    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      MCcall(mcs_parse_dereference_sequence(ps, declarator, &declarator->local_variable_declarator.type_dereference));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, declarator));
    }
    else {
      declarator->local_variable_declarator.type_dereference = NULL;
    }

    MCcall(mcs_parse_through_token(ps, declarator, MC_TOKEN_IDENTIFIER,
                                   &declarator->local_variable_declarator.variable_name));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    bool end_loop = false;
    switch (token0) {
    case MC_TOKEN_SEMI_COLON: {
      end_loop = true;
    } break;
    case MC_TOKEN_COMMA: {
      // Do Nothing more
    } break;
    case MC_TOKEN_ASSIGNMENT_OPERATOR: {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, declarator));

      mc_syntax_node *assignment_initializer;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER, NULL, declarator,
                                       &assignment_initializer));
      declarator->local_variable_declarator.initializer = assignment_initializer;

      MCcall(mcs_parse_through_token(ps, assignment_initializer, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, assignment_initializer));

      MCcall(mcs_parse_expression(ps, assignment_initializer,
                                  &assignment_initializer->local_variable_assignment_initializer.value_expression));
    } break;
    case MC_TOKEN_SQUARE_OPEN_BRACKET: {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, declarator));

      mc_syntax_node *array_initializer;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER, NULL, declarator,
                                       &array_initializer));
      declarator->local_variable_declarator.initializer = array_initializer;

      // [  size  ]
      MCcall(mcs_parse_through_token(ps, array_initializer, MC_TOKEN_SQUARE_OPEN_BRACKET, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, array_initializer));

      MCcall(mcs_parse_expression(ps, array_initializer,
                                  &array_initializer->local_variable_array_initializer.size_expression));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, array_initializer));

      MCcall(mcs_parse_through_token(ps, array_initializer, MC_TOKEN_SQUARE_CLOSE_BRACKET, NULL));
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(2487, "MCS:ERR-token:%s", get_mc_token_type_name(token0));
    }
    }

    if (!end_loop) {
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));

      if (token0 != MC_TOKEN_SEMI_COLON) {
        continue;
      }
    }

    break;
  }

  return 0;
}

// int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
// {
//   // Determine expression type
//   mc_token_type token0;
//   MCcall(mcs_peek_token_type(ps, false, 0, &token0));
//   switch (token0) {
//   case MC_TOKEN_STRING_LITERAL: {
//     mc_syntax_node *string_literal;
//     MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_STRING_LITERAL_EXPRESSION, NULL, parent, &string_literal));
//     if (additional_destination) {
//       *additional_destination = string_literal;
//     }

//     MCcall(mcs_parse_through_token(ps, string_literal, MC_TOKEN_STRING_LITERAL, NULL));

//     while (1) {
//       MCcall(mcs_peek_token_type(ps, false, 0, &token0));
//       if (token0 != MC_TOKEN_STRING_LITERAL) {
//         break;
//       }

//       MCcall(mcs_parse_through_supernumerary_tokens(ps, string_literal));
//       MCcall(mcs_parse_through_token(ps, string_literal, MC_TOKEN_STRING_LITERAL, NULL));
//     }
//   } break;
//   case MC_TOKEN_SUBTRACT_OPERATOR:
//   case MC_TOKEN_PLUS_OPERATOR:
//   case MC_TOKEN_NOT_OPERATOR:
//   case MC_TOKEN_AMPERSAND_CHARACTER: {
//     mc_syntax_node *prepended_unary;
//     MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PREPENDED_UNARY_EXPRESSION, NULL, parent, &prepended_unary));
//     if (additional_destination) {
//       *additional_destination = prepended_unary;
//     }

//     MCcall(mcs_parse_through_token(ps, prepended_unary, token0, &prepended_unary->prepended_unary.prepend_operator));
//     MCcall(mcs_parse_through_supernumerary_tokens(ps, prepended_unary));
//     MCcall(mcs_parse_expression_unary(ps, prepended_unary, &prepended_unary->prepended_unary.unary_expression));
//   } break;
//   case MC_TOKEN_IDENTIFIER: {
//     mc_token_type token1;
//     MCcall(mcs_peek_token_type(ps, false, 1, &token1));
//     switch (token1) {
//     case MC_TOKEN_SQUARE_OPEN_BRACKET:
//     case MC_TOKEN_POINTER_OPERATOR:
//     case MC_TOKEN_DECIMAL_POINT: {
//       MCcall(mcs_parse_expression_variable_access(ps, parent, additional_destination));
//     } break;
//     case MC_TOKEN_SEMI_COLON:
//     case MC_TOKEN_COMMA:
//     case MC_TOKEN_SQUARE_CLOSE_BRACKET:
//     case MC_TOKEN_CLOSING_BRACKET: {
//       MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
//     } break;
//     case MC_TOKEN_OPEN_BRACKET: {
//       MCcall(mcs_parse_function_call(ps, parent, additional_destination));
//     } break;
//     case MC_TOKEN_PLUS_OPERATOR:
//     case MC_TOKEN_SUBTRACT_OPERATOR:
//     case MC_TOKEN_DIVIDE_OPERATOR:
//     case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
//     case MC_TOKEN_LESS_THAN_OPERATOR:
//     case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
//     case MC_TOKEN_MORE_THAN_OPERATOR:
//     case MC_TOKEN_EQUALITY_OPERATOR:
//     case MC_TOKEN_INEQUALITY_OPERATOR: {
//       MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
//       return 0;
//     } break;
//     default: {
//       print_parse_error(ps->code, ps->index, "see-below", "");
//       MCerror(282, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
//     }
//     }
//   } break;
//   case MC_TOKEN_NUMERIC_LITERAL:
//   case MC_TOKEN_CHAR_LITERAL: {
//     MCcall(mcs_parse_through_token(ps, parent, token0, additional_destination));
//   } break;
//   case MC_TOKEN_OPEN_BRACKET: {
//     // Determine if paranthesized expression or cast expression
//     MCcall(mcs_parse_expression_beginning_with_bracket(ps, parent, additional_destination));
//   } break;
//   default: {
//     print_parse_error(ps->code, ps->index, "see-below", "");
//     MCerror(287, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
//   }
//   }

//   return 0;
// }

int mcs_get_token_precedence(mc_token_type token_type, int *scope_level)
{
  switch (token_type) {
  case MC_TOKEN_SUBTRACT_OPERATOR:
  case MC_TOKEN_PLUS_OPERATOR: {
    *scope_level = 6;
  } break;
  case MC_TOKEN_MODULO_OPERATOR:
  case MC_TOKEN_DIVIDE_OPERATOR:
  case MC_TOKEN_MULTIPLY_OPERATOR: {
    *scope_level = 5;
  } break;
  case MC_TOKEN_SEMI_COLON: {
    *scope_level = 18;
  } break;
  default: {
    MCerror(1524, "mcs_get_token_scope_priority:Unsupported-token:%s", get_mc_token_type_name(token_type));
  }
  }

  return 0;
}

int _mcs_parse_expression(parsing_state *ps, int allowable_precedence, mc_syntax_node *parent,
                          mc_syntax_node **additional_destination)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  mc_syntax_node *left;

  // Determine left
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_INT_KEYWORD: {
    // Declaration expression
    MCcall(mcs_parse_local_declaration(ps, parent, additional_destination));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 != MC_TOKEN_SEMI_COLON) {
      MCerror(1753, "TODO");
    }

    printf("was %p\n", *additional_destination);

    return 0;
  }
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_CHAR_LITERAL:
  case MC_TOKEN_NUMERIC_LITERAL: {
    MCcall(mcs_parse_through_token(ps, NULL, token0, &left));
  } break;
  case MC_TOKEN_OPEN_BRACKET: {
    // printf("mpe-open[2]\n");
    MCcall(mcs_parse_expression_beginning_with_bracket(ps, NULL, &left));
  } break;
  case MC_TOKEN_STRING_LITERAL: {
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_STRING_LITERAL_EXPRESSION, NULL, NULL, &left));

    MCcall(mcs_parse_through_token(ps, left, MC_TOKEN_STRING_LITERAL, NULL));

    while (1) {
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));
      if (token0 != MC_TOKEN_STRING_LITERAL) {
        break;
      }

      MCcall(mcs_parse_through_supernumerary_tokens(ps, left));
      MCcall(mcs_parse_through_token(ps, left, MC_TOKEN_STRING_LITERAL, NULL));
    }
  } break;
  case MC_TOKEN_STAR_CHARACTER: {
    const int CASE_PRECEDENCE = 3;

    mc_token_type token1;
    MCcall(mcs_peek_token_type(ps, false, 1, &token1));
    if (token1 != MC_TOKEN_IDENTIFIER) {
      MCerror(1979, "TODO ? %s", get_mc_syntax_token_type_name((mc_syntax_node_type)token1));
    }

    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_DEREFERENCE_EXPRESSION, NULL, NULL, &left));
    if (additional_destination) {
      *additional_destination = left;
    }

    MCcall(mcs_parse_dereference_sequence(ps, left, &left->dereference_expression.deref_sequence));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, left));

    MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, left, &left->dereference_expression.unary_expression));
  } break;
  case MC_TOKEN_DECREMENT_OPERATOR:
  case MC_TOKEN_INCREMENT_OPERATOR: {
    mc_syntax_node *expression;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, NULL, parent, &expression));
    if (additional_destination) {
      *additional_destination = expression;
    }
    expression->fixrement_expression.is_postfix = false;

    MCcall(mcs_parse_through_token(ps, expression, token0, &expression->fixrement_expression.fix_operator));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
    MCcall(_mcs_parse_expression(ps, 3, expression, &expression->fixrement_expression.primary));
    return 0;
  }
  case MC_TOKEN_NOT_OPERATOR:
  case MC_TOKEN_SUBTRACT_OPERATOR:
  case MC_TOKEN_PLUS_OPERATOR:
  case MC_TOKEN_AMPERSAND_CHARACTER: {
    mc_syntax_node *expression;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PREPENDED_UNARY_EXPRESSION, NULL, parent, &expression));
    if (additional_destination) {
      *additional_destination = expression;
    }

    MCcall(mcs_parse_through_token(ps, expression, token0, &expression->prepended_unary.prepend_operator));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
    MCcall(_mcs_parse_expression(ps, 3, expression, &expression->prepended_unary.unary_expression));

    return 0;
  }
  case MC_TOKEN_SIZEOF_KEYWORD: {
    // Cast
    const int CASE_PRECEDENCE = 3;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_SIZEOF_EXPRESSION, NULL, NULL, &left));
    if (additional_destination) {
      *additional_destination = left;
    }

    MCcall(mcs_parse_through_token(ps, left, MC_TOKEN_SIZEOF_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, left));

    MCcall(mcs_parse_through_token(ps, left, MC_TOKEN_OPEN_BRACKET, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, left));

    MCcall(mcs_parse_type_identifier(ps, left, &left->sizeof_expression.type_identifier,
                                     &left->sizeof_expression.mc_type));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, left));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      MCcall(mcs_parse_dereference_sequence(ps, left, &left->sizeof_expression.type_dereference));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, left));
    }
    else {
      left->sizeof_expression.type_dereference = NULL;
    }

    MCcall(mcs_parse_through_token(ps, left, MC_TOKEN_CLOSING_BRACKET, NULL));

    // printf("sizeof: %i\n", left->children->count);
    // print_syntax_node(left, 2);
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1798, "MCS:Unsupported-token:%s allowable_precedence:%i", get_mc_token_type_name(token0),
            allowable_precedence);
  }
  }

  // Middle / Postfix
  while (1) {
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    case MC_TOKEN_DECREMENT_OPERATOR:
    case MC_TOKEN_INCREMENT_OPERATOR: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, NULL, NULL, &expression));
      expression->fixrement_expression.is_postfix = true;

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->fixrement_expression.primary = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->fixrement_expression.fix_operator));

      left = expression;
    } break;
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence < CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_MEMBER_ACCESS_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->member_access_expression.primary = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->member_access_expression.access_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->member_access_expression.identifier));

      left = expression;
    } break;
    case MC_TOKEN_SQUARE_OPEN_BRACKET: {
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->element_access_expression.primary = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, NULL));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_expression(ps, expression, &expression->element_access_expression.access_expression));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_SQUARE_CLOSE_BRACKET, NULL));

      left = expression;
    } break;
    case MC_TOKEN_OPEN_BRACKET: {
      // printf("mpe-open[2]\n");
      const int CASE_PRECEDENCE = 2;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_INVOCATION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->invocation.function_identity = left;
      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));

      {
        if ((mc_token_type)expression->invocation.function_identity->type != MC_TOKEN_IDENTIFIER) {
          MCerror(2100, "TODO");
        }

        // Check if the function info exists in midge
        void *mc_vargs[3];
        mc_vargs[0] = (void *)&expression->invocation.mc_function_info;
        mc_vargs[1] = (void *)&command_hub->global_node; // TODO -- from state?
        mc_vargs[2] = (void *)&expression->invocation.function_identity->text;
        MCcall(find_function_info(3, mc_vargs));
      }

      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_OPEN_BRACKET, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));

      while (1) {
        MCcall(mcs_peek_token_type(ps, false, 0, &token0));

        if (token0 == MC_TOKEN_CLOSING_BRACKET) {
          break;
        }

        if (expression->invocation.arguments->count) {
          MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_COMMA, NULL));
          MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
        }

        mc_syntax_node *argument;
        MCcall(mcs_parse_expression(ps, expression, &argument));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));

        MCcall(append_to_collection((void ***)&expression->invocation.arguments->items,
                                    &expression->invocation.arguments->alloc, &expression->invocation.arguments->count,
                                    argument));
      }
      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_CLOSING_BRACKET, NULL));

      left = expression;
    } break;
    case MC_TOKEN_MODULO_OPERATOR:
    case MC_TOKEN_STAR_CHARACTER:
    case MC_TOKEN_DIVIDE_OPERATOR: {
      const int CASE_PRECEDENCE = 5;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_OPERATIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->operational_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->operational_expression.operational_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->operational_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_SUBTRACT_OPERATOR:
    case MC_TOKEN_PLUS_OPERATOR: {
      const int CASE_PRECEDENCE = 6;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_OPERATIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->operational_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->operational_expression.operational_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->operational_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_LESS_THAN_OPERATOR:
    case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_MORE_THAN_OPERATOR:
    case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR: {
      const int CASE_PRECEDENCE = 9;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_RELATIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->relational_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->relational_expression.relational_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->relational_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_LOGICAL_AND_OPERATOR: {
      const int CASE_PRECEDENCE = 14;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->conditional_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->conditional_expression.conditional_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->conditional_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_LOGICAL_OR_OPERATOR: {
      const int CASE_PRECEDENCE = 15;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        printf("||:allowable_precedence:%i\n", allowable_precedence);
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->conditional_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->conditional_expression.conditional_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->conditional_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_EQUALITY_OPERATOR:
    case MC_TOKEN_INEQUALITY_OPERATOR: {
      const int CASE_PRECEDENCE = 10;
      if (allowable_precedence <= CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->conditional_expression.left = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->conditional_expression.conditional_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(_mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->conditional_expression.right));

      left = expression;
    } break;
    case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_MODULO_AND_ASSIGN_OPERATOR:
    case MC_TOKEN_ASSIGNMENT_OPERATOR: {
      const int CASE_PRECEDENCE = 16;
      if (allowable_precedence < CASE_PRECEDENCE) {
        // Left is it
        MCcall(mcs_add_syntax_node_to_parent(parent, left));
        if (additional_destination) {
          *additional_destination = left;
        }
        return 0;
      }

      mc_syntax_node *expression = NULL;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ASSIGNMENT_EXPRESSION, NULL, NULL, &expression));

      MCcall(mcs_add_syntax_node_to_parent(expression, left));
      expression->assignment_expression.variable = left;

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, token0, &expression->assignment_expression.assignment_operator));

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(
          _mcs_parse_expression(ps, CASE_PRECEDENCE, expression, &expression->assignment_expression.value_expression));

      left = expression;
    } break;
    case MC_TOKEN_SEMI_COLON:
    case MC_TOKEN_COMMA:
    case MC_TOKEN_CLOSING_BRACKET:
    case MC_TOKEN_SQUARE_CLOSE_BRACKET: {
      // Left is it
      MCcall(mcs_add_syntax_node_to_parent(parent, left));
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
  MCcall(_mcs_parse_expression(ps, 16, parent, additional_destination));

  // printf("Conditional-Expression!:\n");
  // MCcall(print_syntax_node(*additional_destination, 1));
  return 0;
}

int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // Find the end of the expression

  // MCcall(mcs_find_precedent_operator(ps, 0, -1, 18))

  //   mc_token_type lpo_token_type;
  //   int lpo_peek_loc, lpo_peek_loc2;

  MCcall(_mcs_parse_expression(ps, 17, parent, additional_destination));

  // printf("Expression!:\n");
  if (!*additional_destination) {
    MCerror(2258, "expression was null");
  }
  // MCcall(print_syntax_node(*additional_destination, 1));
  // MCcall(_mcs_obtain_precedence_information(ps, 0, -1, &lpo_token_type, &lpo_peek_loc, &lpo_peek_loc2));

  // mc_syntax_node *expression;
  // MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION, parent, &expression));
  // if (additional_destination) {
  //   *additional_destination = expression;
  // }

  // MCcall(mcs_parse_expression_conditional(ps, parent, additional_destination));

  return 0;
}

int mcs_parse_return_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *return_statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_RETURN_STATEMENT, NULL, parent, &return_statement));
  if (additional_destination) {
    *additional_destination = return_statement;
  }

  MCcall(mcs_parse_through_token(ps, return_statement, MC_TOKEN_RETURN_KEYWORD, NULL));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_SEMI_COLON) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, return_statement));
    MCcall(mcs_parse_expression(ps, return_statement, &return_statement->return_statement.expression));
  }
  else {
    return_statement->return_statement.expression = NULL;
  }

  MCcall(mcs_parse_through_supernumerary_tokens(ps, return_statement));
  MCcall(mcs_parse_through_token(ps, return_statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_local_declaration_statement(parsing_state *ps, mc_syntax_node *parent,
                                          mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_DECLARATION_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_local_declaration(ps, statement, &statement->declaration_statement.declaration));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  // printf("local_declaration:\n");
  // print_syntax_node(local_declaration, 1);

  return 0;
}

int mcs_parse_for_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FOR_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_FOR_KEYWORD, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  // Initialization
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_INT_KEYWORD: {
    MCcall(mcs_parse_expression(ps, statement, &statement->for_statement.initialization));
  } break;
  case MC_TOKEN_SEMI_COLON: {
    statement->for_statement.initialization = NULL;
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(873, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  // Conditional
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_IDENTIFIER: {
    MCcall(mcs_parse_expression_conditional(ps, statement, &statement->for_statement.conditional));
  } break;
  case MC_TOKEN_SEMI_COLON: {
    statement->for_statement.conditional = NULL;
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(882, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  // UpdateFix Expression
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_CLOSING_BRACKET: {
  } break;
  case MC_TOKEN_INCREMENT_OPERATOR:
  case MC_TOKEN_DECREMENT_OPERATOR: {
    MCcall(mcs_parse_expression(ps, statement, &statement->for_statement.fix_expression));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(891, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_CURLY_OPENING_BRACKET) {
    MCerror(876, "TODO single statement");
  }

  MCcall(mcs_parse_code_block(ps, statement, &statement->for_statement.code_block));

  return 0;
}

int mcs_parse_while_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_WHILE_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_DO_KEYWORD) {
    // do { } while (condition);
    statement->while_statement.do_first = true;

    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_DO_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 != MC_TOKEN_CURLY_OPENING_BRACKET) {
      MCerror(876, "TODO single statement");
    }
    MCcall(mcs_parse_code_block(ps, statement, &statement->while_statement.code_block));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_WHILE_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_expression_conditional(ps, statement, &statement->while_statement.conditional));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
    return 0;
  }

  // while (condition) { }
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_WHILE_KEYWORD, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_expression_conditional(ps, statement, &statement->while_statement.conditional));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_code_block(ps, statement, &statement->while_statement.code_block));

  return 0;
}

int mcs_parse_switch_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *switch_statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_STATEMENT, NULL, parent, &switch_statement));
  if (additional_destination) {
    *additional_destination = switch_statement;
  }

  // printf("mpss-0\n");
  MCcall(mcs_parse_through_token(ps, switch_statement, MC_TOKEN_SWITCH_KEYWORD, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));
  MCcall(mcs_parse_through_token(ps, switch_statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));
  MCcall(mcs_parse_expression_conditional(ps, switch_statement, &switch_statement->switch_statement.conditional));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));
  MCcall(mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));

  // printf("mpss-1\n");
  MCcall(mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CURLY_OPENING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));

  mc_token_type token0;
  mc_syntax_node *switch_section = NULL;
  while (1) {
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_CURLY_CLOSING_BRACKET) {
      break;
    }
    // printf("mpss-2\n");

    if (switch_section == NULL) {
      // Construct section
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_SECTION, NULL, switch_statement, &switch_section));
    }

    if (token0 == MC_TOKEN_CASE_KEYWORD) {
      // printf("mpss-3\n");
      mc_syntax_node *case_label;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_CASE_LABEL, NULL, switch_section, &case_label));
      MCcall(mcs_parse_through_token(ps, case_label, MC_TOKEN_CASE_KEYWORD, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, case_label));

      // printf("mpss-3a\n");
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));
      switch (token0) {
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_CHAR_LITERAL:
      case MC_TOKEN_NUMERIC_LITERAL: {
        MCcall(mcs_parse_through_token(ps, case_label, token0, &case_label->switch_case_label.constant));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, case_label));
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(2694, "MCS:>CASE_CONSTANT:ERR-token:%s", get_mc_token_type_name(token0));
      }
      }

      // printf("mpss-3b\n");
      MCcall(mcs_parse_through_token(ps, case_label, MC_TOKEN_COLON, NULL));

      MCcall(append_to_collection((void ***)&switch_section->switch_section.labels->items,
                                  &switch_section->switch_section.labels->alloc,
                                  &switch_section->switch_section.labels->count, case_label));
      // printf("mpss-3c\n");
    }
    else if (token0 == MC_TOKEN_DEFAULT_KEYWORD) {
      // printf("mpss-4\n");
      mc_syntax_node *default_label;
      MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_SWITCH_DEFAULT_LABEL, NULL, switch_section, &default_label));
      MCcall(mcs_parse_through_token(ps, default_label, MC_TOKEN_DEFAULT_KEYWORD, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, default_label));

      MCcall(mcs_parse_through_token(ps, default_label, MC_TOKEN_COLON, NULL));

      MCcall(append_to_collection((void ***)&switch_section->switch_section.labels->items,
                                  &switch_section->switch_section.labels->alloc,
                                  &switch_section->switch_section.labels->count, default_label));
    }
    else {
      // printf("mpss-5\n");
      MCcall(mcs_parse_statement_list(ps, switch_section, &switch_section->switch_section.statement_list));
      // printf("~switch_section\n");

      MCcall(append_to_collection((void ***)&switch_statement->switch_statement.sections->items,
                                  &switch_statement->switch_statement.sections->alloc,
                                  &switch_statement->switch_statement.sections->count, switch_section));
      switch_section = NULL;
      // printf("~switch_section2\n");
    }

    // printf("mpss-6\n");
    if (switch_section) {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_section));
    }
    else {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, switch_statement));
    }
    // printf("mpss-7\n");
  }

  MCcall(mcs_parse_through_token(ps, switch_statement, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL));
  // printf("mpss-8\n");

  return 0;
}

int mcs_parse_if_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_IF_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_IF_KEYWORD, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_expression_conditional(ps, statement, &statement->if_statement.conditional));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_CURLY_OPENING_BRACKET) {
    MCerror(1449, "TODO single statement");
  }

  MCcall(mcs_parse_code_block(ps, statement, &statement->if_statement.code_block));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_ELSE_KEYWORD) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ELSE_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_IF_KEYWORD) {
      MCcall(mcs_parse_if_statement(ps, statement, &statement->if_statement.else_continuance));
    }
    else if (token0 == MC_TOKEN_CURLY_OPENING_BRACKET) {
      MCcall(mcs_parse_code_block(ps, statement, &statement->if_statement.else_continuance));
    }
    else {
      MCerror(1618, "TODO");
    }
  }
  else {
    statement->if_statement.else_continuance = NULL;
  }

  return 0;
}

int mcs_parse_expression_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION_STATEMENT, NULL, parent, &statement));

  MCcall(mcs_parse_expression(ps, statement, &statement->expression_statement.expression));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_simple_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_CONTINUE_KEYWORD: {
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONTINUE_STATEMENT, NULL, parent, &statement));

    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CONTINUE_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  } break;
  case MC_TOKEN_BREAK_KEYWORD: {
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_BREAK_STATEMENT, NULL, parent, &statement));

    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_BREAK_KEYWORD, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(2613, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_statement_list(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement_list_node;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_STATEMENT_LIST, NULL, parent, &statement_list_node));
  if (additional_destination) {
    *additional_destination = statement_list_node;
  }

  bool loop = true;
  while (loop) {
    // printf("ps->index:%i\n", ps->index);

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));

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

    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement_list_node));

    mc_syntax_node *statement;
    switch (token0) {
    case MC_TOKEN_CURLY_OPENING_BRACKET: {
      MCcall(mcs_parse_code_block(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_FOR_KEYWORD: {
      MCcall(mcs_parse_for_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_IF_KEYWORD: {
      MCcall(mcs_parse_if_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_SWITCH_KEYWORD: {
      MCcall(mcs_parse_switch_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_DO_KEYWORD:
    case MC_TOKEN_WHILE_KEYWORD: {
      MCcall(mcs_parse_while_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_BREAK_KEYWORD:
    case MC_TOKEN_CONTINUE_KEYWORD: {
      MCcall(mcs_parse_simple_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_RETURN_KEYWORD: {
      MCcall(mcs_parse_return_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_OPEN_BRACKET:
    case MC_TOKEN_STAR_CHARACTER:
    case MC_TOKEN_INCREMENT_OPERATOR:
    case MC_TOKEN_DECREMENT_OPERATOR: {
      MCcall(mcs_parse_expression_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_INT_KEYWORD:
    case MC_TOKEN_CHAR_KEYWORD:
    case MC_TOKEN_BOOL_KEYWORD:
    case MC_TOKEN_LONG_KEYWORD:
    case MC_TOKEN_FLOAT_KEYWORD:
    case MC_TOKEN_VOID_KEYWORD:
    case MC_TOKEN_UNSIGNED_KEYWORD: {
      // Some Sort of Declarative Statement
      MCcall(mcs_parse_local_declaration_statement(ps, statement_list_node, &statement));
    } break;
    case MC_TOKEN_IDENTIFIER: {
      mc_token_type token1;
      MCcall(mcs_peek_token_type(ps, false, 1, &token1));
      // printf("token1:%s\n", get_mc_syntax_token_type_name((mc_syntax_node_type)token1));
      switch (token1) {
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_STAR_CHARACTER: {
        // Some Sort of Declarative Statement
        MCcall(mcs_parse_local_declaration_statement(ps, statement_list_node, &statement));
      } break;
      case MC_TOKEN_OPEN_BRACKET: {
        // A standalone function call statement
        MCcall(mcs_parse_expression_statement(ps, statement_list_node, &statement));
      } break;
      case MC_TOKEN_SQUARE_OPEN_BRACKET:
      case MC_TOKEN_ASSIGNMENT_OPERATOR: {
        MCcall(mcs_parse_expression_statement(ps, statement_list_node, &statement));
      } break;
      case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
      case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR: {
        MCcall(mcs_parse_expression_statement(ps, statement_list_node, &statement));
      } break;
      case MC_TOKEN_POINTER_OPERATOR:
      case MC_TOKEN_DECIMAL_POINT: {
        // Some sort of member-access statement
        MCcall(mcs_parse_expression_statement(ps, statement_list_node, &statement));

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

    // printf("token0:%s statement:%p\n", get_mc_syntax_token_type_name((mc_syntax_node_type)token0), statement);
    MCcall(append_to_collection((void ***)&statement_list_node->statement_list.statements->items,
                                &statement_list_node->statement_list.statements->alloc,
                                &statement_list_node->statement_list.statements->count, statement));
  }

  // printf("~mcs_parse_statement_list()\n");
  return 0;
}

int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  register_midge_error_tag("mcs_parse_code_block()");
  mc_syntax_node *block_node;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_BLOCK, NULL, parent, &block_node));
  if (additional_destination) {
    *additional_destination = block_node;
  }

  // printf("ps->index:%i\n", ps->index);
  MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_CURLY_OPENING_BRACKET, NULL));
  // MCcall(print_parse_error(ps->code, ps->index - 1, "parse-code-block", "opening"));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));

  MCcall(mcs_parse_statement_list(ps, block_node, &block_node->block_node.statement_list));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));

  MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL));
  // MCcall(print_parse_error(ps->code, ps->index - 1, "parse-code-block", "closing"));

  // printf("~mcs_parse_code_block()\n");
  return 0;
}

int parse_mc_to_syntax_tree_v1(char *mcode, mc_syntax_node **function_ast, bool allow_imperfect_parse)
{
  register_midge_error_tag("parse_mc_to_syntax_tree_v1()");
  // printf("mc_syntax_node:%zu\n", sizeof(mc_syntax_node));
  parsing_state ps;
  ps.code = mcode;
  ps.allow_imperfect_parse = allow_imperfect_parse;
  ps.index = 0;
  ps.line = 0;
  ps.col = 0;

  mc_syntax_node *function;
  MCcall(mcs_construct_syntax_node(&ps, MC_SYNTAX_FUNCTION, NULL, NULL, &function));

  // MCcall(print_syntax_node(function, 0));

  mc_token_type token0;
  MCcall(mcs_parse_type_identifier(&ps, function, &function->function.return_type_identifier,
                                   &function->function.return_mc_type));

  register_midge_error_tag("parse_mc_to_syntax_tree_v1()-2");
  // MCcall(print_syntax_node(function, 0));
  MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

  // MCcall(print_syntax_node(function, 0));

  MCcall(mcs_peek_token_type(&ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    MCcall(mcs_parse_dereference_sequence(&ps, function, &function->function.return_type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));
  }
  else {
    function->function.return_type_dereference = NULL;
  }

  // MCcall(print_syntax_node(function, 0));

  MCcall(mcs_parse_through_token(&ps, function, MC_TOKEN_IDENTIFIER, &function->function.name));
  MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

  MCcall(mcs_parse_through_token(&ps, function, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

  while (1) {
    MCcall(mcs_peek_token_type(&ps, false, 0, &token0));
    if (token0 == MC_TOKEN_CLOSING_BRACKET) {
      break;
    }

    // Comma
    if (function->function.parameters->count) {
      MCcall(mcs_parse_through_token(&ps, function, MC_TOKEN_COMMA, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));
    }

    // Parse the parameter
    mc_syntax_node *parameter;
    MCcall(mcs_construct_syntax_node(&ps, MC_SYNTAX_PARAMETER_DECLARATION, NULL, function, &parameter));

    MCcall(
        mcs_parse_type_identifier(&ps, function, &parameter->parameter.type_identifier, &parameter->parameter.mc_type));
    MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

    MCcall(mcs_peek_token_type(&ps, false, 0, &token0));
    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      MCcall(mcs_parse_dereference_sequence(&ps, function, &parameter->parameter.type_dereference));
      MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));
    }
    else {
      parameter->parameter.type_dereference = NULL;
    }

    MCcall(mcs_parse_through_token(&ps, function, MC_TOKEN_IDENTIFIER, &parameter->parameter.name));
    MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

    MCcall(append_to_collection((void ***)&function->function.parameters->items, &function->function.parameters->alloc,
                                &function->function.parameters->count, parameter));
  }

  MCcall(mcs_parse_through_token(&ps, function, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(&ps, function));

  // TODO -- memory isn't cleared if this fails, and if this fails it is handled higher up. so memory is never cleared
  MCcall(mcs_parse_code_block(&ps, function, &function->function.code_block));

  *function_ast = function;
  // MCcall(print_syntax_node(function, 0));
  register_midge_error_tag("parse_mc_to_syntax_tree_v1(~)");
  return 0;
}