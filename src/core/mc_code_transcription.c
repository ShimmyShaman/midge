/* mc_code_parser.c */

#include "core/mc_code_transcription.h"

struct mct_transcription_state;
int mct_transcribe_code_block(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_statement_list(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_expression(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_field(mct_transcription_state *ts, mc_syntax_node *syntax_node);

#define MCT_TS_MAX_VARIABLES 64
#define MCT_TS_MAX_SCOPE_DEPTH 256
typedef struct mct_transcription_state {
  // Options
  bool report_invocations_to_error_stack;
  bool tag_on_function_entry;

  // State Values
  mc_syntax_node *transcription_root;
  c_str *str;
  int indent;

  // Variable scopes
  struct {
    int variable_count;
    struct {
      mc_syntax_node *declaration_node;
      char *name;
    } variables[64];
  } scope[256];
  int scope_index;
} mct_transcription_state;

int mct_append_node_text_to_c_str(c_str *str, mc_syntax_node *syntax_node)
{
  char *node_text;
  copy_syntax_node_to_text(syntax_node, &node_text);
  // printf("copied:'%s' to text\n", node_text);
  append_to_c_str(str, node_text);
  free(node_text);

  return 0;
}

int mct_append_indent_to_c_str(mct_transcription_state *ts)
{
  const char *INDENT = "  ";
  for (int i = 0; i < ts->indent; ++i) {
    append_to_c_str(ts->str, INDENT);
  }
  return 0;
}

int mct_append_to_c_str(mct_transcription_state *ts, const char *text)
{
  mct_append_indent_to_c_str(ts);
  append_to_c_str(ts->str, text);

  return 0;
}

int mct_increment_scope_depth(mct_transcription_state *ts)
{
  if (ts->scope_index + 1 >= MCT_TS_MAX_SCOPE_DEPTH) {
    MCerror(65, "Function Scope depth reached maxim allowed");
  }

  ++ts->scope_index;
  ts->scope[ts->scope_index].variable_count = 0;

  return 0;
}

int mct_decrement_scope_depth(mct_transcription_state *ts)
{
  if (ts->scope_index == 0) {
    MCerror(76, "scope reporting was messed up somewhere");
  }

  int n = ts->scope[ts->scope_index].variable_count;
  for (int i = 0; i < n; ++i) {
    if (ts->scope[ts->scope_index].variables[i].name)
      free(ts->scope[ts->scope_index].variables[i].name);
  }

  --ts->scope_index;

  return 0;
}

int mct_add_scope_variable(mct_transcription_state *ts, mc_syntax_node *variable_syntax_node)
{
  int variable_index_in_scope = ts->scope[ts->scope_index].variable_count;
  if (variable_index_in_scope >= MCT_TS_MAX_VARIABLES) {
    MCerror(93, "Variables in this scope have exceeded the maximum allocated");
  }

  switch (variable_syntax_node->type) {
  case MC_SYNTAX_PARAMETER_DECLARATION: {
    ts->scope[ts->scope_index].variables[variable_index_in_scope].declaration_node = variable_syntax_node;
    copy_syntax_node_to_text(variable_syntax_node->parameter.name,
                             &ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
  } break;
  case MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR: {
    ts->scope[ts->scope_index].variables[variable_index_in_scope].declaration_node = variable_syntax_node;
    copy_syntax_node_to_text(variable_syntax_node->local_variable_declarator.variable_name,
                             &ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
  } break;
  default:
    MCerror(105, "Unsupported:%s", get_mc_syntax_token_type_name(variable_syntax_node->type));
    break;
  }

  // printf("$-#%i#-'%s'\n", ts->scope_index, ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
  ++ts->scope[ts->scope_index].variable_count;

  return 0;
}

int determine_type_of_expression(mct_transcription_state *ts, mc_syntax_node *expression, char **type_identity,
                                 int *deref_count)
{
  switch (expression->type) {
  case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
    // Determine the type of the left-hand-side
    determine_type_of_expression(ts, expression->operational_expression.left, type_identity, deref_count);
  } break;
  case MC_TOKEN_IDENTIFIER: {
    *type_identity = NULL;
    *deref_count = 0;

    for (int d = ts->scope_index; d >= 0; --d) {
      // printf("scope:%i c-:%i\n", d, ts->scope[d].variable_count);
      for (int b = 0; b < ts->scope[d].variable_count; ++b) {
        // printf("%s<>%s\n", ts->scope[d].variables[b].name, expression->text);
        if (!strcmp(ts->scope[d].variables[b].name, expression->text)) {
          switch (ts->scope[d].variables[b].declaration_node->type) {
          case MC_SYNTAX_PARAMETER_DECLARATION: {
            mc_syntax_node *param_decl = ts->scope[d].variables[b].declaration_node;

            // char *type_identity;
            copy_syntax_node_to_text(param_decl->parameter.type_identifier, type_identity);

            if (param_decl->parameter.type_dereference) {
              *deref_count = param_decl->parameter.type_dereference->dereference_sequence.count;
            }
            else {
              deref_count = 0;
            }
          } break;
          default:
            MCerror(134, "Unsupported :%s",
                    get_mc_syntax_token_type_name(ts->scope[d].variables[b].declaration_node->type));
          }
        }
      }
    }

    // printf("couldn't find type for:'%s'\n", expression->text);
  } break;
  default:
    MCerror(129, "Unsupported:%s", get_mc_syntax_token_type_name(expression->type));
  }

  return 0;
}

int mct_transcribe_va_list_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_append_indent_to_c_str(ts);
  append_to_c_strf(ts->str, "int %s = -1;\n", syntax_node->va_list_expression.list_identity->text);

  return 0;
}

int mct_contains_mc_invoke(mc_syntax_node *syntax_node, bool *result)
{
  register_midge_error_tag("mct_contains_mc_invoke(%s)", get_mc_syntax_token_type_name(syntax_node->type));

  *result = false;
  if ((mc_token_type)syntax_node->type <= MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    return 0;
  }

  register_midge_error_tag("mct_contains_mc_invoke()-1");
  if (syntax_node->type == MC_SYNTAX_INVOCATION &&
      (mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER) {

    function_info *func_info;
    find_function_info(syntax_node->invocation.function_identity->text, &func_info);

    if (func_info) {
      // print_syntax_node(syntax_node, 0);
      // MCerror(35, "TODO : %s", syntax_node->invocation.function_identity->text);
      register_midge_error_tag("mct_contains_mc_invoke()-2");

      // printf("mcmi-3\n");
      *result = true;
      return 0;
    }
  }

  register_midge_error_tag("mct_contains_mc_invoke()-3 child_count:%i", syntax_node->children->count);
  for (int i = 0; i < syntax_node->children->count; ++i) {
    mct_contains_mc_invoke(syntax_node->children->items[i], result);
    if (*result) {
      return 0;
    }
  }

  register_midge_error_tag("mct_contains_mc_invoke(~)");
  return 0;
}

int mct_transcribe_mc_invocation(mct_transcription_state *ts, mc_syntax_node *syntax_node, char *return_variable_name)
{
  register_midge_error_tag("mct_transcribe_mc_invocation()");

  // print_syntax_node(syntax_node, 0);

  if (syntax_node->type != MC_SYNTAX_INVOCATION) {
    MCerror(70, "TODO %s", get_mc_syntax_token_type_name(syntax_node->type));
  }

  function_info *func_info;
  char *function_name;
  copy_syntax_node_to_text(syntax_node->invocation.function_identity, &function_name);
  find_function_info(function_name, &func_info);
  free(function_name);

  if (!func_info) {
    print_syntax_node(syntax_node, 0);
    MCerror(86, "Not an mc_function?");
  }
  // print_syntax_node(syntax_node, 0);
  // printf("mtmi-2 %p\n", finfo);
  // printf("mtmi-2 %i\n", finfo->parameter_count + 1);

  mct_append_to_c_str(ts, "{\n");
  ++ts->indent;
  mct_append_to_c_str(ts, "void *mc_vargs[");
  append_to_c_strf(ts->str, "%i];\n", func_info->parameter_count + 1);

  register_midge_error_tag("mct_transcribe_mc_invocation-parameters");
  if (func_info->parameter_count != syntax_node->invocation.arguments->count) {
    MCerror(79, "argument count not equal to required parameters, invoke:%s, expected:%i, passed:%i", func_info->name,
            func_info->parameter_count, syntax_node->invocation.arguments->count);
  }

  for (int i = 0; i < func_info->parameter_count; ++i) {
    // printf("mtmi-3\n");
    mct_append_indent_to_c_str(ts);
    mc_syntax_node *argument = syntax_node->invocation.arguments->items[i];
    switch (argument->type) {
    case MC_SYNTAX_CAST_EXPRESSION: {
      // printf("mtmi-4\n");
      bool contains_mc_function_call;
      if (argument->cast_expression.expression) {
        mct_contains_mc_invoke(argument->cast_expression.expression, &contains_mc_function_call);
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      if (argument->cast_expression.expression->type == MC_SYNTAX_PREPENDED_UNARY_EXPRESSION &&
          (mc_token_type)argument->cast_expression.expression->prepended_unary.prepend_operator->type ==
              MC_TOKEN_AMPERSAND_CHARACTER) {
        copy_syntax_node_to_text(argument->cast_expression.expression, &text);
        append_to_c_strf(ts->str, "void *mc_varg_%i = (void *)%s;\n", i, text);
        mct_append_indent_to_c_str(ts);
        append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_varg_%i;\n", i, i);
      }
      else {
        copy_syntax_node_to_text(argument->cast_expression.expression, &text);
        append_to_c_strf(ts->str, "mc_vargs[%i] = &%s;\n", i, text);
      }
      free(text);
    } break;
    case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
      // printf("mtmi-5\n");
      // Do MC_invokes
      bool contains_mc_function_call;
      {
        mct_contains_mc_invoke(argument, &contains_mc_function_call);
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      copy_syntax_node_to_text(argument, &text);
      append_to_c_strf(ts->str, "mc_vargs[%i] = &%s;\n", i, text);
      free(text);

    } break;
    case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
      // printf("mtmi-4\n");
      {
        bool contains_mc_function_call;
        mct_contains_mc_invoke(argument, &contains_mc_function_call);
        if (contains_mc_function_call) {
          MCerror(290, "TODO");
        }
      }

      // Find the type of the expression
      char *type_str;
      int type_deref_count;
      determine_type_of_expression(ts, argument, &type_str, &type_deref_count);
      // printf("Type:'%s':%i\n", type_str, type_deref_count);

      // Evaluate it to a local field
      append_to_c_str(ts->str, type_str);
      append_to_c_str(ts->str, " ");
      for (int d = 0; d < type_deref_count; ++d) {
        append_to_c_str(ts->str, "*");
      }
      append_to_c_strf(ts->str, "mc_vargs_%i = ", i);
      free(type_str);

      char *expression_text;
      copy_syntax_node_to_text(argument, &expression_text);
      append_to_c_str(ts->str, expression_text);
      append_to_c_str(ts->str, ";\n");
      free(expression_text);

      // Set to parameter reference
      mct_append_indent_to_c_str(ts);
      append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);

      // printf("After:\n%s||\n", ts->str->text);
    } break;
    case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
      // printf("mtmi-6\n");
      // Do MC_invokes
      bool contains_mc_function_call;
      if (argument->element_access_expression.primary) {
        mct_contains_mc_invoke(argument->element_access_expression.primary, &contains_mc_function_call);
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }
      if (argument->element_access_expression.access_expression) {

        mct_contains_mc_invoke(argument->element_access_expression.access_expression, &contains_mc_function_call);
        if (contains_mc_function_call) {
          MCerror(104, "TODO");
        }
      }

      char *text;
      copy_syntax_node_to_text(argument, &text);
      mct_append_indent_to_c_str(ts);
      append_to_c_strf(ts->str, "mc_vargs[%i] = &%s;\n", i, text);
      free(text);

    } break;
    case MC_SYNTAX_STRING_LITERAL_EXPRESSION: {
      // printf("mtmi-7\n");
      char *text;
      copy_syntax_node_to_text(argument, &text);
      append_to_c_strf(ts->str, "const char *mc_vargs_%i = %s;\n", i, text);
      free(text);
      mct_append_indent_to_c_str(ts);
      append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);
    } break;
    case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
      // printf("mtmi-8\n");
      char *text;
      if ((mc_token_type)argument->prepended_unary.prepend_operator->type == MC_TOKEN_AMPERSAND_CHARACTER) {
        copy_syntax_node_to_text(argument, &text);
        append_to_c_strf(ts->str, "void *mc_varg_%i = (void *)%s;\n", i, text);
        mct_append_indent_to_c_str(ts);
        append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_varg_%i;\n", i, i);
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
        append_to_c_strf(ts->str, "int mc_vargs_%i = %s;\n", i, argument->text);
        mct_append_indent_to_c_str(ts);
        append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);
      } break;
      case MC_TOKEN_IDENTIFIER: {
        if (!strcmp(argument->text, "NULL")) {
          append_to_c_strf(ts->str, "void *mc_vargs_%i = NULL;\n", i, argument->text);
          mct_append_indent_to_c_str(ts);
          append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);
        }
        else if (!strcmp(argument->text, "false")) {
          append_to_c_strf(ts->str, "unsigned char mc_vargs_%i = false;\n", i, argument->text);
          mct_append_indent_to_c_str(ts);
          append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);
        }
        else if (!strcmp(argument->text, "true")) {
          append_to_c_strf(ts->str, "unsigned char mc_vargs_%i = true;\n", i, argument->text);
          mct_append_indent_to_c_str(ts);
          append_to_c_strf(ts->str, "mc_vargs[%i] = &mc_vargs_%i;\n", i, i);
        }
        else {
          append_to_c_strf(ts->str, "mc_vargs[%i] = &%s;\n", i, argument->text);
        }
      } break;
      default:
        print_syntax_node(argument, 0);
        MCerror(225, "Unsupported:%s", get_mc_syntax_token_type_name(argument->type));
      }
    }
    }
  }

  register_midge_error_tag("mct_transcribe_mc_invocation-return");
  if (strcmp(func_info->return_type.name, "void") || func_info->return_type.deref_count) {
    if (!return_variable_name) {
      // Use a dummy value
      mct_append_to_c_str(ts, func_info->return_type.name);
      append_to_c_str(ts->str, " ");
      for (int i = 0; i < func_info->return_type.deref_count; ++i) {
        append_to_c_str(ts->str, "*");
      }

      append_to_c_str(ts->str, "mc_vargs_dummy_rv");
      if (func_info->return_type.deref_count) {
        append_to_c_str(ts->str, " = NULL;\n");
      }
      else {
        append_to_c_str(ts->str, ";\n");
      }

      mct_append_to_c_str(ts, "mc_vargs[");
      append_to_c_strf(ts->str, "%i] = &mc_vargs_dummy_rv;\n", func_info->parameter_count);
    }
    else {
      mct_append_to_c_str(ts, "mc_vargs[");

      // char *name;
      // copy_syntax_node_to_text(return_variable_name, &name);
      append_to_c_strf(ts->str, "%i] = &%s;\n", func_info->parameter_count, return_variable_name);
      // free(name);
    }
  }

  if (ts->report_invocations_to_error_stack) {
    mct_append_to_c_str(ts, "{\n");
    ++ts->indent;
    mct_append_to_c_str(ts, "int midge_error_stack_index;\n");

    mct_append_indent_to_c_str(ts);
    append_to_c_strf(ts->str, "register_midge_stack_invocation(\"%s\", __FILE__, %i, &midge_error_stack_index);\n",
                     func_info->name, syntax_node->begin.line);
  }

  mct_append_indent_to_c_str(ts);
  append_to_c_strf(ts->str, "%s(%i, mc_vargs);\n", func_info->name, func_info->parameter_count + 1);

  if (ts->report_invocations_to_error_stack) {
    mct_append_indent_to_c_str(ts);
    append_to_c_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");

    --ts->indent;
    mct_append_to_c_str(ts, "}\n");
  }
  --ts->indent;
  mct_append_to_c_str(ts, "}\n");

  register_midge_error_tag("mct_transcribe_mc_invocation(~)");
  return 0;
}

