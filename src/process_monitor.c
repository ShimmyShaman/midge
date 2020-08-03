/* process_monitor.c */

#include "core/midge_core.h"

int mpm_obtain_context_node_for_cursor(mc_syntax_node *syntax_node, mc_code_editor_state_v1 *cestate,
                                       mc_syntax_node **context_node)
{

  if ((mc_token_type)syntax_node->type < MC_TOKEN_STANDARD_MAX_VALUE) {
    *context_node = syntax_node;
    return 0;
  }

  if (!syntax_node->children || !syntax_node->children->alloc) {
    MCerror(13, "what now?");
  }
  mc_syntax_node *child_floor = NULL;
  for (int i = 0; i < syntax_node->children->count; ++i) {
    mc_syntax_node *child = syntax_node->children->items[i];
    if (child->begin.line > cestate->cursor.line ||
        (child->begin.line == cestate->cursor.line && child->begin.col > cestate->cursor.line)) {
      break;
    }
    child_floor = child;
  }

  if (child_floor == NULL) {
    return 0;
  }

  MCcall(mpm_obtain_context_node_for_cursor(child_floor, cestate, context_node));

  return 0;
}

int process_editor_insertion(mc_code_editor_state_v1 *cestate, char *text)
{
  printf("process_editor_insertion:%s now %u,%u\n", text, cestate->cursor.line, cestate->cursor.col);

  // // Find the context
  // mc_syntax_node *context_node;
  // MCcall(mpm_obtain_context_node_for_cursor(cestate->code.syntax, cestate, &context_node));

  // if (!context_node) {
  //   MCerror(46, "TODO");
  // }

  // printf("context-Node:%s\n", get_mc_syntax_token_type_name(context_node->type));
  // // if(context_node->type == MC_SYNTAX_BLOCK) {
  // // }
  // MCcall(print_syntax_node(context_node, 0));

  return 0;
}

int process_editor_load(mc_code_editor_state_v1 *cestate)
{
  // printf("process_editor_load:%s\n", cestate->edit_ast->function.name->text);

  return 0;
}