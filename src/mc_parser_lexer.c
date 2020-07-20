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

struct mc_syntax_node;

typedef enum mc_node_token_type {
  MC_SYNTAX_NODE_ROOT = MC_TOKEN_STANDARD_MAX_VALUE + 1,
  MC_SYNTAX_NODE_BLOCK,
  MC_SYNTAX_NODE_LOCAL_DECLARATION,
  MC_SYNTAX_NODE_ASSIGNMENT_STATEMENT,
  MC_SYNTAX_NODE_INVOKE_STATEMENT,

  MC_SYNTAX_NODE_SUPERNUMERARY,
  MC_SYNTAX_NODE_EXPRESSION,
  MC_SYNTAX_NODE_DEREFERENCE_SEQUENCE,
  MC_SYNTAX_NODE_MEMBER_ACCESS,
} mc_syntax_node_type;

typedef struct mc_syntax_node_list {
  uint alloc;
  uint count;
  mc_syntax_node **items;
} mc_syntax_node_list;

typedef struct mc_syntax_node {
  mc_syntax_node_type type;
  struct {
    int line;
    int col;
  } begin;
  union {
    char *text;
    struct {
      mc_syntax_node_list *children;
      union {
        struct {
          mc_syntax_node *type_identifier;
          mc_syntax_node *type_dereference;
          mc_syntax_node *variable_name;
        } local_declaration;
        struct {
          mc_syntax_node *function_identity;
          mc_syntax_node_list *arguments;
        } invocation;
        struct {
          mc_syntax_node *variable;
          mc_syntax_node *value_expression;
        } assignment;
      };
    };
  };
} mc_syntax_node;

int mcs_construct_syntax_node(parsing_state *ps, mc_syntax_node_type node_type, mc_syntax_node *parent,
                              mc_syntax_node **result)
{
  mc_syntax_node *syntax_node = (mc_syntax_node *)malloc(sizeof(mc_syntax_node));
  syntax_node->type = node_type;
  syntax_node->begin.line = ps->line;
  syntax_node->begin.col = ps->col;

  syntax_node->children = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
  syntax_node->children->alloc = 0;
  syntax_node->children->count = 0;

  switch (node_type) {
  case MC_SYNTAX_NODE_BLOCK: {
  } break;
  case MC_SYNTAX_NODE_LOCAL_DECLARATION: {
    syntax_node->local_declaration.type_identifier = NULL;
    syntax_node->local_declaration.type_dereference = NULL;
    syntax_node->local_declaration.variable_name = NULL;
  } break;
  case MC_SYNTAX_NODE_ASSIGNMENT_STATEMENT: {
    syntax_node->assignment.variable = NULL;
    syntax_node->assignment.value_expression = NULL;
  } break;
  case MC_SYNTAX_NODE_INVOKE_STATEMENT: {
    syntax_node->invocation.arguments = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
    syntax_node->invocation.arguments->alloc = 0;
    syntax_node->invocation.arguments->count = 0;
  } break;
  case MC_SYNTAX_NODE_MEMBER_ACCESS:
  case MC_SYNTAX_NODE_DEREFERENCE_SEQUENCE:
  case MC_SYNTAX_NODE_SUPERNUMERARY:
  case MC_SYNTAX_NODE_EXPRESSION: {
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
    MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                                syntax_node));
  }

  *result = syntax_node;
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
  case '(': {
    *token_type = MC_TOKEN_OPEN_BRACKET;
    if (text) {
      allocate_and_copy_cstr(*text, "(");
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
  default: {
    // Identifier
    if (isalpha(code[*index]) || code[*index] == '_') {
      int s = *index;
      while (isalnum(code[*index]) || code[*index] == '_') {
        ++*index;
      }

      *token_type = MC_TOKEN_IDENTIFIER;
      if (text) {
        allocate_and_copy_cstrn(*text, code + s, *index - s);
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

  // printf("issupernum: %s = %s\n", get_mc_token_type_name(token_type), (*is_supernumerary_token) ? "is!" : "is not!");

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

  MCcall(mcs_construct_syntax_node(ps, (mc_syntax_node_type)type, parent, token_result));
  (*token_result)->text = text;
  (*token_result)->begin.line = ps->line;
  (*token_result)->begin.col = ps->col;

  MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                              *token_result));

  printf("parse:'%s'\n", text);

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

int mcs_parse_through_member_access_expression(parsing_state *ps, mc_syntax_node *parent,
                                               mc_syntax_node **additional_destination)
{
  mc_syntax_node *member_access;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_MEMBER_ACCESS, parent, &member_access));
  if (additional_destination) {
    *additional_destination = member_access;
  }

  MCcall(mcs_parse_through_token(ps, member_access, MC_TOKEN_IDENTIFIER, NULL));

  bool access_loop = true;
  while (access_loop) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    case MC_TOKEN_DECIMAL_POINT: {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, member_access));
      MCcall(mcs_parse_through_token(ps, member_access, MC_TOKEN_DECIMAL_POINT, NULL));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, member_access));
      MCcall(mcs_parse_through_token(ps, member_access, MC_TOKEN_IDENTIFIER, NULL));

    } break;
    case MC_TOKEN_ASSIGNMENT_OPERATOR: {
    case MC_TOKEN_OPEN_BRACKET:
      access_loop = false;
      break;
    }
    default:
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(289, "ERR:unsupported-token:%s", get_mc_token_type_name(token0));
    }
  }

  return 0;
}