int mct_transcribe_declarator(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  if (syntax_node->local_variable_declarator.type_dereference) {
    mct_append_node_text_to_c_str(ts->str, syntax_node->local_variable_declarator.type_dereference);
  }
  append_to_c_str(ts->str, " ");
  mct_append_node_text_to_c_str(ts->str, syntax_node->local_variable_declarator.variable_name);

  if (syntax_node->local_variable_declarator.initializer) {
    if (syntax_node->local_variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER) {
      append_to_c_str(ts->str, " = ");
      mct_transcribe_expression(
          ts,
          syntax_node->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression);
    }
    else {
      append_to_c_str(ts->str, "[");
      mct_transcribe_expression(
          ts, syntax_node->local_variable_declarator.initializer->local_variable_array_initializer.size_expression);
      append_to_c_str(ts->str, "]");
    }
  }

  return 0;
}

int mct_transcribe_type_identifier(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  // Const
  if (syntax_node->type_identifier.is_const) {
    append_to_c_str(ts->str, "const ");
  }

  // Signing
  if (syntax_node->type_identifier.is_signed != -1) {
    if (syntax_node->type_identifier.is_signed == 0) {
      append_to_c_str(ts->str, "unsigned ");
    }
    else if (syntax_node->type_identifier.is_signed == 1) {
      append_to_c_str(ts->str, "signed ");
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
    append_to_c_str(ts->str, "struct ");
  }

  // type identifier
  char *mc_declared_name = NULL;
  struct_info *structure_info;
  find_struct_info(syntax_node->type_identifier.identifier->text, &structure_info);
  if (structure_info) {
    append_to_c_str(ts->str, structure_info->mc_declared_name);
  }
  else {
    enumeration_info *enum_info;
    find_enumeration_info(syntax_node->type_identifier.identifier->text, &enum_info);
    if (enum_info) {
      append_to_c_str(ts->str, enum_info->mc_declared_name);
    }
    else {
      mct_append_node_text_to_c_str(ts->str, syntax_node->type_identifier.identifier);
    }
  }

  return 0;
}

int mct_transcribe_declaration_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_declaration_statement()");

  mc_syntax_node *declaration = syntax_node->declaration_statement.declaration;

  // va-args exception
  if (declaration->local_variable_declaration.declarators->count == 1) {
    mc_syntax_node *declarator = declaration->local_variable_declaration.declarators->items[0];
    if (declarator->local_variable_declarator.initializer &&
        declarator->local_variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER &&
        declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression
                ->type == MC_SYNTAX_VA_ARG_EXPRESSION) {

      mc_syntax_node *function_node = syntax_node->parent;
      while (function_node->type != MC_SYNTAX_FUNCTION) {
        function_node = function_node->parent;
        if (function_node == NULL) {
          MCerror(376, "Couldn't find function ancestor");
        }
      }
      if ((mc_token_type)function_node->function.name->type != MC_TOKEN_IDENTIFIER) {
        MCerror(380, "expected otherwise");
      }
      function_info *housing_finfo;
      find_function_info(function_node->function.name->text, &housing_finfo);
      if (!housing_finfo) {
        MCerror(385, "expected otherwise");
      }
      // print_syntax_node(declarator, 0);

      mc_syntax_node *va_arg_expression =
          declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression;

      mct_append_indent_to_c_str(ts);
      append_to_c_strf(
          ts->str, "if(%i + %s + 1 >= mc_argsc) { MCerror(%i, \"va_args access exceeded argument count\");}\n",
          housing_finfo->parameter_count + 1, va_arg_expression->va_arg_expression.list_identity->text, 392);

      mct_append_indent_to_c_str(ts);
      mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
      append_to_c_str(ts->str, " ");
      if (declarator->local_variable_declarator.type_dereference) {
        for (int d = 0; d < declarator->local_variable_declarator.type_dereference->dereference_sequence.count; ++d) {
          append_to_c_str(ts->str, "*");
        }
      }
      mct_append_node_text_to_c_str(ts->str, declarator->local_variable_declarator.variable_name);

      append_to_c_str(ts->str, " = *(");
      mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
      append_to_c_str(ts->str, " ");
      if (declarator->local_variable_declarator.type_dereference) {
        for (int d = 0; d < declarator->local_variable_declarator.type_dereference->dereference_sequence.count; ++d) {
          append_to_c_str(ts->str, "*");
        }
      }
      append_to_c_strf(ts->str, "*)mc_argsv[%i + ++%s];\n", housing_finfo->parameter_count + 1,
                       va_arg_expression->va_arg_expression.list_identity->text);

      return 0;
    }
  }

  // Do MC_invokes
  // if (contains_mc_function_call) {
  mct_append_indent_to_c_str(ts);
  mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
  append_to_c_str(ts->str, " ");

  for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
    if (i > 0) {
      append_to_c_str(ts->str, ", ");
    }

    mc_syntax_node *declarator = declaration->local_variable_declaration.declarators->items[i];
    if (declarator->local_variable_declarator.type_dereference) {
      mct_append_node_text_to_c_str(ts->str, declarator->local_variable_declarator.type_dereference);
    }
    mct_append_node_text_to_c_str(ts->str, declarator->local_variable_declarator.variable_name);

    mct_add_scope_variable(ts, declarator);

    if (!declarator->local_variable_declarator.initializer) {
      continue;
    }

    bool contains_mc_function_call;
    mct_contains_mc_invoke(declarator->local_variable_declarator.initializer, &contains_mc_function_call);
    if (contains_mc_function_call) {
      // Skip - do later
      continue;
    }

    if (declarator->local_variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER) {
      append_to_c_str(ts->str, " = ");
      mct_transcribe_expression(
          ts,
          declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression);
    }
    else {
      append_to_c_str(ts->str, "[");
      mct_transcribe_expression(
          ts, declarator->local_variable_declarator.initializer->local_variable_array_initializer.size_expression);
      append_to_c_str(ts->str, "]");
    }
  }
  append_to_c_str(ts->str, ";\n");

  for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
    mc_syntax_node *declarator = declaration->local_variable_declaration.declarators->items[i];

    if (!declarator->local_variable_declarator.initializer) {
      continue;
    }
    bool contains_mc_function_call;
    mct_contains_mc_invoke(declarator->local_variable_declarator.initializer, &contains_mc_function_call);
    if (!contains_mc_function_call) {
      continue;
    }

    if (declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression
            ->type != MC_SYNTAX_INVOCATION) {
      MCerror(250, "Nested mc invokes not yet supported");
    }

    // MCerror(412, "TODO -- integrate new invocation methods with this");
    char *return_variable_name;
    copy_syntax_node_to_text(declarator->local_variable_declarator.variable_name, &return_variable_name);
    mct_transcribe_mc_invocation(
        ts, declarator->local_variable_declarator.initializer->local_variable_assignment_initializer.value_expression,
        return_variable_name);
    free(return_variable_name);
    continue;
  }

  return 0;
}

