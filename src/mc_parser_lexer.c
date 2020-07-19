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

typedef struct mcs_token {
  mc_token_type type;
  union {
    char *text;
    mc_syntax_node *syntax_node;
  };
} mcs_token;

typedef enum mc_node_token_type {
  MC_SYNTAX_NODE_ROOT = MC_TOKEN_STANDARD_MAX_VALUE + 1,
  MC_SYNTAX_NODE_BLOCK,
  MC_SYNTAX_NODE_DECLARATION,
  MC_SYNTAX_NODE_ASSIGNMENT,
} mc_node_token_type;

typedef struct mc_syntax_node {
  struct {
    int line;
    int col;
  } begin;
  struct {
    uint alloc;
    uint count;
    mcs_token *items;
  } children;
} mc_syntax_node;

int mcs_release_token(mcs_token *token)
{
  if (!token) {
    return 0;
  }

  if (token->text) {
    free(token->text);
  }
  free(token);

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

int mcs_parse_token(parsing_state *ps, mcs_token **token_result)
{
  mc_token_type type;
  char *text;
  MCcall(_mcs_parse_token(ps->code, &ps->index, &type, &text));

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

  (*token_result) = (mcs_token *)malloc(sizeof(mcs_token));
  (*token_result)->type = type;
  (*token_result)->text = text;

  return 0;
}

int mcs_parse_through_token(parsing_state *ps, mc_syntax_node *parent, mc_token_type expected_token)
{
  mcs_token *token;
  MCcall(mcs_parse_token(ps, &token));

  mc_token_type was_type = token->type;

  if (expected_token != MC_TOKEN_NULL && was_type != expected_token) {
    MCcall(print_parse_error(ps->code, ps->index - strlen(token->text), "mcs_parse_through_token", ""));
    MCerror(87, "Unexpected token. Expected:%s Was:%s", get_mc_token_type_name(expected_token),
            get_mc_token_type_name(was_type));
  }

  MCcall(
      append_to_collection((void ***)&parent->children.items, &parent->children.alloc, &parent->children.count, token));

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

    printf("sps->index:%i\n", ps->index);
    mcs_token *token;
    MCcall(mcs_parse_token(ps, &token));

    MCcall(append_to_collection((void ***)&parent->children.items, &parent->children.alloc, &parent->children.count,
                                token));
  }

  printf("sps->index:%i\n", ps->index);
  return 0;
}

int mcs_construct_syntax_node_token(parsing_state *ps, mc_node_token_type node_type, mc_syntax_node *parent,
                                    mcs_token **result)
{
  mcs_token *token = (mcs_token *)malloc(sizeof(mcs_token));
  token->type = (mc_token_type)node_type;

  token->syntax_node = (mc_syntax_node *)malloc(sizeof(mc_syntax_node));
  token->syntax_node->begin.line = ps->line;
  token->syntax_node->begin.col = ps->col;
  token->syntax_node->children.alloc = 0;
  token->syntax_node->children.count = 0;

  if (parent) {
    MCcall(append_to_collection((void ***)&parent->children.items, &parent->children.alloc, &parent->children.count,
                                token));
  }

  *result = token;
  return 0;
}

int mcs_parse_expression(parsing_state *ps, mc_syntax_node *parent)
{
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
      MCerror(282, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
    }
    }
  } break;
  case MC_TOKEN_NUMERIC_LITERAL: {
    MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_NUMERIC_LITERAL));
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

int mcs_parse_through_member_access_expression(parsing_state *ps, mc_syntax_node *parent)
{
  MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER));

  bool access_loop = true;
  while (access_loop) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    case MC_TOKEN_DECIMAL_POINT: {
      MCcall(mcs_parse_through_supernumerary_tokens(ps, parent));
      MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_DECIMAL_POINT));
      MCcall(mcs_parse_through_supernumerary_tokens(ps, parent));
      MCcall(mcs_parse_through_token(ps, parent, MC_TOKEN_IDENTIFIER));

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

