/* mc_code_parser.c */

#include "core/mc_code_transcription.h"

int mct_transcribe_code_block(c_str *str, int indent, mc_syntax_node *syntax_node);
int mct_transcribe_statement_list(c_str *str, int indent, mc_syntax_node *syntax_node);
int mct_transcribe_expression(c_str *str, mc_syntax_node *syntax_node);

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
  register_midge_error_tag("mct_contains_mc_invoke(%s)", get_mc_syntax_token_type_name(syntax_node->type));

  *result = false;
  if ((mc_token_type)syntax_node->type <= MC_TOKEN_STANDARD_MAX_VALUE) {
    return 0;
  }

  register_midge_error_tag("mct_contains_mc_invoke()-1");
  if (syntax_node->type == MC_SYNTAX_INVOCATION) {
    // printf("mcmi-1\n");
    if (!syntax_node->invocation.mc_function_info &&
        (mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER) {
      {
        // printf("mcmi-2\n");
        // Double -check (it is necessary, at least for recursive functions)
        MCcall(find_function_info(syntax_node->invocation.function_identity->text,
                                  &syntax_node->invocation.mc_function_info));
      }
    }

    if (syntax_node->invocation.mc_function_info) {
      // MCcall(print_syntax_node(syntax_node, 0));
      // MCerror(35, "TODO : %s", syntax_node->invocation.function_identity->text);
      register_midge_error_tag("mct_contains_mc_invoke()-2");

      // printf("mcmi-3\n");
      *result = true;
      return 0;
    }
  }

  register_midge_error_tag("mct_contains_mc_invoke()-3 child_count:%i", syntax_node->children->count);
  for (int i = 0; i < syntax_node->children->count; ++i) {
    MCcall(mct_contains_mc_invoke(syntax_node->children->items[i], result));
    if (*result) {
      return 0;
    }
  }

  register_midge_error_tag("mct_contains_mc_invoke(~)");
  return 0;
}

