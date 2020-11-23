/* mc_code_transcription.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include "midge_error_handling.h"

#include "core/mc_code_transcription.h"

#define MCT_TS_MAX_VARIABLES 64
#define MCT_TS_MAX_SCOPE_DEPTH 256

typedef struct mct_transcription_scope_variable {
  mc_syntax_node *declaration_node;
  char *name;
} mct_transcription_scope_variable;

typedef struct mct_transcription_scope {
  int variable_count;
  mct_transcription_scope_variable *variables;
} mct_transcription_scope;

typedef struct mct_transcription_state {
  // Options
  mct_function_transcription_options *options;

  // State Values
  mc_syntax_node *transcription_root;
  const char *function_name;
  mc_str *str;
  int indent;

  // Variable scopes
  mct_transcription_scope *scope;
  int scope_index;
} mct_transcription_state;

typedef struct mct_statement_transcription_info {
  unsigned int begin_index, prefix_end_index;
  bool variable_reported;
} mct_statement_transcription_info;

typedef struct mct_expression_type_info {
  // If is fptr - then type_identifier will be the return - type identifier
  char *type_name;
  // If is fptr - then deref count will be return type deref count
  unsigned int deref_count;
  bool is_array;
  bool is_fptr;
} mct_expression_type_info;

int mct_transcribe_code_block(mct_transcription_state *ts, mc_syntax_node *syntax_node, bool function_root);
int mct_transcribe_statement_list(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_expression(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                              mc_syntax_node *syntax_node);
int mct_transcribe_field(mct_transcription_state *ts, mc_syntax_node *syntax_node);
int mct_transcribe_function_end(mct_transcription_state *ts, mc_syntax_node *result_expression);

int mct_release_expression_type_info_fields(mct_expression_type_info *eti)
{
  if (eti->type_name) {
    free(eti->type_name);
    eti->type_name = NULL;
  }

  return 0;
}

// TODO -- this belongs in c_parser_lexer mcs
int mct_syntax_descendants_contain_node_type(mc_syntax_node *syntax_node, mc_syntax_node_type syntax_node_type,
                                             bool *result)
{

  if ((int)syntax_node->type < (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    *result = false;
    return 0;
  }

  for (int i = 0; i < syntax_node->children->count; ++i) {
    if (syntax_node->children->items[i] == NULL) {
      continue;
    }

    if ((int)syntax_node->children->items[i]->type == (int)syntax_node_type) {
      *result = true;
      return 0;
    }

    if ((int)syntax_node->children->items[i]->type >= (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
      mct_syntax_descendants_contain_node_type(syntax_node->children->items[i], syntax_node_type, result);
      if (*result) {
        return 0;
      }
    }
  }
  *result = false;

  return 0;
}

int mct_transcribe_indent(mct_transcription_state *ts)
{
  const char *INDENT = "  ";
  for (int i = 0; i < ts->indent; ++i) {
    append_to_mc_str(ts->str, INDENT);
  }
  return 0;
}

int mct_transcribe_text_with_indent(mct_transcription_state *ts, const char *text)
{
  mct_transcribe_indent(ts);
  append_to_mc_str(ts->str, text);

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

    switch (variable_syntax_node->parameter.type) {
    case PARAMETER_KIND_STANDARD: {
      mcs_copy_syntax_node_to_text(variable_syntax_node->parameter.name,
                                   &ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
    } break;
    case PARAMETER_KIND_FUNCTION_POINTER: {
      mcs_copy_syntax_node_to_text(variable_syntax_node->parameter.function_pointer->fptr_declarator.name,
                                   &ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
    } break;
    default:
      MCerror(113, "NotSupported:%i", variable_syntax_node->parameter.type);
    }
  } break;
  case MC_SYNTAX_VARIABLE_DECLARATOR: {
    ts->scope[ts->scope_index].variables[variable_index_in_scope].declaration_node = variable_syntax_node;

    mcs_copy_syntax_node_to_text(variable_syntax_node->variable_declarator.variable_name,
                                 &ts->scope[ts->scope_index].variables[variable_index_in_scope].name);
  } break;
  case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR: {
    ts->scope[ts->scope_index].variables[variable_index_in_scope].declaration_node = variable_syntax_node;

    mcs_copy_syntax_node_to_text(variable_syntax_node->fptr_declarator.name,
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

int _determine_type_of_expression_subsearch(field_info_list *parent_type_fields, mc_syntax_node *expression,
                                            mct_expression_type_info *result)
{
  result->type_name = NULL;

  // printf("_determine_type_of_expression_subsearch()\n");
  // print_syntax_node(expression, 0);
  // // DEBUG
  // printf("parent fields: (count=%i)\n", parent_type_fields->count);
  // for (int i = 0; i < parent_type_fields->count; ++i) {
  //   switch (parent_type_fields->items[i]->field_type) {
  //   case FIELD_KIND_STANDARD: {
  //     printf("%s ", parent_type_fields->items[i]->field.type_name);
  //     for (int j = 0; j < parent_type_fields->items[i]->field.declarators->count; ++j) {
  //       if (j > 0)
  //         printf(", ");
  //       printf("%s", parent_type_fields->items[i]->field.declarators->items[j]->name);
  //     }
  //     printf(";\n");
  //   } break;
  //   default:
  //     printf("-anotherfield type:%i\n", parent_type_fields->items[i]->field_type);
  //     break;
  //   }
  // }
  // DEBUG

  switch (expression->type) {
  case MC_TOKEN_IDENTIFIER: {
    for (int f = 0; f < parent_type_fields->count; ++f) {
      field_info *ptfield = parent_type_fields->items[f];
      char *identifier_name = expression->text;

      switch (ptfield->field_type) {
      case FIELD_KIND_STANDARD: {
        for (int g = 0; g < ptfield->field.declarators->count; ++g) {
          // printf("p:%s<>%s\n", identifier_name, ptfield->field.declarators->items[g]->name);
          if (!strcmp(identifier_name, ptfield->field.declarators->items[g]->name)) {
            // Found!
            result->is_array = ptfield->field.declarators->items[g]->is_array;
            result->is_fptr = false;
            result->type_name = strdup(ptfield->field.type_name);
            result->deref_count = ptfield->field.declarators->items[g]->deref_count;
            return 0;
          }
        }
      } break;
      case FIELD_KIND_NESTED_STRUCT:
      case FIELD_KIND_NESTED_UNION: {
        if (ptfield->sub_type.is_anonymous) {
          // printf("%s<><anon-%s>\n", identifier_name,
          //        ptfield->field_type == 2 ? "struct" : (ptfield->field_type == 3 ? "union" : "unknown"));

          // Search within the sub-types fields as if they were the parent types
          _determine_type_of_expression_subsearch(ptfield->sub_type.fields, expression, result);
          if (result->type_name) {
            return 0;
          }
        }
        // else {
        //   for (int g = 0; g < ptfield->sub_type.declarators->count; ++g) {
        //     printf("%s<>%s\n", identifier_name, ptfield->sub_type.declarators->items[g]->name);
        //     if (!strcmp(identifier_name, ptfield->sub_type.declarators->items[g]->name)) {

        //       // _determine_type_of_expression_subsearch(ptfield->sub_type.fields,
        //       expression->member_access_expression.)

        //       // Found!
        //       result->is_array = false; // TODO ? this exists? 166 ptfield->field.declarators->items[g]
        //       result->is_fptr = false;
        //       allocate_and_copy_cstr(result->type_name, ptfield->field.type_name);
        //       result->deref_count = ptfield->field.declarators->items[g]->deref_count;
        //       return 0;
        //     }
        //   }
        // }
      } break;
      default:
        print_syntax_node(expression, 0);
        MCerror(7332, "TODO:%i", ptfield->field_type);
      }
    }

    // Could not Find it!
    return 0;
  } break;
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
    mc_syntax_node *ma_primary = expression->member_access_expression.primary;
    if ((mc_token_type)ma_primary->type != MC_TOKEN_IDENTIFIER) {
      MCerror(341, "TODO");
    }

    // Find the primary 'type'
    for (int f = 0; f < parent_type_fields->count; ++f) {
      field_info *ptfield = parent_type_fields->items[f];
      char *primary_name = ma_primary->text;

      // printf("ptfield:%i primary_name:%s\n", f, primary_name);
      switch (ptfield->field_type) {
      case FIELD_KIND_STANDARD: {
        // printf("standard-ptfield:%s\n", ptfield->field.declarators->items[0]->name);
        for (int g = 0; g < ptfield->field.declarators->count; ++g) {
          if (!strcmp(primary_name, ptfield->field.declarators->items[g]->name)) {
            // Found!
            switch (expression->member_access_expression.identifier->type) {
            case MC_TOKEN_IDENTIFIER: {
              // This is the type
              struct_info *primary_type_info;
              find_struct_info(ptfield->field.type_name, &primary_type_info);
              if (ptfield->field.declarators->items[g]->deref_count == 1 &&
                  (mc_token_type)expression->member_access_expression.access_operator->type !=
                      MC_TOKEN_POINTER_OPERATOR) {
                MCerror(8311, "TODO");
              }
              else if (ptfield->field.declarators->items[g]->deref_count == 0 &&
                       (mc_token_type)expression->member_access_expression.access_operator->type !=
                           MC_TOKEN_DECIMAL_POINT) {
                MCerror(218, "TODO");
              }
              else if (ptfield->field.declarators->items[g]->deref_count > 1) {
                MCerror(221, "TODO");
              }
              if (!primary_type_info) {
                MCerror(224, "uh oh, what do we do now...? %s", ptfield->field.type_name);
              }

              _determine_type_of_expression_subsearch(primary_type_info->fields,
                                                      expression->member_access_expression.identifier, result);

              if (!result->type_name) {
                print_syntax_node(expression, 0);
                MCerror(230, "TODO");
              }
              return 0;
            }
            case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
              struct_info *primary_type_info;
              find_struct_info(ptfield->field.type_name, &primary_type_info);
              if (!primary_type_info) {
                MCerror(6393, "uh oh, what do we do now...?");
              }

              // printf("parent_struct_info:%s\n", parent_struct_info->name);
              // printf("panda\n");
              // print_syntax_node(expression, 0);
              _determine_type_of_expression_subsearch(primary_type_info->fields,
                                                      expression->member_access_expression.identifier, result);
              if (!result->type_name) {
                print_syntax_node(expression, 0);
                MCerror(8348, "couldn't obtain result for expression");
              }
            } break;
            default:
              print_syntax_node(expression, 0);
              MCerror(7335, "TODO - Unsupported:%s",
                      get_mc_syntax_token_type_name(expression->member_access_expression.identifier->type));
            }
            return 0;
          }
        }
      } break;
      case FIELD_KIND_NESTED_STRUCT:
      case FIELD_KIND_NESTED_UNION: {
        if (ptfield->sub_type.is_anonymous) {
          // Search within the sub-types fields as if they were the parent types
          _determine_type_of_expression_subsearch(ptfield->sub_type.fields, expression, result);
          // printf("gets here \n");
          if (result->type_name) {
            return 0;
            // struct_info *primary_type_info;
            // find_struct_info(result->type_name, &primary_type_info);
            // if (!primary_type_info) {
            //   MCerror(7239, "uh oh, what do we do now...? %s", result->type_name);
            // }

            // _determine_type_of_expression_subsearch(primary_type_info->fields,
            //                                         expression->member_access_expression.identifier, result);
            // if (!result->type_name) {
            //   print_syntax_node(expression, 0);
            // MCerror(8337, "TODO:%s", result->type_name);
            // }
          }
          // printf("gets here \n");
          break;
        }

        if (!ptfield->sub_type.declarators || !ptfield->sub_type.declarators->count) {
          MCerror(232, "Mislabled subtype");
        }

        // Search declarators to pin as the sub-type
        bool found = false;
        for (int g = 0; g < ptfield->sub_type.declarators->count; ++g) {
          if (!strcmp(primary_name, ptfield->sub_type.declarators->items[g]->name)) {
            found = true;
            break;
          }
        }

        if (!found)
          break;
        // printf("found !%s\n", primary_name);

        // Search for the identifier type within this one
        _determine_type_of_expression_subsearch(ptfield->sub_type.fields,
                                                expression->member_access_expression.identifier, result);
        if (!result->type_name) {
          MCerror(222, "TODO?");
        }
        // printf("RESULT:%s\n", result->type_name);

        return 0;
      } break;
      default:
        MCerror(378, "TODO:%i", ptfield->field_type);
      }
    }

  } break;
  default:
    MCerror(343, "TODO %s", get_mc_syntax_token_type_name(expression->type));
  }

  // print_syntax_node(expression, 0);
  // MCerror(189, "TODO : shouldn't reach here %s", result->type_name);
  return 0;
}

// int get_keyword_const_text_name(mc_token_type keyword_type, const char **p_text)
// {
//   switch (keyword_type) {
//   case MC_TOKEN_CHAR_KEYWORD: {
//     const char *CHAR_KEYWORD_NAME = "char";
//     *p_text = (char *)CHAR_KEYWORD_NAME;
//   } break;
//   case MC_TOKEN_INT_KEYWORD: {
//     const char *INT_KEYWORD_NAME = "int";
//     *p_text = (char *)INT_KEYWORD_NAME;
//   } break;
//   case MC_TOKEN_VOID_KEYWORD: {
//     const char *VOID_KEYWORD_NAME = "void";
//     *p_text = (char *)VOID_KEYWORD_NAME;
//   } break;
//   case MC_TOKEN_LONG_KEYWORD: {
//     const char *LONG_KEYWORD_NAME = "long";
//     *p_text = (char *)LONG_KEYWORD_NAME;
//   } break;
//   default: {
//     // print_syntax_node(keyword_type, 0);
//     MCerror(363, "TODO? %s", get_mc_token_type_name(keyword_type));
//   } break;
//   }

//   return 0;
// }

int determine_type_of_expression(mct_transcription_state *ts, mc_syntax_node *expression,
                                 mct_expression_type_info *result)
{
  result->type_name = NULL;

  switch (expression->type) {
  case MC_SYNTAX_CAST_EXPRESSION: {
    // Make it the cast type
    if (expression->cast_expression.type_identifier->type == MC_SYNTAX_FUNCTION_POINTER_DECLARATION) {
      result->is_fptr = true;
      result->is_array = false; // TODO -- rare corner case

      // mcs_copy_syntax_node_to_text(expression->cast_expression.type_identifier, &result->type_name);
      // if (expression->cast_expression.type_identifier->fptr_declaration.return_type_dereference) {
      //   result->deref_count = expression->cast_expression.type_identifier->fptr_declaration.return_type_dereference
      //                             ->dereference_sequence.count;
      // }
      // else {
      result->deref_count = 0;
      // }
    }
    else {
      result->is_array = false; // TODO ...
      result->is_fptr = false;
      mcs_copy_syntax_node_to_text(expression->cast_expression.type_identifier, &result->type_name);
      if (expression->cast_expression.type_dereference) {
        result->deref_count = expression->cast_expression.type_dereference->dereference_sequence.count;
      }
      else {
        result->deref_count = 0;
      }
    }
  } break;
  case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
    // Determine the type of the left-hand-side
    determine_type_of_expression(ts, expression->operational_expression.left, result);
  } break;
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
    mct_expression_type_info parent_type_info;
    determine_type_of_expression(ts, expression->member_access_expression.primary, &parent_type_info);
    if (!parent_type_info.type_name) {
      MCerror(272, "WhatDo?");
    }
    // printf("typeofparent-'%s' %i \n", parent_type_info.type_name, parent_type_info.deref_count);

    if (parent_type_info.deref_count == 1 &&
        (mc_token_type)expression->member_access_expression.access_operator->type != MC_TOKEN_POINTER_OPERATOR) {
      MCerror(271, "TODO");
    }
    else if (parent_type_info.deref_count == 0 &&
             (mc_token_type)expression->member_access_expression.access_operator->type != MC_TOKEN_DECIMAL_POINT) {
      MCerror(275, "TODO");
    }
    else if (parent_type_info.deref_count > 1) {
      MCerror(279, "TODO");
    }

    struct_info *parent_struct_info;
    find_struct_info(parent_type_info.type_name, &parent_struct_info);
    if (!parent_struct_info) {
      MCerror(292, "uh oh, what do we do now...?");
    }

    mct_release_expression_type_info_fields(&parent_type_info);

    // printf("parent_struct_info:%s\n", parent_struct_info->name);
    // printf("panda\n");
    // print_syntax_node(expression, 0);
    _determine_type_of_expression_subsearch(parent_struct_info->fields, expression->member_access_expression.identifier,
                                            result);
    if (!result->type_name) {
      print_syntax_node(expression, 0);
      MCerror(7470, "couldn't obtain result for expression");
    }
  } break;
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
    // Determine the type of the left-hand-side
    determine_type_of_expression(ts, expression->element_access_expression.primary, result);

    // print_syntax_node(expression, 0);
    // printf("type %s array\n", result->is_array ? "is" : "is NOT");
    if (result->is_array) {
      // It is already implicitly dereferenced with its type info
    }
    else {
      if (!result->deref_count) {
        MCerror(9467, "Don't know how to return element access type if it isn't dereferenceable:%s", result->type_name);
      }
      --result->deref_count;
    }
  } break;
  case MC_SYNTAX_BITWISE_EXPRESSION: {
    // Determine the type of the left-hand-side
    determine_type_of_expression(ts, expression->bitwise_expression.left, result);
  } break;
  case MC_TOKEN_IDENTIFIER: {
    for (int d = ts->scope_index; d >= 0; --d) {
      // printf("scope:%i c-:%i\n", d, ts->scope[d].variable_count);
      for (int b = 0; b < ts->scope[d].variable_count; ++b) {
        // printf("%s<>%s\n", ts->scope[d].variables[b].name, expression->text);
        if (!strcmp(ts->scope[d].variables[b].name, expression->text)) {
          switch (ts->scope[d].variables[b].declaration_node->type) {
          case MC_SYNTAX_PARAMETER_DECLARATION: {
            mc_syntax_node *param_decl = ts->scope[d].variables[b].declaration_node;

            switch (param_decl->parameter.type) {
            case PARAMETER_KIND_STANDARD: {
              // TODO -- array and function pointer types
              result->is_array = false;
              result->is_fptr = false;

              // char *type_identity
              mcs_copy_syntax_node_to_text(param_decl->parameter.type_identifier, &result->type_name);
              // if ((mc_token_type)param_decl->parameter.type_identifier->type_identifier.identifier->type ==
              //     MC_TOKEN_IDENTIFIER) {
              //   result->type_name = param_decl->parameter.type_identifier->type_identifier.identifier->text;
              // }
              // else {
              //   get_keyword_const_text_name(
              //       (mc_token_type)param_decl->parameter.type_identifier->type_identifier.identifier->type,
              //       (const char **)&result->type_name);
              // }

              if (param_decl->parameter.type_dereference) {
                result->deref_count = param_decl->parameter.type_dereference->dereference_sequence.count;
              }
              else {
                result->deref_count = 0;
              }
            } break;
            case PARAMETER_KIND_FUNCTION_POINTER: {
              printf("passed by here pk\n");
              result->is_array = false;
              result->is_fptr = true;

              mcs_copy_syntax_node_to_text(param_decl->parameter.type_identifier, &result->type_name);
              if (param_decl->parameter.type_dereference) {
                result->deref_count = param_decl->parameter.type_dereference->dereference_sequence.count;
              }
              else {
                result->deref_count = 0;
              }

            } break;
            default:
              MCerror(387, "TODO:%i", param_decl->parameter.type);
            }
          } break;
          case MC_SYNTAX_VARIABLE_DECLARATOR: {
            mc_syntax_node *var_declarator = ts->scope[d].variables[b].declaration_node;
            mc_syntax_node *declaration = var_declarator->parent;

            // TODO -- keep check?
            if (declaration->type != MC_SYNTAX_LOCAL_VARIABLE_DECLARATION) {
              MCerror(192, "Why isn't it a declaration?");
            }

            result->is_fptr = false;
            result->is_array =
                (var_declarator->variable_declarator.initializer &&
                 var_declarator->variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER);

            mcs_copy_syntax_node_to_text(declaration->local_variable_declaration.type_identifier, &result->type_name);

            if (var_declarator->variable_declarator.type_dereference) {
              result->deref_count = var_declarator->variable_declarator.type_dereference->dereference_sequence.count;
            }
            else {
              result->deref_count = 0;
            }
          } break;
          case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR: {
            mc_syntax_node *fptr_declarator = ts->scope[d].variables[b].declaration_node;
            mc_syntax_node *declaration = fptr_declarator->parent;

            result->is_fptr = true;
            result->is_array = false; // TODO ---- fptr arrays...

            mcs_copy_syntax_node_to_text(declaration->local_variable_declaration.type_identifier, &result->type_name);

            if (fptr_declarator->fptr_declarator.return_type_dereference) {
              result->deref_count =
                  fptr_declarator->fptr_declarator.return_type_dereference->dereference_sequence.count;
            }
            else {
              result->deref_count = 0;
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
  case MC_TOKEN_NUMERIC_LITERAL: {
    result->deref_count = 0;
    for (int i = 0; i < strlen(expression->text); ++i) {
      if (!isdigit(expression->text[i])) {
        printf("WARNING: determine_expression_type 473 this number wasn't really an int '%s'\n", expression->text);
      }
    }
    result->type_name = strdup("int");
    result->is_array = false;
    result->is_fptr = false;
  } break;
  case MC_TOKEN_CHAR_LITERAL: {
    result->deref_count = 0;
    result->type_name = strdup("char");
    result->is_array = false;
    result->is_fptr = false;
  } break;
  case MC_SYNTAX_DEREFERENCE_EXPRESSION: {
    // Determine the type of the left-hand-side
    determine_type_of_expression(ts, expression->dereference_expression.unary_expression, result);

    if (!result->deref_count) {
      MCerror(9559, "Can't dereference with no deref count");
    }

    if (!result->type_name) {
      MCerror(9563, "TODO");
    }

    --result->deref_count;
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    // Determine the type of the inner expression
    determine_type_of_expression(ts, expression->parenthesized_expression.expression, result);
  } break;
  default:
    print_syntax_node(expression, 0);
    MCerror(7544, "Unsupported:%s", get_mc_syntax_token_type_name(expression->type));
  }

  return 0;
}

int mct_transcribe_va_list_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_transcribe_indent(ts);
  append_to_mc_strf(ts->str, "va_list %s;\n", syntax_node->va_list_expression.list_identity->text);

  return 0;
}

// int mct_contains_mc_invoke(mc_syntax_node *syntax_node, bool *result)
// {
//   const char *type_name = get_mc_syntax_token_type_name(syntax_node->type);
//   register_midge_error_tag("mct_contains_mc_invoke(%s)", type_name);

//   *result = false;
//   if ((mc_token_type)syntax_node->type <= MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
//     return 0;
//   }

//   register_midge_error_tag("mct_contains_mc_invoke()-1");
//   if (syntax_node->type == MC_SYNTAX_INVOCATION &&
//       (mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER) {

//     function_info *func_info;
//     find_function_info(syntax_node->invocation.function_identity->text, &func_info);

//     if (func_info) {
//       // print_syntax_node(syntax_node, 0);
//       // MCerror(35, "TODO : %s", syntax_node->invocation.function_identity->text);
//       register_midge_error_tag("mct_contains_mc_invoke()-2");

//       // printf("mcmi-3\n");
//       *result = true;
//       return 0;
//     }
//   }

//   register_midge_error_tag("mct_contains_mc_invoke()-3 child_count:%i", syntax_node->children->count);
//   for (int i = 0; i < syntax_node->children->count; ++i) {
//     mct_contains_mc_invoke(syntax_node->children->items[i], result);
//     if (*result) {
//       return 0;
//     }
//   }

//   register_midge_error_tag("mct_contains_mc_invoke(~)");
//   return 0;
// }

int mct_transcribe_type_identifier(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  // Const
  if (syntax_node->type_identifier.is_const) {
    append_to_mc_str(ts->str, "const ");
  }

  // Signing
  if (syntax_node->type_identifier.is_signed != -1) {
    if (syntax_node->type_identifier.is_signed == 0) {
      append_to_mc_str(ts->str, "unsigned ");
    }
    else if (syntax_node->type_identifier.is_signed == 1) {
      append_to_mc_str(ts->str, "signed ");
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
    append_to_mc_str(ts->str, "struct ");
  }

  // type identifier
  struct_info *structure_info;
  find_struct_info(syntax_node->type_identifier.identifier->text, &structure_info);
  if (structure_info) {
    append_to_mc_str(ts->str, structure_info->name);
  }
  else {
    enumeration_info *enum_info;
    find_enumeration_info(syntax_node->type_identifier.identifier->text, &enum_info);
    if (enum_info) {
      append_to_mc_str(ts->str, enum_info->name);
    }
    else {
      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->type_identifier.identifier);
    }
  }

  return 0;
}

// int mct_transcribe_mc_invocation_argument(mct_transcription_state *ts, parameter_info *parameter,
//                                           mc_syntax_node *argument, const char *argument_data_name, int arg_index)
// {
//   // printf("mtmi-3\n");
//   mct_transcribe_indent(ts);

//   // while (argument->type == MC_SYNTAX_CAST_EXPRESSION) {
//   //   argument = argument->cast_expression.expression;
//   // }

//   // printf("argument->type:%i\n", argument->type);
//   // print_syntax_node(argument, 0);
//   switch (argument->type) {
//   case MC_SYNTAX_CAST_EXPRESSION: {
//     int tslen = ts->str->len;

//     mct_transcribe_type_identifier(ts, argument->cast_expression.type_identifier);
//     append_to_mc_str(ts->str, " ");
//     if (argument->cast_expression.type_dereference) {
//       for (int a = 0; a < argument->cast_expression.type_dereference->dereference_sequence.count; ++a) {
//         append_to_mc_str(ts->str, "*");
//       }
//     }

//     append_to_mc_strf(ts->str, "%s_%i = (", argument_data_name, arg_index);

//     mct_transcribe_type_identifier(ts, argument->cast_expression.type_identifier);
//     append_to_mc_str(ts->str, " ");
//     if (argument->cast_expression.type_dereference) {
//       for (int a = 0; a < argument->cast_expression.type_dereference->dereference_sequence.count; ++a) {
//         append_to_mc_str(ts->str, "*");
//       }
//     }

//     append_to_mc_str(ts->str, ")");

//     mcs_append_syntax_node_to_mc_str(ts->str, argument->cast_expression.expression);
//     append_to_mc_str(ts->str, ";\n");

//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = (void *)&%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//                      arg_index);

//     // printf("cast_expression_transcription:\n%s\n", ts->str->text + tslen);
//     //   // printf("mtmi-4\n");
//     //   bool contains_mc_function_call;
//     //   if (argument->cast_expression.expression) {
//     //     mct_contains_mc_invoke(argument->cast_expression.expression, &contains_mc_function_call);
//     //     if (contains_mc_function_call) {
//     //       MCerror(104, "TODO");
//     //     }
//     //   }

//     //   char *text;
//     //   if (argument->cast_expression.expression->type == MC_SYNTAX_PREPENDED_UNARY_EXPRESSION &&
//     //       (mc_token_type)argument->cast_expression.expression->prepended_unary.prepend_operator->type ==
//     //           MC_TOKEN_AMPERSAND_CHARACTER) {
//     //     mcs_copy_syntax_node_to_text(argument->cast_expression.expression, &text);
//     //     append_to_mc_strf(ts->str, "void *mc_varg_%i = (void *)%s;\n", i, text);
//     //     mct_transcribe_indent(ts);
//     //     append_to_mc_strf(ts->str, "mc_vargs[%i] = &mc_varg_%i;\n", i, i);
//     //   }
//     //   else {
//     //     mcs_copy_syntax_node_to_text(argument->cast_expression.expression, &text);
//     //     append_to_mc_strf(ts->str, "mc_vargs[%i] = &%s;\n", i, text);
//     //   }
//     //   free(text);
//   } break;
//   case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
//     // printf("mtmi-5\n");
//     // Do MC_invokes
//     bool contains_mc_function_call;
//     {
//       mct_contains_mc_invoke(argument, &contains_mc_function_call);
//       if (contains_mc_function_call) {
//         MCerror(750, "TODO");
//       }
//     }

//     char *text;
//     mcs_copy_syntax_node_to_text(argument, &text);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s;\n", argument_data_name, arg_index, text);
//     free(text);

//   } break;
//   case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
//     // Find the type of the expression
//     mct_expression_type_info expr_type_info;
//     // print_syntax_node(argument, 0);
//     determine_type_of_expression(ts, argument->parenthesized_expression.expression, &expr_type_info);
//     // printf("Type:%s:'%s':%i\n", expr_type_info.is_array ? "is_ary" : "not_ary", expr_type_info.type_name,
//     //        expr_type_info.deref_count);
//     if (!expr_type_info.type_name) {
//       print_syntax_node(argument, 0);
//       MCerror(566, "TODO");
//     }

//     if (expr_type_info.is_fptr || expr_type_info.is_array) {
//       print_syntax_node(argument, 0);
//       MCerror(8741, "TODO %i %i", expr_type_info.is_fptr, expr_type_info.is_array);
//     }

//     // Evaluate it to a local field
//     append_to_mc_str(ts->str, expr_type_info.type_name);
//     append_to_mc_str(ts->str, " ");
//     for (int d = 0; d < expr_type_info.deref_count; ++d) {
//       // MCerror(6748, "TODO -- pointers...");
//       append_to_mc_str(ts->str, "*");
//     }
//     append_to_mc_strf(ts->str, "%s_%i = ", argument_data_name, arg_index);

//     mct_transcribe_expression(ts, NULL, argument->parenthesized_expression.expression);
//     append_to_mc_str(ts->str, ";\n");

//     // Set to parameter reference
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);

//     mct_release_expression_type_info_fields(&expr_type_info);
//     // printf("After:\n%s||\n", ts->str->text);

//   } break;
//   case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
//     // printf("mtmi-4\n");
//     {
//       bool contains_mc_function_call;
//       mct_contains_mc_invoke(argument, &contains_mc_function_call);
//       if (contains_mc_function_call) {
//         MCerror(290, "TODO");
//       }
//     }

//     // Find the type of the expression
//     mct_expression_type_info expr_type_info;
//     determine_type_of_expression(ts, argument, &expr_type_info);
//     // printf("Type:%s:'%s':%i\n", expr_type_info.is_array ? "is_ary" : "not_ary", expr_type_info.type_name,
//     //        expr_type_info.deref_count);
//     if (!expr_type_info.type_name) {
//       MCerror(566, "TODO");
//     }

//     // Evaluate it to a local field
//     append_to_mc_str(ts->str, expr_type_info.type_name);
//     append_to_mc_str(ts->str, " ");
//     for (int d = 0; d < expr_type_info.deref_count; ++d) {
//       // print_syntax_node(argument, 0);
//       // MCerror(780, "TODO -- pointers...");
//       append_to_mc_str(ts->str, "*");
//     }
//     append_to_mc_strf(ts->str, "%s_%i = ", argument_data_name, arg_index);

//     mct_transcribe_expression(ts, NULL, argument);
//     append_to_mc_str(ts->str, ";\n");

//     // Set to parameter reference
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);

//     mct_release_expression_type_info_fields(&expr_type_info);
//     // printf("After:\n%s||\n", ts->str->text);
//   } break;
//   case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
//     // printf("mtmi-6\n");
//     // Do MC_invokes
//     bool contains_mc_function_call;
//     if (argument->element_access_expression.primary) {
//       mct_contains_mc_invoke(argument->element_access_expression.primary, &contains_mc_function_call);
//       if (contains_mc_function_call) {
//         MCerror(104, "TODO");
//       }
//     }
//     if (argument->element_access_expression.access_expression) {

//       mct_contains_mc_invoke(argument->element_access_expression.access_expression, &contains_mc_function_call);
//       if (contains_mc_function_call) {
//         MCerror(848, "TODO");
//       }
//     }

//     char *text;
//     mcs_copy_syntax_node_to_text(argument, &text);
//     append_to_mc_strf(ts->str, "%s[%i] = (void *)&%s;\n", argument_data_name, arg_index, text);
//     free(text);

//   } break;
//   case MC_SYNTAX_DEREFERENCE_EXPRESSION: {
//     append_to_mc_strf(ts->str, "%s[%i] = (void *)", argument_data_name, arg_index);
//     for (int d = 1; d < argument->dereference_expression.deref_sequence->dereference_sequence.count; ++d)
//       append_to_mc_str(ts->str, "*");

//     char *text;
//     mcs_copy_syntax_node_to_text(argument->dereference_expression.unary_expression, &text);
//     append_to_mc_str(ts->str, text);
//     free(text);

//     append_to_mc_str(ts->str, ";\n");
//   } break;
//   case MC_SYNTAX_STRING_LITERAL_EXPRESSION: {
//     // printf("mtmi-7\n");
//     char *text;
//     mcs_copy_syntax_node_to_text(argument, &text);
//     append_to_mc_strf(ts->str, "const char *%s_%i = %s;\n", argument_data_name, arg_index, text);
//     free(text);
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);
//   } break;
//   case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
//     // printf("mtmi-8\n");
//     char *text;
//     if ((mc_token_type)argument->prepended_unary.prepend_operator->type == MC_TOKEN_AMPERSAND_CHARACTER) {
//       mcs_copy_syntax_node_to_text(argument, &text);
//       append_to_mc_strf(ts->str, "void *%s_%i = (void *)%s;\n", argument_data_name, arg_index, text);
//       mct_transcribe_indent(ts);
//       append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//       arg_index);
//     }
//     else {
//       MCerror(96, "TODO");
//     }
//     free(text);
//   } break;
//   case MC_SYNTAX_BITWISE_EXPRESSION: {
//     append_to_mc_strf(ts->str, "int %s_%i = ", argument_data_name, arg_index);
//     mcs_append_syntax_node_to_mc_str(ts->str, argument);
//     append_to_mc_str(ts->str, ";\n");
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);
//   } break;
//   case MC_SYNTAX_TERNARY_CONDITIONAL: {
//     // printf("mtmi-4\n");
//     {
//       bool contains_mc_function_call;
//       mct_contains_mc_invoke(argument, &contains_mc_function_call);
//       if (contains_mc_function_call) {
//         MCerror(290, "TODO");
//       }
//     }

//     // Find the type of the expression
//     mct_expression_type_info expr_type_info;
//     determine_type_of_expression(ts, argument->ternary_conditional.true_expression, &expr_type_info);
//     // printf("Type:%s:'%s':%i\n", expr_type_info.is_array ? "is_ary" : "not_ary", expr_type_info.type_name,
//     //        expr_type_info.deref_count);
//     if (!expr_type_info.type_name) {
//       MCerror(566, "TODO");
//     }

//     // Evaluate it to a local field
//     append_to_mc_str(ts->str, expr_type_info.type_name);
//     append_to_mc_str(ts->str, " ");
//     for (int d = 0; d < expr_type_info.deref_count; ++d) {
//       append_to_mc_str(ts->str, "*");
//     }
//     append_to_mc_strf(ts->str, "%s_%i = ", argument_data_name, arg_index);

//     mcs_append_syntax_node_to_mc_str(ts->str, argument);
//     append_to_mc_str(ts->str, ";\n");

//     // Set to parameter reference
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);

//     mct_release_expression_type_info_fields(&expr_type_info);
//     // printf("After:\n%s||\n", ts->str->text);
//   } break;
//   case MC_SYNTAX_SIZEOF_EXPRESSION: {
//     append_to_mc_strf(ts->str, "size_t %s_%i = sizeof(", argument_data_name, arg_index);
//     mct_transcribe_type_identifier(ts, argument->sizeof_expression.type_identifier);
//     append_to_mc_str(ts->str, " ");
//     if (argument->sizeof_expression.type_dereference) {
//       for (int d = 0; d < argument->sizeof_expression.type_dereference->dereference_sequence.count; ++d) {
//         append_to_mc_str(ts->str, "*");
//       }
//     }
//     append_to_mc_str(ts->str, ");\n");
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name, arg_index);
//   } break;
//   case MC_SYNTAX_FIXREMENT_EXPRESSION: {

//     char *primary;
//     mcs_copy_syntax_node_to_text(argument->fixrement_expression.primary, &primary);

//     if (!argument->fixrement_expression.is_postfix) {
//       mcs_append_syntax_node_to_mc_str(ts->str, argument);
//       append_to_mc_str(ts->str, ";\n");
//       mct_transcribe_indent(ts);
//       append_to_mc_strf(ts->str, "%s[%i] = &%s;\n", argument_data_name, arg_index, primary);
//     }
//     else {
//       append_to_mc_strf(ts->str, "int %s_%i = (int)%s;\n", argument_data_name, arg_index, primary);
//       mct_transcribe_indent(ts);
//       append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//       arg_index);

//       mct_transcribe_indent(ts);
//       mcs_append_syntax_node_to_mc_str(ts->str, argument);
//       append_to_mc_str(ts->str, ";\n");
//     }
//     free(primary);

//   } break;
//   default: {
//     // printf("mtmi-9\n");
//     switch ((mc_token_type)argument->type) {
//     case MC_TOKEN_CHAR_LITERAL: {
//       append_to_mc_strf(ts->str, "char %s_%i = %s;\n", argument_data_name, arg_index, argument->text);
//       mct_transcribe_indent(ts);
//       append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//       arg_index);
//     } break;
//     case MC_TOKEN_NUMERIC_LITERAL: {
//       // printf("mtmi-9\n");
//       char *parameter_type_name = (char *)"int";
//       if (parameter) {
//         if (parameter->type_deref_count) {
//           print_syntax_node(argument, 0);
//           MCerror(915, "Incorrect argument?");
//         }
//         if (parameter->parameter_type != PARAMETER_KIND_STANDARD) {
//           print_syntax_node(argument, 0);
//           MCerror(918, "Incorrect argument by paramter type");
//         }

//         // TODO -- work on this more
//         // won't work on function pointers, more checking is required

//         parameter_type_name = (char *)parameter->type_name;
//       }

//       append_to_mc_strf(ts->str, "%s %s_%i = %s;\n", parameter_type_name, argument_data_name, arg_index,
//       argument->text); mct_transcribe_indent(ts); append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n",
//       argument_data_name, arg_index, argument_data_name, arg_index);
//     } break;
//     case MC_TOKEN_IDENTIFIER: {
//       if (!strcmp(argument->text, "NULL")) {
//         append_to_mc_strf(ts->str, "void *%s_%i = NULL;\n", argument_data_name, arg_index);
//         mct_transcribe_indent(ts);
//         append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//         arg_index); break;
//       }
//       if (!strcmp(argument->text, "false")) {
//         append_to_mc_strf(ts->str, "unsigned char %s_%i = false;\n", argument_data_name, arg_index);
//         mct_transcribe_indent(ts);
//         append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//         arg_index); break;
//       }
//       if (!strcmp(argument->text, "true")) {
//         append_to_mc_strf(ts->str, "unsigned char %s_%i = true;\n", argument_data_name, arg_index);
//         mct_transcribe_indent(ts);
//         append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n", argument_data_name, arg_index, argument_data_name,
//         arg_index); break;
//       }

//       // Search enum values...
//       enumeration_info *enum_info;
//       enum_member_info *enum_member;
//       find_enum_member_info(argument->text, &enum_info, &enum_member);
//       // if (!strcmp(argument->text, "MC_SYNTAX_DEREFERENCE_SEQUENCE")) {
//       //   printf("enum was :%p\n", enum_member);
//       // }
//       if (enum_member) {
//         append_to_mc_strf(ts->str, "%s %s_%i = %s;\n", enum_info->name, argument_data_name, arg_index,
//         argument->text); mct_transcribe_indent(ts); append_to_mc_strf(ts->str, "%s[%i] = &%s_%i;\n",
//         argument_data_name, arg_index, argument_data_name, arg_index); break;
//       }

//       // if (!strcmp(argument->text, "buf")) {
//       // Array implicit decay fubberjiggle workaround
//       {
//         mct_expression_type_info arg_type_info;
//         determine_type_of_expression(ts, argument, &arg_type_info);
//         if (arg_type_info.type_name && arg_type_info.is_array) {
//           // print_syntax_node(argument, 0);
//           // printf("Type:%s:'%s':%i\n", arg_type_info.is_array ? "is_ary" : "not_ary", arg_type_info.type_name,
//           //        arg_type_info.deref_count);

//           append_to_mc_strf(ts->str, "%s *%s_%i = %s;\n", arg_type_info.type_name, argument_data_name, arg_index,
//                            argument->text);
//           mct_transcribe_indent(ts);
//           append_to_mc_strf(ts->str, "%s[%i] = (void *)&%s_%i;\n", argument_data_name, arg_index,
//           argument_data_name,
//                            arg_index);
//           break;
//         }
//         mct_release_expression_type_info_fields(&arg_type_info);
//       }

//       // Maybe it's a reference to a function
//       {
//         function_info *func_info;
//         find_function_info(argument->text, &func_info);
//         if (func_info) {
//           // void *vv = (void *)mct_transcribe_mc_invocation;
//           // printf("printf(\"mc_vargs[%%i] = %p;\", %i, %s);\n", i, argument->text);
//           append_to_mc_strf(ts->str, "printf(\"%s[%%i] = %%p;\", %i, %s);\n", argument_data_name, arg_index,
//                            argument->text);
//           append_to_mc_strf(ts->str, "%s[%i] = (void *)%s;\n", argument_data_name, arg_index, argument->text);

//           break;
//         }
//       }

//       // TODO -- ?? assigning to 'void *' from incompatible type 'const int *'
//       append_to_mc_strf(ts->str, "%s[%i] = (void *)&%s;\n", argument_data_name, arg_index, argument->text);
//     } break;
//     default:
//       print_syntax_node(argument, 0);
//       MCerror(1072, "NotYetSupported:%s", get_mc_syntax_token_type_name(argument->type));
//     }
//   }
//   }
//   return 0;
// }

// int mct_transcribe_mc_invocation(mct_transcription_state *ts, mc_syntax_node *syntax_node, char
// *return_variable_name)
// {
//   register_midge_error_tag("mct_transcribe_mc_invocation()");

//   // print_syntax_node(syntax_node, 0);
//   // DEBUG
//   int was_index = ts->str->len;
//   // DEBUG

//   //{
//   //   if (ts->options->report_function_entry_exit_to_stack) {
//   //     mct_transcribe_text_with_indent(ts, "{\n");
//   //     ++ts->indent;
//   //     mct_transcribe_text_with_indent(ts, "int midge_error_stack_index;\n");

//   //     mct_transcribe_indent(ts);
//   //     append_to_mc_str(ts->str, "register_midge_stack_invocation(\"");

//   //     mcs_append_syntax_node_to_mc_str(ts->str,
//   //                                   syntax_node->expression_statement.expression->invocation.function_identity);
//   //     // append_to_mc_strf(ts->str, "\", \"%s\", %i, &midge_error_stack_index);\n", "unknown-file",
//   //     //                  syntax_node->expression_statement.expression->begin.line);
//   //     append_to_mc_str(ts->str, "\", \"unknown-file\", ");
//   //     append_to_mc_strf(ts->str, "%i", syntax_node->expression_statement.expression->begin.line);
//   //     append_to_mc_str(ts->str, ", &midge_error_stack_index);\n");
//   //   }
//   // }

//   // mct_transcribe_indent(ts);

//   // register_midge_error_tag("mct_transcribe_statement_list-ES5");
//   // mct_transcribe_expression(ts, NULL,syntax_node->expression_statement.expression);
//   // append_to_mc_str(ts->str, ";\n");
//   // register_midge_error_tag("mct_transcribe_statement_list-ES9");

//   // if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
//   //   if (ts->options->report_function_entry_exit_to_stack) {
//   //     mct_transcribe_indent(ts);
//   //     append_to_mc_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");
//   //     --ts->indent;
//   //     mct_transcribe_text_with_indent(ts, "}\n");
//   //   }
//   // }
//   //}

//   if (syntax_node->type != MC_SYNTAX_INVOCATION) {
//     MCerror(5570, "TODO %s", get_mc_syntax_token_type_name(syntax_node->type));
//   }

//   function_info *func_info;
//   char *function_name;
//   mcs_copy_syntax_node_to_text(syntax_node->invocation.function_identity, &function_name);
//   find_function_info(function_name, &func_info);
//   // if (!func_info) {

//   //   print_syntax_node(syntax_node, 0);
//   //   MCerror(86, "Not an mc_function?");
//   // }

//   // if(!func_info)
//   // // Maybe its a function pointer
//   // mct_expression_type_info var_type;
//   // determine_type_of_expression(ts, syntax_node->invocation.function_identity, &var_type);
//   // if (var_type.is_fptr) {
//   //   *result = true;
//   //   return 0;
//   // }

//   // print_syntax_node(syntax_node, 0);
//   // printf("mtmi-2 %p\n", finfo);
//   // printf("mtmi-2 %i\n", finfo->parameter_count + 1);

//   mct_transcribe_text_with_indent(ts, "{\n");
//   ++ts->indent;

//   // mct_transcribe_text_with_indent(ts, "int is_mc_invoke = ");
//   // if (func_info) {
//   //   append_to_mc_str(ts->str, "1");
//   // }
//   // else {
//   //   // Search for the pointer
//   //   append_to_mc_str(ts->str, "0");
//   // }
//   // append_to_mc_str(ts->str, ";\n");

//   // ++ts->indent;
//   const char *ARGUMENT_DATA_NAME = "mc_vargs";
//   int mc_argument_data_count = 0;
//   if (func_info) {
//     // Function not null check
//     if (ts->options->check_mc_functions_not_null) {
//       mct_transcribe_text_with_indent(ts, "if (!");
//       append_to_mc_str(ts->str, function_name);
//       append_to_mc_str(ts->str, ") {\n");
//       mct_transcribe_text_with_indent(ts, "MCerror(1174, \"Attempted to invoke declared but undefined function:'");
//       append_to_mc_str(ts->str, function_name);
//       append_to_mc_str(ts->str, "'\");\n");
//       mct_transcribe_text_with_indent(ts, "}\n");
//     }

//     mct_transcribe_indent(ts);
//     mc_argument_data_count = syntax_node->invocation.arguments->count + 1;

//     append_to_mc_strf(ts->str, "void *%s[%i];\n", ARGUMENT_DATA_NAME, mc_argument_data_count);

//     register_midge_error_tag("mct_transcribe_mc_invocation-parameters");
//     if (func_info && func_info->parameter_count != syntax_node->invocation.arguments->count) {
//       // Unless the function has variable arguments
//       bool valid =
//           func_info->parameter_count < syntax_node->invocation.arguments->count &&
//           func_info->parameters[func_info->parameter_count - 1]->parameter_type == PARAMETER_KIND_VARIABLE_ARGS;

//       if (!valid) {
//         MCerror(79, "argument count not equal to required parameters, invoke:%s, expected:%i, passed:%i",
//                 func_info->name, func_info->parameter_count, syntax_node->invocation.arguments->count);
//       }
//     }

//     if (!func_info) {
//       print_syntax_node(syntax_node, 0);
//       MCerror(1110, "func info required for parameter type argument\n");
//     }

//     for (int i = 0; i < syntax_node->invocation.arguments->count; ++i) {

//       parameter_info *parameter = NULL;
//       if (i < func_info->parameter_count) {
//         parameter = func_info->parameters[i];
//       }

//       mct_transcribe_mc_invocation_argument(ts, parameter, syntax_node->invocation.arguments->items[i],
//                                             ARGUMENT_DATA_NAME, i);
//     }

//     register_midge_error_tag("mct_transcribe_mc_invocation-return");
//     if (strcmp(func_info->return_type.name, "void") || func_info->return_type.deref_count) {
//       if (!return_variable_name) {
//         // Use a dummy value
//         mct_transcribe_text_with_indent(ts, func_info->return_type.name);
//         append_to_mc_str(ts->str, " ");
//         for (int i = 0; i < func_info->return_type.deref_count; ++i) {
//           append_to_mc_str(ts->str, "*");
//         }

//         append_to_mc_strf(ts->str, "%s_dummy_rv", ARGUMENT_DATA_NAME);
//         if (func_info->return_type.deref_count) {
//           append_to_mc_str(ts->str, " = NULL;\n");
//         }
//         else {
//           append_to_mc_str(ts->str, ";\n");
//         }

//         mct_transcribe_indent(ts);
//         append_to_mc_strf(ts->str, "%s[%i] = &%s_dummy_rv;\n", ARGUMENT_DATA_NAME,
//                          syntax_node->invocation.arguments->count, ARGUMENT_DATA_NAME);
//       }
//       else {
//         mct_transcribe_indent(ts);

//         // char *name;
//         // mcs_copy_syntax_node_to_text(return_variable_name, &name);
//         append_to_mc_strf(ts->str, "%s[%i] = &%s;\n", ARGUMENT_DATA_NAME, syntax_node->invocation.arguments->count,
//                          return_variable_name);
//         // free(name);
//       }
//     }
//     else {
//       mct_transcribe_indent(ts);
//       append_to_mc_strf(ts->str, "%s[%i] = ((void *)0);\n", ARGUMENT_DATA_NAME,
//                        syntax_node->invocation.arguments->count);
//     }
//   }

//   if (ts->options->report_function_entry_exit_to_stack) {
//     mct_transcribe_text_with_indent(ts, "{\n");
//     ++ts->indent;
//     mct_transcribe_text_with_indent(ts, "int midge_error_stack_index;\n");

//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "register_midge_stack_invocation(\"%s\", __FILE__, %i, &midge_error_stack_index);\n",
//                      function_name, syntax_node->begin.line + 1);
//   }

//   // mct_transcribe_text_with_indent(ts, "if (is_mc_invoke) {\n");
//   // ++ts->indent;
//   if (func_info) {
//     // Mc_invocation
//     mct_transcribe_indent(ts);
//     append_to_mc_strf(ts->str, "int mcfc_result = %s(%i, %s);\n", function_name, mc_argument_data_count,
//                      ARGUMENT_DATA_NAME);
//     mct_transcribe_text_with_indent(ts, "if (mcfc_result) {\n");
//     ++ts->indent;
//     mct_transcribe_text_with_indent(ts, "printf(\"--");
//     append_to_mc_str(ts->str, function_name);
//     append_to_mc_strf(ts->str, " line :%i ERR:%%i\\n\", mcfc_result);\n", syntax_node->begin.line + 1);
//     mct_transcribe_text_with_indent(ts, "return mcfc_result;\n");
//     --ts->indent;
//     mct_transcribe_text_with_indent(ts, "}\n");
//   }
//   else {
//     // Standard Function Call
//     // --ts->indent;
//     // mct_transcribe_text_with_indent(ts, "} else {\n");
//     // ++ts->indent;

//     mct_transcribe_indent(ts);
//     mct_transcribe_expression(ts, NULL, syntax_node);
//     append_to_mc_str(ts->str, ";\n");

//     // --ts->indent;
//     // mct_transcribe_text_with_indent(ts, "}\n");
//   }

//   if (ts->options->report_function_entry_exit_to_stack) {
//     mct_transcribe_indent(ts);
//     append_to_mc_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");

//     --ts->indent;
//     mct_transcribe_text_with_indent(ts, "}\n");
//   }
//   // --ts->indent;
//   // mct_transcribe_text_with_indent(ts, "}\n");

//   --ts->indent;
//   mct_transcribe_text_with_indent(ts, "}\n");

//   // if (!strcmp(func_info->name, "append_to_mc_strf")) {
//   //   printf("appendcstf-call:\n%s||\n", ts->str->text + was_index);
//   // }

//   free(function_name);

//   register_midge_error_tag("mct_transcribe_mc_invocation(~)");
//   return 0;
// }

int mct_transcribe_function_pointer_declarator(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                                               mc_syntax_node *syntax_node)
{
  // puts("mct_transcribe_function_pointer_declarator");
  // print_syntax_node(syntax_node, 0);
  // printf("type:%s\n", get_mc_syntax_token_type_name(syntax_node->type));
  // printf("syntax_node->fptr_declarator.name='%s'\n", syntax_node->fptr_declarator.name ?
  // syntax_node->fptr_declarator.name->text : "(null)");
  if (syntax_node->fptr_declarator.return_type_dereference) {
    for (int d = 0; d < syntax_node->fptr_declarator.return_type_dereference->dereference_sequence.count; ++d) {
      append_to_mc_str(ts->str, "*");
    }
  }

  append_to_mc_str(ts->str, "(");
  if (syntax_node->fptr_declarator.fp_dereference) {
    for (int d = 0; d < syntax_node->fptr_declarator.fp_dereference->dereference_sequence.count; ++d) {
      append_to_mc_str(ts->str, "*");
    }
  }

  if (syntax_node->fptr_declarator.name) {
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->fptr_declarator.name);
  }
  append_to_mc_str(ts->str, ")(");

  for (int p = 0; p < syntax_node->fptr_declarator.parameters->count; ++p) {
    if (p > 0) {
      append_to_mc_str(ts->str, ",");
    }

    mc_syntax_node *parameter = syntax_node->fptr_declarator.parameters->items[p];
    if (parameter->parameter.function_pointer) {
      MCerror(2518, "TODO fptr parameter transcription for a function pointer declarator");
    }

    mct_transcribe_type_identifier(ts, parameter->parameter.type_identifier);
    bool space_added = false;
    if (parameter->parameter.type_dereference) {
      append_to_mc_str(ts->str, " ");
      space_added = true;

      for (int d = 0; d < parameter->parameter.type_dereference->dereference_sequence.count; ++d) {
        append_to_mc_str(ts->str, "*");
      }
    }

    if (parameter->parameter.name) {
      if (!space_added)
        append_to_mc_str(ts->str, " ");

      mcs_append_syntax_node_to_mc_str(ts->str, parameter->parameter.name);
    }
  }
  append_to_mc_str(ts->str, ")");

  if (syntax_node->fptr_declarator.initializer) {
    mc_syntax_node *initializer = syntax_node->fptr_declarator.initializer;
    MCcall(append_to_mc_str(ts->str, " = "));

    MCcall(mct_transcribe_expression(ts, st_info, initializer->variable_assignment_initializer.value_expression));
  }
  return 0;
}

int mct_transcribe_function_pointer_declaration(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_statement_transcription_info st_info = {};
  st_info.begin_index = st_info.prefix_end_index = ts->str->len;
  st_info.variable_reported = false;

  mct_transcribe_type_identifier(ts, syntax_node->fptr_declaration.return_type_identifier);
  append_to_mc_str(ts->str, " ");

  mct_transcribe_function_pointer_declarator(ts, &st_info, syntax_node->fptr_declaration.declarator);

  return 0;
}

int mct_transcribe_variable_declarator(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                                       mc_syntax_node *declarator)
{
  if (declarator->variable_declarator.type_dereference) {
    mcs_append_syntax_node_to_mc_str(ts->str, declarator->variable_declarator.type_dereference);
  }
  append_to_mc_str(ts->str, " ");

  mcs_append_syntax_node_to_mc_str(ts->str, declarator->variable_declarator.variable_name);

  if (declarator->variable_declarator.initializer) {
    if (declarator->variable_declarator.initializer->type == MC_SYNTAX_VARIABLE_ASSIGNMENT_INITIALIZER) {
      append_to_mc_str(ts->str, " = ");
      mct_transcribe_expression(
          ts, st_info, declarator->variable_declarator.initializer->variable_assignment_initializer.value_expression);
    }
    else if (declarator->variable_declarator.initializer->type == MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER) {
      mc_syntax_node *array_initialization = declarator->variable_declarator.initializer;

      append_to_mc_str(ts->str, "[");
      if (array_initialization->local_variable_array_initializer.size_expression) {
        mct_transcribe_expression(ts, NULL, array_initialization->local_variable_array_initializer.size_expression);
      }
      append_to_mc_str(ts->str, "]");

      if (declarator->variable_declarator.initializer->local_variable_array_initializer.assignment_expression) {
        mc_syntax_node_list *array_values =
            array_initialization->local_variable_array_initializer.assignment_expression->initializer_expression.list;

        append_to_mc_str(ts->str, " = {");

        if (array_values->count) {
          append_to_mc_str(ts->str, "\n");
          ++ts->indent;

          for (int a = 0; a < array_values->count; ++a) {
            if (a > 0) {
              append_to_mc_str(ts->str, ",\n");
            }

            mct_transcribe_indent(ts);
            mct_transcribe_expression(ts, NULL, array_values->items[a]);
          }

          append_to_mc_str(ts->str, "\n");
          --ts->indent;
          mct_transcribe_indent(ts);
        }
        append_to_mc_str(ts->str, "}");
      }
    }
    else {
      MCerror(1802, "Unsupported:%s", get_mc_syntax_token_type_name(declarator->variable_declarator.initializer->type));
    }
  }

  return 0;
}

int mct_transcribe_goto_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{

  mct_transcribe_indent(ts);
  append_to_mc_str(ts->str, "goto ");
  mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->goto_statement.label);
  append_to_mc_str(ts->str, ";");

  return 0;
}

int mct_transcribe_label_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_transcribe_indent(ts);
  mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->label_statement.label);
  append_to_mc_str(ts->str, ":");

  return 0;
}

int mct_transcribe_global_var_declaration_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  print_syntax_node(syntax_node, 0);

  MCerror(1562, "TODO");

  // mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
  // append_to_mc_str(ts->str, " ");

  // mc_syntax_node *declarator;
  // for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
  //   if (i > 0) {
  //     append_to_mc_str(ts->str, ", ");
  //   }

  //   declarator = declaration->local_variable_declaration.declarators->items[i];
  //   if (declarator->local_variable_declarator.function_pointer) {
  //     mct_transcribe_function_pointer_declarator(ts, declarator->local_variable_declarator.function_pointer);
  //     // // print_syntax_node(declarator, 0);
  //     // if (declarator->local_variable_declarator.type_dereference) {
  //     //   mcs_append_syntax_node_to_mc_str(ts->str, declarator->local_variable_declarator.type_dereference);
  //     // }

  //     // mcs_append_syntax_node_to_mc_str(ts->str, declarator->local_variable_declarator.function_pointer);
  //   }
  //   else {
  //     if (declarator->local_variable_declarator.type_dereference) {
  //       mcs_append_syntax_node_to_mc_str(ts->str, declarator->local_variable_declarator.type_dereference);
  //     }
  //     mcs_append_syntax_node_to_mc_str(ts->str, declarator->local_variable_declarator.variable_name);
  //   }

  //   // Add to local scope
  //   mct_add_scope_variable(ts, declarator);

  //   // Initializer Query
  //   if (!declarator->local_variable_declarator.initializer) {
  //     continue;
  //   }

  //   if (declarator->local_variable_declarator.initializer->type == MC_SYNTAX_VARIABLE_ASSIGNMENT_INITIALIZER) {
  //     // Any invocation, do it later
  //     // if
  //     (declarator->local_variable_declarator.initializer->variable_assignment_initializer.value_expression
  //     //         ->type == MC_SYNTAX_INVOCATION)
  //     //   continue;
  //     // mct_syntax_descendants_contain_node_type(declarator->local_variable_declarator.initializer,
  //     // MC_SYNTAX_INVOCATION,
  //     //                                          &contains_invocation);
  //     // if (contains_invocation)
  //     //   continue;

  //     append_to_mc_str(ts->str, " = ");
  //     mct_transcribe_expression(
  //         ts, NULL,
  //         declarator->local_variable_declarator.initializer->variable_assignment_initializer.value_expression);
  //   }
  //   else {
  //     mc_syntax_node *array_initialization = declarator->local_variable_declarator.initializer;

  //     append_to_mc_str(ts->str, "[");
  //     if (array_initialization->local_variable_array_initializer.size_expression) {
  //       mct_transcribe_expression(ts, NULL, array_initialization->local_variable_array_initializer.size_expression);
  //     }
  //     append_to_mc_str(ts->str, "]");

  //     if (declarator->local_variable_declarator.initializer->local_variable_array_initializer.assignment_expression)
  //     {
  //       mc_syntax_node_list *array_values =
  //           array_initialization->local_variable_array_initializer.assignment_expression->initializer_expression.list;

  //       append_to_mc_str(ts->str, " = {");

  //       if (array_values->count) {
  //         append_to_mc_str(ts->str, "\n");
  //         ++ts->indent;

  //         for (int a = 0; a < array_values->count; ++a) {
  //           if (a > 0) {
  //             append_to_mc_str(ts->str, ",\n");
  //           }

  //           mct_transcribe_indent(ts);
  //           mct_transcribe_expression(ts, NULL, array_values->items[a]);
  //         }

  //         append_to_mc_str(ts->str, "\n");
  //         --ts->indent;
  //         mct_transcribe_indent(ts);
  //       }
  //       append_to_mc_str(ts->str, "}");
  //     }
  //   }
  // }
  // append_to_mc_str(ts->str, ";\n");

  return 0;
}

int mct_transcribe_pthread_create_exception(mct_transcription_state *ts, mc_syntax_node *declaration,
                                            mc_syntax_node *lv_declarator, mc_syntax_node *lvd_initializer)
{
  // const char *parent_type_name = get_mc_syntax_token_type_name(syntax_node->parent->type);
  // printf("fptr parent type: '%s'\n", parent_type_name);

  mc_syntax_node *thr_invocation = lvd_initializer->variable_assignment_initializer.value_expression;
  if (thr_invocation->invocation.arguments->count != 4) {
    MCerror(1196, "Invalid Argument Count for pthread_create() : requires 4");
  }

  mct_transcribe_indent(ts);
  mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
  append_to_mc_str(ts->str, " ");
  mcs_append_syntax_node_to_mc_str(
      ts->str, declaration->local_variable_declaration.declarators->items[0]->variable_declarator.variable_name);
  append_to_mc_str(ts->str, ";\n");

  mct_transcribe_text_with_indent(ts, "{\n");
  ++ts->indent;

  mct_transcribe_indent(ts);
  append_to_mc_str(ts->str, "void **mcti_wrapper_state = (void **)malloc(sizeof(void *) * 2);\n");
  mct_transcribe_indent(ts);
  append_to_mc_str(ts->str, "mcti_wrapper_state[0] = (void *)");
  mcs_append_syntax_node_to_mc_str(ts->str, thr_invocation->invocation.arguments->items[2]);
  append_to_mc_str(ts->str, ";\n");
  mct_transcribe_indent(ts);
  append_to_mc_str(ts->str, "mcti_wrapper_state[1] = (void *)");
  mcs_append_syntax_node_to_mc_str(ts->str, thr_invocation->invocation.arguments->items[3]);
  append_to_mc_str(ts->str, ";\n");

  // if (ts->options->report_function_entry_exit_to_stack) {
  //   mct_transcribe_text_with_indent(ts, "{\n");
  //   ++ts->indent;
  //   mct_transcribe_text_with_indent(ts, "int midge_error_stack_index;\n");

  //   mct_transcribe_indent(ts);
  //   append_to_mc_strf(ts->str, "register_midge_stack_invocation(\"%s\", __FILE__, %i,
  //                                 & midge_error_stack_index);\n",
  //                    "pthread_create", syntax_node->begin.line + 1);
  // }

  mct_transcribe_indent(ts);
  mcs_append_syntax_node_to_mc_str(ts->str, lv_declarator->variable_declarator.variable_name);
  append_to_mc_str(ts->str, " = ");
  append_to_mc_str(ts->str, "pthread_create(");

  mcs_append_syntax_node_to_mc_str(ts->str, thr_invocation->invocation.arguments->items[0]);
  append_to_mc_str(ts->str, ", ");
  mcs_append_syntax_node_to_mc_str(ts->str, thr_invocation->invocation.arguments->items[1]);
  append_to_mc_str(ts->str, ", _mca_thread_entry_wrap, (void *)mcti_wrapper_state);\n");

  // if (ts->options->report_function_entry_exit_to_stack) {
  //   mct_transcribe_indent(ts);
  //   append_to_mc_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");

  //   --ts->indent;
  //   mct_transcribe_text_with_indent(ts, "}\n");
  // }

  --ts->indent;
  mct_transcribe_text_with_indent(ts, "}\n");

  return 0;
}

int mct_transcribe_declaration_statement(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                                         mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_declaration_statement()");

  mc_syntax_node *declaration = syntax_node->declaration_statement.declaration;
  mc_syntax_node *declarator;

  // TODO -- this is a weak wrapper atm
  // pthread_create exception
  if (declaration->local_variable_declaration.declarators->count == 1) {
    declarator = declaration->local_variable_declaration.declarators->items[0];
    if (declarator->type == MC_SYNTAX_VARIABLE_DECLARATOR && declarator->variable_declarator.initializer) {
      mc_syntax_node *lvd_initializer = declarator->variable_declarator.initializer;

      if (lvd_initializer->type == MC_SYNTAX_VARIABLE_ASSIGNMENT_INITIALIZER &&
          lvd_initializer->variable_assignment_initializer.value_expression->type == MC_SYNTAX_INVOCATION &&
          (mc_token_type)lvd_initializer->variable_assignment_initializer.value_expression->invocation
                  .function_identity->type == MC_TOKEN_IDENTIFIER &&
          !strcmp(lvd_initializer->variable_assignment_initializer.value_expression->invocation.function_identity->text,
                  "pthread_create")) {
        MCcall(mct_transcribe_pthread_create_exception(ts, declaration, declarator, lvd_initializer));
        return 0;
      }
    }
  }

  // Temporary -- TODO -- until invokes are handled more eloquently

  // Do MC_invokes
  // if (contains_mc_function_call) {
  // const char *tyin = get_mc_syntax_token_type_name(declaration->local_variable_declaration.type_identifier->type);
  // register_midge_error_tag("lvd->t:%s", tyin);
  mct_transcribe_indent(ts);
  mct_transcribe_type_identifier(ts, declaration->local_variable_declaration.type_identifier);
  append_to_mc_str(ts->str, " ");

  for (int i = 0; i < declaration->local_variable_declaration.declarators->count; ++i) {
    declarator = declaration->local_variable_declaration.declarators->items[i];

    // Seperating comma
    if (i > 0) {
      append_to_mc_str(ts->str, ", ");
    }

    if (declarator->type == MC_SYNTAX_VARIABLE_DECLARATOR) {
      mct_transcribe_variable_declarator(ts, st_info, declarator);
    }
    else if (declarator->type == MC_SYNTAX_FUNCTION_POINTER_DECLARATOR) {
      mct_transcribe_function_pointer_declarator(ts, st_info, declarator);
    }
    else {
      // TODO -- any more and turn it into a switch statement
      MCerror(1775, "Unsupported declarator:%s", get_mc_syntax_token_type_name(declarator->type));
    }

    // Add to local scope
    mct_add_scope_variable(ts, declarator);
  }
  append_to_mc_str(ts->str, ";");

  return 0;
}

// int mct_transcribe_mcerror(mct_transcription_state *ts, mc_syntax_node *syntax_node)
// {
//   mct_transcribe_text_with_indent(ts, "{\n");
//   ++ts->indent;
//   // mct_transcribe_indent(ts);
//   // append_to_mc_strf(ts->str, "*mc_return_value = %s;\n", syntax_node->invocation.arguments->items[0]->text);
//   mct_transcribe_indent(ts);

//   bool has_call_to_get_mc_syntax_token_type_name = false;
//   int ttp = 0;
//   for (int p = 1; p < syntax_node->invocation.arguments->count; ++p) {
//     mc_syntax_node *mce_argument = syntax_node->invocation.arguments->items[p];
//     if (mce_argument->type == MC_SYNTAX_INVOCATION &&
//         (mc_token_type)mce_argument->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
//         (!strcmp(mce_argument->invocation.function_identity->text, "get_mc_syntax_token_type_name") ||
//          !strcmp(mce_argument->invocation.function_identity->text, "get_mc_token_type_name"))) {
//       // // Get the return type
//       // function_info *func_info;
//       // find_function_info("get_mc_syntax_token_type_name", &func_info);
//       // if (!func_info) {
//       //   MCerror(709, "Not Expected");
//       // }
//       // printf("%s\n", func_info->return_type.name);
//       // enumeration_info *enum_info;
//       // find_enumeration_info(func_info->return_type.name, &enum_info);
//       // if (!enum_info) {
//       //   MCerror(715, "Not Expected");
//       // }

//       mc_str *temp_name;
//       init_mc_str(&temp_name);
//       append_to_mc_strf(temp_name, "mcs_tt_name_%i", ttp++);

//       mct_transcribe_text_with_indent(ts, "const char *");
//       append_to_mc_str(ts->str, temp_name->text);
//       append_to_mc_str(ts->str, ";\n");

//       mct_transcribe_mc_invocation(ts, mce_argument, temp_name->text);
//       release_mc_str(temp_name, true);
//     }
//   }

//   mct_transcribe_indent(ts);
//   append_to_mc_str(ts->str, "MCerror(");
//   mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->invocation.arguments->items[0]);
//   ttp = 0;
//   for (int p = 1; p < syntax_node->invocation.arguments->count; ++p) {
//     append_to_mc_str(ts->str, ", ");
//     mc_syntax_node *mce_argument = syntax_node->invocation.arguments->items[p];
//     if (mce_argument->type == MC_SYNTAX_INVOCATION &&
//         (mc_token_type)mce_argument->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
//         (!strcmp(mce_argument->invocation.function_identity->text, "get_mc_syntax_token_type_name") ||
//          !strcmp(mce_argument->invocation.function_identity->text, "get_mc_token_type_name"))) {

//       char temp_name[32];
//       sprintf(temp_name, "mcs_tt_name_%i", ttp++);
//       append_to_mc_str(ts->str, temp_name);
//     }
//     else {
//       mcs_append_syntax_node_to_mc_str(ts->str, mce_argument);
//     }
//   }

//   append_to_mc_str(ts->str, ");\n");
//   --ts->indent;
//   mct_transcribe_text_with_indent(ts, "}\n");

//   // if (ttp)
//   //   printf("def:\n%s||\n", ts->str->text);

//   return 0;
// }

int mct_transcribe_invocation(mct_transcription_state *ts, mc_syntax_node *invocation)
{
  mcs_append_syntax_node_to_mc_str(ts->str, invocation->invocation.function_identity);
  append_to_mc_str(ts->str, "(");
  for (int a = 0; a < invocation->invocation.arguments->count; ++a) {
    if (a > 0) {
      append_to_mc_str(ts->str, ", ");
    }

    mct_transcribe_expression(ts, NULL, invocation->invocation.arguments->items[a]);
  }
  append_to_mc_str(ts->str, ")");

  return 0;
}

int mct_transcribe_fptr_invocation(mct_transcription_state *ts, mc_syntax_node *fptr,
                                   mct_expression_type_info *fptr_type_info)
{
  print_syntax_node(fptr, 0);
  MCerror(1980, "TODO");

  // mct_transcribe_text_with_indent(ts, "{\n");
  // ++ts->indent;

  // mct_transcribe_text_with_indent(ts, "function_info_mc_v1 *ptr_func_info;\n");
  // mct_transcribe_text_with_indent(ts, "{\n");
  // ++ts->indent;

  // // Determine if the function pointer is an mc-function
  // mct_transcribe_text_with_indent(ts, "void *mc_vargs[3];\n");
  // mct_transcribe_text_with_indent(ts, "mc_vargs[0] = &");
  // mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->invocation.function_identity);
  // append_to_mc_str(ts->str, ";\n");
  // mct_transcribe_text_with_indent(ts, "void *mc_vargs_1 = &ptr_func_info;\n");
  // mct_transcribe_text_with_indent(ts, "mc_vargs[1] = &mc_vargs_1;\n");
  // mct_transcribe_text_with_indent(ts, "int mc_dummy_return_value = 0;\n");
  // mct_transcribe_text_with_indent(ts, "mc_vargs[2] = &mc_dummy_return_value;\n");
  // // mct_transcribe_text_with_indent(ts, "printf(\"entering find_function_info_by_ptr!\\n\");\n");
  // mct_transcribe_text_with_indent(ts, "find_function_info_by_ptr(3, mc_vargs);\n");
  // // mct_transcribe_text_with_indent(ts, "printf(\"returned find_function_info_by_ptr!\\n\");\n");
  // --ts->indent;
  // mct_transcribe_text_with_indent(ts, "}\n");

  // // MC-Function
  // mct_transcribe_text_with_indent(ts, "if (ptr_func_info) {\n");
  // ++ts->indent;

  // // // DEBUG - TIME
  // //
  // //
  // // // DEBUG - TIME
  // // DEBUG

  // mct_transcribe_indent(ts); // TODO -- count arguments based on function pointer type?
  // const char *ARGUMENT_DATA_NAME = "mc_fp_vargs";
  // append_to_mc_strf(ts->str, "void *%s[%i];\n", ARGUMENT_DATA_NAME, syntax_node->invocation.arguments->count + 1);

  // for (int a = 0; a < syntax_node->invocation.arguments->count; ++a) {
  //   mct_transcribe_mc_invocation_argument(ts, NULL, syntax_node->invocation.arguments->items[a],
  //   ARGUMENT_DATA_NAME, a);
  // }

  // if (strcmp(fptr_type_info->type_name, "void") || fptr_type_info->deref_count) {
  //   if (syntax_node->parent->type == MC_SYNTAX_EXPRESSION_STATEMENT) {
  //     mct_transcribe_indent(ts);
  //     mct_transcribe_text_with_indent(ts, fptr_type_info->type_name);
  //     append_to_mc_str(ts->str, " ");
  //     for (int i = 0; i < fptr_type_info->deref_count; ++i) {
  //       append_to_mc_str(ts->str, "*");
  //     }

  //     append_to_mc_strf(ts->str, "%s_dummy_rv", ARGUMENT_DATA_NAME);
  //     if (fptr_type_info->deref_count) {
  //       append_to_mc_str(ts->str, " = NULL;\n");
  //     }
  //     else {
  //       append_to_mc_str(ts->str, ";\n");
  //     }

  //     mct_transcribe_indent(ts);
  //     append_to_mc_strf(ts->str, "%s[%i] = &%s_dummy_rv;\n", ARGUMENT_DATA_NAME,
  //                      syntax_node->invocation.arguments->count, ARGUMENT_DATA_NAME);
  //   }
  //   else {
  //     // print_syntax_node(syntax_node->parent, 0);
  //     MCerror(1461, "progress");
  //   }
  // }
  // else {
  //   mct_transcribe_indent(ts);
  //   append_to_mc_strf(ts->str, "%s[%i] = NULL;\n", ARGUMENT_DATA_NAME, syntax_node->invocation.arguments->count);
  // }

  // // mct_transcribe_text_with_indent(ts, "printf(\"was ptr_func_info!\\n\");\n");
  // // int mc_argument_data_count;
  // // mct_transcribe_mc_invocation_argument_data(ts, syntax_node, func_info, return_variable_name,
  // // &mc_argument_data_count); mct_transcribe_text_with_indent(ts, "void *vvvmcv[1];\n");
  // // mct_transcribe_text_with_indent(ts, "int vvv0 = 4;\n"); mct_transcribe_text_with_indent(ts, "vvvmcv[0] =
  // // &vvv0;\n"); mct_transcribe_indent(ts);

  // // mct_transcribe_text_with_indent(ts, "sprintf(buf, \"printf(\\\"vvv0:%%i\\\", *(int *)((void **)%p)[0]);\",
  // // &vvvmcv);\n");
  // // // mct_transcribe_text_with_indent(ts, "printf(\"buf:\\n%s||\\n\", buf);\n");
  // // mct_transcribe_text_with_indent(ts, "clint_process(buf);");
  // // mct_transcribe_text_with_indent(ts, "\n\n");
  // if (ts->options->report_function_entry_exit_to_stack) {
  //   mct_transcribe_text_with_indent(ts, "{\n");
  //   ++ts->indent;
  //   mct_transcribe_text_with_indent(ts, "int midge_error_stack_index;\n");

  //   mct_transcribe_indent(ts);
  //   append_to_mc_strf(ts->str, "register_midge_stack_invocation(ptr_func_info->name, __FILE__, %i,
  //                                 & midge_error_stack_index);
  //   \n ", syntax_node->begin.line + 1);
  // }

  // mct_transcribe_text_with_indent(ts, "char buf[512];\n");
  // mct_transcribe_text_with_indent(ts, "int mc_fptr_result;\n");
  // mct_transcribe_text_with_indent(
  //     ts, "sprintf(buf, \"*((int *)%p) = (*(int (**)(int, void **))%p)(%i, (void **)%p);\", &mc_fptr_result, ");
  // mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->invocation.function_identity);
  // append_to_mc_strf(ts->str, ", %i, &%s);\n", syntax_node->invocation.arguments->count + 1, ARGUMENT_DATA_NAME);

  // // DEBUG
  // mct_transcribe_text_with_indent(ts, "// DEBUG - TIME\n");
  // mct_transcribe_text_with_indent(ts, "struct timespec debug_fptr_start_time, debug_fptr_end_time;\n");
  // mct_transcribe_text_with_indent(ts, "clock_gettime(CLOCK_REALTIME, &debug_fptr_start_time);\n");
  // mct_transcribe_text_with_indent(ts, "// DEBUG - TIME\n");
  // // DEBUG

  // // mct_transcribe_text_with_indent(ts, "printf(\"buf:\\n%s||\\n\", buf);\n");
  // mct_transcribe_text_with_indent(ts, "clint_process(buf);\n");

  // // DEBUG
  // mct_transcribe_text_with_indent(ts, "\n// DEBUG - TIME\n");
  // mct_transcribe_text_with_indent(ts, "clock_gettime(CLOCK_REALTIME, &debug_fptr_end_time);\n");
  // mct_transcribe_text_with_indent(ts, "if (!strcmp(\"_mco_render_mo_data_present\", ptr_func_info->name)) {\n");
  // mct_transcribe_text_with_indent(ts, "  printf(\"\\nfptr-invocation took %.2fms\\n\",\n");
  // mct_transcribe_text_with_indent(
  //     ts, "              1000.f * (debug_fptr_end_time.tv_sec - debug_fptr_start_time.tv_sec) +\n");
  // mct_transcribe_text_with_indent(
  //     ts, "              1e-6 * (debug_fptr_end_time.tv_nsec - debug_fptr_start_time.tv_nsec));\n");
  // mct_transcribe_text_with_indent(ts, "}\n\n");
  // mct_transcribe_text_with_indent(ts, "// DEBUG - TIME\n\n");
  // // DEBUG

  // mct_transcribe_text_with_indent(ts, "if (mc_fptr_result) {\n");
  // ++ts->indent;
  // mct_transcribe_text_with_indent(ts, "printf(\"--");
  // mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->invocation.function_identity);
  // mct_transcribe_text_with_indent(ts, "->%s line:%i ERR:%i\\n\", ptr_func_info->name, __LINE__,
  // mc_fptr_result);\n");
  // --ts->indent;
  // mct_transcribe_text_with_indent(ts, "}");

  // if (ts->options->report_function_entry_exit_to_stack) {
  //   mct_transcribe_indent(ts);
  //   append_to_mc_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");

  //   --ts->indent;
  //   mct_transcribe_text_with_indent(ts, "}\n");
  // }

  // // External non-MC function
  // --ts->indent;
  // mct_transcribe_text_with_indent(ts, "} else {\n");
  // ++ts->indent;

  // mct_transcribe_indent(ts);
  // // mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
  // mct_transcribe_non_mc_invocation(ts, syntax_node);
  // append_to_mc_str(ts->str, ";\n");
  // // mct_transcribe_text_with_indent(ts, "return 1559;\n");

  // --ts->indent;
  // mct_transcribe_text_with_indent(ts, "}\n");
  // --ts->indent;
  // mct_transcribe_text_with_indent(ts, "}");

  return 0;
}

int mct_transcribe_variable_value_report(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                                         mc_syntax_node *variable)
{
  mct_expression_type_info type_info;
  determine_type_of_expression(ts, variable, &type_info);

  insert_into_mc_str(ts->str, "{\n", st_info->prefix_end_index);
  st_info->prefix_end_index += 2;

  char *variable_name;
  mcs_copy_syntax_node_to_text(variable, &variable_name);

  // The arguments for the fptr report invocation
  char buf[1536];
  sprintf(buf,
          "  int (*mc_fp_report_variable_value)(int, void *) = *(int (**)(int, void *))%p;\n"
          "  void *mc_fld_vargs[8];\n"
          "  void *mc_fld_report_index = (void *)%p;\n"
          "  mc_fld_vargs[0] = (void *)&mc_fld_report_index;\n"
          "  mc_fld_vargs[1] = (void *)&mc_fld_function_call_uid;\n"
          // TODO -- the use of info_mc_v1 eeegh
          "  mct_expression_type_info_mc_v1 variable_type;\n"
          "  variable_type.type_name = (char *)\"%s\";\n"
          "  variable_type.deref_count = %u;\n"
          "  variable_type.is_array = (bool)%u;\n"
          "  variable_type.is_fptr = (bool)%u;\n"
          "  void *mc_fld_type_identifier = (void *)&variable_type;\n"
          "  mc_fld_vargs[2] = (void *)&mc_fld_type_identifier;\n"
          "  const char *mc_fld_variable_name = \"%s\";\n"
          "  mc_fld_vargs[3] = (void *)&mc_fld_variable_name;\n"
          "  int mc_fld_line = %i;\n"
          "  mc_fld_vargs[4] = (void *)&mc_fld_line;\n"
          "  int mc_fld_col = %i;\n"
          "  mc_fld_vargs[5] = (void *)&mc_fld_col;\n"
          "  void *mc_fld_p_value = (void *)&(%s);\n"
          "  mc_fld_vargs[6] = (void *)&mc_fld_p_value;\n"
          "  int mc_fld_dummy_ret;\n"
          "  mc_fld_vargs[7] = (void *)&mc_fld_dummy_ret;\n",
          ts->options->report_variable_values->report_variable_value_delegate, ts->options->report_variable_values,
          type_info.type_name, type_info.deref_count, type_info.is_array,
          type_info.is_fptr, // TODO is_ary, fptr, etc...
          variable_name, variable->begin.line, variable->begin.col, variable_name);
  insert_into_mc_str(ts->str, buf, st_info->prefix_end_index);
  st_info->prefix_end_index += strlen(buf);

  // The invocation (& stack reporting*)
  buf[0] = '\0';
  // if (ts->options->report_function_entry_exit_to_stack) {
  //   strcat(buf, "\n  int midge_error_stack_index;\n");
  //   strcat(buf, "  register_midge_stack_invocation(\"(*mc_fp_report_variable_value)\", __FILE__, -1, "
  //               "&midge_error_stack_index);\n");
  // }

  strcat(buf, "  int mc_fld_res = mc_fp_report_variable_value(8, mc_fld_vargs);\n"
              // "  printf(\"mc_fld_res=%%i\\n\", mc_fld_res);\n"
              "  if(mc_fld_res) {\n"
              "    printf(\"ERR[%i]: report_variable_value_delegate\\n\", mc_fld_res);\n"
              "    return mc_fld_res;\n"
              "  }\n");

  // if (ts->options->report_function_entry_exit_to_stack) {
  //   strcat(buf, "\n  register_midge_stack_return(midge_error_stack_index);\n\n");
  // }
  insert_into_mc_str(ts->str, buf, st_info->prefix_end_index);
  st_info->prefix_end_index += strlen(buf);

  // Wrap it up
  insert_into_mc_str(ts->str, "}\n", st_info->prefix_end_index);
  st_info->prefix_end_index += 2;

  free(variable_name);
  mct_release_expression_type_info_fields(&type_info);

  return 0;
}

int mct_transcribe_expression(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                              mc_syntax_node *syntax_node)
{
  const char *syntax_node_type_name = get_mc_syntax_token_type_name(syntax_node->type);
  register_midge_error_tag("mct_transcribe_expression(%s)", syntax_node_type_name);
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

    append_to_mc_str(ts->str, " ");

    for (int a = 0; a < syntax_node->local_variable_declaration.declarators->count; ++a) {
      if (a > 0) {
        append_to_mc_str(ts->str, ", ");
      }

      mc_syntax_node *declarator = syntax_node->local_variable_declaration.declarators->items[a];

      switch (declarator->type) {
      case MC_SYNTAX_FUNCTION_POINTER_DECLARATOR:
        mct_transcribe_function_pointer_declarator(ts, st_info, declarator);
        break;
      case MC_SYNTAX_VARIABLE_DECLARATOR:
        mct_transcribe_variable_declarator(ts, st_info, declarator);
        break;
      default:
        MCerror(2337, "Unsupported:'%s'", get_mc_syntax_token_type_name(declarator->type));
      }
    }
  } break;
  case MC_SYNTAX_ASSIGNMENT_EXPRESSION: {
    mct_transcribe_expression(ts, st_info, syntax_node->assignment_expression.variable);
    append_to_mc_str(ts->str, " ");
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->assignment_expression.assignment_operator);
    append_to_mc_str(ts->str, " ");
    mct_transcribe_expression(ts, st_info, syntax_node->assignment_expression.value_expression);
  } break;
  case MC_SYNTAX_PARENTHESIZED_EXPRESSION: {
    append_to_mc_str(ts->str, "(");
    mct_transcribe_expression(ts, st_info, syntax_node->parenthesized_expression.expression);
    append_to_mc_str(ts->str, ")");
  } break;
  case MC_SYNTAX_TYPE_INITIALIZER: {
    append_to_mc_str(ts->str, "{");
    // mct_transcribe_expression(ts, NULL,syntax_node->initializer_expression);
    if (syntax_node->children->count > 2) {
      MCerror(1573, "TODO :%s", get_mc_syntax_token_type_name(syntax_node->children->items[1]->type));
    }
    append_to_mc_str(ts->str, "}");
  } break;
  case MC_SYNTAX_INITIALIZER_EXPRESSION: {
    append_to_mc_str(ts->str, "{");

    if (syntax_node->initializer_expression.list->count) {
      for (int a = 0; a < syntax_node->initializer_expression.list->count; ++a) {
        if (a > 0) {
          append_to_mc_str(ts->str, ",");
        }
        append_to_mc_str(ts->str, " ");

        mct_transcribe_expression(ts, st_info, syntax_node->initializer_expression.list->items[a]);
      }
      append_to_mc_str(ts->str, " ");
    }

    append_to_mc_str(ts->str, "}");
  } break;
  case MC_SYNTAX_OPERATIONAL_EXPRESSION: {
    if (!syntax_node->operational_expression.right) {
      MCerror(745, "TODO");
    }

    mct_transcribe_expression(ts, st_info, syntax_node->operational_expression.left);
    append_to_mc_str(ts->str, " ");
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->operational_expression.operational_operator);
    append_to_mc_str(ts->str, " ");
    mct_transcribe_expression(ts, st_info, syntax_node->operational_expression.right);
    // printf("%s\n", ts->str->text);
  } break;
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {

    if (ts->options->report_variable_values && st_info && !st_info->variable_reported) {
      st_info->variable_reported = true;
      mct_transcribe_variable_value_report(ts, st_info, syntax_node);
    }

    // printf("indenta:%i %s  last10:'%s'\n", ts->indent,
    //        get_mc_syntax_token_type_name(syntax_node->member_access_expression.primary->type),
    //        ts->str->text + (ts->str->len - 10));
    mct_transcribe_expression(ts, st_info, syntax_node->member_access_expression.primary);
    // printf("indentb:%i\n", ts->indent);
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->member_access_expression.access_operator);
    mct_transcribe_expression(ts, st_info, syntax_node->member_access_expression.identifier);
    // printf("%s %s\n", st_info ? (st_info->variable_reported ? "reported" : "reset") : "(null)",
    //        ts->str->text + ts->str->len - 20);

    if (ts->options->report_variable_values && st_info) {
      st_info->variable_reported = false;
    }
  } break;
  case MC_SYNTAX_DEREFERENCE_EXPRESSION: {
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->dereference_expression.deref_sequence);
    mct_transcribe_expression(ts, st_info, syntax_node->dereference_expression.unary_expression);
    // printf("%s\n", ts->str->text);
  } break;
  case MC_SYNTAX_LOGICAL_EXPRESSION: {
    if (!syntax_node->logical_expression.right) {
      MCerror(764, "TODO");
    }

    mct_transcribe_expression(ts, st_info, syntax_node->logical_expression.left);
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->logical_expression.logical_operator);
    mct_transcribe_expression(ts, st_info, syntax_node->logical_expression.right);
  } break;
  case MC_SYNTAX_BITWISE_EXPRESSION: {
    if (!syntax_node->logical_expression.right) {
      MCerror(764, "TODO");
    }

    mct_transcribe_expression(ts, st_info, syntax_node->bitwise_expression.left);
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->bitwise_expression.bitwise_operator);
    mct_transcribe_expression(ts, st_info, syntax_node->bitwise_expression.right);
  } break;
  case MC_SYNTAX_RELATIONAL_EXPRESSION: {
    if (!syntax_node->relational_expression.right) {
      MCerror(777, "TODO");
    }

    mct_transcribe_expression(ts, st_info, syntax_node->relational_expression.left);
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->relational_expression.relational_operator);
    mct_transcribe_expression(ts, st_info, syntax_node->relational_expression.right);
  } break;
  case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION: {
    mct_transcribe_expression(ts, st_info, syntax_node->element_access_expression.primary);
    append_to_mc_str(ts->str, "[");
    mct_transcribe_expression(ts, st_info, syntax_node->element_access_expression.access_expression);
    append_to_mc_str(ts->str, "]");
  } break;

    // WILL have to redo in future
  // case MC_SYNTAX_DEREFERENCE_EXPRESSION:
  // case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION:
  // case MC_SYNTAX_LOGICAL_EXPRESSION:
  // case MC_SYNTAX_OPERATIONAL_EXPRESSION:
  // case MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION:
  // case MC_SYNTAX_RELATIONAL_EXPRESSION: {
  //   mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
  // } break;
  case MC_SYNTAX_CAST_EXPRESSION: {
    append_to_mc_str(ts->str, "(");

    // if (!strcmp(ts->transcription_root->function.name->text, "print_syntax_node")) {
    // printf("LLL>>>>\n");
    // print_syntax_node(syntax_node, 0);
    // }
    if (syntax_node->cast_expression.type_identifier->type == MC_SYNTAX_FUNCTION_POINTER_DECLARATION) {
      append_to_mc_str(ts->str, "/*!*/");
      mct_transcribe_function_pointer_declaration(ts, syntax_node->cast_expression.type_identifier);
      // mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->cast_expression.type_identifier);
    }
    else {
      mct_transcribe_type_identifier(ts, syntax_node->cast_expression.type_identifier);
    }

    if (syntax_node->cast_expression.type_dereference) {
      append_to_mc_str(ts->str, " ");
      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->cast_expression.type_dereference);
    }
    append_to_mc_str(ts->str, ")");

    mct_transcribe_expression(ts, st_info, syntax_node->cast_expression.expression);
  } break;
  case MC_SYNTAX_VA_ARG_EXPRESSION: {
    register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-0");
    // print_syntax_node(syntax_node, 0);
    // MCerror(2366, "PROGRESS");

    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);

    // append_to_mc_str(ts->str, "va_arg(");

    // printf("$$ %p\n", syntax_node);
    // printf("$$ %p %p\n", syntax_node->va_arg_expression.list_identity,
    // syntax_node->va_arg_expression.type_identifier);
    // mct_transcribe_expression(ts,syntax_node->va_arg_expression.list_identity);
    // register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-1");
    // append_to_mc_str(ts->str, ", ");
    // mct_transcribe_type_identifier(ts, syntax_node->va_arg_expression.type_identifier);
    // register_midge_error_tag("mct_transcribe_expression:VA_ARG_EXPRESSION-2");
    // append_to_mc_str(ts->str, ")\n");
  } break;
  case MC_SYNTAX_TERNARY_CONDITIONAL: {
    mct_transcribe_expression(ts, st_info, syntax_node->ternary_conditional.condition);
    append_to_mc_str(ts->str, " ? ");
    mct_transcribe_expression(ts, st_info, syntax_node->ternary_conditional.true_expression);
    append_to_mc_str(ts->str, " : ");
    mct_transcribe_expression(ts, st_info, syntax_node->ternary_conditional.false_expression);
  } break;
  case MC_SYNTAX_SIZEOF_EXPRESSION: {
    append_to_mc_str(ts->str, "sizeof(");

    mct_transcribe_type_identifier(ts, syntax_node->cast_expression.type_identifier);

    if (syntax_node->sizeof_expression.type_dereference) {
      append_to_mc_str(ts->str, " ");
      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->sizeof_expression.type_dereference);
    }
    append_to_mc_str(ts->str, ")");
  } break;
  case MC_SYNTAX_OFFSETOF_EXPRESSION: {
    append_to_mc_str(ts->str, "offsetof(");

    mct_transcribe_type_identifier(ts, syntax_node->cast_expression.type_identifier);

    if (syntax_node->offsetof_expression.type_dereference) {
      append_to_mc_str(ts->str, " ");
      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->offsetof_expression.type_dereference);
    }

    append_to_mc_str(ts->str, ", ");

    append_to_mc_str(ts->str, syntax_node->offsetof_expression.field_identity->text);

    append_to_mc_str(ts->str, ")");
  } break;
  case MC_SYNTAX_INVOCATION: {

    // Thread Creation
    function_info *func_info;
    char *function_identity;
    mcs_copy_syntax_node_to_text(syntax_node->invocation.function_identity, &function_identity);
    if (!strcmp(function_identity, "pthread_create")) {
      MCerror(1484,
              "Not supposed to reach here, use the int result = midge_redir_pthread_create(...); statement please");
    }

    // MCerror
    // if ((mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER &&
    //     !strcmp(function_identity, "MCerror")) {
    //   print_syntax_node(syntax_node->parent, 0);
    //   MCerror(550, "TODO");

    //   // mct_transcribe_mcerror(ts, syntax_node);
    //   break;
    // }
    // free(function_identity);

    // Function Pointer Invocation
    // if ((mc_token_type)syntax_node->invocation.function_identity->type == MC_TOKEN_IDENTIFIER) {
    //   mct_expression_type_info eti;
    //   determine_type_of_expression(ts, syntax_node->invocation.function_identity, &eti);
    //   if (eti.type_name && eti.is_fptr) {
    //     if (syntax_node->parent->type != MC_SYNTAX_EXPRESSION_STATEMENT) {
    //       const char *parent_type_name = get_mc_syntax_token_type_name(syntax_node->parent->type);
    //       printf("fptr parent type: '%s'\n", parent_type_name);
    //       MCerror(1510, "CHECK");
    //     }

    //     mct_transcribe_fptr_invocation(ts, syntax_node, &eti);
    //     break;
    //     // MCerror(1501, "progress");
    //   }
    //   mct_release_expression_type_info_fields(&eti);
    // }

    // Simple Invocation
    mct_transcribe_invocation(ts, syntax_node);

  } break;
  case MC_SYNTAX_PREPENDED_UNARY_EXPRESSION: {
    if (syntax_node->prepended_unary.unary_expression->type == MC_SYNTAX_TYPE_IDENTIFIER) {
      mct_expression_type_info eti;
      determine_type_of_expression(ts, syntax_node->prepended_unary.unary_expression, &eti);
      if (eti.is_fptr) {
        MCerror(1465, "progress");
      }
      mct_release_expression_type_info_fields(&eti);
    }

    // Just straight copy
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node->prepended_unary.prepend_operator);
    mct_transcribe_expression(ts, st_info, syntax_node->prepended_unary.unary_expression);
  } break;
  case MC_SYNTAX_STRING_LITERAL_EXPRESSION:
  case MC_SYNTAX_FIXREMENT_EXPRESSION: {
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
  } break;
  default:
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_NUMERIC_LITERAL:
    case MC_TOKEN_CHAR_LITERAL: {
      // mct_expression_type_info eti;
      // determine_type_of_expression(ts, syntax_node, &eti);
      // if (eti.type_name && eti.is_fptr) {

      //   print_syntax_node(syntax_node, 0);
      //   MCerror(1473, "progress");
      // }

      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
    } break;
    case MC_TOKEN_IDENTIFIER: {
      if (ts->options->report_variable_values && st_info && !st_info->variable_reported) {

        // DEBUG
        // Theres a more complete way, but for now lets ignore identifiers true/false && containing capital letters
        if (strcmp(syntax_node->text, "true") && strcmp(syntax_node->text, "false") &&
            isalpha(syntax_node->text[0]) == 2) {

          mct_transcribe_variable_value_report(ts, st_info, syntax_node);
        }
        // DEBUG
      }

      // {
      //   function_info *func_info;
      //   find_function_info(syntax_node->text, &func_info);
      //   if (func_info) {
      //     // void *vv = (void *)mct_transcribe_mc_invocation;
      //     // printf("printf(\"mc_vargs[%%i] = %p;\", %i, %s);\n", i, argument->text);
      //     // append_to_mc_strf(ts->str, "printf(\"%s[%%i] = %%p;\", %i, %s);\n", argument_data_name, arg_index,
      //     //                  syntax_node->text);
      //     // append_to_mc_strf(ts->str, "%s[%i] = (void *)%s;\n", argument_data_name, arg_index,
      // syntax_node->text);
      //     // %s_mc_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
      //     append_to_mc_str(ts->str, func_info->name);
      //     append_to_mc_strf(ts->str, "_mc_v%u", func_info->latest_iteration);
      //     break;
      //   }
      // }

      // // TODO DEBUG
      // if (!strcmp(syntax_node->text, "mcu_context_menu_option_clicked")) {
      //   MCerror(2091, "reached");
      // }

      mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
    } break;
    default:
      MCerror(291, "MCT:Unsupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
    }
  }

  register_midge_error_tag("mct_transcribe_expression(~)");
  return 0;
}

