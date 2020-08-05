/* mc_code_parser.c */

#include "core/midge_core.h"

int mct_transcribe_code_block(c_str *str, int indent, mc_syntax_node *syntax_node);
int mct_transcribe_statement_list(c_str *str, int indent, mc_syntax_node *syntax_node);

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
  register_midge_error_tag("mct_contains_mc_invoke()");
  *result = false;
  if ((mc_token_type)syntax_node->type <= MC_TOKEN_STANDARD_MAX_VALUE) {
    return 0;
  }

  register_midge_error_tag("mct_contains_mc_invoke()-1");
  if (syntax_node->type == MC_SYNTAX_INVOCATION && syntax_node->invocation.mc_function_info) {
    // MCcall(print_syntax_node(syntax_node, 0));
    // MCerror(35, "TODO : %s", syntax_node->invocation.function_identity->text);
    register_midge_error_tag("mct_contains_mc_invoke()-2");

    *result = true;
    return 0;
  }

  register_midge_error_tag("mct_contains_mc_invoke()-3");
  for (int i = 0; i < syntax_node->children->count; ++i) {
    MCcall(mct_contains_mc_invoke(syntax_node->children->items[i], result));
    if (*result) {
      return 0;
    }
  }

  register_midge_error_tag("mct_contains_mc_invoke(~)");
  return 0;
}

int mct_transcribe_mc_invocation(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_mc_invocation()");

  if (syntax_node->type != MC_SYNTAX_INVOCATION) {
    MCerror(70, "TODO %s", get_mc_syntax_token_type_name(syntax_node->type));
  }
  mc_function_info_v1 *finfo = syntax_node->invocation.mc_function_info;

  MCcall(mct_append_to_c_str(str, indent, "{\n"));
  MCcall(mct_append_to_c_str(str, indent + 1, "void *mc_vargs["));
  MCvacall(append_to_c_strf(str, "%i];\n", finfo->parameter_count + 1));

  if (finfo->parameter_count != syntax_node->invocation.arguments->count) {
    MCerror(79, "argument count not equal to required parameters");
  }

  for (int i = 0; i < finfo->parameter_count; ++i) {
    MCcall(mct_append_indent_to_c_str(str, indent + 1));
    mc_syntax_node *argument = syntax_node->invocation.arguments->items[i];
    switch (argument->type) {
    case MC_TOKEN_IDENTIFIER: {
      MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &%s;\n", i, argument->text));
    } break;
    case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
      char *text;
      if (argument->prepended_unary.prepend_operator->type == MC_TOKEN_AMPERSAND_CHARACTER) {
        MCcall(copy_syntax_node_to_text(argument, &text));
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "void *mc_varg_%i = %s;\n", i, text));
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &mc_varg_%i;\n", i, i));
      }
      else {
        MCerror(96, "TODO");
      }
      free(text);
    } break;
    default:
      MCerror(92, "Unsupported:%s", get_mc_syntax_token_type_name(argument->type));
    }
  }

  MCcall(mct_append_indent_to_c_str(str, indent + 1));
  MCvacall(append_to_c_strf(str, "%s(%i, mc_vargs);\n", finfo->name, finfo->parameter_count + 1));

  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  register_midge_error_tag("mct_transcribe_mc_invocation(~)");
  return 0;
}

int mct_transcribe_expression(c_str *str, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_expression(%s)", get_mc_syntax_token_type_name(syntax_node->type));
  // printf("mct_transcribe_expression(%s)\n", get_mc_syntax_token_type_name(syntax_node->type));
  // print_syntax_node(syntax_node, 0);

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
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION:
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
  case MC_SYNTAX_CONDITIONAL_EXPRESSION:
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
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_IDENTIFIER: {
      MCcall(mct_append_node_text_to_c_str(str, syntax_node));
    } break;
    default:
      MCerror(56, "MCT:Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    }
  }

  register_midge_error_tag("mct_transcribe_expression(~)");
  return 0;
}