int mcs_parse_assignment(parsing_state *ps, mc_syntax_node *parent)
{
  mcs_token *statement;
  MCcall(mcs_construct_syntax_node_token(ps, MC_SYNTAX_NODE_ASSIGNMENT, parent, &statement));

  MCcall(mcs_parse_through_member_access_expression(ps, statement->syntax_node));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));

  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_ASSIGNMENT_OPERATOR));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));

  MCcall(mcs_parse_expression(ps, statement->syntax_node));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));
  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_SEMI_COLON));

  return 0;
}

int mcs_parse_function_call_statement(parsing_state *ps, mc_syntax_node *parent)
{
  mcs_token *statement;
  MCcall(mcs_construct_syntax_node_token(ps, MC_SYNTAX_NODE_DECLARATION, parent, &statement));

  MCcall(mcs_parse_through_member_access_expression(ps, statement->syntax_node));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));

  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_OPEN_BRACKET));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));

  bool parameter_loop = true;
  while (parameter_loop) {
    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
    default:
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(444, "ERR:unsupported-token:%s", get_mc_token_type_name(token0));
    }
  }
  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_CLOSE_BRACKET));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));
  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_SEMI_COLON));

  return 0;
}

int mcs_parse_standard_declaration(parsing_state *ps, mc_syntax_node *parent)
{
  mcs_token *statement;
  MCcall(mcs_construct_syntax_node_token(ps, MC_SYNTAX_NODE_DECLARATION, parent, &statement));

  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_IDENTIFIER));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));
  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_IDENTIFIER));
  MCcall(mcs_parse_through_supernumerary_tokens(ps, statement->syntax_node));
  MCcall(mcs_parse_through_token(ps, statement->syntax_node, MC_TOKEN_SEMI_COLON));

  return 0;
}

int mcs_parse_code_block(parsing_state *ps, mc_syntax_node *parent)
{
  mcs_token *block;
  MCcall(mcs_construct_syntax_node_token(ps, MC_SYNTAX_NODE_BLOCK, parent, &block));

  printf("ps->index:%i\n", ps->index);
  MCcall(mcs_parse_through_token(ps, block->syntax_node, MC_TOKEN_CURLY_OPEN_BRACKET));

  while (1) {
    MCcall(mcs_parse_through_supernumerary_tokens(ps, block->syntax_node));

    printf("ps->index:%i\n", ps->index);

    mc_token_type token0;
    MCcall(mcs_peek_token_type(ps, false, 0, &token0));
    switch (token0) {
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
          MCcall(mcs_parse_standard_declaration(ps, block->syntax_node));
        } break;
        default: {
          print_parse_error(ps->code, ps->index, "see-below", "");
          MCerror(314, "MCS:>IDENTIFIER>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token2));
        }
        }
      } break;
      case MC_TOKEN_OPEN_BRACKET: {
        // A standalone function call
        MCcall(mcs_parse_function_call_statement(ps, block->syntax_node));
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
            MCcall(mcs_parse_assignment(ps, block->syntax_node));
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

  MCcall(mcs_parse_through_token(ps, block->syntax_node, MC_TOKEN_CURLY_CLOSING_BRACKET));

  return 0;
}

int parse_mc_to_syntax_tree_v1(char *mcode)
{
  parsing_state *ps = (parsing_state *)malloc(sizeof(parsing_state));
  ps->code = mcode;
  ps->index = 0;
  ps->line = 0;
  ps->col = 0;

  mc_syntax_node *syntax_tree = (mc_syntax_node *)malloc(sizeof(mc_syntax_node));
  syntax_tree->begin.line = ps->line;
  syntax_tree->begin.col = ps->col;
  syntax_tree->children.alloc = 0;
  syntax_tree->children.count = 0;
  MCcall(mcs_parse_code_block(ps, syntax_tree));

  return 0;
}