int mct_transcribe_if_statement(mct_transcription_state *ts, mct_statement_transcription_info *st_info,
                                mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_if_statement()");
  // printf("cb: %p\n", syntax_node);

  // // DEBUG
  // const char *conditional_type = get_mc_syntax_token_type_name(syntax_node->if_statement.conditional->type);
  // printf("if-conditional:%s\n", conditional_type);
  // // DEBUG

  // Initialization
  mct_transcribe_text_with_indent(ts, "if (");
  mct_transcribe_expression(ts, st_info, syntax_node->if_statement.conditional);
  append_to_mc_str(ts->str, ") ");

  if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_increment_scope_depth(ts);
  }
  mct_transcribe_statement(ts, syntax_node->if_statement.do_statement);
  if (syntax_node->if_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK) {
    mct_decrement_scope_depth(ts);
  }

  if (syntax_node->if_statement.else_continuance) {
    mct_transcribe_text_with_indent(ts, "else ");

    if (syntax_node->if_statement.else_continuance->type == MC_SYNTAX_IF_STATEMENT) {

      mct_transcribe_if_statement(ts, st_info, syntax_node->if_statement.else_continuance);
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
    //   mct_transcribe_code_block(ts, syntax_node->if_statement.else_continuance, false);
    // }
    // else {
    //   MCerror(119, "TODO: %s",
    // get_mc_syntax_token_type_name((mc_syntax_node_type)syntax_node->if_statement.else_continuance->type));
    // }
  }

  register_midge_error_tag("mct_transcribe_if_statement(~)");
  return 0;
}