int mct_transcribe_mcerror(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_append_to_c_str(ts, "{\n");
  ++ts->indent;
  mct_append_indent_to_c_str(ts);
  append_to_c_strf(ts->str, "*mc_return_value = %s;\n", syntax_node->invocation.arguments->items[0]->text);
  mct_append_indent_to_c_str(ts);

  bool has_call_to_get_mc_syntax_token_type_name = false;
  int ttp = 0;
  for (int p = 1; p < syntax_node->invocation.arguments->count; ++p) {
    mc_syntax_node *mce_argument = syntax_node->invocation.arguments->items[p];
    if (mce_argument->type == MC_SYNTAX_INVOCATION &&
        (mc_token_type)mce_argument->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
        !strcmp(mce_argument->invocation.function_identity->text, "get_mc_syntax_token_type_name")) {
      // // Get the return type
      // function_info *func_info;
      // find_function_info("get_mc_syntax_token_type_name", &func_info);
      // if (!func_info) {
      //   MCerror(709, "Not Expected");
      // }
      // printf("%s\n", func_info->return_type.name);
      // enumeration_info *enum_info;
      // find_enumeration_info(func_info->return_type.name, &enum_info);
      // if (!enum_info) {
      //   MCerror(715, "Not Expected");
      // }

      char *temp_name;
      cprintf(temp_name, "mcs_tt_name_%i", ttp++);

      mct_append_to_c_str(ts, "const char *");
      append_to_c_str(ts->str, temp_name);
      append_to_c_str(ts->str, ";\n");

      mct_transcribe_mc_invocation(ts, mce_argument, temp_name);
      free(temp_name);
    }
  }

  mct_append_indent_to_c_str(ts);
  append_to_c_str(ts->str, "MCerror(");
  mct_append_node_text_to_c_str(ts->str, syntax_node->invocation.arguments->items[0]);
  ttp = 0;
  for (int p = 1; p < syntax_node->invocation.arguments->count; ++p) {
    append_to_c_str(ts->str, ", ");
    mc_syntax_node *mce_argument = syntax_node->invocation.arguments->items[p];
    if (mce_argument->type == MC_SYNTAX_INVOCATION &&
        (mc_token_type)mce_argument->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
        !strcmp(mce_argument->invocation.function_identity->text, "get_mc_syntax_token_type_name")) {

      char *temp_name;
      cprintf(temp_name, "mcs_tt_name_%i", ttp++);

      append_to_c_str(ts->str, temp_name);
      free(temp_name);
    }
    else {
      mct_append_node_text_to_c_str(ts->str, mce_argument);
    }
  }

  append_to_c_str(ts->str, ");\n");
  --ts->indent;
  // mct_append_to_c_str(ts, "}\n");
  // if (ttp)
  //   printf("def:\n%s||\n", ts->str->text);

  return 0;
}

