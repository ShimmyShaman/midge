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

} parsing_state;

int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);

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
  case MC_TOKEN_CURLY_OPEN_BRACKET:
    return "MC_TOKEN_CURLY_OPEN_BRACKET";
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
  default:
    return "TODO_ENCODE_THIS_TYPE_OR_UNSUPPORTED";
  }
}

const char *get_mc_syntax_token_type_name(mc_syntax_node_type type)
{
  switch ((mc_syntax_node_type)type) {
  case MC_SYNTAX_FUNCTION:
    return "MC_SYNTAX_FUNCTION";
  case MC_SYNTAX_BLOCK:
    return "MC_SYNTAX_BLOCK";
  case MC_SYNTAX_FOR_STATEMENT:
    return "MC_SYNTAX_FOR_STATEMENT";
  case MC_SYNTAX_WHILE_STATEMENT:
    return "MC_SYNTAX_WHILE_STATEMENT";
  case MC_SYNTAX_IF_STATEMENT:
    return "MC_SYNTAX_IF_STATEMENT";
  case MC_SYNTAX_LOCAL_DECLARATION:
    return "MC_SYNTAX_LOCAL_DECLARATION";
  case MC_SYNTAX_LOCAL_ARRAY_DECLARATION:
    return "MC_SYNTAX_LOCAL_ARRAY_DECLARATION";
  case MC_SYNTAX_LOCAL_DECLARATION_AND_ASSIGN:
    return "MC_SYNTAX_LOCAL_DECLARATION_AND_ASSIGN";
  case MC_SYNTAX_ASSIGNMENT_STATEMENT:
    return "MC_SYNTAX_ASSIGNMENT_STATEMENT";
  case MC_SYNTAX_ARITHMETIC_ASSIGNMENT_STATEMENT:
    return "MC_SYNTAX_ARITHMETIC_ASSIGNMENT_STATEMENT";
  case MC_SYNTAX_RETURN_STATEMENT:
    return "MC_SYNTAX_RETURN_STATEMENT";
  case MC_SYNTAX_INVOKE_STATEMENT:
    return "MC_SYNTAX_INVOKE_STATEMENT";
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
  default:
    return get_mc_token_type_name((mc_token_type)type);
  }
}

int mcs_print_syntax_tree(mc_syntax_node *syntax_node, int depth)
{
  for (int i = 0; i < depth; ++i) {
    printf("|  ");
  }
  printf("|--%s", get_mc_syntax_token_type_name(syntax_node->type));
  // printf("mpst-0\n");

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
      MCcall(mcs_print_syntax_tree(syntax_node->children->items[i], depth + 1));
    }
    // printf("mpst-5\n");
  }

  return 0;
}

int mcs_add_syntax_node_to_parent(mc_syntax_node *parent, mc_syntax_node *child)
{
  if (parent) {
    MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                                child));
  }
  return 0;
}