int mct_transcribe_switch_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_switch_statement()");

  mct_transcribe_text_with_indent(ts, "switch (");
  mct_transcribe_expression(ts, NULL, syntax_node->switch_statement.conditional);
  append_to_mc_str(ts->str, ") {\n");
  ++ts->indent;
  mct_increment_scope_depth(ts);

  for (int i = 0; i < syntax_node->switch_statement.sections->count; ++i) {
    mc_syntax_node *switch_section = syntax_node->switch_statement.sections->items[i];

    for (int j = 0; j < switch_section->switch_section.labels->count; ++j) {
      mct_transcribe_indent(ts);

      mcs_append_syntax_node_to_mc_str(ts->str, switch_section->switch_section.labels->items[j]);
      append_to_mc_str(ts->str, "\n");
    }

    mct_transcribe_statement_list(ts, switch_section->switch_section.statement_list);
  }

  mct_decrement_scope_depth(ts);
  --ts->indent;
  mct_transcribe_text_with_indent(ts, "}\n");
  register_midge_error_tag("mct_transcribe_switch_statement(~)");
  return 0;
}

int mct_transcribe_for_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mct_transcribe_for_statement()");

  // Initialization
  mct_increment_scope_depth(ts);
  mct_transcribe_text_with_indent(ts, "for (");
  if (syntax_node->for_statement.initialization) {
    mct_transcribe_expression(ts, NULL, syntax_node->for_statement.initialization);

    if (syntax_node->for_statement.initialization->type == MC_SYNTAX_LOCAL_VARIABLE_DECLARATION) {
      for (int a = 0; a < syntax_node->for_statement.initialization->local_variable_declaration.declarators->count;
           ++a) {
        mct_add_scope_variable(
            ts, syntax_node->for_statement.initialization->local_variable_declaration.declarators->items[a]);
      }
    }
  }
  mct_transcribe_text_with_indent(ts, "; ");
  if (syntax_node->for_statement.conditional) {
    mct_transcribe_expression(ts, NULL, syntax_node->for_statement.conditional);
  }
  mct_transcribe_text_with_indent(ts, "; ");
  if (syntax_node->for_statement.fix_expression) {
    mct_transcribe_expression(ts, NULL, syntax_node->for_statement.fix_expression);
  }
  mct_transcribe_text_with_indent(ts, ") ");

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

  bool do_is_single_statement = syntax_node->while_statement.do_statement->type != MC_SYNTAX_CODE_BLOCK;
  if (syntax_node->while_statement.do_first) {
    mct_transcribe_text_with_indent(ts, "do ");
    if (do_is_single_statement) {
      ++ts->indent;
    }

    mct_transcribe_statement(ts, syntax_node->while_statement.do_statement);

    if (do_is_single_statement) {
      --ts->indent;
    }

    mct_transcribe_text_with_indent(ts, "while (");
    mct_transcribe_expression(ts, NULL, syntax_node->while_statement.conditional);
    append_to_mc_str(ts->str, ");");
  }
  else {
    // Initialization
    mct_transcribe_text_with_indent(ts, "while (");
    mct_transcribe_expression(ts, NULL, syntax_node->while_statement.conditional);
    mct_transcribe_text_with_indent(ts, ") ");
    if (do_is_single_statement) {
      ++ts->indent;
      append_to_mc_str(ts->str, "\n");
    }

    mct_transcribe_statement(ts, syntax_node->while_statement.do_statement);

    if (do_is_single_statement) {
      --ts->indent;
    }
  }

  register_midge_error_tag("mct_transcribe_while_statement(~)");
  return 0;
}