int mct_transcribe_mc_invocation(c_str *str, int indent, mc_syntax_node *syntax_node, char *return_variable_name)
{
  register_midge_error_tag("mct_transcribe_mc_invocation()");

  if (syntax_node->type != MC_SYNTAX_INVOCATION) {
    MCerror(70, "TODO %s", get_mc_syntax_token_type_name(syntax_node->type));
  }
  if (!syntax_node->invocation.mc_function_info) {
    MCcall(print_syntax_node(syntax_node, 0));
    MCerror(86, "Argument Cannot Be Null (or its not an mc_function...)");
  }
  function_info *finfo = syntax_node->invocation.mc_function_info;
  // print_syntax_node(syntax_node, 0);
  // printf("mtmi-2 %p\n", finfo);
  // printf("mtmi-2 %i\n", finfo->parameter_count + 1);

  MCcall(mct_append_to_c_str(str, indent, "{\n"));
  MCcall(mct_append_to_c_str(str, indent + 1, "void *mc_vargs["));
  MCvacall(append_to_c_strf(str, "%i];\n", finfo->parameter_count + 1));

  register_midge_error_tag("mct_transcribe_mc_invocation-parameters");
  if (finfo->parameter_count != syntax_node->invocation.arguments->count) {
    MCerror(79, "argument count not equal to required parameters, invoke:%s, expected:%i, passed:%i", finfo->name,
            finfo->parameter_count, syntax_node->invocation.arguments->count);
  }

  for (int i = 0; i < finfo->parameter_count; ++i) {
    // printf("mtmi-3\n");
    MCcall(mct_append_indent_to_c_str(str, indent + 1));
    mc_syntax_node *argument = syntax_node->invocation.arguments->items[i];
    switch (argument->type) {
    case MC_SYNTAX_CAST_EXPRESSION: {
      // printf("mtmi-4\n");
      bool contains_mc_function_call;
      if (argument->cast_expression.expression) {
        MCcall(mct_contains_mc_invoke(argument->cast_expression.expression, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      MCcall(copy_syntax_node_to_text(argument->cast_expression.expression, &text));
      MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &%s;\n", i, text));
      free(text);
    } break;
    case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
      // printf("mtmi-5\n");
      // Do MC_invokes
      bool contains_mc_function_call;
      if (argument) {
        MCcall(mct_contains_mc_invoke(argument, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      MCcall(copy_syntax_node_to_text(argument, &text));
      MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &%s;\n", i, text));
      free(text);

    } break;
    case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
      // printf("mtmi-6\n");
      // Do MC_invokes
      bool contains_mc_function_call;
      if (argument->element_access_expression.primary) {
        MCcall(mct_contains_mc_invoke(argument->element_access_expression.primary, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }
      if (argument->element_access_expression.access_expression) {
        MCcall(
            mct_contains_mc_invoke(argument->element_access_expression.access_expression, &contains_mc_function_call));
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      MCcall(copy_syntax_node_to_text(argument, &text));
      MCcall(mct_append_indent_to_c_str(str, indent + 1));
      MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &%s;\n", i, text));
      free(text);

    } break;
    case MC_SYNTAX_STRING_LITERAL_EXPRESSION: {
      // printf("mtmi-7\n");
      MCcall(mct_append_indent_to_c_str(str, indent + 1));
      char *text;
      MCcall(copy_syntax_node_to_text(argument, &text));
      MCvacall(append_to_c_strf(str, "const char *mc_vargs_%i = %s;\n", i, text));
      free(text);
      MCcall(mct_append_indent_to_c_str(str, indent + 1));
      MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i));
    } break;
    case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
      // printf("mtmi-8\n");
      char *text;
      if ((mc_token_type)argument->prepended_unary.prepend_operator->type == MC_TOKEN_AMPERSAND_CHARACTER) {
        MCcall(copy_syntax_node_to_text(argument, &text));
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "void *mc_varg_%i = (void *)%s;\n", i, text));
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &mc_varg_%i;\n", i, i));
      }
      else {
        MCerror(96, "TODO");
      }
      free(text);
    } break;
    default: {
      // printf("mtmi-9\n");
      switch ((mc_token_type)argument->type) {
      case MC_TOKEN_NUMERIC_LITERAL: {
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "int mc_vargs_%i = %s;\n", i, argument->text));
        MCcall(mct_append_indent_to_c_str(str, indent + 1));
        MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i));
      } break;
      case MC_TOKEN_IDENTIFIER: {
        if (!strcmp(argument->text, "NULL")) {
          MCcall(mct_append_indent_to_c_str(str, indent + 1));
          MCvacall(append_to_c_strf(str, "void *mc_vargs_%i = NULL;\n", i, argument->text));
          MCcall(mct_append_indent_to_c_str(str, indent + 1));
          MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i));
        }
        else {
          MCcall(mct_append_indent_to_c_str(str, indent + 1));
          MCvacall(append_to_c_strf(str, "mc_vargs[%i] = &%s;\n", i, argument->text));
        }
      } break;
      default:
        MCerror(92, "Unsupported:%s", get_mc_syntax_token_type_name(argument->type));
      }
    }
    }
  }

  register_midge_error_tag("mct_transcribe_mc_invocation-return");
  if (strcmp(finfo->return_type.name, "void") || finfo->return_type.deref_count) {
    if (!return_variable_name) {
      // Use a dummy value
      MCcall(mct_append_to_c_str(str, indent + 1, finfo->return_type.name));
      MCcall(append_to_c_str(str, " "));
      for (int i = 0; i < finfo->return_type.deref_count; ++i) {
        MCcall(append_to_c_str(str, "*"));
      }

      MCcall(append_to_c_str(str, "mc_vargs_dummy_rv"));
      if (finfo->return_type.deref_count) {
        MCcall(append_to_c_str(str, " = NULL;\n"));
      }
      else {
        MCcall(append_to_c_str(str, ";\n"));
      }

      MCcall(mct_append_to_c_str(str, indent + 1, "mc_vargs["));
      MCvacall(append_to_c_strf(str, "%i] = &mc_vargs_dummy_rv;\n", finfo->parameter_count));
    }
    else {
      MCcall(mct_append_to_c_str(str, indent + 1, "mc_vargs["));
      MCvacall(append_to_c_strf(str, "%i] = &%s;\n", finfo->parameter_count, return_variable_name));
    }
  }

  MCcall(mct_append_to_c_str(str, indent + 1, "{\n"));
  MCcall(mct_append_to_c_str(str, indent + 2, "int midge_error_stack_index;\n"));

  MCcall(mct_append_indent_to_c_str(str, indent + 2));
  MCvacall(append_to_c_strf(str, "register_midge_stack_invocation(\"%s\", __FILE__, %i, &midge_error_stack_index);\n",
                            finfo->name, syntax_node->begin.line));

  MCcall(mct_append_indent_to_c_str(str, indent + 2));
  MCvacall(append_to_c_strf(str, "%s(%i, mc_vargs);\n", finfo->name, finfo->parameter_count + 1));

  MCcall(mct_append_indent_to_c_str(str, indent + 2));
  MCvacall(append_to_c_str(str, "register_midge_stack_return(midge_error_stack_index);\n"));

  MCcall(mct_append_to_c_str(str, indent + 1, "}\n"));
  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  register_midge_error_tag("mct_transcribe_mc_invocation(~)");
  return 0;
}

