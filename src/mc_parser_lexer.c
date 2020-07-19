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

typedef struct mcs_token {
  mc_token_type type;
  char *text;
} mcs_token;

typedef enum mc_syntax_node_type {
  MC_SYNTAX_NODE_NULL = 0,
  MC_SYNTAX_NODE_BLOCK,
} mc_syntax_node_type;

typedef struct mc_syntax_node {
  mc_syntax_node_type type;
  struct {
    int line;
    int col;
  } begin;
  struct {
    uint alloc;
    uint count;

  } children;
} mc_syntax_node;

typedef struct mc_syntax_tree {
  mc_syntax_node *root_block;
} mc_syntax_tree;

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
  case ';': {
    *token_type = MC_TOKEN_SEMI_COLON;
    if (text) {
      allocate_and_copy_cstr(*text, ";");
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
  default: {
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
    is_supernumerary_token = false;
    break;
  }

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
  }

  (*token_result) = (mcs_token *)malloc(sizeof(mcs_token));
  (*token_result)->type = type;
  (*token_result)->text = text;

  return 0;
}

int mcs_parse_through_token(parsing_state *ps, mc_syntax_node **parent, mc_token_type expected_token)
{
  mcs_token *token;
  MCcall(mcs_parse_token(ps, &token));

  mc_token_type was_type = token->type;

  if (expected_token != MC_TOKEN_NULL && was_type != expected_token) {
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
    MCcall(mcs_peek_token_type(ps, false, 0, &token_type));
    bool is_supernumerary_token;
    MCcall(mcs_is_supernumerary_token(token_type, &is_supernumerary_token));

    if (!is_supernumerary_token) {
      break;
    }

    mc_token *token;
    MCcall(mcs_parse_token(ps, &token));

    MCcall(append_to_collection((void ***)&parent->children.items, &parent->children.alloc, &parent->children.count,
                                token));
  }

  return 0;
}

int mcs_parse_syntax_block(parsing_state *ps, mc_syntax_node *parent)
{
  mc_syntax_node *root_block = (mc_syntax_node *)malloc(sizeof(mc_syntax_node));
  root_block->type = MC_SYNTAX_NODE_BLOCK;
  root_block->begin.line = ps->line;
  root_block->begin.col = ps->col;

  MCcall(mcs_parse_through_token(ps, root_block, MC_TOKEN_CURLY_OPEN_BRACKET));

  MCcall(mcs_parse_through_supernumerary_tokens(ps, root_block));

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
        MCcall(mcs_parse_declaration());
      } break;
      default: {
        print_parse_error(ps->code, ps->index, "see-below", "");
        MCerror(111, "MCS:>IDENTIFIER>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token2));
      }
      }
    } break;
    default: {
      print_parse_error(ps->code, ps->index, "see-below", "");
      MCerror(111, "MCS:>IDENTIFIER:ERR-token:%s", get_mc_token_type_name(token1));
    }
    }
  } break;
  default: {
    MCerror(116, "MCS:Unsupported-token:%s", get_mc_token_type_name(token0));
  }
  }

  MCcall(mcs_parse_past_token(ps, MC_TOKEN_CURLY_CLOSING_BRACKET));

  return 0;
}

int parse_mc_to_syntax_tree_v1(char *mcode)
{
  parsing_state *ps = (parsing_state *)malloc(sizeof(parsing_state));
  ps->code = mcode;
  ps->index = 0;
  ps->line = 0;
  ps->col = 0;

  mc_syntax_tree *syntax_tree = (mc_syntax_tree *)malloc(sizeof(mc_syntax_tree));
  MCcall(mcs_parse_syntax_block(ps, &syntax_tree->root));

  return 0;
}