int mcs_construct_syntax_node(parsing_state *ps, mc_syntax_node_type node_type, char *mc_token_primitive_text,
                              mc_syntax_node *parent, mc_syntax_node **result)
{
  mc_syntax_node *syntax_node = (mc_syntax_node *)calloc(sizeof(mc_syntax_node), 1);
  syntax_node->type = node_type;
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
  case MC_SYNTAX_RETURN_STATEMENT: {
    syntax_node->return_expression.expression = NULL;
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
  case MC_SYNTAX_LOCAL_DECLARATION: {
    syntax_node->local_declaration.type_identifier = NULL;
    syntax_node->local_declaration.mc_type = NULL;
    syntax_node->local_declaration.type_dereference = NULL;
    syntax_node->local_declaration.variable_name = NULL;
  } break;
  case MC_SYNTAX_LOCAL_ARRAY_DECLARATION: {
    syntax_node->local_array_declaration.local_declaration = NULL;
    syntax_node->local_array_declaration.array_size_expression = NULL;
  } break;
  case MC_SYNTAX_LOCAL_DECLARATION_AND_ASSIGN: {
    syntax_node->local_declaration_and_assign.local_declaration = NULL;
    syntax_node->local_declaration_and_assign.assignment_expression = NULL;
  } break;
  case MC_SYNTAX_ASSIGNMENT_STATEMENT: {
    syntax_node->assignment.variable = NULL;
    syntax_node->assignment.value_expression = NULL;
  } break;
  case MC_SYNTAX_ARITHMETIC_ASSIGNMENT_STATEMENT: {
    syntax_node->arithmetic_assignment.variable = NULL;
    syntax_node->arithmetic_assignment.assignment_operator = NULL;
    syntax_node->arithmetic_assignment.value_expression = NULL;
  } break;
  case MC_SYNTAX_INVOKE_STATEMENT: {
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
  case MC_SYNTAX_CAST_EXPRESSION: {
    syntax_node->cast_expression.type_identifier = NULL;
    syntax_node->cast_expression.type_dereference = NULL;
    syntax_node->cast_expression.mc_type = NULL;
    syntax_node->cast_expression.expression = NULL;
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
    MCerror(355, "Unsupported:%i", node_type);
  }
  }

  if (parent) {
    MCcall(mcs_add_syntax_node_to_parent(parent, syntax_node));
  }

  *result = syntax_node;
  return 0;
}

void release_syntax_node(mc_syntax_node *syntax_node)
{
  if (!syntax_node) {
    return;
  }

  if ((int)syntax_node->type >= (int)MC_TOKEN_STANDARD_MAX_VALUE) {
    if (syntax_node->children) {
      if (syntax_node->children->alloc) {
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
  case MC_SYNTAX_INVOKE_STATEMENT: {
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
    *token_type = MC_TOKEN_CURLY_OPEN_BRACKET;
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
          MCerror(72, "Unexpected end-of-file\n");
        }
        if (code[*index] == '\n') {
          break;
        }
      }

      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
      }
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
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  mc_token_type modifier_type = MC_TOKEN_NULL;
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

  // Convert to the appropriate type
  if (token_type == MC_TOKEN_IDENTIFIER) {
    void *vargs[3];
    vargs[0] = &command_hub->nodespace;
    vargs[1] = &(*type_identifier)->text;
    vargs[2] = &(*mc_type);
    find_struct_info(3, vargs);
  }
  else {
    *mc_type = NULL;
  }

  return 0;
}

int mcs_parse_dereference_sequence(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_dereference_sequence()\n");
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

int mcs_parse_function_call(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_function_call()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_INVOKE_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_expression_variable_access(ps, statement, &statement->invocation.function_identity));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  while (1) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));

    if (token0 == MC_TOKEN_CLOSING_BRACKET) {
      break;
    }

    if (statement->invocation.arguments->count) {
      MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_COMMA, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    }

    mc_syntax_node *argument;
    MCcall(mcs_parse_expression(ps, statement, &argument));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

    MCcall(append_to_collection((void ***)&statement->invocation.arguments->items,
                                &statement->invocation.arguments->alloc, &statement->invocation.arguments->count,
                                argument));
  }
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSING_BRACKET, NULL));

  // printf("mcs_parse_function_call(ret)\n");
  return 0;
}

int mcs_parse_fixrement_operation(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, NULL, parent, &expression));
  if (additional_destination) {
    *additional_destination = expression;
  }

  mc_token_type token0;
  bool operator_set;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_DECREMENT_OPERATOR) {
    MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_DECREMENT_OPERATOR,
                                   &expression->fixrement_expression.fix_operator));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
    operator_set = true;
  }
  else if (token0 == MC_TOKEN_INCREMENT_OPERATOR) {
    MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_INCREMENT_OPERATOR,
                                   &expression->fixrement_expression.fix_operator));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
    operator_set = true;
  }
  else {
    MCcall(mcs_parse_expression_variable_access(ps, expression, &expression->fixrement_expression.primary));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
    operator_set = false;
  }

  if (operator_set) {
    MCcall(mcs_parse_expression_variable_access(ps, expression, &expression->fixrement_expression.primary));
    expression->fixrement_expression.postfix = false;
  }
  else {
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_DECREMENT_OPERATOR) {
      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_DECREMENT_OPERATOR,
                                     &expression->fixrement_expression.fix_operator));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      expression->fixrement_expression.postfix = true;
    }
    else {
      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_INCREMENT_OPERATOR,
                                     &expression->fixrement_expression.fix_operator));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      expression->fixrement_expression.postfix = true;
    }
  }

  return 0;
}

