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

int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);
int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination);

const char *get_mc_token_type_name(mc_token_type type)
{
  switch (type) {
  case MC_TOKEN_NULL:
    return "MC_TOKEN_NULL";
  case MC_TOKEN_STAR_OPERATOR:
    return "MC_TOKEN_STAR_OPERATOR";
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
  case MC_TOKEN_SUBTRACT_OPERATOR:
    return "MC_TOKEN_SUBTRACT_OPERATOR";
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
  case MC_TOKEN_FOR_KEYWORD:
    return "MC_TOKEN_FOR_KEYWORD";
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

int mcs_add_syntax_node_to_parent(mc_syntax_node *parent, mc_syntax_node *child)
{
  if (parent) {
    MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                                child));
  }
  return 0;
}

int mcs_construct_syntax_node(parsing_state *ps, mc_syntax_node_type node_type, mc_syntax_node *parent,
                              mc_syntax_node **result)
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
  case MC_SYNTAX_IF_STATEMENT: {
    syntax_node->if_statement.conditional = NULL;
    syntax_node->if_statement.code_block = NULL;
    syntax_node->if_statement.else_continuance = NULL;
  } break;
  case MC_SYNTAX_PARAMETER_DECLARATION: {
    syntax_node->parameter.type_identifier = NULL;
    syntax_node->parameter.mc_type = NULL;
    syntax_node->parameter.type_dereference = NULL;
    syntax_node->parameter.name = NULL;
  } break;
  case MC_SYNTAX_LOCAL_DECLARATION_STATEMENT: {
    syntax_node->local_declaration.type_identifier = NULL;
    syntax_node->local_declaration.mc_type = NULL;
    syntax_node->local_declaration.type_dereference = NULL;
    syntax_node->local_declaration.variable_name = NULL;
    syntax_node->local_declaration.assignment_expression = NULL;
  } break;
  case MC_SYNTAX_ASSIGNMENT_STATEMENT: {
    syntax_node->assignment.variable = NULL;
    syntax_node->assignment.value_expression = NULL;
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
  case ';': {
    *token_type = MC_TOKEN_SEMI_COLON;
    if (text) {
      allocate_and_copy_cstr(*text, ";");
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
    *token_type = MC_TOKEN_STAR_OPERATOR;
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
        allocate_and_copy_cstr(*text, "--");
      }
      *index += 2;
      break;
    }

    *token_type = MC_TOKEN_PLUS_OPERATOR;
    if (text) {
      allocate_and_copy_cstr(*text, "-");
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
    default:
      MCcall(print_parse_error(code, *index, "_mcs_parse_token", "forward-slash"));
      MCerror(75, "");
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
        if (slen == 3 && !strncmp(code + s, "for", slen)) {
          *token_type = MC_TOKEN_FOR_KEYWORD;
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

int mcs_parse_token(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **token_result)
{
  mc_token_type type;
  char *text;
  MCcall(_mcs_parse_token(ps->code, &ps->index, &type, &text));
  // printf("-%s-\n", text);

  MCcall(mcs_construct_syntax_node(ps, (mc_syntax_node_type)type, parent, token_result));
  (*token_result)->text = text;
  (*token_result)->begin.line = ps->line;
  (*token_result)->begin.col = ps->col;

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
    MCerror(318, "Unexpected token. Expected:%s Was:%s", get_mc_token_type_name(expected_token),
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

  mc_token_type token_type;
  MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
  switch (token_type) {
  case MC_TOKEN_IDENTIFIER:
  case MC_TOKEN_LONG_KEYWORD:
  case MC_TOKEN_FLOAT_KEYWORD:
  case MC_TOKEN_BOOL_KEYWORD:
  case MC_TOKEN_INT_KEYWORD: {
    // Valid
    break;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(808, "MCS:>:ERR-token:%s", get_mc_token_type_name(token_type));
  }
  }

  MCcall(mcs_parse_through_token(ps, parent, token_type, type_identifier));

  // Convert to the appropriate type
  {
    void *vargs[3];
    vargs[0] = &command_hub->nodespace;
    vargs[1] = &(*type_identifier)->text;
    vargs[2] = &(*mc_type);
    find_struct_info(3, vargs);
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
  case MC_TOKEN_CLOSING_BRACKET: {
    // End-of-line
    // Expected tokens that come at the end of a unary expression
    MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
  } break;
  case MC_TOKEN_SQUARE_OPEN_BRACKET: {
    // Element access
    mc_syntax_node *element_access;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION, parent, &element_access));
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
    MCerror(872, "MCS:>Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  if (is_member_access) {
    // Member-access
    mc_syntax_node *member_access;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_MEMBER_ACCESS_EXPRESSION, parent, &member_access));
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
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_INVOKE_STATEMENT, parent, &statement));
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

  return 0;
}

int mcs_parse_fixrement_operation(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FIXREMENT_EXPRESSION, parent, &expression));
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