int mct_transcribe_expression(mct_transcription_state *ts, mc_syntax_node *syntax_node)
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
    mct_transcribe_type_identifier(ts, syntax_node->local_variable_declaration.type_identifier);

    append_to_c_str(ts->str, " ");

    for (int a = 0; a < syntax_node->local_variable_declaration.declarators->count; ++a) {
      if (a > 0) {
        append_to_c_str(ts->str, ", ");
      }
      mct_transcribe_declarator(ts, syntax_node->local_variable_declaration.declarators->items[a]);
    }
  } break;
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION: {
    mct_append_node_text_to_c_str(ts->str, syntax_node->assignment_expression.variable);
    append_to_c_str(ts->str, " ");
    mct_append_node_text_to_c_str(ts->str, syntax_node->assignment_expression.assignment_operator);
    append_to_c_str(ts->str, " ");
    mct_transcribe_expression(ts, syntax_node->assignment_expression.value_expression);
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    append_to_c_str(ts->str, "(");
    mct_transcribe_expression(ts, syntax_node->parenthesized_expression.expression);
    append_to_c_str(ts->str, ")");
  } break;
  case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
    if (!syntax_node->operational_expression.right) {
      MCerror(745, "TODO");
    }

    mct_transcribe_expression(ts, syntax_node->operational_expression.left);
    append_to_c_str(ts->str, " ");
    mct_append_node_text_to_c_str(ts->str, syntax_node->operational_expression.operational_operator);
    append_to_c_str(ts->str, " ");
    mct_transcribe_expression(ts, syntax_node->operational_expression.right);
    // printf("%s\n", ts->str->text);
  } break;
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
    mct_transcribe_expression(ts, syntax_node->member_access_expression.primary);
    mct_append_node_text_to_c_str(ts->str, syntax_node->member_access_expression.access_operator);
    mct_transcribe_expression(ts, syntax_node->member_access_expression.identifier);
    // printf("%s\n", ts->str->text);
  } break;
  case MC_SYNTAX_DEREFERENCE_EXPRESSION: {
    mct_append_node_text_to_c_str(ts->str, syntax_node->dereference_expression.deref_sequence);
    mct_transcribe_expression(ts, syntax_node->dereference_expression.unary_expression);
    // printf("%s\n", ts->str->text);
  } break;
  case MC_SYNTAX_CONDITIONAL_EXPRESSION: {
    if (!syntax_node->conditional_expression.right) {
      MCerror(764, "TODO");
    }

    mct_transcribe_expression(ts, syntax_node->conditional_expression.left);
    mct_append_node_text_to_c_str(ts->str, syntax_node->conditional_expression.conditional_operator);
    mct_transcribe_expression(ts, syntax_node->conditional_expression.right);
  } break;
  case MC_SYNTAX_RELATIONAL_EXPRESSION: {
    if (!syntax_node->relational_expression.right) {
      MCerror(777, "TODO");
    }

    mct_transcribe_expression(ts, syntax_node->relational_expression.left);
    mct_append_node_text_to_c_str(ts->str, syntax_node->relational_expression.relational_operator);
    mct_transcribe_expression(ts, syntax_node->relational_expression.right);
  } break;
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
    mct_transcribe_expression(ts, syntax_node->element_access_expression.primary);
    append_to_c_str(ts->str, "[");
    mct_transcribe_expression(ts, syntax_node->element_access_expression.access_expression);
    append_to_c_str(ts->str, "]");
  } break;

    // WILL have to redo in future
  // case MC_SYNTAX_DEREFERENCE_EXPRESSION:
  // case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
  // case MC_SYNTAX_CONDITIONAL_EXPRESSION:
  // case MC_SYNTAX_OPERATIONAL_EXPRESSION:
  // case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION:
  // case MC_SYNTAX_RELATIONAL_EXPRESSION: {
  //   mct_append_node_text_to_c_str(ts->str, syntax_node);
  // } break;
  case MC_SYNTAX_CAST_EXPRESSION: {
    append_to_c_str(ts->str, "(");

    // if (!strcmp(ts->transcription_root->function.name->text, "print_syntax_node")) {
    // printf("LLL>>>>\n");
    // print_syntax_node(syntax_node, 0);
    // }
    mct_transcribe_type_identifier(ts, syntax_node->cast_expression.type_identifier);

    if (syntax_node->cast_expression.type_dereference) {
      append_to_c_str(ts->str, " ");
      mct_append_node_text_to_c_str(ts->str, syntax_node->cast_expression.type_dereference);
    }
    append_to_c_str(ts->str, ")");

    mct_transcribe_expression(ts, syntax_node->cast_expression.expression);
  } break;
  case MC_SYNTAX_VA_ARG_EXPRESSION: {
    MCerror(517, "COTTONEYE");
    // register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-0");
    // append_to_c_str(ts->str, "va_arg(");

    // printf("$$ %p\n", syntax_node);
    // printf("$$ %p %p\n", syntax_node->va_arg_expression.list_identity,
    // syntax_node->va_arg_expression.type_identifier);
    // print_syntax_node(syntax_node->va_arg_expression.list_identity, 0);
    // mct_transcribe_expression(ts,syntax_node->va_arg_expression.list_identity);
    // register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-1");
    // append_to_c_str(ts->str, ", ");
    // mct_transcribe_type_identifier(ts, syntax_node->va_arg_expression.type_identifier);
    // register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-2");
    // append_to_c_str(ts->str, ")\n");
  } break;
  case MC_SYNTAX_SIZEOF_EXPRESSION: {
    append_to_c_str(ts->str, "sizeof(");

    mct_transcribe_type_identifier(ts, syntax_node->cast_expression.type_identifier);

    if (syntax_node->sizeof_expression.type_dereference) {
      append_to_c_str(ts->str, " ");
      mct_append_node_text_to_c_str(ts->str, syntax_node->sizeof_expression.type_dereference);
    }
    append_to_c_str(ts->str, ")");
  } break;
  case MC_SYNTAX_INVOCATION: {
    function_info *func_info;
    char *function_name;
    copy_syntax_node_to_text(syntax_node->invocation.function_identity, &function_name);
    find_function_info(function_name, &func_info);
    free(function_name);
    if (func_info) {
      MCerror(247, "Not supported from here, have to deal with it earlier");
    }

    if ((mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
        !strcmp(syntax_node->invocation.function_identity->text, "MCerror")) {
      print_syntax_node(syntax_node->parent, 0);
      MCerror(550, "TODO");
      // mct_transcribe_mcerror(ts, syntax_node);
      break;
    }

    mct_append_node_text_to_c_str(ts->str, syntax_node->invocation.function_identity);
    append_to_c_str(ts->str, "(");
    for (int a = 0; a < syntax_node->invocation.arguments->count; ++a) {
      if (a > 0) {
        append_to_c_str(ts->str, ", ");
      }

      mct_transcribe_expression(ts, syntax_node->invocation.arguments->items[a]);
    }
    append_to_c_str(ts->str, ")");
  } break;

  // PROBABLY won't have to redo
  // case MC_SYNTAX_DECLARATION_STATEMENT: {

  // }break;
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION:
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION:
  case MC_SYNTAX_FIXREMENT_EXPRESSION: {
    mct_append_node_text_to_c_str(ts->str, syntax_node);
  } break;
  default:
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_NUMERIC_LITERAL:
    case MC_TOKEN_CHAR_LITERAL:
    case MC_TOKEN_IDENTIFIER: {
      mct_append_node_text_to_c_str(ts->str, syntax_node);
    } break;
    default:
      MCerror(291, "MCT:Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    }
  }

  register_midge_error_tag("mct_transcribe_expression(~)");
  return 0;
}

