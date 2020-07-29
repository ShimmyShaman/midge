/* mc_code_parser.c */

#include "core/midge_core.h"

int mct_append_node_text_to_c_str(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  char *node_text;
  MCcall(copy_syntax_node_to_text(syntax_node, &node_text));
  MCcall(append_to_c_str(str, node_text));
  free(node_text);

  return 0;
}

int mct_append_to_c_str(c_str *str, int indent, const char *text)
{
  const char *INDENT = "  ";
  for (int i = 0; i < indent; ++i) {
    MCcall(append_to_c_str(str, INDENT));
  }

  MCcall(append_to_c_str(str, text));

  return 0;
}

int mct_contains_mc_invoke(mc_syntax_node *syntax_node, bool *result)
{
  *result = false;
  if (syntax_node->type <= MC_TOKEN_STANDARD_MAX_VALUE) {
    return 0;
  }

  if (syntax_node->type == MC_SYNTAX_INVOCATION) {
    MCerror(35, "TODO : %s", syntax_node->function.name);
  }

  for (int i = 0; i < syntax_node->children->count; ++i) {
    MCcall(mct_contains_mc_invoke(syntax_node->children->items[i], result));
    if (*result) {
      return 0;
    }
  }

  return 0;
}

int mct_transcribe_expression(c_str *str, mc_syntax_node *syntax_node)
{
  switch (syntax_node->type) {
  default:
    MCerror(56, "Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    break;
  }

  return 0;
}

int mct_transcribe_code_block(c_str *str, mc_syntax_node *syntax_node, int indent);

int mct_transcribe_statement(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  switch (syntax_node->type) {
  case MC_SYNTAX_BLOCK: {
    MCcall(mct_transcribe_code_block(str, syntax_node, indent));
  } break;
  case MC_SYNTAX_FOR_STATEMENT: {
    bool contains_mc_function_call;

    // Do MC_invokes
    if (syntax_node->for_statement.initialization) {
      MCcall(mct_contains_mc_invoke(syntax_node->for_statement.initialization, &contains_mc_function_call));
      if (contains_mc_function_call) {
        MCerror(65, "TODO");
      }
    }
    if (syntax_node->for_statement.conditional) {
      MCcall(mct_contains_mc_invoke(syntax_node->for_statement.conditional, &contains_mc_function_call));
      if (contains_mc_function_call) {
        MCerror(71, "TODO");
      }
    }
    if (syntax_node->for_statement.fix_expression) {
      MCcall(mct_contains_mc_invoke(syntax_node->for_statement.fix_expression, &contains_mc_function_call));
      if (contains_mc_function_call) {
        MCerror(77, "TODO");
      }
    }

    // Initialization
    MCcall(mct_append_to_c_str(str, indent, "for ("));
    if (syntax_node->for_statement.initialization) {
      MCcall(mct_transcribe_expression(str, syntax_node->for_statement.initialization));
    }
    MCcall(mct_append_to_c_str(str, indent, "; "));
    if (syntax_node->for_statement.conditional) {
      MCcall(mct_transcribe_expression(str, syntax_node->for_statement.conditional));
    }
    MCcall(mct_append_to_c_str(str, indent, "; "));
    if (syntax_node->for_statement.fix_expression) {
      MCcall(mct_transcribe_expression(str, syntax_node->for_statement.fix_expression));
    }
    MCcall(mct_append_to_c_str(str, indent, ") "));

    if (syntax_node->for_statement.code_block->type != MC_SYNTAX_BLOCK) {
      MCerror(97, "TODO");
    }
    MCcall(mct_transcribe_code_block(str, syntax_node->for_statement.code_block, indent));

  } break;
  default:
    MCerror(87, "Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
  }

  return 0;
}

int mct_transcribe_statement_list(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  for (int i = 0; i < syntax_node->statement_list.statements->count; ++i) {
    MCcall(mct_transcribe_statement(str, syntax_node->statement_list.statements->items[i], indent + 1));
  }

  return 0;
}

int mct_transcribe_code_block(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  MCcall(mct_append_to_c_str(str, indent, "{\n"));

  if (syntax_node->block_node.statement_list) {
    MCcall(mct_transcribe_statement_list(str, syntax_node->block_node.statement_list, indent + 1));
  }

  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  return 0;
}

int transcribe_code_block_ast_to_mc_definition_v1(mc_syntax_node *syntax_node, char **output)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (syntax_node->type != MC_SYNTAX_BLOCK) {
    MCerror(8, "Not Supported");
  }

  c_str *str;
  MCcall(init_c_str(&str));

  if (syntax_node->block_node.statement_list) {
    MCcall(mct_transcribe_statement_list(str, syntax_node->block_node.statement_list, 1));
  }

  *output = str->text;
  MCcall(release_c_str(str, false));

  return 0;
}