int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  mc_syntax_node *expression;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_EXPRESSION, parent, &expression));
  if (additional_destination) {
    *additional_destination = expression;
  }

  // Determine expression type
  mc_token_type token0;
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_STRING_LITERAL: {
    MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_STRING_LITERAL, NULL));

    while (1) {
      MCcall(mcs_peek_token_type(ps, false, 0, &token0));
      if (token0 != MC_TOKEN_STRING_LITERAL) {
        break;
      }

      MCcall(mcs_parse_through_supernumerary_tokens(ps, expression));
      MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_STRING_LITERAL, NULL));
    }

    return 0;
  }
  case MC_TOKEN_IDENTIFIER: {
    mc_token_type token1;
    MCcall(mcs_peek_token_type(ps, false, 1, &token1));
    switch (token1) {
    case MC_TOKEN_DECIMAL_POINT: {
      MCcall(mcs_parse_through_member_access_expression(ps, expression, NULL));
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(282, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
    }
    }
  } break;
  case MC_TOKEN_NUMERIC_LITERAL: {
    MCcall(mcs_parse_through_token(ps, expression, MC_TOKEN_NUMERIC_LITERAL, NULL));
  } break;
  default: {
    MCerror(287, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  // The after
  MCcall(mcs_peek_token_type(ps, false, 0, &token0));
  switch (token0) {
  case MC_TOKEN_SEMI_COLON: {
    // Do Nothing
  } break;
  default: {
    MCerror(357, "MCS:After-initial-expression:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  return 0;
}

int mcs_parse_assignment(parsing_state *ps, mc_syntax_node *parent)
{
  printf("mcs_parse_assignment()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_ASSIGNMENT_STATEMENT, parent, &statement));

  MCcall(mcs_parse_through_member_access_expression(ps, statement, &statement->assignment.variable));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_ASSIGNMENT_OPERATOR, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_expression(ps, statement, &statement->assignment.value_expression));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_function_call_statement(parsing_state *ps, mc_syntax_node *parent)
{
  printf("mcs_parse_function_call_statement()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_INVOKE_STATEMENT, parent, &statement));

  MCcall(mcs_parse_through_member_access_expression(ps, statement, &statement->invocation.function_identity));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_OPEN_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));

  while (1) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));

    if (token0 == MC_TOKEN_CLOSE_BRACKET) {
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
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_CLOSE_BRACKET, NULL));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_dereference_sequence(parsing_state *ps, mc_syntax_node *parent, mc_syntax_node **additional_destination)
{
  printf("mcs_parse_dereference_sequence()\n");
  mc_syntax_node *sequence;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_DEREFERENCE_SEQUENCE, parent, &sequence));
  if (additional_destination) {
    *additional_destination = sequence;
  }

  while (1) {
    MCcall(mcs_parse_through_token(ps, sequence, MC_TOKEN_STAR_OPERATOR, NULL));

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    if (token0 != MC_TOKEN_STAR_OPERATOR) {
      break;
    }

    MCcall(mcs_parse_through_supernumerary_tokens(ps, sequence));
  }

  return 0;
}

int mcs_parse_local_declaration(parsing_state *ps, mc_syntax_node *parent)
{
  printf("mcs_parse_local_declaration()\n");
  mc_syntax_node *statement;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_LOCAL_DECLARATION, parent, &statement));

  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_IDENTIFIER, &statement->local_declaration.type_identifier));
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
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement));
  MCcall(mcs_parse_through_token(ps, statement, MC_TOKEN_SEMI_COLON, NULL));

  return 0;
}

int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent)
{
  printf("mcs_parse_code_block()\n");
  mc_syntax_node *block_node;
  MCcall(mcs_construct_syntax_node(ps, MC_SYNTAX_NODE_BLOCK, parent, &block_node));

  printf("ps->index:%i\n", ps->index);
  MCcall(mcs_parse_through_token(ps, block_node, MC_TOKEN_CURLY_OPEN_BRACKET, NULL));

  bool loop = true;
  while (loop) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, block_node));

    printf("ps->index:%i\n", ps->index);

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    case MC_TOKEN_CURLY_CLOSING_BRACKET: {
      loop = false;
      break;
    }
    case MC_TOKEN_IDENTIFIER: {
      mc_token_type token1;
      MCcall(mcs_peek_token_type(ps, false, 1, &token1));
      switch (token1) {
      case MC_TOKEN_IDENTIFIER: {
        // Some Sort of Declarative Statement
        mc_token_type token2;
        MCcall(mcs_peek_token_type(ps, false, 2, &token2));
        switch (token2) {
        case MC_TOKEN_SEMI_COLON: {
          MCcall(mcs_parse_local_declaration(ps, block_node));
        } break;
        default: {
          print_parse_error(ps->code, ps->index, "see-below", "");
          MCerror(314, "MCS:>IDENTIFIER>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token2));
        }
        }
      } break;
      case MC_TOKEN_OPEN_BRACKET: {
        // A standalone function call
        MCcall(mcs_parse_function_call_statement(ps, block_node));
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
          case MC_TOKEN_DECIMAL_POINT: {
          } break;
          case MC_TOKEN_ASSIGNMENT_OPERATOR: {
            MCcall(mcs_parse_assignment(ps, block_node));
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

int parse_mc_to_syntax_tree_v1(char *mcode)
{
  // printf("mc_syntax_node:%zu\n", sizeof(mc_syntax_node));
  parsing_state *ps = (parsing_state *)malloc(sizeof(parsing_state));
  ps->code = mcode;
  ps->index = 0;
  ps->line = 0;
  ps->col = 0;

  mc_syntax_node parent;
  parent.children = (mc_syntax_node_list *)malloc(sizeof(mc_syntax_node_list));
  parent.children->alloc = 0;
  parent.children->count = 0;
  MCcall(mcs_parse_code_block(ps, &parent));

  // mc_syntax_node *block = (mc_syntax_node *)parent.children.items[0];

  return 0;
}