int mct_transcribe_declarator(c_str *str, mc_syntax_node *syntax_node)
{
  if (syntax_node->local_variable_declarator.type_dereference) {
    MCcall(mct_append_node_text_to_c_str(str, syntax_node->local_variable_declarator.type_dereference));
  }
  MCcall(append_to_c_str(str, " "));
  MCcall(mct_append_node_text_to_c_str(str, syntax_node->local_variable_declarator.variable_name));

  if (syntax_node->local_variable_declarator.initializer) {
    if (syntax_node->local_variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER) {
      MCcall(append_to_c_str(str, " = "));
      MCcall(mct_transcribe_expression(
          str,
          syntax_node->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression));
    }
    else {
      MCcall(append_to_c_str(str, "["));
      MCcall(mct_transcribe_expression(
          str, syntax_node->local_variable_declarator.initializer->local_variable_array_initializer.size_expression));
      MCcall(append_to_c_str(str, "]"));
    }
  }

  return 0;
}

int mct_transcribe_type_identifier(c_str *str, mc_syntax_node *syntax_node)
{
  // Const
  if (syntax_node->type_identifier.is_const) {
    MCcall(append_to_c_str(str, "const "));
  }

  // Signing
  if (syntax_node->type_identifier.is_signed != -1) {
    if (syntax_node->type_identifier.is_signed == 0) {
      MCcall(append_to_c_str(str, "unsigned "));
    }
    else if (syntax_node->type_identifier.is_signed == 1) {
      MCcall(append_to_c_str(str, "signed "));
    }
    else {
      MCerror(265, "TODO");
    }
  }
  if (syntax_node->type_identifier.size_modifiers) {
    MCerror(270, "TODO");
  }

  // struct prepend
  if (syntax_node->type_identifier.has_struct_prepend) {
    MCcall(append_to_c_str(str, "struct "));
  }

  // mc_type
  MCcall(find_struct_info(syntax_node->type_identifier.identifier->text, &syntax_node->type_identifier.mc_type));
  // printf("mcs: find_struct_info(%s)=='%s'\n", (*type_identifier)->text,
  //        (*mc_type) == NULL ? "(null)" : (*mc_type)->declared_mc_name);

  if (syntax_node->type_identifier.mc_type) {
    // MCcall(print_syntax_node(syntax_node, 0));
    // printf("syntax_node:%p\n", syntax_node);
    // printf("syntax_node->type_identifier.mc_type:%p\n", syntax_node->type_identifier.mc_type);
    // printf("syntax_node->type_identifier.mc_type->mc_declared_name:%p\n",
    //        syntax_node->type_identifier.mc_type->mc_declared_name);
    // printf("syntax_node->type_identifier.mc_type->mc_declared_name:%s\n",
    //        syntax_node->type_identifier.mc_type->mc_declared_name);
    MCcall(append_to_c_str(str, syntax_node->type_identifier.mc_type->mc_declared_name));
  }
  else {
    MCcall(mct_append_node_text_to_c_str(str, syntax_node->type_identifier.identifier));
  }

  return 0;
}