int mct_transcribe_if_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_if_statement()");
  // printf("cb: %p\n", syntax_node);

  // Do MC_invokes
  bool contains_mc_function_call;
  if (syntax_node->if_statement.conditional) {
    mct_contains_mc_invoke(syntax_node->if_statement.conditional, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(104, "TODO");
    }
  }

  // Initialization
  mct_append_to_c_str(ts, "if (");
  mct_transcribe_expression(ts, syntax_node->if_statement.conditional);
  append_to_c_str(ts->str, ") ");

  if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_increment_scope_depth(ts);
  }
  mct_transcribe_statement(ts, syntax_node->if_statement.do_statement);
  if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_decrement_scope_depth(ts);
  }

  if (syntax_node->if_statement.else_continuance) {
    mct_append_to_c_str(ts, "else ");

    if (syntax_node->if_statement.else_continuance->type == MC_SYNTAX_IF_STATEMENT) {

      mct_transcribe_if_statement(ts, syntax_node->if_statement.else_continuance);
    }
    else {
      if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
        mct_increment_scope_depth(ts);
      }
      mct_transcribe_statement(ts, syntax_node->if_statement.else_continuance);
      if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
        mct_decrement_scope_depth(ts);
      }
    }
    // else if (syntax_node->if_statement.else_continuance->type == MC_SYNTAX_CODE_BLOCK) {
    //   mct_transcribe_code_block(ts, syntax_node->if_statement.else_continuance);
    // }
    // else {
    //   MCerror(119, "TODO: %s",
    //           get_mc_syntax_token_type_name((mc_syntax_node_type)syntax_node->if_statement.else_continuance->type));
    // }
  }

  register_midge_error_tag("mct_transcribe_if_statement(~)");
  return 0;
}

int mct_transcribe_switch_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_switch_statement()");
  // Do MC_invokes
  bool contains_mc_function_call;
  if (syntax_node->switch_statement.conditional) {
    mct_contains_mc_invoke(syntax_node->switch_statement.conditional, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(104, "TODO");
    }
  }

  mct_append_to_c_str(ts, "switch (");
  mct_transcribe_expression(ts, syntax_node->switch_statement.conditional);
  append_to_c_str(ts->str, ") {\n");
  ++ts->indent;
  mct_increment_scope_depth(ts);

  for (int i = 0; i < syntax_node->switch_statement.sections->count; ++i) {
    mc_syntax_node *switch_section = syntax_node->switch_statement.sections->items[i];

    for (int j = 0; j < switch_section->switch_section.labels->count; ++j) {
      mct_append_indent_to_c_str(ts);

      mct_append_node_text_to_c_str(ts->str, switch_section->switch_section.labels->items[j]);
      append_to_c_str(ts->str, "\n");
    }

    mct_transcribe_statement_list(ts, switch_section->switch_section.statement_list);
  }

  mct_decrement_scope_depth(ts);
  --ts->indent;
  mct_append_to_c_str(ts, "}\n");
  register_midge_error_tag("mct_transcribe_switch_statement(~)");
  return 0;
}