int mcs_parse_expression_unary(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // mc_syntax_node *unary;
  // MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, parent, &unary));
  // if (additional_destination) {
  //   *additional_destination = unary;
  // }

  // MCcall(mcs_parse_expression_unary(ps, parent, additional_destination));

  // Determine expression type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_STRING_LITERAL: {
    mc_syntax_node *string_literal;
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_STRING_LITERAL_EXPRESSION, parent, additional_destination));
    // if (additional_destination) {
    //   *additional_destination = expression;
    // }

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
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token1;
    MCcall(mcs_peek_token_type(ps, false, 1, &token1));
    switch (token1) {
    case MC_TOKEN_POINTER_OPERATOR:
    case MC_TOKEN_DECIMAL_POINT: {
      MCcall(mcs_parse_expression_variable_access(ps, parent, additional_destination));
    } break;
    case MC_TOKEN_SQUARE_CLOSE_BRACKET: {
      MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER, additional_destination));
    } break;
    case MC_TOKEN_OPEN_BRACKET: {
      MCcall(mcs_parse_function_call(ps, parent, additional_destination));
    } break;
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
  case MC_TOKEN_NUMERIC_LITERAL: {
    MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_NUMERIC_LITERAL, additional_destination));
  } break;
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(287, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  return 0;
}

int mcs_parse_expression_operational(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *operational;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, parent, &operational));
  if (additional_destination) {
    *additional_destination = operational;
  }

  MCcall(mcs_parse_expression_unary(ps, parent, additional_destination));

  // Determine expression type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token1;
    MCcall(mcs_peek_token_type(ps, false, 1, &token1));
    switch (token1) {
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(742, "MCS:MC_TOKEN_IDENTIFIER>Unsupported-token:%s", get_mc_token_type_name(token1));
    }
    }
  }
  case MC_TOKEN_SEMI_COLON:
  case MC_TOKEN_SQUARE_CLOSE_BRACKET:
  case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_LESS_THAN_OPERATOR:
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_MORE_THAN_OPERATOR:
  case MC_TOKEN_EQUALITY_OPERATOR:
  case MC_TOKEN_INEQUALITY_OPERATOR: {
    // Return
    return 0;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(757, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  MCerror(746, "TODO--implement");
}

int mcs_parse_expression_conditional(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *conditional;

  mc_syntax_node *left;
  MCcall(mcs_parse_expression_operational(ps, NULL, &left));

  // Determine expression type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_LESS_THAN_OPERATOR:
  case MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR:
  case MC_TOKEN_MORE_THAN_OPERATOR:
  case MC_TOKEN_EQUALITY_OPERATOR:
  case MC_TOKEN_INEQUALITY_OPERATOR: {
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_CONDITIONAL_EXPRESSION, parent, &conditional));
    if (additional_destination) {
      *additional_destination = conditional;
    }
    conditional->conditional_expression.left = left;
    MCcall(mcs_add_syntax_node_to_parent(conditional, left));

    MCcall(mcs_parse_through_supernumerary_tokens(ps, conditional));
    MCcall(mcs_parse_through_token(ps, conditional, token0, &conditional->conditional_expression.conditional_operator));
  } break;
  case MC_TOKEN_SEMI_COLON:
  case MC_TOKEN_SQUARE_CLOSE_BRACKET: {
    // Expression ends here
    if (parent) {
      MCcall(mcs_add_syntax_node_to_parent(parent, left));
    }
    *additional_destination = left;
    return 0;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(867, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  MCcall(mcs_parse_through_supernumerary_tokens(ps, conditional));
  MCcall(mcs_parse_expression_conditional(ps, NULL, &conditional->conditional_expression.right));

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
  // mc_syntax_node *expression;
  // MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_EXPRESSION, parent, &expression));
  // if (additional_destination) {
  //   *additional_destination = expression;
  // }

  MCcall(mcs_parse_expression_conditional(ps, parent, additional_destination));

  // Determine next type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_SEMI_COLON:
  case MC_TOKEN_SQUARE_CLOSE_BRACKET: {
    return 0;
  }
  default: {
    print_parse_error(ps->code, ps->index, "see-below", "");
    MCerror(287, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  return 0;
}

int mcs_parse_assignment(parsing_state *ps, mc_syntax_node *parent)
{
  // printf("mcs_parse_assignment()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_ASSIGNMENT_STATEMENT, parent, &statement));

  MCcall(mcs_parse_expression_variable_access(ps, statement, &statement->assignment.variable));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_expression(ps, statement, &statement->assignment.value_expression));

  return 0;
}

int mcs_parse_dereference_sequence(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_dereference_sequence()\n");
  mc_syntax_node *sequence;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_DEREFERENCE_SEQUENCE, parent, &sequence));
  if (additional_destination) {
    *additional_destination = sequence;
  }

  while (1) {
    MCcall(mcs_parse_through_token(ps, sequence, MC_TOKEN_STAR_OPERATOR, NULL));
    ++sequence->dereference_sequence.count;

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 != MC_TOKEN_STAR_OPERATOR) {
      break;
    }

    MCcall(mcs_parse_through_supernumerary_tokens(ps, sequence));
  }

  return 0;
}

