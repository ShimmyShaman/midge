/* process_monitor.c */

#include "core/midge_core.h"

int process_editor_insertion(mc_code_editor_state_v1 *cestate, char *text)
{

  printf("process_editor_insertion:%s now %u,%u\n", text, cestate->cursorLine, cestate->cursorCol);

  // Find the context
  mc_syntax_node *syntax_node = cestate->source_interpretation.function_ast;

  while (syntax_node->begin.line < cestate->cursorLine && syntax_node->begin.col < cestate->cursorCol) {

    MCerror(15, "TODO");
  }

  return 0;
}

int process_editor_load(mc_code_editor_state_v1 *cestate)
{
  printf("process_editor_load:%s\n", cestate->source_interpretation.function_ast->function.name->text);

  return 0;
}