int mcs_parse_bracketed_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // Determine expression type
  mc_token_type token_type;
  MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
  if (token_type != MC_TOKEN_OPEN_BRACKET) {
    MCerror(1247, "Flow format");
  }

  MCcall(mcs_peek_token_type(ps, false, 1, &token_type));
  switch (token_type) {
  case MC_TOKEN_INT_KEYWORD:
  case MC_TOKEN_CHAR_KEYWORD:
  case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_UNSIGNED_KEYWORD: {
    // Casted expression
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

    MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
    if (token_type == MC_TOKEN_STAR_CHARACTER) {
      MCcall(mcs_parse_dereference_sequence(ps, cast_expression, &cast_expression->cast_expression.type_dereference));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));
    }
    else {
      cast_expression->cast_expression.type_dereference = NULL;
    }

    MCcall(mcs_parse_through_token(ps, cast_expression, MC_TOKEN_CLOSING_BRACKET, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, cast_expression));

    MCcall(mcs_parse_expression_unary(ps, cast_expression, &cast_expression->cast_expression.expression));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1325, "MCS:Unsupported-token:%s", get_mc_token_type_name(token_type));
  }
  }

  return 0;
}

int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // Determine expression type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_STRING_LITERAL: {
    mc_syntax_node *string_literal;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_STRING_LITERAL_EXPRESSION, NULL, parent, &string_literal));
    if (additional_destination) {
      *additional_destination = string_literal;
    }

    MCcall(mcs_parse_through_token(ps, string_literal, MC_TOKEN_STRING_LITERAL, NULL));

    while (1) {
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));
      if (token0 != MC_TOKEN_STRING_LITERAL) {
        break;
      }

      MCcall(mcs_parse_through_supernumerary_tokens(ps, string_literal));
      MCcall(mcs_parse_through_token(ps, string_literal, MC_TOKEN_STRING_LITERAL, NULL));
    }
  } break;
  case MC_TOKEN_SUBTRACT_OPERATOR:
  case MC_TOKEN_PLUS_OPERATOR:
  case MC_TOKEN_NOT_OPERATOR:
  case MC_TOKEN_AMPERSAND_CHARACTER: {
    mc_syntax_node *prepended_unary;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PREPENDED_UNARY_EXPRESSION, NULL, parent, &prepended_unary));
    if (additional_destination) {
      *additional_destination = prepended_unary;
    }

    MCcall(mcs_parse_through_token(ps, prepended_unary, token0, &prepended_unary->prepended_unary.prepend_operator));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, prepended_unary));
    MCcall(mcs_parse_expression_unary(ps, prepended_unary, &prepended_unary->prepended_unary.unary_expression));
  } break;
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token1;
    MCcall(mcs_peek_token_type(ps, false, 1, &token1));
    switch (token1) {
    case MC_TOKEN_SQUARE_OPEN_BRACKET:
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT: {
      MCcall(mcs_parse_expression_variable_access(ps, parent, additional_destination));
    } break;
    case MC_TOKEN_SEMI_COLON:
    case MC_TOKEN_COMMA:
    case MC_TOKEN_SQUARE_CLOSE_BRACKET:
    case MC_TOKEN_CLOSING_BRACKET: {
      MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
    } break;
    case MC_TOKEN_OPEN_BRACKET: {
      MCcall(mcs_parse_function_call(ps, parent, additional_destination));
    } break;
    case MC_TOKEN_PLUS_OPERATOR:
    case MC_TOKEN_SUBTRACT_OPERATOR:
    case MC_TOKEN_DIVIDE_OPERATOR:
    case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_LESS_THAN_OPERATOR:
    case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
    case MC_TOKEN_MORE_THAN_OPERATOR:
    case MC_TOKEN_EQUALITY_OPERATOR:
    case MC_TOKEN_INEQUALITY_OPERATOR: {
      MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
      return 0;
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(282, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
    }
    }
  } break;
  case MC_TOKEN_NUMERIC_LITERAL:
  case MC_TOKEN_CHAR_LITERAL: {
    MCcall(mcs_parse_through_token(ps, parent, token0, additional_destination));
  } break;
  case MC_TOKEN_OPEN_BRACKET: {
    // Determine if paranthesized expression or cast expression
    MCcall(mcs_parse_bracketed_expression(ps, parent, additional_destination));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(287, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  return 0;
}

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

// int _mcs_obtain_lowest_precedence_operator(parsing_state *ps, int peek_ahead_begin, int peek_ahead_exclusive_max,
//                                            mc_token_type lowest_precedence_token_type, int
//                                            lowest_precedence_peek_loc)
// {

//   return 0;
// }

// int mcs_find_scope_exclusive_token(parsing_state *ps, int scope_max_level, int *exclusive_ahead_index)
// {
//   int peek_ahead = 0, peek_ahead_end_index = 0;
//   mc_token_type prev_prev_type = MC_TOKEN_NULL;
//   mc_token_type prev_type = MC_TOKEN_NULL;
//   mc_token_type token_type = MC_TOKEN_NULL;
//   while (true) {
//     prev_prev_type = prev_type;
//     prev_type = token_type;
//     MCcall(mcs_peek_token_type_and_index(ps, false, peek_ahead, &token_type, &peek_ahead_end_index));
//     ++peek_ahead;

//     switch (token_type) {
//     case MC_TOKEN_SUBTRACT_OPERATOR:
//     case MC_TOKEN_PLUS_OPERATOR:
//     case MC_TOKEN_NUMERIC_LITERAL:
//     case MC_TOKEN_SEMI_COLON: {
//       break;
//     }
//     case MC_TOKEN_STAR_CHARACTER: {
//       // Determine
//       switch (prev_type) {
//       case MC_TOKEN_NUMERIC_LITERAL: {
//         // Multiply
//         token_type = MC_TOKEN_MULTIPLY_OPERATOR;
//       } break;
//       case MC_TOKEN_NULL: {
//         // Dereference
//         token_type = MC_TOKEN_DEREFERENCE_OPERATOR;
//       } break;
//       default:
//         print_parse_error(ps->code, peek_ahead_end_index - 1, "see-below", "");
//         MCerror(1547, "MCS:find_scope_exclusive:TOKEN_STAR>prev-token:%s", get_mc_token_type_name(prev_type));
//       }
//     } break;
//     default:
//       print_parse_error(ps->code, peek_ahead_end_index - 1, "see-below", "");
//       // printf("peek-ahead:%s%i", peek_ahead ? "+" : "", peek_ahead);
//       MCerror(1575, "MCS:find_scope_exclusive:ROOT>Unsupported-token:%s", get_mc_token_type_name(token_type),
//               get_mc_token_type_name(prev_type), get_mc_token_type_name(prev_prev_type));
//     }
//     int scope_level;
//     MCcall(mcs_get_token_scope_level(token_type, &scope_level));
//     if (scope_level > scope_max_level) {
//       printf("scope_level=%i scope_max_level=%i\n", scope_level, scope_max_level);
//       *exclusive_ahead_index = peek_ahead;
//       return 0;
//     }
//   }

//   MCerror(1578, "Invalid Flow");
// }

int mcs_parse_expression_operational(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_expression_operational()\n");
  mc_syntax_node *operational;

  // Find the end of the operational scope
  // int operational_scope_level, operational_scope_end;
  // MCcall(mcs_get_token_scope_level(MC_TOKEN_MULTIPLY_OPERATOR, operational_scope_level));
  // MCcall(mcs_find_scope_exclusive_token(ps, OPERATIONAL_SCOPE_MAXIMUM, &operational_scope_end));
  // printf("operational_scope_end=%i\n", operational_scope_end);

  // mc_syntax_node *left;
  // MCcall(mcs_parse_expression_unary(ps, NULL, &left));

  // // Determine expression type
  // mc_token_type token0;
  // MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  // switch (token0) {
  // case MC_TOKEN_IDENTIFIER: {
  //   mc_token_type token1;
  //   MCcall(mcs_peek_token_type(ps, false, 1, &token1));
  //   switch (token1) {
  //   default: {
  //     print_parse_error(ps->code, ps->index, "see-below", "");
  //     MCerror(742, "MCS:MC_TOKEN_IDENTIFIER>Unsupported-token:%s", get_mc_token_type_name(token1));
  //   }
  //   }
  // }
  // case MC_TOKEN_STAR_CHARACTER:
  // case MC_TOKEN_SUBTRACT_OPERATOR:
  // case MC_TOKEN_PLUS_OPERATOR:
  // case MC_TOKEN_DIVIDE_OPERATOR:
  // case MC_TOKEN_MODULO_OPERATOR: {
  //   // Arithmetic operation
  //   MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_OPERATIONAL_EXPRESSION, NULL, parent, &operational));
  //   if (additional_destination) {
  //     *additional_destination = operational;
  //   }
  //   operational->operational_expression.left = left;
  //   MCcall(mcs_add_syntax_node_to_parent(operational, left));

  //   MCcall(mcs_parse_through_supernumerary_tokens(ps, operational));
  //   MCcall(mcs_parse_through_token(ps, operational, token0,
  //   &operational->operational_expression.operational_operator));

  //   // Right Side
  //   MCcall(mcs_parse_through_supernumerary_tokens(ps, operational));
  //   MCcall(mcs_parse_expression_operational(ps, operational, &operational->operational_expression.right));
  //   return 0;
  // }
  // case MC_TOKEN_SEMI_COLON:
  // case MC_TOKEN_SQUARE_CLOSE_BRACKET:
  // case MC_TOKEN_CLOSING_BRACKET:
  // case MC_TOKEN_COMMA:
  // case MC_TOKEN_LOGICAL_AND_OPERATOR:
  // case MC_TOKEN_LOGICAL_OR_OPERATOR:
  // case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
  // case MC_TOKEN_LESS_THAN_OPERATOR:
  // case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
  // case MC_TOKEN_MORE_THAN_OPERATOR:
  // case MC_TOKEN_EQUALITY_OPERATOR:
  // case MC_TOKEN_INEQUALITY_OPERATOR: {
  //   // Expression ends here
  //   if (parent) {
  //     MCcall(mcs_add_syntax_node_to_parent(parent, left));
  //   }
  //   if (additional_destination) {
  //     *additional_destination = left;
  //   }
  //   return 0;
  // }
  // default: {
  //   print_parse_error(ps->code, ps->index, "see-below", "");
  //   MCerror(1202, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  // }
  // }

  MCerror(1589, "Invalid Flow");
}

int _mcs_parse_expression(parsing_state *ps, int allowable_precedence, mc_syntax_node *parent,
                          mc_syntax_node **additional_destination)
{
  mc_syntax_node *left;

  // Determine left
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_NUMERIC_LITERAL: {
    MCcall(mcs_parse_through_token(ps, NULL, token0, &left));
  } break;
  case MC_TOKEN_STAR_CHARACTER:
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
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(1798, "MCS:Unsupported-token:%s allowable_precedence:%i", get_mc_token_type_name(token0),
            allowable_precedence);
  }
  }

  while (1) {
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
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

  printf("Conditional-Expression!:\n");
  MCcall(mcs_print_syntax_tree(*additional_destination, 1));
  return 0;
}