int mct_transcribe_declaration_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_declaration_statement()");

  mc_syntax_node *declaration = syntax_node->declaration_statement.declaration;

  // Do MC_invokes
  // if (contains_mc_function_call) {
  MCcall(mct_append_indent_to_c_str(str, indent));
  MCcall(mct_transcribe_type_identifier(str, declaration->local_variable_declaration.type_identifier));
  MCcall(append_to_c_str(str, " "));

  for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
    if (i > 0) {
      MCcall(append_to_c_str(str, ", "));
    }

    mc_syntax_node *declarator = declaration->local_variable_declaration.declarators->items[i];
    if (declarator->local_variable_declarator.type_dereference) {
      MCcall(mct_append_node_text_to_c_str(str, declarator->local_variable_declarator.type_dereference));
    }
    MCcall(mct_append_node_text_to_c_str(str, declarator->local_variable_declarator.variable_name));

    if (!declarator->local_variable_declarator.initializer) {
      continue;
    }

    bool contains_mc_function_call;
    MCcall(mct_contains_mc_invoke(declarator->local_variable_declarator.initializer, &contains_mc_function_call));
    if (contains_mc_function_call) {
      // Skip - do later
      continue;
    }

    if (declarator->local_variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER) {
      MCcall(append_to_c_str(str, " = "));
      MCcall(mct_transcribe_expression(
          str,
          declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression));
    }
    else {
      MCcall(append_to_c_str(str, "["));
      MCcall(mct_transcribe_expression(
          str, declarator->local_variable_declarator.initializer->local_variable_array_initializer.size_expression));
      MCcall(append_to_c_str(str, "]"));
    }
  }
  MCcall(append_to_c_str(str, ";\n"));

  for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
    mc_syntax_node *declarator = declaration->local_variable_declaration.declarators->items[i];

    if (!declarator->local_variable_declarator.initializer) {
      continue;
    }
    bool contains_mc_function_call;
    MCcall(mct_contains_mc_invoke(declarator->local_variable_declarator.initializer, &contains_mc_function_call));
    if (!contains_mc_function_call) {
      continue;
    }

    if (declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression
            ->type != MC_SYNTAX_INVOCATION) {
      MCerror(250, "Nested mc invokes not yet supported");
    }

    MCerror(412, "TODO -- integrate new invocation methods with this");
    MCcall(mct_transcribe_mc_invocation(
        str, indent,
        declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression,
        declarator->local_variable_declarator.variable_name));
    continue;
  }

  return 0;
}