int mct_transcribe_if_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_if_statement()");
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
  MCcall(append_to_c_str(str, ") "));

  if (syntax_node->if_statement.code_block->type != MC_SYNTAX_BLOCK) {
    MCerror(97, "TODO");
  }
  MCcall(mct_transcribe_code_block(str, indent, syntax_node->if_statement.code_block));

  if (syntax_node->if_statement.else_continuance) {
    MCcall(mct_append_to_c_str(str, indent, "else "));

    if (syntax_node->if_statement.else_continuance->type == MC_SYNTAX_IF_STATEMENT) {

      MCcall(mct_transcribe_if_statement(str, indent + 1, syntax_node->if_statement.else_continuance));
    }
    else if (syntax_node->if_statement.else_continuance->type == MC_SYNTAX_BLOCK) {
      MCcall(mct_transcribe_code_block(str, indent, syntax_node->if_statement.else_continuance));
    }
    else {
      MCerror(119, "TODO: %s",
              get_mc_syntax_token_type_name((mc_syntax_node_type)syntax_node->if_statement.else_continuance->type));
    }
  }

  register_midge_error_tag("mct_transcribe_if_statement(~)");
  return 0;
}

int mct_transcribe_switch_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_switch_statement()");
  // Do MC_invokes
  bool contains_mc_function_call;
  if (syntax_node->switch_statement.conditional) {
    MCcall(mct_contains_mc_invoke(syntax_node->switch_statement.conditional, &contains_mc_function_call));
    if (contains_mc_function_call) {
      MCerror(104, "TODO");
    }
  }

  MCcall(mct_append_to_c_str(str, indent, "switch ("));
  MCcall(mct_transcribe_expression(str, syntax_node->switch_statement.conditional));
  MCcall(append_to_c_str(str, ") {\n"));

  for (int i = 0; i < syntax_node->switch_statement.sections->count; ++i) {
    mc_syntax_node *switch_section = syntax_node->switch_statement.sections->items[i];

    for (int j = 0; j < switch_section->switch_section.labels->count; ++j) {
      MCcall(mct_append_indent_to_c_str(str, indent + 1));

      MCcall(mct_append_node_text_to_c_str(str, switch_section->switch_section.labels->items[j]));
      MCcall(append_to_c_str(str, "\n"));
    }

    MCcall(mct_transcribe_statement_list(str, indent, switch_section->switch_section.statement_list));
  }

  MCcall(mct_append_to_c_str(str, indent, "}\n"));
  register_midge_error_tag("mct_transcribe_switch_statement(~)");
  return 0;
}

int mct_transcribe_for_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_for_statement()");

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

  register_midge_error_tag("mct_transcribe_for_statement(~)");
  return 0;
}

int mct_transcribe_while_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_while_statement()");

  bool contains_mc_function_call;

  // Do MC_invokes
  if (syntax_node->while_statement.conditional) {
    MCcall(mct_contains_mc_invoke(syntax_node->while_statement.conditional, &contains_mc_function_call));
    if (contains_mc_function_call) {
      MCerror(65, "TODO");
    }
  }

  if (syntax_node->while_statement.do_first) {
    MCerror(311, "TODO");
  }

  // Initialization
  MCcall(mct_append_to_c_str(str, indent, "while ("));
  MCcall(mct_transcribe_expression(str, syntax_node->while_statement.conditional));
  MCcall(mct_append_to_c_str(str, indent, ") "));

  if (syntax_node->while_statement.code_block->type != MC_SYNTAX_BLOCK) {
    MCerror(320, "TODO");
  }
  MCcall(mct_transcribe_code_block(str, indent, syntax_node->while_statement.code_block));

  register_midge_error_tag("mct_transcribe_while_statement(~)");
  return 0;
}