/*
unary-expression:
  literal
  numeric
  member-access
  increment
  decrement
  invoke
  bracket-expression
  not-expression

operation-expression:
  left: unary-expression
  operator: NULL, *+/-||, &&, |, ^
  right: NULL, operation-expression
-additive/multiplicative/modular expression

:evaluates to bool
conditional-expression:
  left: operation-expression
  operator: NULL, ||, &&, |, ^, relational-operator, ==<> !=
  right: NULL, conditional-expression
-conditional-or-expression 	&&
-conditional-and-expression 	||
-inclusive-or-expression	|
-exclusive-or-expression 	&
-equality-expression		== / !=
-relational-expression

inline-conditional-expression
  conditional-expression
  operation-expression ? conditional-expression : conditional-expression

expression:
  inline-conditional-expression */
int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // Find the end of the expression

  // MCcall(mcs_find_precedent_operator(ps, 0, -1, 18))

  //   mc_token_type lpo_token_type;
  //   int lpo_peek_loc, lpo_peek_loc2;

  MCcall(_mcs_parse_expression(ps, 16, parent, additional_destination));

  printf("Expression!:\n");
  MCcall(mcs_print_syntax_tree(*additional_destination, 1));
  // MCcall(_mcs_obtain_precedence_information(ps, 0, -1, &lpo_token_type, &lpo_peek_loc, &lpo_peek_loc2));

  // mc_syntax_node *expression;
  // MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION, parent, &expression));
  // if (additional_destination) {
  //   *additional_destination = expression;
  // }

  // MCcall(mcs_parse_expression_conditional(ps, parent, additional_destination));

  return 0;
}