int mct_transcribe_expression(c_str *str, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_expression(%s)", get_mc_syntax_token_type_name(syntax_node->type));
  // printf("mct_transcribe_expression(%s)\n", get_mc_syntax_token_type_name(syntax_node->type));
  // print_syntax_node(syntax_node, 0);

  switch (syntax_node->type) {
  // {
  //   print_syntax_node(syntax_node, 1);
  //   MCerror(254, "TODO");
  // } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATION: {
    // printf("Local_declaration:\n");
    // print_syntax_node(syntax_node, 1);
    MCcall(mct_transcribe_type_identifier(str, syntax_node->local_variable_declaration.type_identifier));

    MCcall(append_to_c_str(str, " "));

    for (int a = 0; a < syntax_node->local_variable_declaration.declarators->count; ++a) {
      if (a > 0) {
        MCcall(append_to_c_str(str, ", "));
      }
      MCcall(mct_transcribe_declarator(str, syntax_node->local_variable_declaration.declarators->items[a]));
    }
  } break;
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION: {
    MCcall(mct_append_node_text_to_c_str(str, syntax_node->assignment_expression.variable));
    MCcall(append_to_c_str(str, " "));
    MCcall(mct_append_node_text_to_c_str(str, syntax_node->assignment_expression.assignment_operator));
    MCcall(append_to_c_str(str, " "));
    MCcall(mct_transcribe_expression(str, syntax_node->assignment_expression.value_expression));
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    MCcall(append_to_c_str(str, "("));
    MCcall(mct_transcribe_expression(str, syntax_node->parenthesized_expression.expression));
    MCcall(append_to_c_str(str, ")"));
  } break;

    // WILL have to redo in future
  case MC_SYNTAX_DEREFERENCE_EXPRESSION:
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
  case MC_SYNTAX_CONDITIONAL_EXPRESSION:
  case MC_SYNTAX_OPERATIONAL_EXPRESSION:
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION:
  case MC_SYNTAX_RELATIONAL_EXPRESSION: {
    MCcall(mct_append_node_text_to_c_str(str, syntax_node));
  } break;
  case MC_SYNTAX_CAST_EXPRESSION: {
    MCcall(append_to_c_str(str, "("));

    MCcall(mct_transcribe_type_identifier(str, syntax_node->cast_expression.type_identifier));
    // if (syntax_node->cast_expression.type_identifier->type_identifier.mc_type) {
    //   printf("cast expression had mc type:'%s'\n",
    //          syntax_node->cast_expression.type_identifier->type_identifier.mc_type->declared_mc_name);
    //   MCcall(append_to_c_str(str,
    //                          syntax_node->cast_expression.type_identifier->type_identifier.mc_type->declared_mc_name));
    // }
    // else {
    //   printf("cast expression had type:\n");
    //   print_syntax_node(syntax_node->cast_expression.type_identifier, 1);
    //   MCcall(mct_append_node_text_to_c_str(str, syntax_node->cast_expression.type_identifier));
    // }

    if (syntax_node->cast_expression.type_dereference) {
      MCcall(append_to_c_str(str, " "));
      MCcall(mct_append_node_text_to_c_str(str, syntax_node->cast_expression.type_dereference));
    }
    MCcall(append_to_c_str(str, ")"));

    MCcall(mct_transcribe_expression(str, syntax_node->cast_expression.expression));
  } break;
  case MC_SYNTAX_VA_ARG_EXPRESSION: {
    MCcall(append_to_c_str(str, "va_arg("));

    MCcall(mct_transcribe_type_identifier(str, syntax_node->va_arg_expression.list_identity));
    MCcall(append_to_c_str(str, ", "));
    MCcall(mct_transcribe_type_identifier(str, syntax_node->va_arg_expression.type_identifier));
    MCcall(append_to_c_str(str, ")\n"));
  } break;
  case MC_SYNTAX_SIZEOF_EXPRESSION: {
    MCcall(append_to_c_str(str, "sizeof("));

    MCcall(mct_transcribe_type_identifier(str, syntax_node->cast_expression.type_identifier));
    // if (syntax_node->sizeof_expression.type_identifier->type_identifier.mc_type) {
    //   MCcall(append_to_c_str(
    //       str, syntax_node->sizeof_expression.type_identifier->type_identifier.mc_type->declared_mc_name));
    // }
    // else {
    //   MCcall(mct_append_node_text_to_c_str(str, syntax_node->sizeof_expression.type_identifier));
    // }

    if (syntax_node->sizeof_expression.type_dereference) {
      MCcall(append_to_c_str(str, " "));
      MCcall(mct_append_node_text_to_c_str(str, syntax_node->sizeof_expression.type_dereference));
    }
    MCcall(append_to_c_str(str, ")"));
  } break;
  case MC_SYNTAX_INVOCATION: {
    if (syntax_node->invocation.mc_function_info) {
      MCerror(247, "Not supported from here, have to deal with it earlier");
    }

    MCcall(mct_append_node_text_to_c_str(str, syntax_node->invocation.function_identity));
    MCcall(append_to_c_str(str, "("));
    for (int a = 0; a < syntax_node->invocation.arguments->count; ++a) {
      if (a > 0) {
        MCcall(append_to_c_str(str, ", "));
      }

      MCcall(mct_transcribe_expression(str, syntax_node->invocation.arguments->items[a]));
    }
    MCcall(append_to_c_str(str, ")"));
  } break;

  // PROBABLY won't have to redo
  // case MC_SYNTAX_DECLARATION_STATEMENT: {

  // }break;
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION:
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION:
  case MC_SYNTAX_FIXREMENT_EXPRESSION: {
    MCcall(mct_append_node_text_to_c_str(str, syntax_node));
  } break;
  default:
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_NUMERIC_LITERAL:
    case MC_TOKEN_CHAR_LITERAL:
    case MC_TOKEN_IDENTIFIER: {
      MCcall(mct_append_node_text_to_c_str(str, syntax_node));
    } break;
    default:
      MCerror(291, "MCT:Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    }
  }

  register_midge_error_tag("mct_transcribe_expression(~)");
  return 0;
}

int mct_transcribe_if_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_if_statement()");
  // printf("cb: %p\n", syntax_node);

  // Do MC_invokes
  bool contains_mc_function_call;
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

int _mc_transcribe_invocation(c_str *str, int indent, mc_syntax_node *syntax_node, int mc_invocation_result_depth)
{

  return 0;
}

