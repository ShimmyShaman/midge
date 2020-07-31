/* mc_code_parser.c */

#include "core/midge_core.h"

int mct_transcribe_code_block(c_str *str, int indent, mc_syntax_node *syntax_node);

int mct_append_node_text_to_c_str(c_str *str, mc_syntax_node *syntax_node)
{
  char *node_text;
  MCcall(copy_syntax_node_to_text(syntax_node, &node_text));
  MCcall(append_to_c_str(str, node_text));
  free(node_text);

  return 0;
}

int mct_append_indent_to_c_str(c_str *str, int indent)
{
  const char *INDENT = "  ";
  for (int i = 0; i < indent; ++i) {
    MCcall(append_to_c_str(str, INDENT));
  }
  return 0;
}

int mct_append_to_c_str(c_str *str, int indent, const char *text)
{
  MCcall(mct_append_indent_to_c_str(str, indent));
  MCcall(append_to_c_str(str, text));

  return 0;
}

int mct_contains_mc_invoke(mc_syntax_node *syntax_node, bool *result)
{
  *result = false;
  if ((mc_token_type)syntax_node->type <= MC_TOKEN_STANDARD_MAX_VALUE) {
    return 0;
  }

  if (syntax_node->type == MC_SYNTAX_INVOCATION && syntax_node->invocation.mc_function_info) {
    MCcall(print_syntax_node(syntax_node, 0));
    MCerror(35, "TODO : %s", syntax_node->invocation.function_identity->text);
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
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATION: {
    if (syntax_node->local_variable_declaration.mc_type) {
      MCcall(mct_append_to_c_str(str, 0, syntax_node->local_variable_declaration.mc_type->declared_mc_name));
    }
    else {
      MCcall(mct_append_node_text_to_c_str(str, syntax_node->local_variable_declaration.type_identifier));
    }

    MCcall(append_to_c_str(str, " "));

    for (int a = 2; a < syntax_node->children->count; ++a) {
      mc_syntax_node *child = syntax_node->children->items[a];

      MCcall(mct_append_node_text_to_c_str(str, child));
    }
  } break;
  // WILL have to redo in future
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
  case MC_SYNTAX_RELATIONAL_EXPRESSION: {

    MCcall(mct_append_node_text_to_c_str(str, syntax_node));
  } break;
  // PROBABLY won't have to redo
  case MC_SYNTAX_INVOCATION:
  case MC_SYNTAX_DECLARATION_STATEMENT:
  case MC_SYNTAX_FIXREMENT_EXPRESSION: {

    MCcall(mct_append_node_text_to_c_str(str, syntax_node));
  } break;
  default:
    MCerror(56, "MCT:Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    break;
  }

  return 0;
}

int mct_transcribe_if_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  printf("mct_transcribe_for_statement()\n");
  // printf("cb: %p\n", syntax_node);

  bool contains_mc_function_call;

  // Do MC_invokes
  if (syntax_node->if_statement.conditional) {
    MCcall(mct_contains_mc_invoke(syntax_node->if_statement.conditional, &contains_mc_function_call));
    if (contains_mc_function_call) {
      MCerror(104, "TODO");
    }
  }

  // Initialization
  MCcall(mct_append_to_c_str(str, indent, "if ("));
  MCcall(mct_transcribe_expression(str, syntax_node->if_statement.conditional));
  MCcall(mct_append_to_c_str(str, indent, ") "));

  if (syntax_node->if_statement.code_block->type != MC_SYNTAX_BLOCK) {
    MCerror(97, "TODO");
  }
  MCcall(mct_transcribe_code_block(str, indent, syntax_node->if_statement.code_block));

  if (syntax_node->if_statement.else_continuance) {
    MCerror(119, "TODO: %s",
            get_mc_syntax_token_type_name((mc_syntax_node_type)syntax_node->if_statement.else_continuance->type));
  }

  return 0;
}

int mct_transcribe_for_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  printf("mct_transcribe_for_statement()\n");
  // printf("cb: %p\n", syntax_node);

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
  MCcall(mct_transcribe_code_block(str, indent, syntax_node->for_statement.code_block));

  return 0;
}

int mct_transcribe_statement_list(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  printf("mct_transcribe_statement_list()\n");
  if (syntax_node->type != MC_SYNTAX_STATEMENT_LIST) {
    MCerror(149, "INVALID ARGUMENT: %s '%s'", get_mc_syntax_token_type_name(syntax_node->type), syntax_node->text);
  }

  for (int i = 0; i < syntax_node->children->count; ++i) {
    mc_syntax_node *child = syntax_node->children->items[i];

    switch (child->type) {
    case (mc_syntax_node_type)MC_TOKEN_NEW_LINE:
    case (mc_syntax_node_type)MC_TOKEN_SPACE_SEQUENCE:
    case (mc_syntax_node_type)MC_TOKEN_TAB_SEQUENCE:
    case (mc_syntax_node_type)MC_TOKEN_LINE_COMMENT:
    case MC_SYNTAX_CONTINUE_STATEMENT:
    case MC_SYNTAX_BREAK_STATEMENT: {
      MCcall(mct_append_node_text_to_c_str(str, child));
    } break;
    case MC_SYNTAX_RETURN_STATEMENT: {
      bool contains_mc_function_call;
      if (child->return_statement.expression) {
        MCcall(mct_contains_mc_invoke(child->return_statement.expression, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(200, "TODO");
        }
      }
      MCcall(mct_append_node_text_to_c_str(str, child));
    } break;
    case MC_SYNTAX_BLOCK: {
      MCcall(mct_transcribe_code_block(str, indent, child));
    } break;
    case MC_SYNTAX_FOR_STATEMENT: {
      MCcall(mct_transcribe_for_statement(str, indent, child))
    } break;
    case MC_SYNTAX_IF_STATEMENT: {
      MCcall(mct_transcribe_if_statement(str, indent, child))
    } break;
    case MC_SYNTAX_DECLARATION_STATEMENT: {
      MCcall(mct_append_indent_to_c_str(str, indent));

      MCcall(mct_transcribe_expression(str, child->declaration_statement.declaration));
      MCcall(append_to_c_str(str, ";\n"));
    } break;
    default:
      MCerror(168, "MCT:Statement-Unsupported:%s", get_mc_syntax_token_type_name(child->type));
    }
    // printf("transcription:\n%s||\n", str->text);
  }

  return 0;
}

int mct_transcribe_code_block(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  MCcall(mct_append_to_c_str(str, indent, "{\n"));

  if (syntax_node->block_node.statement_list) {
    MCcall(mct_transcribe_statement_list(str, indent + 1, syntax_node->block_node.statement_list));
  }

  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  return 0;
}

int transcribe_code_block_ast_to_mc_definition_v1(mc_syntax_node *syntax_node, char **output)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // MCcall(print_syntax_node(syntax_node, 0));

  if (syntax_node->type != MC_SYNTAX_BLOCK) {
    MCerror(147, "MCT:Not Supported");
  }

  c_str *str;
  MCcall(init_c_str(&str));

  if (syntax_node->block_node.statement_list) {
    MCcall(mct_transcribe_statement_list(str, 1, syntax_node->block_node.statement_list));
  }

  *output = str->text;
  MCcall(release_c_str(str, false));

  return 0;
}