int mcs_parse_local_declaration(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_LOCAL_DECLARATION_STATEMENT, parent, &statement));
  if (additional_destination) {
    *additional_destination = statement;
  }

  // Type
  MCcall(mcs_parse_type_identifier(ps, statement, &statement->local_declaration.type_identifier,
                                   &statement->local_declaration.mc_type));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_OPERATOR) {
    MCcall(mcs_parse_dereference_sequence(ps, statement, &statement->local_declaration.type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  }
  else {
    statement->local_declaration.type_dereference = NULL;
  }

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, &statement->local_declaration.variable_name));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_SEMI_COLON) {
    statement->local_declaration.assignment_expression = NULL;

    return 0;
  }

  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_expression(ps, statement, &statement->local_declaration.assignment_expression));

  // // Local-declaration children:
  // printf("localdec CHILDREN:%u\n", statement->children->count);
  // for (int a = 0; a < statement->children->count; ++a) {
  //   printf("--%i\n", statement->children->items[a]->type);
  // }

  return 0;
}

int mcs_parse_for_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  // printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FOR_STATEMENT, parent, &statement));
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

int mcs_parse_if_statement(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_IF_STATEMENT, parent, &statement));
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

  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 != MC_TOKEN_CURLY_OPEN_BRACKET) {
    MCerror(876, "TODO single statement");
  }

  MCcall(mcs_parse_code_block(ps, statement, &statement->if_statement.code_block));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_ELSE_KEYWORD) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
    MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ELSE_KEYWORD, NULL));

    MCcall(mcs_parse_if_statement(ps, statement, &statement->if_statement.else_continuance));

    MCerror(876, "TODO single statement");
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
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_BLOCK, parent, &block_node));
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
    case MC_TOKEN_FOR_KEYWORD: {
      MCcall(mcs_parse_for_statement(ps, block_node, NULL));
    } break;
    case MC_TOKEN_IF_KEYWORD: {
      MCcall(mcs_parse_if_statement(ps, block_node, NULL));
    } break;
    case MC_TOKEN_IDENTIFIER: {
      mc_token_type token1;
      MCcall(mcs_peek_token_type(ps, false, 1, &token1));
      switch (token1) {
      case MC_TOKEN_IDENTIFIER:
      case MC_TOKEN_STAR_OPERATOR: {
        // Some Sort of Declarative Statement
        MCcall(mcs_parse_local_declaration(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
        // mc_token_type token2;
        // MCcall(mcs_peek_token_type(ps, false, 2, &token2));
        // switch (token2) {
        // case MC_TOKEN_SEMI_COLON: {
        // } break;
        // default: {
        //   print_parse_error(ps->code, ps->index, "see-below", "");
        //   MCerror(314, "MCS:>IDENTIFIER>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token2));
        // }
        // }
      } break;
      case MC_TOKEN_OPEN_BRACKET: {
        // A standalone function call statement
        MCcall(mcs_parse_function_call(ps, block_node, NULL));
        MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));
        MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_SEMI_COLON, NULL));
      } break;
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
            MCcall(mcs_parse_assignment(ps, block_node));
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
        MCerror(320, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
      }
      }
    } break;
    default: {
      MCerror(325, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
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
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_FUNCTION, NULL, &function));

  mc_token_type token0;
  MCcall(mcs_parse_type_identifier(ps, function, &function->function.return_type_identifier,
                                   &function->function.return_mc_type));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  if (token0 == MC_TOKEN_STAR_OPERATOR) {
    MCcall(mcs_parse_dereference_sequence(ps, function, &function->function.return_type_dereference));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, function));
  }
  else {
    function->function.return_type_dereference = NULL;
  }

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
    MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_PARAMETER_DECLARATION, function, &parameter));

    MCcall(
        mcs_parse_type_identifier(ps, function, &parameter->parameter.type_identifier, &parameter->parameter.mc_type));
    MCcall(mcs_parse_through_supernumerary_tokens(ps, function));

    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 == MC_TOKEN_STAR_OPERATOR) {
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
  return 0;
}