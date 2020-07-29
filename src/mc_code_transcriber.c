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

int mct_transcribe_statement(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  switch (syntax_node->type) {
  default:
    MCerror(32, "Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    break;
  }

  return 0;
}

int mct_transcribe_code_block(c_str *str, mc_syntax_node *syntax_node, int indent)
{
  MCcall(mct_append_to_c_str(str, indent, "{\n"));

  for (int i = 0; i < syntax_node->block_node.statement_list->statement_list.statements->count; ++i) {
    MCcall(mct_transcribe_statement(str, syntax_node, indent + 1));
  }

  MCcall(mct_append_to_c_str(str, indent, "}"));
  if (indent > 0) {
    MCcall(mct_append_to_c_str(str, indent, "\n"));
  }

  return 0;
}

int transcribe_code_block_ast_to_mc_definition_v1(mc_syntax_node *function_syntax, char **output)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (function_syntax->type != MC_SYNTAX_BLOCK) {
    MCerror(8, "Not Supported");
  }

  c_str *str;
  MCcall(init_c_str(&str));

  MCcall(mct_transcribe_code_block(str, function_syntax, 0));

  *output = str->text;
  MCcall(release_c_str(str, false));

  return 0;
}