int mcs_parse_return_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *return_expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_RETURN_STATEMENT, NULL, parent, &return_expression));
  if (additional_destination) {
    *additional_destination = return_expression;
  }

  MCcall(mcs_parse_through_token(ps, return_expression, MC_TOKEN_RETURN_KEYWORD, NULL));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_SEMI_COLON) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, return_expression));
    MCcall(mcs_parse_expression(ps, return_expression, &return_expression->return_expression.expression));
  }
  else {
    return_expression->return_expression.expression = NULL;
  }

  return 0;
}

int mcs_parse_assignment(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ASSIGNMENT_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_expression_variable_access(ps, statement, &statement->assignment.variable));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_expression(ps, statement, &statement->assignment.value_expression));

  return 0;
}

int mcs_parse_arithmetic_assignment(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ARITHMETIC_ASSIGNMENT_STATEMENT, NULL, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  MCcall(mcs_parse_expression_variable_access(ps, statement, &statement->arithmetic_assignment.variable));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
  case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR: {
    // Valid
  } break;
  default: {
    MCerror(1335, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  MCcall(mcs_parse_through_token(ps, statement, token0, &statement->arithmetic_assignment.assignment_operator));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_expression(ps, statement, &statement->arithmetic_assignment.value_expression));

  return 0;
}

int mcs_parse_local_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *local_declaration;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_DECLARATION, NULL, NULL, &local_declaration));

  // Type
  MCcall(mcs_parse_type_identifier(ps, local_declaration, &local_declaration->local_declaration.type_identifier,
                                   &local_declaration->local_declaration.mc_type));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    MCcall(
        mcs_parse_dereference_sequence(ps, local_declaration, &local_declaration->local_declaration.type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration));
  }
  else {
    local_declaration->local_declaration.type_dereference = NULL;
  }

  MCcall(mcs_parse_through_token(ps, local_declaration, MC_TOKEN_IDENTIFIER,
                                 &local_declaration->local_declaration.variable_name));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_SEMI_COLON) {
    MCcall(mcs_add_syntax_node_to_parent(parent, local_declaration));
    if (additional_destination) {
      *additional_destination = local_declaration;
    }

    return 0;
  }
  if (token0 == MC_TOKEN_SQUARE_OPEN_BRACKET) {
    // Array declaration
    mc_syntax_node *local_array_declaration;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_ARRAY_DECLARATION, NULL, parent, &local_array_declaration));
    if (additional_destination) {
      *additional_destination = local_array_declaration;
    }
    local_array_declaration->local_array_declaration.local_declaration = local_declaration;
    MCcall(mcs_add_syntax_node_to_parent(local_array_declaration, local_declaration));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, local_array_declaration));
    MCcall(mcs_parse_through_token(ps, local_array_declaration, MC_TOKEN_SQUARE_OPEN_BRACKET, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, local_array_declaration));
    MCcall(mcs_parse_expression_unary(ps, local_array_declaration, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, local_array_declaration));
    MCcall(mcs_parse_through_token(ps, local_array_declaration, MC_TOKEN_SQUARE_CLOSE_BRACKET, NULL));

    return 0;
  }

  // Local declare & assign
  mc_syntax_node *local_declaration_and_assign;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_DECLARATION_AND_ASSIGN, NULL, parent,
                                   &local_declaration_and_assign));
  if (additional_destination) {
    *additional_destination = local_declaration_and_assign;
  }
  local_declaration_and_assign->local_declaration_and_assign.local_declaration = local_declaration;
  MCcall(mcs_add_syntax_node_to_parent(local_declaration_and_assign, local_declaration));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration_and_assign));
  MCcall(mcs_parse_through_token(ps, local_declaration_and_assign, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, local_declaration_and_assign));
  MCcall(mcs_parse_expression(ps, local_declaration_and_assign,
                              &local_declaration_and_assign->local_declaration_and_assign.assignment_expression));

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
    MCcall(mcs_parse_local_declaration(ps, statement, &statement->for_statement.initialization));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  } break;
  case MC_TOKEN_SEMI_COLON: {
    statement->for_statement.initialization = NULL;
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(873, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  // Conditional
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_SEMI_COLON: {
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  } break;
  case MC_TOKEN_IDENTIFIER: {
    MCcall(mcs_parse_expression_conditional(ps, statement, &statement->for_statement.conditional));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(882, "MCS:>:ERR-token:%s", get_mc_token_type_name(token0));
  }
  }
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  // UpdateFix Expression
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_CLOSING_BRACKET: {
  } break;
  case MC_TOKEN_INCREMENT_OPERATOR: {
    MCcall(mcs_parse_fixrement_operation(ps, statement, &statement->for_statement.fix_expression));
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
  if (token0 != MC_TOKEN_CURLY_OPEN_BRACKET) {
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
    if (token0 != MC_TOKEN_CURLY_OPEN_BRACKET) {
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
  if (token0 != MC_TOKEN_CURLY_OPEN_BRACKET) {
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
    else if (token0 == MC_TOKEN_CURLY_OPEN_BRACKET) {
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

int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_code_block()\n");
  mc_syntax_node *block_node;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_BLOCK, NULL, parent, &block_node));
  if (additional_destination) {
    *additional_destination = block_node;
  }

  // printf("ps->index:%i\n", ps->index);
  MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_CURLY_OPEN_BRACKET, NULL));

  bool loop = true;
  while (loop) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));

    // printf("ps->index:%i\n", ps->index);

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    case MC_TOKEN_CURLY_CLOSING_BRACKET: {
      loop = false;
      break;
    }
    case MC_TOKEN_CURLY_OPEN_BRACKET: {
      MCcall(mcs_parse_code_block(ps, block_node, NULL));
    } break;
    case MC_TOKEN_FOR_KEYWORD: {
      MCcall(mcs_parse_for_statement(ps, block_node, NULL));
    } break;
    case MC_TOKEN_IF_KEYWORD: {
      MCcall(mcs_parse_if_statement(ps, block_node, NULL));
    } break;
    case MC_TOKEN_DO_KEYWORD:
    case MC_TOKEN_WHILE_KEYWORD: {
      MCcall(mcs_parse_while_statement(ps, block_node, NULL));
    } break;
    case MC_TOKEN_CONTINUE_KEYWORD:
    case MC_TOKEN_BREAK_KEYWORD: {
      MCcall(mcs_parse_through_token(ps, block_node, token0, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
      MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
    } break;
    case MC_TOKEN_RETURN_KEYWORD: {
      MCcall(mcs_parse_return_expression(ps, block_node, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
      MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
    } break;
    case MC_TOKEN_INCREMENT_OPERATOR:
    case MC_TOKEN_DECREMENT_OPERATOR: {
      MCcall(mcs_parse_fixrement_operation(ps, block_node, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
      MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
    } break;
    case MC_TOKEN_INT_KEYWORD:
    case MC_TOKEN_CHAR_KEYWORD:
    case MC_TOKEN_BOOL_KEYWORD:
    case MC_TOKEN_LONG_KEYWORD:
    case MC_TOKEN_FLOAT_KEYWORD:
    case MC_TOKEN_UNSIGNED_KEYWORD: {
      // Some Sort of Declarative Statement
      MCcall(mcs_parse_local_declaration(ps, block_node, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
      MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
    } break;
    case MC_TOKEN_IDENTIFIER: {
      mc_token_type token1;
      MCcall(mcs_peek_token_type(ps, false, 1, &token1));
      switch (token1) {
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_STAR_CHARACTER: {
        // Some Sort of Declarative Statement
        MCcall(mcs_parse_local_declaration(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
      } break;
      case MC_TOKEN_OPEN_BRACKET: {
        // A standalone function call statement
        MCcall(mcs_parse_function_call(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
      } break;
      case MC_TOKEN_ASSIGNMENT_OPERATOR: {
        MCcall(mcs_parse_assignment(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
      } break;
      case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
      case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR: {
        MCcall(mcs_parse_arithmetic_assignment(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
      } break;
      case MC_TOKEN_POINTER_OPERATOR:
      case MC_TOKEN_DECIMAL_POINT: {
        // Some sort of member-access statement

        // Move through repeated accesses
        int peek_ahead = 2;
        bool access_loop = true;
        mc_token_type token2;
        while (access_loop) {
          MCcall(mcs_peek_token_type(ps, false, peek_ahead, &token2));
          if (token2 != MC_TOKEN_IDENTIFIER) {
            print_parse_error(ps->code, ps->index, "see-below", "");
            MCerror(377, "MCS:>IDENTIFIER>(./->)>access-repeat-loop expect IDENTIFIER:ERR-token was:%s",
                    get_mc_token_type_name(token2));
          }

          ++peek_ahead;
          MCcall(mcs_peek_token_type(ps, false, peek_ahead, &token2));
          switch (token2) {
          case MC_TOKEN_POINTER_OPERATOR:
          case MC_TOKEN_DECIMAL_POINT: {
          } break;
          case MC_TOKEN_ASSIGNMENT_OPERATOR: {
            MCcall(mcs_parse_assignment(ps, block_node, NULL));
            MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
            MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
            access_loop = false;
          } break;
          case MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR:
          case MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR: {
            MCcall(mcs_parse_arithmetic_assignment(ps, block_node, NULL));
            MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
            MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
            access_loop = false;
          } break;
          default: {
            print_parse_error(ps->code, ps->index, "see-below", "");
            MCerror(314, "MCS:>IDENTIFIER>(./->)>access-repeat-loop:ERR-token:%s", get_mc_token_type_name(token2));
          }
          }
          ++peek_ahead;
        }
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
  }

  MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_CURLY_CLOSING_BRACKET, NULL));

  return 0;
}

int parse_mc_to_syntax_tree_v1(char *mcode, mc_syntax_node **function_block_ast)
{
  // printf("mc_syntax_node:%zu\n", sizeof(mc_syntax_node));
  parsing_state *ps = (parsing_state *)malloc(sizeof(parsing_state));
  ps->code = mcode;
  ps->index = 0;
  ps->line = 0;
  ps->col = 0;

  mc_syntax_node *function;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FUNCTION, NULL, NULL, &function));

  // printf("pmtst-0\n");
  // MCcall(mcs_print_syntax_tree(function, 0));

  mc_token_type token0;
  MCcall(mcs_parse_type_identifier(ps, function, &function->function.return_type_identifier,
                                   &function->function.return_mc_type));

  // printf("pmtst-1\n");
  // MCcall(mcs_print_syntax_tree(function, 0));
  // printf("pmtst-1b\n");
  MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

  // printf("pmtst-2\n");
  // MCcall(mcs_print_syntax_tree(function, 0));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_CHARACTER) {
    MCcall(mcs_parse_dereference_sequence(ps, function, &function->function.return_type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, function));
  }
  else {
    function->function.return_type_dereference = NULL;
  }

  // printf("pmtst-3\n");
  // MCcall(mcs_print_syntax_tree(function, 0));

  MCcall(mcs_parse_through_token(ps, function, MC_TOKEN_IDENTIFIER, &function->function.name));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

  MCcall(mcs_parse_through_token(ps, function, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

  while (1) {
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_CLOSING_BRACKET) {
      break;
    }

    // Comma
    if (function->function.parameters->count) {
      MCcall(mcs_parse_through_token(ps, function, MC_TOKEN_COMMA, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, function));
    }

    // Parse the parameter
    mc_syntax_node *parameter;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PARAMETER_DECLARATION, NULL, function, &parameter));

    MCcall(
        mcs_parse_type_identifier(ps, function, &parameter->parameter.type_identifier, &parameter->parameter.mc_type));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_STAR_CHARACTER) {
      MCcall(mcs_parse_dereference_sequence(ps, function, &parameter->parameter.type_dereference));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, function));
    }
    else {
      parameter->parameter.type_dereference = NULL;
    }

    MCcall(mcs_parse_through_token(ps, function, MC_TOKEN_IDENTIFIER, &parameter->parameter.name));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

    MCcall(append_to_collection((void ***)&function->function.parameters->items, &function->function.parameters->alloc,
                                &function->function.parameters->count, parameter));
  }

  MCcall(mcs_parse_through_token(ps, function, MC_TOKEN_CLOSING_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

  // TODO -- memory isn't cleared if this fails, and if this fails it is handled higher up. so memory is never cleared
  MCcall(mcs_parse_code_block(ps, function, &function->function.code_block));

  // MCcall(mcs_print_syntax_tree(function, 0));
  return 0;
}