int mc_transcribe_invocation_statement(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  if (syntax_node->type != MC_SYNTAX_INVOCATION) {
    MCerror(728, "TODO");
  }
  bool contains_nested_mc_function_call = false;
  for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
    MCcall(mct_contains_mc_invoke(syntax_node->invocation.arguments->items[i], &contains_nested_mc_function_call));
    if (*contains_nested_mc_function_call) {
      break;
    }
  }

  if (contains_nested_mc_function_call) {
    ++indent;
    MCcall(mct_append_to_c_str(str, indent, "{\n"));
  }
  MCcall(mct_append_indent_to_c_str(str, indent));
  int statement_begin_index = str->len - 1;
  int mc_invoke_result_index = 0;

  ++indent;
  MCcall(append_to_c_str(str, "{\n"));

  MCcall(mct_append_indent_to_c_str(str, indent));
  MCcall(append_to_c_str(str, "int midge_error_stack_index;\n"));
  MCcall(append_to_c_str(str, "register_midge_stack_invocation(\""));
  MCcall(mct_append_node_text_to_c_str(str, syntax_node->invocation.function_identity));
  MCvacall(append_to_c_strf(str, "\", __FILE__, %i, &midge_error_stack_index);\n", syntax_node->begin.line));

  MCcall(mct_append_indent_to_c_str(str, indent));

  if (syntax_node->invocation.mc_function_info) {
    // Do the arguments first
  }
  MCcall(append_to_c_str(str, ))

      MCcall(mct_transcribe_expression(str, child->expression_statement.expression));
  MCcall(append_to_c_str(str, ";\n"));

  MCcall(mct_append_indent_to_c_str(str, indent));
  MCcall(append_to_c_str(str, "register_midge_stack_return(midge_error_stack_index);\n"));
  --indent;
  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  if (contains_nested_mc_function_call) {
    --indent;
    MCcall(mct_append_to_c_str(str, indent, "}\n"));
  }
  // for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
  //   mc_syntax_node *argument = syntax_node->invocation.arguments->items[i];
  //   bool contains_nested_mc_function_call = false;
  //   MCcall(mct_contains_mc_invoke(argument, &contains_nested_mc_function_call));
  //   if (*contains_nested_mc_function_call) {
  //     if (argument->type != MC_SYNTAX_INVOCATION || !argument->invocation.mc_function_info) {
  //       MCerror(742, "Not Yet Supported: mccalls inside normal function calls as an argument to another function
  //       call");
  //     }
  //     MCcall(_mc_transcribe_invocation(str, indent, argument, &mc_invoke_result_index));
  //     break;
  //   }
  // }
  // Do MC_invokes
  // bool is_mc_function_call = syntax_node->invocation.mc_function_info;
  // bool contains_nested_mc_function_call = false;
  // for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
  //   MCcall(mct_contains_mc_invoke(syntax_node->invocation.arguments->items[i], &contains_nested_mc_function_call));
  //   if (*contains_nested_mc_function_call) {
  //     break;
  //   }
  // }

  // if (!is_mc_function_call && !contains_nested_mc_function_call) {
  //   // non-mc invocations
  //   MCcall(mct_append_to_c_str(str, indent, "{\n"));
  //   ++indent;
  //   MCcall(mct_append_to_c_str(str, indent, "int midge_error_stack_index;\n"));

  // }
  // else {
  //   // mc_invocations
  //   // Transcribe all nested invocations first
  //   if (contains_nested_mc_function_call)
  //     int mc_invoke_result_index = 0;
  //   for (int a = 0; a < syntax_node->invocation.arguments->count; ++a) {

  //     _mc_transcribe_
  //   }
  // }

  if ()
    // printf("bb-2\n");
    // MCcall(print_syntax_node(child->expression_statement.expression, 0));
    MCcall(mct_contains_mc_invoke(child->expression_statement.expression, &contains_mc_function_call));
  if (contains_mc_function_call) {
    // printf("bb-3\n");
    // MCcall(print_syntax_node(child->expression_statement.expression, 0));
    MCcall(mct_transcribe_mc_invocation(str, indent, child->expression_statement.expression, NULL));
    break;
  }

  // TODO -- maybe more coverage (atm only doing invocation expresssions. NOT invocations nested in other
  // expressions)
  if (child->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
  }

  return 0;
}