int mct_transcribe_for_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_for_statement()");

  bool contains_mc_function_call;

  // Do MC_invokes
  if (syntax_node->for_statement.initialization) {
    mct_contains_mc_invoke(syntax_node->for_statement.initialization, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(65, "TODO");
    }
  }
  if (syntax_node->for_statement.conditional) {
    mct_contains_mc_invoke(syntax_node->for_statement.conditional, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(71, "TODO");
    }
  }
  if (syntax_node->for_statement.fix_expression) {
    mct_contains_mc_invoke(syntax_node->for_statement.fix_expression, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(77, "TODO");
    }
  }

  // Initialization
  mct_increment_scope_depth(ts);
  mct_append_to_c_str(ts, "for (");
  if (syntax_node->for_statement.initialization) {
    mct_transcribe_expression(ts, syntax_node->for_statement.initialization);

    if (syntax_node->for_statement.initialization->type == MC_SYNTAX_LOCAL_VARIABLE_DECLARATION) {
      for (int a = 0; a < syntax_node->for_statement.initialization->local_variable_declaration.declarators->count;
           ++a) {
        mct_add_scope_variable(
            ts, syntax_node->for_statement.initialization->local_variable_declaration.declarators->items[a]);
      }
    }
  }
  mct_append_to_c_str(ts, "; ");
  if (syntax_node->for_statement.conditional) {
    mct_transcribe_expression(ts, syntax_node->for_statement.conditional);
  }
  mct_append_to_c_str(ts, "; ");
  if (syntax_node->for_statement.fix_expression) {
    mct_transcribe_expression(ts, syntax_node->for_statement.fix_expression);
  }
  mct_append_to_c_str(ts, ") ");

  // printf("55 %p\n", syntax_node->for_statement.loop_statement);
  // print_syntax_node(syntax_node->for_statement.loop_statement, 0);
  // printf("556\n");
  if (syntax_node->for_statement.loop_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_increment_scope_depth(ts);
  }
  mct_transcribe_statement(ts, syntax_node->for_statement.loop_statement);
  if (syntax_node->for_statement.loop_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_decrement_scope_depth(ts);
  }

  mct_decrement_scope_depth(ts);

  register_midge_error_tag("mct_transcribe_for_statement(~)");
  return 0;
}

int mct_transcribe_while_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_while_statement()");

  bool contains_mc_function_call;

  // Do MC_invokes
  if (syntax_node->while_statement.conditional) {
    mct_contains_mc_invoke(syntax_node->while_statement.conditional, &contains_mc_function_call);
    if (contains_mc_function_call) {
      MCerror(65, "TODO");
    }
  }

  if (syntax_node->while_statement.do_first) {
    MCerror(311, "TODO");
  }

  // Initialization
  mct_append_to_c_str(ts, "while (");
  mct_transcribe_expression(ts, syntax_node->while_statement.conditional);
  mct_append_to_c_str(ts, ") ");

  if (syntax_node->while_statement.code_block->type != MC_SYNTAX_CODE_BLOCK) {
    MCerror(320, "TODO");
  }
  mct_transcribe_code_block(ts, syntax_node->while_statement.code_block);

  register_midge_error_tag("mct_transcribe_while_statement(~)");
  return 0;
}

// int mc_transcribe_invocation_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node,
//                                        int nested_mc_invocation_insertion_index, char *result_var_name,
//                                        int nested_invocation_index)
// {
//   if (syntax_node->type != MC_SYNTAX_INVOCATION) {
//     MCerror(728, "TODO");
//   }

//   bool contains_nested_mc_function_call = false;
//   for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
//     mct_contains_mc_invoke(syntax_node->invocation.arguments->items[i], &contains_nested_mc_function_call);
//     if (*contains_nested_mc_function_call) {
//       break;
//     }
//   }

//   if (contains_nested_mc_function_call) {
//     ++indent;
//     mct_append_to_c_str(ts, "{\n");
//   }
//   mct_append_indent_to_c_str(ts);
//   int statement_begin_index = str->len - 1;
//   int mc_invoke_result_index = 0;

//   ++indent;
//   append_to_c_str(ts->str, "{\n");

//   mct_append_indent_to_c_str(ts);
//   append_to_c_str(ts->str, "int midge_error_stack_index;\n");
//   append_to_c_str(ts->str, "register_midge_stack_invocation(\""));
//   mct_append_node_text_to_c_str(ts->str, syntax_node->invocation.function_identity);
//   append_to_c_strf(ts->str, "\", __FILE__, %i, &midge_error_stack_index);\n", syntax_node->begin.line);

//   mct_append_indent_to_c_str(ts);

//   if (~syntax_node->invocation.mc_function_info) {
//     // non-mc invocation
//     mct_transcribe_expression(ts,child->expression_statement.expression);
//     append_to_c_str(ts->str, ";\n");
//   }
//   else {

//   }

//   mct_append_indent_to_c_str(ts);
//   append_to_c_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");
//   --indent;
//   mct_append_to_c_str(ts, "}\n");

//   if (contains_nested_mc_function_call) {
//     --indent;
//     mct_append_to_c_str(ts, "}\n");
//   }
//   // for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
//   //   mc_syntax_node *argument = syntax_node->invocation.arguments->items[i];
//   //   bool contains_nested_mc_function_call = false;
//   //   mct_contains_mc_invoke(argument, &contains_nested_mc_function_call);
//   //   if (*contains_nested_mc_function_call) {
//   //     if (argument->type != MC_SYNTAX_INVOCATION || !argument->invocation.mc_function_info) {
//   //       MCerror(742, "Not Yet Supported: mccalls inside normal function calls as an argument to another function
//   //       call");
//   //     }
//   //     _mc_transcribe_invocation(ts, argument, &mc_invoke_result_index);
//   //     break;
//   //   }
//   // }
//   // Do MC_invokes
//   // bool is_mc_function_call = syntax_node->invocation.mc_function_info;
//   // bool contains_nested_mc_function_call = false;
//   // for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {
//   //   mct_contains_mc_invoke(syntax_node->invocation.arguments->items[i],
//   &contains_nested_mc_function_call);
//   //   if (*contains_nested_mc_function_call) {
//   //     break;
//   //   }
//   // }

//   // if (!is_mc_function_call && !contains_nested_mc_function_call) {
//   //   // non-mc invocations
//   //   mct_append_to_c_str(ts, "{\n");
//   //   ++indent;
//   //   mct_append_to_c_str(ts, "int midge_error_stack_index;\n");

//   // }
//   // else {
//   //   // mc_invocations
//   //   // Transcribe all nested invocations first
//   //   if (contains_nested_mc_function_call)
//   //     int mc_invoke_result_index = 0;
//   //   for (int a = 0; a < syntax_node->invocation.arguments->count; ++a) {

//   //     _mc_transcribe_
//   //   }
//   // }

//   if ()
//     // printf("bb-2\n");
//     // print_syntax_node(child->expression_statement.expression, 0);
//     mct_contains_mc_invoke(child->expression_statement.expression, &contains_mc_function_call);
//   if (contains_mc_function_call) {
//     // printf("bb-3\n");
//     // print_syntax_node(child->expression_statement.expression, 0);
//     mct_transcribe_mc_invocation(ts, child->expression_statement.expression, NULL);
//     break;
//   }

//   // TODO -- maybe more coverage (atm only doing invocation expresssions. NOT invocations nested in other
//   // expressions)
//   if (child->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
//   }

//   return 0;
// }