int mct_transcribe_statement_list(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_statement_list()");
  // printf("mct_transcribe_statement_list()\n");
  if (syntax_node->type != MC_SYNTAX_STATEMENT_LIST) {
    MCerror(149, "INVALID ARGUMENT: %s '%s'", get_mc_syntax_token_type_name(syntax_node->type), syntax_node->text);
  }

  for (int i = 0; i < syntax_node->children->count; ++i) {
    // printf ("h343\n");
    // printf("%p\n", syntax_node->children->items[i]);
    mc_syntax_node *child = syntax_node->children->items[i];
    register_midge_error_tag("mct_transcribe_statement_list-L:%s", get_mc_syntax_token_type_name(child->type));
    // printf("@%i/%i@%s\n", i, syntax_node->children->count, get_mc_syntax_token_type_name(child->type));

    switch (child->type) {
    case MC_SYNTAX_CONTINUE_STATEMENT:
    case MC_SYNTAX_BREAK_STATEMENT: {
      MCcall(mct_append_indent_to_c_str(str, indent));
      MCcall(mct_append_node_text_to_c_str(str, child));
      MCcall(append_to_c_str(str, "\n"));
    } break;
    case MC_SYNTAX_RETURN_STATEMENT: {
      bool contains_mc_function_call;
      if (child->return_statement.expression) {
        MCcall(mct_contains_mc_invoke(child->return_statement.expression, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(200, "TODO");
        }

        MCerror(215, "mc_return_value etc");
      }
      // MCcall(mct_append_node_text_to_c_str(str, child));
      MCcall(append_to_c_str(str, "return 0;\n"));
    } break;
    case MC_SYNTAX_BLOCK: {
      MCcall(mct_transcribe_code_block(str, indent, child));
    } break;
    case MC_SYNTAX_FOR_STATEMENT: {
      MCcall(mct_transcribe_for_statement(str, indent, child));
    } break;
    case MC_SYNTAX_WHILE_STATEMENT: {
      MCcall(mct_transcribe_while_statement(str, indent, child));
    } break;
    case MC_SYNTAX_SWITCH_STATEMENT: {
      MCcall(mct_transcribe_switch_statement(str, indent, child));
    } break;
    case MC_SYNTAX_IF_STATEMENT: {
      MCcall(mct_transcribe_if_statement(str, indent, child));
    } break;
    case MC_SYNTAX_DECLARATION_STATEMENT: {
      // Do MC_invokes
      bool contains_mc_function_call;
      MCcall(mct_contains_mc_invoke(child->declaration_statement.declaration, &contains_mc_function_call));
      if (contains_mc_function_call) {
        MCerror(218, "TODO");
      }

      MCcall(mct_append_indent_to_c_str(str, indent));

      MCcall(mct_transcribe_expression(str, child->declaration_statement.declaration));
      MCcall(append_to_c_str(str, ";\n"));
    } break;
    case MC_SYNTAX_EXPRESSION_STATEMENT: {
      register_midge_error_tag("mct_transcribe_statement_list-ES0");
      // Do MC_invokes
      bool contains_mc_function_call;
      MCcall(mct_contains_mc_invoke(child->expression_statement.expression, &contains_mc_function_call));
      if (contains_mc_function_call) {
        if (child->expression_statement.expression->type != MC_SYNTAX_INVOCATION) {
          MCerror(231, "TODO");
        }

        MCcall(mct_transcribe_mc_invocation(str, indent, child->expression_statement.expression));
        break;
      }

      MCcall(mct_append_indent_to_c_str(str, indent));

      register_midge_error_tag("mct_transcribe_statement_list-ES5");
      MCcall(mct_transcribe_expression(str, child->expression_statement.expression));
      MCcall(append_to_c_str(str, ";\n"));
      register_midge_error_tag("mct_transcribe_statement_list-ES9");
    } break;
    default:
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_NEW_LINE:
      case MC_TOKEN_SPACE_SEQUENCE:
      case MC_TOKEN_TAB_SEQUENCE:
      case MC_TOKEN_LINE_COMMENT: {
        MCcall(mct_append_node_text_to_c_str(str, child));
      } break;
      default:
        MCcall(print_syntax_node(child, 0));
        MCerror(168, "MCT:Statement-Unsupported:%s", get_mc_syntax_token_type_name(child->type));
      }
    }
    // printf("transcription:\n%s||\n", str->text);
  }

  register_midge_error_tag("mct_transcribe_statement_list(~)");
  return 0;
}

int mct_transcribe_code_block(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_code_block()");
  MCcall(mct_append_to_c_str(str, indent, "{\n"));

  if (syntax_node->block_node.statement_list) {
    MCcall(mct_transcribe_statement_list(str, indent + 1, syntax_node->block_node.statement_list));
  }

  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  register_midge_error_tag("mct_transcribe_code_block(~)");
  return 0;
}

int transcribe_code_block_ast_to_mc_definition_v1(mc_syntax_node *syntax_node, char **output)
{
  register_midge_error_tag("transcribe_code_block_ast_to_mc_definition_v1()");

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
  MCcall(append_to_c_str(str, "\n  return 0;\n"));

  *output = str->text;
  MCcall(release_c_str(str, false));

  // // Do MC_invokes
  // bool contains_mc_function_call;
  // MCcall(mct_contains_mc_invoke(syntax_node, &contains_mc_function_call));
  // if (contains_mc_function_call) {
  //   MCerror(231, "TODO");
  // }

  // Transcribe directly (TODO -- for now)
  // MCcall(copy_syntax_node_to_text(syntax_node, output));
  // printf("transcribe:\n%s||\n", *output);

  register_midge_error_tag("transcribe_code_block_ast_to_mc_definition_v1(~)");
  return 0;
}