int mct_transcribe_statement_list(c_str *str, int indent, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_statement_list()");
  // printf("mct_transcribe_statement_list()\n");
  if (syntax_node->type != MC_SYNTAX_STATEMENT_LIST) {
    printf("INVALID! tsl");
    print_syntax_node(syntax_node, 0);
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

        MCcall(mct_append_indent_to_c_str(str, indent));
        MCcall(append_to_c_str(str, "*mc_return_value = "));
        MCcall(mct_transcribe_expression(str, child->return_statement.expression));
        MCcall(append_to_c_str(str, ";\n"));
      }
      {
        // Attempt to obtain function name
        mc_syntax_node *root = syntax_node;
        while (root->type != MC_SYNTAX_FUNCTION && root->parent) {
          root = root->parent;
        }
        if (root->type == MC_SYNTAX_FUNCTION) {
          if ((mc_token_type)root->function.name->type != MC_TOKEN_IDENTIFIER) {
            MCerror(786, "Checkit");
          }

          MCcall(mct_append_indent_to_c_str(str, indent));
          MCvacall(append_to_c_strf(str, "register_midge_error_tag(\"%s(~)\");\n", root->function.name->text));
        }
      }
      MCcall(mct_append_indent_to_c_str(str, indent));
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
      MCcall(mct_transcribe_declaration_statement(str, indent, child));
    } break;
    case MC_SYNTAX_EXPRESSION_STATEMENT: {
      register_midge_error_tag("mct_transcribe_statement_list-ES0");
      if (child->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
        MCcall(mc_transcribe_invocation_statement(str, indent, child->expression_statement.expression));
      }
      else {
        MCcall(mct_transcribe_expression(str, child->expression_statement.expression))
            MCcall(append_to_c_str(str, ";\n"));
      }
    } break;
    default:
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_NEW_LINE:
      case MC_TOKEN_SPACE_SEQUENCE:
      case MC_TOKEN_TAB_SEQUENCE:
      case MC_TOKEN_LINE_COMMENT:
      case MC_TOKEN_MULTI_LINE_COMMENT: {
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

  if (syntax_node->code_block.statement_list) {
    MCcall(mct_transcribe_statement_list(str, indent + 1, syntax_node->code_block.statement_list));
  }

  MCcall(mct_append_to_c_str(str, indent, "}\n"));

  register_midge_error_tag("mct_transcribe_code_block(~)");
  return 0;
}

int mct_transcribe_field(c_str *str, int indent, mc_syntax_node *syntax_node)
{

  MCcall(mct_append_indent_to_c_str(str, indent));

  switch (syntax_node->field.field_kind) {
  case FIELD_KIND_STANDARD: {
    MCcall(mct_transcribe_type_identifier(str, syntax_node->field.type_identifier));
    MCcall(append_to_c_str(str, " "));
    if (syntax_node->field.type_dereference) {
      for (int d = 0; d < syntax_node->field.type_dereference->dereference_sequence.count; ++d) {
        MCcall(append_to_c_str(str, "*"));
      }
    }
    MCcall(mct_append_node_text_to_c_str(str, syntax_node->field.name));
  } break;
  default:
    MCerror(873, "NotSupported:%i", syntax_node->field.field_kind);
  }

  return 0;
}

// int transcribe_code_block_ast_to_mc_definition(mc_syntax_node *syntax_node, char **output)
// {
//   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition()");

//   if (syntax_node->type != MC_SYNTAX_BLOCK) {
//     MCerror(861, "MCT:Not Supported");
//   }

//   c_str *str;
//   MCcall(init_c_str(&str));

//   if (syntax_node->code_block.statement_list) {
//     MCcall(mct_transcribe_statement_list(str, 1, syntax_node->code_block.statement_list));
//   }

//   *output = str->text;
//   MCcall(release_c_str(str, false));

//   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition(~)");
//   return 0;
// }

int transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast, char **mc_transcription)
{
  register_midge_error_tag("transcribe_function_to_mc()");

  if (function_ast->type != MC_SYNTAX_FUNCTION) {
    MCerror(889, "MCT:Not Supported");
  }
  if (!function_ast->function.code_block || function_ast->function.code_block->type != MC_SYNTAX_BLOCK ||
      !function_ast->function.code_block->code_block.statement_list) {
    print_syntax_node(function_ast, 0);
    MCerror(893, "TODO");
  }
  if ((mc_token_type)function_ast->function.name->type != MC_TOKEN_IDENTIFIER) {
    MCerror(898, "TODO");
  }

  c_str *str;
  MCcall(init_c_str(&str));

  // Header
  MCvacall(append_to_c_strf(str, "int %s_v%u(int mc_argsc, void **mc_argsv) {\n", function_ast->function.name->text,
                            func_info->latest_iteration));

  // Initial
  MCvacall(append_to_c_strf(str, "  register_midge_error_tag(\"%s()\");\n\n", function_ast->function.name->text));

  // Function Parameters
  MCcall(append_to_c_str(str, "  // Function Parameters\n"));
  for (int p = 0; p < function_ast->function.parameters->count; ++p) {
    mc_syntax_node *parameter_syntax = function_ast->function.parameters->items[p];

    switch (parameter_syntax->parameter.parameter_kind) {
    case PARAMETER_KIND_STANDARD: {
      MCcall(mct_transcribe_type_identifier(str, parameter_syntax->parameter.type_identifier));
      MCcall(mct_append_indent_to_c_str(str, 1));
      if (parameter_syntax->parameter.type_dereference) {
        for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
          MCcall(append_to_c_str(str, "*"));
        }
      }
      MCcall(mct_append_node_text_to_c_str(str, parameter_syntax->parameter.name));
      MCcall(append_to_c_str(str, " = *("));
      MCcall(mct_transcribe_type_identifier(str, parameter_syntax->parameter.type_identifier));
      MCcall(append_to_c_str(str, " "));
      if (parameter_syntax->parameter.type_dereference) {
        for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
          MCcall(append_to_c_str(str, "*"));
        }
      }
      MCvacall(append_to_c_strf(str, "*)mc_argsv[%i];\n", p));
    } break;
    default:
      MCerror(958, "NotSupported:%i", parameter_syntax->parameter.parameter_kind);
    }

    // if (parameter_syntax->parameter.is_function_pointer_declaration) {
    //   printf("918 TODO\n");
    //   print_syntax_node(parameter_syntax, 0);
    //   MCerror(912, "TODO");
    //   continue;
    // }
  }
  if (function_ast->function.return_type_dereference ||
      strcmp(function_ast->function.return_type_identifier->type_identifier.identifier->text, "void")) {
    MCcall(mct_append_indent_to_c_str(str, 1));

    MCcall(mct_append_node_text_to_c_str(str, function_ast->function.return_type_identifier));
    MCcall(append_to_c_str(str, " "));
    if (function_ast->function.return_type_dereference)
      MCcall(mct_append_node_text_to_c_str(str, function_ast->function.return_type_dereference));
    MCcall(append_to_c_str(str, "*mc_return_value;\n"));
  }

  // Code Block
  MCcall(append_to_c_str(str, "  // Function Code\n"));
  MCcall(mct_transcribe_statement_list(str, 0, function_ast->function.code_block->code_block.statement_list));
  MCvacall(append_to_c_strf(str,
                            "\n"
                            "  register_midge_error_tag(\"%s(~)\");\n"
                            "  return 0;\n}",
                            function_ast->function.name->text));

  *mc_transcription = str->text;
  MCcall(release_c_str(str, false));

  // printf("mc_transcription:\n%s||\n", *mc_transcription);

  register_midge_error_tag("transcribe_function_to_mc(~)");
  return 0;
}

int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *structure_ast, char **mc_transcription)
{
  register_midge_error_tag("transcribe_struct_to_mc()");

  if (structure_ast->type != MC_SYNTAX_STRUCTURE) {
    MCerror(964, "MCT:Not Supported");
  }

  c_str *str;
  MCcall(init_c_str(&str));

  // Header
  MCcall(append_to_c_str(str, "typedef struct \n"));
  MCcall(append_to_c_str(str, structure_info->mc_declared_name));
  MCcall(append_to_c_str(str, " { \n"));

  int indent = 1;
  for (int f = 0; f < structure_ast->structure.fields->count; ++f) {
    mc_syntax_node *field_syntax = structure_ast->structure.fields->items[f];

    MCcall(mct_append_indent_to_c_str(str, indent));
    MCcall(mct_transcribe_field(str, indent, field_syntax));
    MCcall(append_to_c_str(str, ";\n"));
  }

  MCvacall(append_to_c_strf(str, "} mc_%s_v%u;", structure_ast->structure.name->text,
                            structure_info->version)); // TODO -- types not structs

  *mc_transcription = str->text;
  release_c_str(str, false);

  // print_syntax_node(structure_ast, 0);
  // printf("def:\n%s||\n", *mc_transcription);

  register_midge_error_tag("transcribe_struct_to_mc(~)");
  return 0;
}