int mct_transcribe_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  switch (syntax_node->type) {
  case MC_SYNTAX_CONTINUE_STATEMENT:
  case MC_SYNTAX_BREAK_STATEMENT: {
    mct_append_indent_to_c_str(ts);
    mct_append_node_text_to_c_str(ts->str, syntax_node);
    append_to_c_str(ts->str, "\n");
  } break;
  case MC_SYNTAX_RETURN_STATEMENT: {
    bool contains_mc_function_call;
    mct_append_to_c_str(ts, "{\n");
    ++ts->indent;
    if (syntax_node->return_statement.expression) {
      mct_contains_mc_invoke(syntax_node->return_statement.expression, &contains_mc_function_call);
      if (contains_mc_function_call) {
        // print_syntax_node(syntax_node->return_statement.expression, 0);
        mct_transcribe_mc_invocation(ts, syntax_node->return_statement.expression, (char *)"*mc_return_value");
        // printf("Transcription after return statement (str):\n%s||\n", str->text);
      }
      else {
        mct_append_indent_to_c_str(ts);
        append_to_c_str(ts->str, "*mc_return_value = ");
        mct_transcribe_expression(ts, syntax_node->return_statement.expression);
        append_to_c_str(ts->str, ";\n");
      }
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

        mct_append_indent_to_c_str(ts);
        append_to_c_strf(ts->str, "register_midge_error_tag(\"%s(~)\");\n", root->function.name->text);
      }
    }
    mct_append_to_c_str(ts, "return 0;\n");
    --ts->indent;
    mct_append_to_c_str(ts, "}\n");
  } break;
  case MC_SYNTAX_CODE_BLOCK: {
    mct_transcribe_code_block(ts, syntax_node);
  } break;
  case MC_SYNTAX_FOR_STATEMENT: {
    mct_transcribe_for_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_WHILE_STATEMENT: {
    mct_transcribe_while_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_SWITCH_STATEMENT: {
    mct_transcribe_switch_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_IF_STATEMENT: {
    mct_transcribe_if_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_DECLARATION_STATEMENT: {
    mct_transcribe_declaration_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_VA_LIST_STATEMENT: {
    mct_transcribe_va_list_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_VA_START_STATEMENT:
  case MC_SYNTAX_VA_END_STATEMENT: {
    // Ignore these statement...
  } break;
  case MC_SYNTAX_EXPRESSION_STATEMENT: {
    register_midge_error_tag("mct_transcribe_statement_list-ES0");
    // TODO -- MCerror exception
    if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION &&
        (mc_token_type)syntax_node->expression_statement.expression->invocation.function_identity->type ==
            MC_TOKEN_IDENTIFIER &&
        !strcmp(syntax_node->expression_statement.expression->invocation.function_identity->text, "MCerror")) {
      mct_transcribe_mcerror(ts, syntax_node->expression_statement.expression);
      // print_syntax_node(syntax_node, 0);
      // MCerror(1300, "progress");
      break;
    }
    // Do MC_invokes
    bool contains_mc_function_call;
    mct_contains_mc_invoke(syntax_node->expression_statement.expression, &contains_mc_function_call);
    if (contains_mc_function_call) {
      if (syntax_node->expression_statement.expression->type != MC_SYNTAX_INVOCATION) {
        // Nested MC_invocation .. eek
        print_syntax_node(syntax_node, 0);
        MCerror(231, "TODO");
      }
      // printf("bb-3\n");
      mct_transcribe_mc_invocation(ts, syntax_node->expression_statement.expression, NULL);
      break;
    }

    // TODO -- maybe more coverage (atm only doing invocation expresssions. NOT invocations nested in other
    // expressions)
    if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
      if (ts->report_invocations_to_error_stack) {
        mct_append_to_c_str(ts, "{\n");
        ++ts->indent;
        mct_append_to_c_str(ts, "int midge_error_stack_index;\n");

        mct_append_indent_to_c_str(ts);
        append_to_c_str(ts->str, "register_midge_stack_invocation(\"");

        mct_append_node_text_to_c_str(ts->str,
                                      syntax_node->expression_statement.expression->invocation.function_identity);
        append_to_c_strf(ts->str, "\", \"%s\", %i, &midge_error_stack_index);\n", "unknown-file",
                         syntax_node->expression_statement.expression->begin.line);
      }
    }

    mct_append_indent_to_c_str(ts);

    register_midge_error_tag("mct_transcribe_statement_list-ES5");
    mct_transcribe_expression(ts, syntax_node->expression_statement.expression);
    append_to_c_str(ts->str, ";\n");
    register_midge_error_tag("mct_transcribe_statement_list-ES9");

    if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
      if (ts->report_invocations_to_error_stack) {
        mct_append_indent_to_c_str(ts);
        append_to_c_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");
        --ts->indent;
        mct_append_to_c_str(ts, "}\n");
      }
    }
  } break;
  default:
    print_syntax_node(syntax_node, 0);
    MCerror(168, "MCT:Statement-Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
  }

  return 0;
}

int mct_transcribe_statement_list(mct_transcription_state *ts, mc_syntax_node *syntax_node)
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

    switch ((mc_token_type)child->type) {
    case MC_TOKEN_NEW_LINE:
    case MC_TOKEN_SPACE_SEQUENCE:
    case MC_TOKEN_TAB_SEQUENCE:
    case MC_TOKEN_LINE_COMMENT:
    case MC_TOKEN_MULTI_LINE_COMMENT: {
      mct_append_node_text_to_c_str(ts->str, child);
      continue;
    }
    default:
      break;
    }

    mct_transcribe_statement(ts, child);
  }

  register_midge_error_tag("mct_transcribe_statement_list(~)");
  return 0;
}

int mct_transcribe_code_block(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_code_block()");
  mct_append_to_c_str(ts, "{\n");
  mct_increment_scope_depth(ts);

  if (syntax_node->code_block.statement_list) {
    mct_transcribe_statement_list(ts, syntax_node->code_block.statement_list);
  }

  mct_decrement_scope_depth(ts);
  mct_append_to_c_str(ts, "}\n");

  register_midge_error_tag("mct_transcribe_code_block(~)");
  return 0;
}

int mct_transcribe_field_list(mct_transcription_state *ts, mc_syntax_node_list *field_list)
{
  for (int f = 0; f < field_list->count; ++f) {
    mc_syntax_node *field_syntax = field_list->items[f];

    mct_append_indent_to_c_str(ts);
    mct_transcribe_field(ts, field_syntax);
    append_to_c_str(ts->str, ";\n");
  }

  return 0;
}

int mct_transcribe_field_declarators(mct_transcription_state *ts, mc_syntax_node_list *declarators_list)
{
  for (int a = 0; a < declarators_list->count; ++a) {
    if (a > 0) {
      append_to_c_str(ts->str, ",");
    }

    mc_syntax_node *declarator = declarators_list->items[a];
    if (declarator->field_declarator.type_dereference) {
      for (int d = 0; d < declarator->field_declarator.type_dereference->dereference_sequence.count; ++d) {
        append_to_c_str(ts->str, "*");
      }
    }
    mct_append_node_text_to_c_str(ts->str, declarator->field_declarator.name);
  }

  return 0;
}

int mct_transcribe_field(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_append_indent_to_c_str(ts);

  switch (syntax_node->type) {
  case MC_SYNTAX_FIELD_DECLARATION: {
    switch (syntax_node->field.type) {
    case FIELD_KIND_STANDARD: {
      mct_transcribe_type_identifier(ts, syntax_node->field.type_identifier);
      append_to_c_str(ts->str, " ");
      mct_transcribe_field_declarators(ts, syntax_node->field.declarators);
    } break;
    case FIELD_KIND_FUNCTION_POINTER: {
      mct_append_node_text_to_c_str(ts->str, syntax_node->field.function_pointer);
    } break;
    default:
      print_syntax_node(syntax_node, 0);
      MCerror(1122, "NotSupported:%i", syntax_node->field.type);
    }
  } break;
  case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
    mc_syntax_node *type_decl = syntax_node->nested_type.declaration;

    if (type_decl->type == MC_SYNTAX_STRUCTURE) {
      append_to_c_str(ts->str, "struct ");

      if (type_decl->structure.type_name) {
        mct_append_node_text_to_c_str(ts->str, type_decl->structure.type_name);
        append_to_c_str(ts->str, " ");
      }

      append_to_c_str(ts->str, "{\n");
      ++ts->indent;

      mct_transcribe_field_list(ts, type_decl->structure.fields);

      --ts->indent;
      mct_append_to_c_str(ts, "}");
    }
    else if (type_decl->type == MC_SYNTAX_UNION) {
      append_to_c_str(ts->str, "union ");

      if (type_decl->union_decl.type_name) {
        mct_append_node_text_to_c_str(ts->str, type_decl->union_decl.type_name);
        append_to_c_str(ts->str, " ");
      }

      append_to_c_str(ts->str, "{\n");
      ++ts->indent;

      mct_transcribe_field_list(ts, type_decl->union_decl.fields);

      --ts->indent;
      mct_append_to_c_str(ts, "}");
    }
    else {
      MCerror(1136, "TODO");
    }

    if (syntax_node->nested_type.declarators) {
      append_to_c_str(ts->str, " ");
      mct_transcribe_field_declarators(ts, syntax_node->nested_type.declarators);
    }
  } break;
  default:
    print_syntax_node(syntax_node, 0);
    MCerror(1130, "NotSupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
  }

  return 0;
}