int mct_transcribe_statement(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_statement_transcription_info st_info = {};
  st_info.begin_index = st_info.prefix_end_index = ts->str->len;
  st_info.variable_reported = false;

  switch (syntax_node->type) {
  case MC_SYNTAX_CONTINUE_STATEMENT:
  case MC_SYNTAX_BREAK_STATEMENT: {
    mct_transcribe_indent(ts);
    mcs_append_syntax_node_to_mc_str(ts->str, syntax_node);
    // append_to_mc_str(ts->str, "\n");
  } break;
  case MC_SYNTAX_RETURN_STATEMENT: {
    mct_transcribe_function_end(ts, syntax_node->return_statement.expression);
  } break;
  case MC_SYNTAX_CODE_BLOCK: {
    mct_transcribe_code_block(ts, syntax_node, false);
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
    mct_transcribe_if_statement(ts, &st_info, syntax_node);
  } break;
  case MC_SYNTAX_DECLARATION_STATEMENT: {
    mct_transcribe_declaration_statement(ts, &st_info, syntax_node);
  } break;
  case MC_SYNTAX_GLOBAL_VARIABLE_DECLARATION: {
    mct_transcribe_global_var_declaration_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_VA_LIST_STATEMENT: {
    mct_transcribe_va_list_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_GOTO_STATEMENT: {
    mct_transcribe_goto_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_LABEL_STATEMENT: {
    mct_transcribe_label_statement(ts, syntax_node);
  } break;
  case MC_SYNTAX_VA_START_STATEMENT:
  case MC_SYNTAX_VA_END_STATEMENT: {
    // Ignore these statement...
  } break;
  case MC_SYNTAX_EXPRESSION_STATEMENT: {
    mct_transcribe_indent(ts);
    register_midge_error_tag("mct_transcribe_statement_list-ES5");
    mct_transcribe_expression(ts, &st_info, syntax_node->expression_statement.expression);
    append_to_mc_str(ts->str, ";");
    register_midge_error_tag("mct_transcribe_statement_list-ES9");

    // register_midge_error_tag("mct_transcribe_statement_list-ES0");
    // // TODO -- MCerror exception
    // if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION &&
    //     (mc_token_type)syntax_node->expression_statement.expression->invocation.function_identity->type ==
    //         MC_TOKEN_IDENTIFIER &&
    //     !strcmp(syntax_node->expression_statement.expression->invocation.function_identity->text, "MCerror")) {
    //   mct_transcribe_mcerror(ts, syntax_node->expression_statement.expression);
    //   // print_syntax_node(syntax_node, 0);
    //   // MCerror(1300, "progress");
    //   break;
    // }
    // // Do MC_invokes
    // bool contains_mc_function_call;
    // mct_contains_mc_invoke(syntax_node->expression_statement.expression, &contains_mc_function_call);
    // // print_syntax_node(syntax_node, 0);
    // // printf("contains_mc_function_call=%s\n", contains_mc_function_call ? "true" : "false");
    // if (contains_mc_function_call) {
    //   if (syntax_node->expression_statement.expression->type == MC_SYNTAX_ASSIGNMENT_EXPRESSION) {
    //     mc_syntax_node *ass_expr = syntax_node->expression_statement.expression;

    //     if (ass_expr->assignment_expression.value_expression->type != MC_SYNTAX_INVOCATION) {
    //       print_syntax_node(ass_expr, 0);
    //       MCerror(2093, "Unsupported TODO?");
    //     }

    //     char *variable_text;
    //     mcs_copy_syntax_node_to_text(ass_expr->assignment_expression.variable, &variable_text);

    //     mct_transcribe_mc_invocation(ts, ass_expr->assignment_expression.value_expression, variable_text);
    //     free(variable_text);
    //     break;
    //   }

    //   if (syntax_node->expression_statement.expression->type != MC_SYNTAX_INVOCATION) {

    //     // Nested MC_invocation .. eek
    //     print_syntax_node(syntax_node, 0);
    //     MCerror(231, "TODO");
    //   }
    // }

    // // TODO -- maybe more coverage (atm only doing invocation expresssions. NOT invocations nested in other
    // // expressions)
    // if (syntax_node->expression_statement.expression->type == MC_SYNTAX_INVOCATION) {
    //   mct_transcribe_mc_invocation(ts, syntax_node->expression_statement.expression, NULL);
    //   break;
    // }

    // mct_transcribe_indent(ts);

    // register_midge_error_tag("mct_transcribe_statement_list-ES5");
    // mct_transcribe_expression(ts, &st_info, syntax_node->expression_statement.expression);
    // append_to_mc_str(ts->str, ";\n");
    // register_midge_error_tag("mct_transcribe_statement_list-ES9");
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
    MCerror(1527, "INVALID ARGUMENT: %s '%s'", get_mc_syntax_token_type_name(syntax_node->type), syntax_node->text);
  }

  for (int i = 0; i < syntax_node->children->count; ++i) {
    // printf ("h343\n");
    // printf("%p\n", syntax_node->children->items[i]);
    mc_syntax_node *child = syntax_node->children->items[i];
    {
      const char *tsl_type_name = get_mc_syntax_token_type_name(child->type);
      register_midge_error_tag("mct_transcribe_statement_list-L:%s", tsl_type_name);
    }
    // printf("@%i/%i@%s\n", i, syntax_node->children->count, get_mc_syntax_token_type_name(child->type));

    switch ((mc_token_type)child->type) {
    case MC_TOKEN_SPACE_SEQUENCE:
    case MC_TOKEN_TAB_SEQUENCE:
      continue;
    case MC_TOKEN_NEW_LINE:
      mcs_append_syntax_node_to_mc_str(ts->str, child);
      continue;
    case MC_TOKEN_LINE_COMMENT:
    case MC_TOKEN_MULTI_LINE_COMMENT: {
      mct_transcribe_indent(ts);
      mcs_append_syntax_node_to_mc_str(ts->str, child);
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

int mct_transcribe_code_block(mct_transcription_state *ts, mc_syntax_node *syntax_node, bool function_root)
{
  register_midge_error_tag("mct_transcribe_code_block()");
  append_to_mc_str(ts->str, "{\n");
  mct_increment_scope_depth(ts);
  ++ts->indent;

  if (function_root && ts->options->report_function_entry_exit_to_stack &&
      strcmp("_mca_thread_entry_wrap", ts->function_name)) {
    mct_transcribe_text_with_indent(ts, "int midge_error_stack_index;\n");
    mct_transcribe_text_with_indent(ts, "register_midge_stack_function_entry(\"");
    append_to_mc_str(ts->str, ts->function_name);
    append_to_mc_str(ts->str, "\", __FILE__, __LINE__, &midge_error_stack_index);\n\n");
  }

  if (syntax_node->code_block.statement_list) {
    mct_transcribe_statement_list(ts, syntax_node->code_block.statement_list);
    append_to_mc_str(ts->str, "\n");
  }

  mct_decrement_scope_depth(ts);
  --ts->indent;
  mct_transcribe_text_with_indent(ts, "}\n");

  register_midge_error_tag("mct_transcribe_code_block(~)");
  return 0;
}

int mct_transcribe_field_list(mct_transcription_state *ts, mc_syntax_node_list *field_list)
{
  for (int f = 0; f < field_list->count; ++f) {
    mc_syntax_node *field_syntax = field_list->items[f];

    mct_transcribe_indent(ts);
    mct_transcribe_field(ts, field_syntax);
    append_to_mc_str(ts->str, ";\n");
  }

  return 0;
}

int mct_transcribe_field_declarators(mct_transcription_state *ts, mc_syntax_node_list *declarators_list)
{
  for (int a = 0; a < declarators_list->count; ++a) {
    if (a > 0) {
      append_to_mc_str(ts->str, ",");
    }

    mc_syntax_node *declarator = declarators_list->items[a];
    if (declarator->field_declarator.type_dereference) {
      for (int d = 0; d < declarator->field_declarator.type_dereference->dereference_sequence.count; ++d) {
        append_to_mc_str(ts->str, "*");
      }
    }
    if (declarator->field_declarator.function_pointer) {
      // Function Pointer
      mct_transcribe_function_pointer_declarator(ts, NULL, declarator->field_declarator.function_pointer);
    }
    else {
      mcs_append_syntax_node_to_mc_str(ts->str, declarator->field_declarator.name);

      if (declarator->field_declarator.array_size) {
        append_to_mc_str(ts->str, "[");
        mcs_append_syntax_node_to_mc_str(ts->str, declarator->field_declarator.array_size);
        append_to_mc_str(ts->str, "]");
      }
    }
  }

  return 0;
}

int mct_transcribe_field(mct_transcription_state *ts, mc_syntax_node *syntax_node)
{
  mct_transcribe_indent(ts);

  switch (syntax_node->type) {
  case MC_SYNTAX_FIELD_DECLARATION: {
    switch (syntax_node->field.type) {
    case FIELD_KIND_STANDARD: {
      mct_transcribe_type_identifier(ts, syntax_node->field.type_identifier);
      append_to_mc_str(ts->str, " ");
      mct_transcribe_field_declarators(ts, syntax_node->field.declarators);
    } break;
    default:
      print_syntax_node(syntax_node, 0);
      MCerror(1122, "NotSupported:%i", syntax_node->field.type);
    }
  } break;
  case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
    mc_syntax_node *type_decl = syntax_node->nested_type.declaration;

    if (type_decl->type == MC_SYNTAX_STRUCT_DECL) {
      append_to_mc_str(ts->str, "struct ");

      if (type_decl->struct_decl.type_name) {
        mcs_append_syntax_node_to_mc_str(ts->str, type_decl->struct_decl.type_name);
        append_to_mc_str(ts->str, " ");
      }

      append_to_mc_str(ts->str, "{\n");
      ++ts->indent;

      mct_transcribe_field_list(ts, type_decl->struct_decl.fields);

      --ts->indent;
      mct_transcribe_text_with_indent(ts, "}");
    }
    else if (type_decl->type == MC_SYNTAX_UNION_DECL) {
      append_to_mc_str(ts->str, "union ");

      if (type_decl->union_decl.type_name) {
        mcs_append_syntax_node_to_mc_str(ts->str, type_decl->union_decl.type_name);
        append_to_mc_str(ts->str, " ");
      }

      append_to_mc_str(ts->str, "{\n");
      ++ts->indent;

      mct_transcribe_field_list(ts, type_decl->union_decl.fields);

      --ts->indent;
      mct_transcribe_text_with_indent(ts, "}");
    }
    else {
      MCerror(1136, "TODO");
    }

    if (syntax_node->nested_type.declarators) {
      append_to_mc_str(ts->str, " ");
      mct_transcribe_field_declarators(ts, syntax_node->nested_type.declarators);
    }
  } break;
  default:
    print_syntax_node(syntax_node, 0);
    MCerror(1130, "NotSupported:%s", get_mc_syntax_token_type_name(syntax_node->type));
  }

  return 0;
}

int mct_transcribe_function_end(mct_transcription_state *ts, mc_syntax_node *result_expression)
{
  append_to_mc_str(ts->str, "\n");
  mct_transcribe_text_with_indent(ts, "{\n");
  ++ts->indent;
  mct_transcribe_text_with_indent(ts, "// Return\n");

  mct_transcribe_indent(ts);
  if (ts->options->tag_on_function_exit) {
    append_to_mc_strf(ts->str, "register_midge_error_tag(\"%s(~)\");\n", ts->transcription_root->function.name->text);
    mct_transcribe_indent(ts);
  }

  if (ts->options->report_function_entry_exit_to_stack && strcmp("_mca_thread_entry_wrap", ts->function_name)) {
    append_to_mc_str(ts->str, "register_midge_stack_return(midge_error_stack_index);\n");
    mct_transcribe_indent(ts);
  }

  if (ts->options->report_variable_values && ts->options->report_variable_values->end_report_variable_values_delegate) {
    MCerror(3042, "TODO");
    // mct_transcribe_text_with_indent(ts, "{\n");
    // ++ts->indent;

    // mct_transcribe_indent(ts);
    // append_to_mc_strf(ts->str, "int (*end_report_variable_values)(int, void *) = *(int (**)(int, void *))%p;\n",
    //                  ts->options->report_variable_values->end_report_variable_values_delegate);
    // mct_transcribe_text_with_indent(ts, "void *mc_fld_vargs[1];\n");
    // mct_transcribe_text_with_indent(ts, "int mc_fld_dummy_ret;\n");
    // mct_transcribe_text_with_indent(ts, "mc_fld_vargs[0] = (void *)&mc_fld_dummy_ret;\n");
    // mct_transcribe_text_with_indent(ts, "int mc_fld_res = end_report_variable_values(1, mc_fld_vargs);\n");
    // mct_transcribe_text_with_indent(ts, "if(mc_fld_res) {\n");
    // ++ts->indent;
    // mct_transcribe_text_with_indent(ts, "printf(\"ERR[%i]: report_variable_value_delegate\\n\", mc_fld_res);\n");
    // mct_transcribe_text_with_indent(ts, "return mc_fld_res;\n");
    // --ts->indent;
    // mct_transcribe_text_with_indent(ts, "}\n");
    // --ts->indent;
    // mct_transcribe_text_with_indent(ts, "}\n");
  }

  // And the return statement
  mct_statement_transcription_info st_info = {};
  if (result_expression) {
    st_info.begin_index = st_info.prefix_end_index = ts->str->len;
  }

  append_to_mc_str(ts->str, "return");
  if (result_expression) {
    append_to_mc_str(ts->str, " ");
    mct_transcribe_expression(ts, &st_info, result_expression);
  }
  append_to_mc_str(ts->str, ";\n");
  --ts->indent;
  mct_transcribe_text_with_indent(ts, "}\n");

  return 0;
}

// // int transcribe_code_block_ast_to_mc_definition(mc_syntax_node *syntax_node, char **output)
// // {
// //   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition()");

// //   if (syntax_node->type != MC_SYNTAX_CODE_BLOCK) {
// //     MCerror(861, "MCT:Not Supported");
// //   }

// //   mc_str *str;
// //   init_mc_str(&str);

// //   if (syntax_node->code_block.statement_list) {
// //     mct_transcribe_statement_list(ts->str, 1, syntax_node->code_block.statement_list);
// //   }

// //   *output = str->text;
// //   release_mc_str(ts->str, false);

// //   register_midge_error_tag("transcribe_code_block_ast_to_mc_definition(~)");
// //   return 0;
// // }

int mct_transcribe_function(mct_transcription_state *ts, mc_syntax_node *function_ast)
{
  if (function_ast->type != MC_SYNTAX_FUNCTION) {
    MCerror(889, "MCT:Not Supported");
  }
  // if (!function_ast->function.code_block || function_ast->function.code_block->type != MC_SYNTAX_CODE_BLOCK ||
  //     !function_ast->function.code_block->code_block.statement_list) {
  //   print_syntax_node(function_ast, 0);
  //   MCerror(893, "TODO");
  // }
  if ((mc_token_type)function_ast->function.name->type != MC_TOKEN_IDENTIFIER) {
    MCerror(898, "TODO");
  }

  // Increment Scope
  mct_increment_scope_depth(ts);
  // ts->scope_index = 1;
  // ts->scope[ts->scope_index].variable_count = 0;

  // TODO -- static & other function modifiers?
  // Return specifier
  mcs_append_syntax_node_to_mc_str(ts->str, function_ast->function.return_type_identifier);
  append_to_mc_str(ts->str, " ");
  if (function_ast->function.return_type_dereference) {
    for (int a = 0; a < function_ast->function.return_type_dereference->dereference_sequence.count; ++a)
      append_to_mc_str(ts->str, "*");
  }

  // Identity
  ts->function_name = function_ast->function.name->text;
  mcs_append_syntax_node_to_mc_str(ts->str, function_ast->function.name);
  append_to_mc_str(ts->str, "(");

  // Parameters
  for (int p = 0; p < function_ast->function.parameters->count; ++p) {
    if (p) {
      append_to_mc_str(ts->str, ", ");
    }
    mcs_append_syntax_node_to_mc_str(ts->str, function_ast->function.parameters->items[p]);
  }
  append_to_mc_str(ts->str, ")");

  if (function_ast->function.code_block) {
    append_to_mc_str(ts->str, " ");
    mct_transcribe_code_block(ts, function_ast->function.code_block, true);
  }
  else
    append_to_mc_str(ts->str, ";");

  // Decrement Scope
  mct_decrement_scope_depth(ts);
  // ts->scope_index = 0;
  ts->function_name = NULL;

  return 0;
}

// int mct_transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast,
//                                   mct_function_transcription_options *options, char **mc_transcription)
// {
//   register_midge_error_tag("mct_transcribe_function_to_mc()");

//   if (function_ast->type != MC_SYNTAX_FUNCTION) {
//     MCerror(889, "MCT:Not Supported");
//   }
//   if (!function_ast->function.code_block || function_ast->function.code_block->type != MC_SYNTAX_CODE_BLOCK ||
//       !function_ast->function.code_block->code_block.statement_list) {
//     print_syntax_node(function_ast, 0);
//     MCerror(893, "TODO");
//   }
//   if ((mc_token_type)function_ast->function.name->type != MC_TOKEN_IDENTIFIER) {
//     MCerror(898, "TODO");
//   }

//   mct_transcription_state ts;
//   // -- options
//   ts.options = options;
//   // -- state
//   ts.transcription_root = function_ast;
//   ts.indent = 0;
//   init_mc_str(&ts.str);
//   // -- scope
//   mct_transcription_scope scope[MCT_TS_MAX_SCOPE_DEPTH];
//   mct_transcription_scope_variable scope_variables[MCT_TS_MAX_SCOPE_DEPTH * MCT_TS_MAX_VARIABLES];
//   for (int a = 0; a < MCT_TS_MAX_SCOPE_DEPTH; ++a) {
//     scope[a].variables = &scope_variables[a * MCT_TS_MAX_VARIABLES];
//   }
//   ts.scope = &scope[0];
//   ts.scope_index = 0;
//   ts.scope[ts.scope_index].variable_count = 0;

//   // printf("%s - %u\n", function_ast->function.name->text, func_info->latest_iteration);
//   // Header
//   // if (!strcmp(function_ast->function.name->text, "midge_initialize_app")) {
//   //   append_to_mc_str(ts.str, "int ");
//   //   append_to_mc_strf(ts.str, "%s", function_ast->function.name->text);
//   //   append_to_mc_str(ts.str, "_mc_v");
//   //   printf("mia-beforeu:\n%s||\n", ts.str->text);
//   //   append_to_mc_strf(ts.str, "%u", func_info->latest_iteration);
//   //   printf("mia-afteru:\n%s||\n", ts.str->text);
//   //   append_to_mc_str(ts.str, "(int mc_argsc, void **mc_argsv) {\n");
//   //   printf("hit-bounty!\n");
//   // }
//   // else {
//   append_to_mc_strf(ts.str, "int %s_mc_v%u(int mc_argsc, void **mc_argsv) {\n", function_ast->function.name->text,
//                    func_info->latest_iteration);
//   // }
//   mct_increment_scope_depth(&ts);
//   ++ts.indent;

//   // Initial
//   if (ts.options->tag_on_function_entry) {
//     mct_transcribe_indent(&ts);
//     append_to_mc_strf(ts.str, "register_midge_error_tag(\"%s()\");\n\n", function_ast->function.name->text);
//   }
//   if (ts.options->report_variable_values) {
//     char buf[256];
//     sprintf(buf,
//             "  // Function Variable Value Reporting UID\n"
//             "  unsigned int mc_fld_function_call_uid;\n"
//             "  {\n"
//             "    unsigned int *mc_fld_function_call_uid_counter = ((unsigned int *)%p);\n"
//             "    mc_fld_function_call_uid = *mc_fld_function_call_uid_counter;\n"
//             "    ++*mc_fld_function_call_uid_counter;\n"
//             "  }\n\n",
//             // "  {\n"
//             // "    int (*begin_report_variable_values)(int, void *) = *(int (**)(int, void *))%p;\n"
//             // "    void *mc_fld_vargs[2];\n"
//             // "    unsigned int *mc_fld_call_uid = &mc_fld_function_call_uid;\n"
//             // "    mc_fld_vargs[0] = (void *)&mc_fld_call_uid;\n"
//             // "    int mc_fld_dummy_ret;\n"
//             // "    mc_fld_vargs[1] = (void *)&mc_fld_dummy_ret;\n"
//             // "    int mc_fld_res = begin_report_variable_values(2, mc_fld_vargs);\n"
//             // // "  printf(\"mc_fld_res=%%i\\n\", mc_fld_res);\n"
//             // "    if(mc_fld_res) {\n"
//             // "      printf(\"ERR[%%i]: report_variable_value_delegate\\n\", mc_fld_res);\n"
//             // "      return mc_fld_res;\n"
//             // "  }\n",
//             // ts.options->report_variable_values->begin_report_variable_values_delegate);
//             &ts.options->report_variable_values->call_uid_counter);
//     append_to_mc_str(ts.str, buf);
//   }

//   // Function Parameters
//   append_to_mc_str(ts.str, "  // Function Parameters\n");
//   ts.scope[ts.scope_index].variable_count = 0;
//   for (int p = 0; p < function_ast->function.parameters->count; ++p) {
//     mc_syntax_node *parameter_syntax = function_ast->function.parameters->items[p];

//     mct_transcribe_indent(&ts);
//     switch (parameter_syntax->parameter.type) {
//     case PARAMETER_KIND_STANDARD: {

//       mct_add_scope_variable(&ts, parameter_syntax);

//       // print_syntax_node(parameter_syntax->parameter.type_identifier, 0);
//       mct_transcribe_type_identifier(&ts, parameter_syntax->parameter.type_identifier);
//       append_to_mc_str(ts.str, " ");
//       if (parameter_syntax->parameter.type_dereference) {
//         for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
//           append_to_mc_str(ts.str, "*");
//         }
//       }
//       mcs_append_syntax_node_to_mc_str(ts.str, parameter_syntax->parameter.name);
//       append_to_mc_str(ts.str, " = *(");
//       mct_transcribe_type_identifier(&ts, parameter_syntax->parameter.type_identifier);
//       append_to_mc_str(ts.str, " ");
//       if (parameter_syntax->parameter.type_dereference) {
//         for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
//           append_to_mc_str(ts.str, "*");
//         }
//       }
//       append_to_mc_strf(ts.str, "*)mc_argsv[%i];\n", p);

//       if (ts.options->report_variable_values) {
//         mct_statement_transcription_info st_info = {};
//         st_info.begin_index = st_info.prefix_end_index = ts.str->len;
//         mct_transcribe_variable_value_report(&ts, &st_info, parameter_syntax->parameter.name);
//       }
//     } break;
//     case PARAMETER_KIND_FUNCTION_POINTER: {
//       mct_add_scope_variable(&ts, parameter_syntax);
//       // print_syntax_node(parameter_syntax, 0);

//       mct_transcribe_type_identifier(&ts, parameter_syntax->parameter.type_identifier);
//       append_to_mc_str(ts.str, " ");
//       if (parameter_syntax->parameter.type_dereference) {
//         for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
//           append_to_mc_str(ts.str, "*");
//         }
//       }
//       mcs_append_syntax_node_to_mc_str(ts.str, parameter_syntax->parameter.function_pointer);

//       append_to_mc_str(ts.str, " = *(");

//       mcs_append_syntax_node_to_mc_str(ts.str, parameter_syntax->parameter.type_identifier);
//       append_to_mc_str(ts.str, " ");
//       if (parameter_syntax->parameter.type_dereference) {
//         for (int d = 0; d < parameter_syntax->parameter.type_dereference->dereference_sequence.count; ++d) {
//           append_to_mc_str(ts.str, "*");
//         }
//       }
//       append_to_mc_str(ts.str, "(");
//       if (parameter_syntax->parameter.function_pointer->fptr_declarator.fp_dereference) {
//         for (int d = 0;
//              d <
//              parameter_syntax->parameter.function_pointer->fptr_declarator.fp_dereference->dereference_sequence.count;
//              ++d) {
//           append_to_mc_str(ts.str, "*");
//         }
//       }
//       append_to_mc_str(ts.str, "*)(");

//       for (int p = 0; p < parameter_syntax->parameter.function_pointer->fptr_declarator.parameters->count; ++p) {
//         if (p > 0) {
//           append_to_mc_str(ts.str, ", ");
//         }

//         mc_syntax_node *fp_param =
//         parameter_syntax->parameter.function_pointer->fptr_declarator.parameters->items[p];

//         switch (fp_param->parameter.type) {
//         case PARAMETER_KIND_STANDARD: {
//           mcs_append_syntax_node_to_mc_str(ts.str, fp_param->parameter.type_identifier);
//           if (fp_param->parameter.type_dereference) {
//             for (int d = 0; d < fp_param->parameter.type_dereference->dereference_sequence.count; ++d) {
//               append_to_mc_str(ts.str, "*");
//             }
//           }
//         } break;
//         default:
//           MCerror(2119, "unsupported:%i", fp_param->parameter.type);
//         }
//       }
//       append_to_mc_strf(ts.str, "))mc_argsv[%i];\n", p);

//     } break;
//     case PARAMETER_KIND_VARIABLE_ARGS: {
//     } break;
//     default:
//       MCerror(958, "NotSupported:%i", parameter_syntax->parameter.type);
//     }

//     // if (parameter_syntax->parameter.is_function_pointer_declaration) {
//     //   printf("918 TODO\n");
//     //   print_syntax_node(parameter_syntax, 0);
//     //   MCerror(912, "TODO");
//     //   continue;
//     // }
//   }

//   if (function_ast->function.return_type_dereference ||
//       strcmp(function_ast->function.return_type_identifier->type_identifier.identifier->text, "void")) {
//     mct_transcribe_indent(&ts);

//     mct_transcribe_type_identifier(&ts, function_ast->function.return_type_identifier);
//     append_to_mc_str(ts.str, " ");
//     if (function_ast->function.return_type_dereference)
//       mcs_append_syntax_node_to_mc_str(ts.str, function_ast->function.return_type_dereference);

//     append_to_mc_str(ts.str, "*mc_return_value = (");
//     mct_transcribe_type_identifier(&ts, function_ast->function.return_type_identifier);
//     append_to_mc_str(ts.str, " ");
//     if (function_ast->function.return_type_dereference)
//       mcs_append_syntax_node_to_mc_str(ts.str, function_ast->function.return_type_dereference);

//     append_to_mc_str(ts.str, "*)mc_argsv[mc_argsc - 1];\n");
//   }

//   // Code Block
//   append_to_mc_str(ts.str, "\n  // Function Code\n");
//   ++ts.indent;
//   mct_increment_scope_depth(&ts);

//   mct_transcribe_statement_list(&ts, function_ast->function.code_block->code_block.statement_list);

//   mct_decrement_scope_depth(&ts);
//   --ts.indent;

//   // if (!strcmp(ts.transcription_root->function.name->text, "print_syntax_node")) {
//   //   printf("LLL>>>>\n");
//   //   print_syntax_node(ts.transcription_root, 0);
//   // }

//   // TODO -- check function returns if it has a return value

//   // Return Statement
//   mct_transcribe_function_end(&ts);
//   mct_decrement_scope_depth(&ts);
//   append_to_mc_str(ts.str, "}");
//   *mc_transcription = ts.str->text;
//   release_mc_str(ts.str, false);

//   // printf("mc_transcription:\n%s||\n", *mc_transcription);

//   register_midge_error_tag("mct_transcribe_function_to_mc(~)");
//   return 0;
// }

// int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *structure_ast, char **mc_transcription)
// {
//   register_midge_error_tag("transcribe_struct_to_mc()");

//   if (structure_ast->type != MC_SYNTAX_STRUCT_DECL && structure_ast->type != MC_SYNTAX_UNION_DECL) {
//     MCerror(1242, "MCT:Invalid Argument");
//   }

//   mct_transcription_state ts;
//   // -- options
//   ts.options = NULL;
//   // ts.options->report_function_entry_exit_to_stack = true;
//   // ts.report_simple_args_to_error_stack = true;
//   // ts.check_mc_functions_not_null = true;
//   // ts.options->tag_on_function_entry = true;
//   // ts.options->tag_on_function_exit = true;
//   // -- state
//   ts.transcription_root = structure_ast;
//   ts.indent = 0;
//   init_mc_str(&ts.str);
//   // -- scope
//   mct_transcription_scope scope[MCT_TS_MAX_SCOPE_DEPTH];
//   mct_transcription_scope_variable scope_variables[MCT_TS_MAX_SCOPE_DEPTH * MCT_TS_MAX_VARIABLES];
//   for (int a = 0; a < MCT_TS_MAX_SCOPE_DEPTH; ++a) {
//     scope[a].variables = &scope_variables[a * MCT_TS_MAX_VARIABLES];
//   }
//   ts.scope = &scope[0];
//   ts.scope_index = 0;
//   ts.scope[ts.scope_index].variable_count = 0;

//   // Header
//   if (structure_ast->type == MC_SYNTAX_STRUCT_DECL)
//     append_to_mc_str(ts.str, "struct \n");
//   else if (structure_ast->type == MC_SYNTAX_UNION_DECL)
//     append_to_mc_str(ts.str, "union \n");
//   append_to_mc_str(ts.str, structure_info->name);

//   if (structure_ast->struct_decl.fields) {
//     append_to_mc_str(ts.str, " {\n");
//     ++ts.indent;

//     mct_transcribe_field_list(&ts, structure_ast->struct_decl.fields);

//     --ts.indent;
//     append_to_mc_str(ts.str, "}");
//     // append_to_mc_strf(ts.str, "}", structure_ast->struct_decl.type_name->text,
//     //                  structure_info->latest_iteration); // TODO -- types not structs
//   }
//   append_to_mc_str(ts.str, ";");

//   *mc_transcription = ts.str->text;
//   release_mc_str(ts.str, false);

//   // print_syntax_node(structure_ast, 0);
//   // printf("def:\n%s||\n", *mc_transcription);

//   register_midge_error_tag("transcribe_struct_to_mc(~)");
//   return 0;
// }

int mct_transcribe_enum_declaration(mct_transcription_state *ts, mc_syntax_node *enum_ast)
{
  if (enum_ast->type != MC_SYNTAX_ENUM_DECL) {
    MCerror(1242, "MCT:Invalid Argument");
  }

  mcs_append_syntax_node_to_mc_str(ts->str, enum_ast);

  return 0;
}

int mct_transcribe_struct_declaration(mct_transcription_state *ts, mc_syntax_node *structure_ast)
{
  if (structure_ast->type != MC_SYNTAX_STRUCT_DECL && structure_ast->type != MC_SYNTAX_UNION_DECL) {
    MCerror(1242, "MCT:Invalid Argument");
  }

  // Header
  if (structure_ast->type == MC_SYNTAX_STRUCT_DECL)
    append_to_mc_str(ts->str, "struct ");
  else if (structure_ast->type == MC_SYNTAX_UNION_DECL)
    append_to_mc_str(ts->str, "union ");
  mcs_append_syntax_node_to_mc_str(ts->str, structure_ast->struct_decl.type_name);

  if (structure_ast->struct_decl.fields) {
    // Definition
    append_to_mc_str(ts->str, " {\n");
    ++ts->indent;

    mct_transcribe_field_list(ts, structure_ast->struct_decl.fields);

    --ts->indent;
    append_to_mc_str(ts->str, "}");
  }
  else {
    // Just a declaration
  }

  return 0;
}

int mct_transcribe_type_alias(mct_transcription_state *ts, mc_syntax_node *type_define)
{
  append_to_mc_str(ts->str, "typedef ");

  switch (type_define->type_alias.type_descriptor->type) {
  case MC_SYNTAX_UNION_DECL:
  case MC_SYNTAX_STRUCT_DECL: {
    mct_transcribe_struct_declaration(ts, type_define->type_alias.type_descriptor);
  } break;
  case MC_SYNTAX_ENUM_DECL: {
    mct_transcribe_enum_declaration(ts, type_define->type_alias.type_descriptor);
  } break;
  default:
    MCerror(3455, "mct_transcribe_type_alias: Unsupported type-descriptor=%s",
            get_mc_syntax_token_type_name(type_define->type_alias.type_descriptor->type));
  }
  append_to_mc_str(ts->str, " ");
  mcs_append_syntax_node_to_mc_str(ts->str, type_define->type_alias.alias);
  append_to_mc_str(ts->str, ";\n");

  return 0;
}

int mct_transcribe_file_root_children(mct_transcription_state *ts, mc_syntax_node_list *root_children)
{
  mc_syntax_node *child;
  for (int a = 0; a < root_children->count; ++a) {
    child = root_children->items[a];

    // Tokens
    if (child->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
      // printf("ts->str (%s):\n%s||\n", get_mc_syntax_token_type_name(child->type), ts->str->text);
      // print_syntax_node(child, 0);
      // printf("%u %u\n", ts->str->alloc, ts->str->len);
      mcs_append_syntax_node_to_mc_str(ts->str, child);
      // printf("ts->str (%s):\n%s||\n", get_mc_syntax_token_type_name(child->type), ts->str->text);
      // MCerror(3554, "HERE");
      continue;
    }

    // Syntax-Nodes
    switch (child->type) {
    case MC_SYNTAX_PP_DIRECTIVE_DEFINE:
    case MC_SYNTAX_PP_DIRECTIVE_UNDEFINE:
    case MC_SYNTAX_PP_DIRECTIVE_IFDEF:
    case MC_SYNTAX_PP_DIRECTIVE_IFNDEF:
    case MC_SYNTAX_PP_DIRECTIVE_INCLUDE: {
      MCcall(mct_transcribe_file_root_children(ts, child->children));
      // usleep(10000);
    } break;
    case MC_SYNTAX_TYPE_ALIAS: {
      mct_transcribe_type_alias(ts, child);
    } break;
    case MC_SYNTAX_FUNCTION: {
      mct_transcribe_function(ts, child);
    } break;
    case MC_SYNTAX_ENUM_DECL: {
      mct_transcribe_enum_declaration(ts, child);
    } break;
    case MC_SYNTAX_UNION_DECL: {
      mct_transcribe_struct_declaration(ts, child);

      // if (!child->union_decl.fields) {
      // Forward declaration
      append_to_mc_str(ts->str, ";");
      // }
    } break;
    case MC_SYNTAX_GLOBAL_VARIABLE_DECLARATION: {
      mct_transcribe_field(ts, child->global_var_decl_statement.declaration);
      append_to_mc_str(ts->str, ";");
    } break;
    case MC_SYNTAX_STRUCT_DECL: {
      mct_transcribe_struct_declaration(ts, child);

      // if (!child->struct_decl.fields) {
      // Forward declaration
      append_to_mc_str(ts->str, ";");
      // }
    } break;
    default:
      MCerror(3479, "mct_transcribe_file_root_children: Unsupported Root=%s",
              get_mc_syntax_token_type_name(child->type));
    }
  }

  return 0;
}

int mct_transcribe_file_ast(mc_syntax_node *file_root, mct_function_transcription_options *options, char **generated)
{
  if (file_root->type != MC_SYNTAX_FILE_ROOT) {
    MCerror(3437, "Invalid Argument");
  }

  // Transcription Options/State
  mct_transcription_state ts;
  ts.options = options;
  ts.transcription_root = file_root;
  ts.indent = 0;
  init_mc_str(&ts.str);

  // -- scope
  mct_transcription_scope scope[MCT_TS_MAX_SCOPE_DEPTH];
  mct_transcription_scope_variable scope_variables[MCT_TS_MAX_SCOPE_DEPTH * MCT_TS_MAX_VARIABLES];
  for (int a = 0; a < MCT_TS_MAX_SCOPE_DEPTH; ++a) {
    scope[a].variables = &scope_variables[a * MCT_TS_MAX_VARIABLES];
  }
  ts.scope = &scope[0];
  ts.scope_index = 0;
  ts.scope[ts.scope_index].variable_count = 0;

  // Transcribe
  append_to_mc_str(ts.str, "#include \"midge_error_handling.h\"\n\n");
  MCcall(mct_transcribe_file_root_children(&ts, file_root->children));
  *generated = ts.str->text;

  release_mc_str(ts.str, false);

  return 0;
}