// int transcribe_code_block_ast_to_mc_definition(mc_syntax_node *syntax_node, char **output)
// {
//   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition()");

//   if (syntax_node->type != MC_SYNTAX_CODE_BLOCK) {
//     MCerror(861, "MCT:Not Supported");
//   }

//   c_str *str;
//   init_c_str(&str);

//   if (syntax_node->code_block.statement_list) {
//     mct_transcribe_statement_list(ts->str, 1, syntax_node->code_block.statement_list);
//   }

//   *output = str->text;
//   release_c_str(ts->str, false);

//   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition(~)");
//   return 0;
// }

int transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast, char **mc_transcription)
{
  register_midge_error_tag("transcribe_function_to_mc()");

  if (function_ast->type != MC_SYNTAX_FUNCTION) {
    MCerror(889, "MCT:Not Supported");
  }
  if (!function_ast->function.code_block || function_ast->function.code_block->type != MC_SYNTAX_CODE_BLOCK ||
      !function_ast->function.code_block->code_block.statement_list) {
    print_syntax_node(function_ast, 0);
    MCerror(893, "TODO");
  }
  if ((mc_token_type)function_ast->function.name->type != MC_TOKEN_IDENTIFIER) {
    MCerror(898, "TODO");
  }

  mct_transcription_state ts;
  // -- options
  ts.report_invocations_to_error_stack = true;
  ts.tag_on_function_entry = true;
  // -- state
  ts.transcription_root = function_ast;
  ts.indent = 0;
  init_c_str(&ts.str);
  // -- scope
  ts.scope_index = 0;
  ts.scope[ts.scope_index].variable_count = 0;

  // Header
  append_to_c_strf(ts.str, "int %s_mc_v%u(int mc_argsc, void **mc_argsv) {\n", function_ast->function.name->text,
                   func_info->latest_iteration);
  mct_increment_scope_depth(&ts);
  ++ts.indent;

  // Initial
  if (ts.tag_on_function_entry) {
    append_to_c_strf(ts.str, "  register_midge_error_tag(\"%s()\");\n\n", function_ast->function.name->text);
  }

  // Function Parameters
  append_to_c_str(ts.str, "  // Function Parameters\n");
  ts.scope[ts.scope_index].variable_count = 0;
  for (int p = 0; p < function_ast->function.parameters->count; ++p) {
    mc_syntax_node *parameter_syntax = function_ast->function.parameters->items[p];

    switch (parameter_syntax->parameter.type) {
    case PARAMETER_KIND_STANDARD: {

      mct_add_scope_variable(&ts, parameter_syntax);

      mct_transcribe_type_identifier(&ts, parameter_syntax->parameter.type_identifier);
      mct_append_indent_to_c_str(&ts);
      if (parameter_syntax->parameter.type_dereference) {
        for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
          append_to_c_str(ts.str, "*");
        }
      }
      mct_append_node_text_to_c_str(ts.str, parameter_syntax->parameter.name);
      append_to_c_str(ts.str, " = *(");
      mct_transcribe_type_identifier(&ts, parameter_syntax->parameter.type_identifier);
      append_to_c_str(ts.str, " ");
      if (parameter_syntax->parameter.type_dereference) {
        for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
          append_to_c_str(ts.str, "*");
        }
      }
      append_to_c_strf(ts.str, "*)mc_argsv[%i];\n", p);
    } break;
    case PARAMETER_KIND_VARIABLE_ARGS: {
    } break;
    default:
      MCerror(958, "NotSupported:%i", parameter_syntax->parameter.type);
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
    mct_append_indent_to_c_str(&ts);

    mct_append_node_text_to_c_str(ts.str, function_ast->function.return_type_identifier);
    append_to_c_str(ts.str, " ");
    if (function_ast->function.return_type_dereference)
      mct_append_node_text_to_c_str(ts.str, function_ast->function.return_type_dereference);
    append_to_c_str(ts.str, "*mc_return_value;\n");
  }

  // Code Block
  append_to_c_str(ts.str, "  // Function Code\n");
  ++ts.indent;
  mct_increment_scope_depth(&ts);

  mct_transcribe_statement_list(&ts, function_ast->function.code_block->code_block.statement_list);

  mct_decrement_scope_depth(&ts);
  --ts.indent;

  // if (!strcmp(ts.transcription_root->function.name->text, "print_syntax_node")) {
  //   printf("LLL>>>>\n");
  //   print_syntax_node(ts.transcription_root, 0);
  // }

  // Return Statement
  append_to_c_strf(ts.str,
                   "\n"
                   "  register_midge_error_tag(\"%s(~)\");\n"
                   "  return 0;\n",
                   function_ast->function.name->text);
  mct_decrement_scope_depth(&ts);
  append_to_c_str(ts.str, "}");
  *mc_transcription = ts.str->text;
  release_c_str(ts.str, false);

  // printf("mc_transcription:\n%s||\n", *mc_transcription);

  register_midge_error_tag("transcribe_function_to_mc(~)");
  return 0;
}

int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *structure_ast, char **mc_transcription)
{
  register_midge_error_tag("transcribe_struct_to_mc()");

  if (structure_ast->type != MC_SYNTAX_STRUCTURE) {
    MCerror(1242, "MCT:Invalid Argument");
  }

  mct_transcription_state ts;
  // -- options
  ts.report_invocations_to_error_stack = true;
  ts.tag_on_function_entry = true;
  // -- state
  ts.transcription_root = structure_ast;
  ts.indent = 0;
  init_c_str(&ts.str);
  // -- scope
  ts.scope_index = 0;
  ts.scope[ts.scope_index].variable_count = 0;

  // Header
  append_to_c_str(ts.str, "struct \n");
  append_to_c_str(ts.str, structure_info->mc_declared_name);

  if (structure_ast->structure.fields) {
    append_to_c_str(ts.str, " { \n");
    ++ts.indent;

    mct_transcribe_field_list(&ts, structure_ast->structure.fields);

    --ts.indent;
    append_to_c_strf(ts.str, "}", structure_ast->structure.type_name->text,
                     structure_info->latest_iteration); // TODO -- types not structs
  }
  append_to_c_str(ts.str, ";");

  *mc_transcription = ts.str->text;
  release_c_str(ts.str, false);

  // print_syntax_node(structure_ast, 0);
  // printf("def:\n%s||\n", *mc_transcription);

  register_midge_error_tag("transcribe_struct_to_mc(~)");
  return 0;
}