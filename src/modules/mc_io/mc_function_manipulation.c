/* mc_function_manipulation.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "midge_error_handling.h"

#include "modules/mc_io/mc_function_manipulation.h"

typedef struct mc_function_code_cursor {
  function_info *function;

  mc_syntax_node *root_block, *pos;
  int depth;

} mc_function_code_cursor;

int _mcs_attach_token(mc_syntax_node *parent, mc_token_type type, const char *text,
                      mc_syntax_node **additional_destination)
{
  mc_syntax_node *token = (mc_syntax_node *)malloc(sizeof(mc_syntax_node));

  token->type = (mc_syntax_node_type)type;
  token->parent = parent;
  token->text = strdup(text);
  token->begin.col = token->begin.index = token->begin.line = 0;

  MCcall(append_to_collection((void ***)&parent->children->items, &parent->children->alloc, &parent->children->count,
                              token));
  if (additional_destination) {
    *additional_destination = token;
  }

  return 0;
}

/*
 * @returns 0 for success, -1 for no statement found, and -2 if there are more than one for loops present.
 * @error 7239 if in invalid position
 */
int mcs_fc_move_into_only_for_loop(mc_function_code_cursor *cursor)
{
  int a;

  mc_syntax_node *sn = NULL;

  switch (cursor->pos->type) {
  case MC_SYNTAX_CODE_BLOCK: {
    mc_syntax_node_list *stn = cursor->pos->code_block.statement_list;

    for (int a = 0; a < stn->count; ++a) {
      if (stn->items[a]->type == MC_SYNTAX_FOR_STATEMENT) {
        if (sn) {
          return -2;
        }
        sn = stn->items[a];
      }
    }
  } break;
  default:
    MCerror(7239, "TODO : %s", get_mc_syntax_token_type_name(cursor->pos->type));
  }

  if (!sn) {
    // No Statement Found
    return -1;
  }

  cursor->pos = sn;
  return 0;
}

/*
 * @returns 0 for success, -1 for no statement found, and -2 if there are more than one for loops present.
 * @error 5201 if in invalid position
 */
int mcs_fc_move_into_only_switch_statement(mc_function_code_cursor *cursor)
{
  int a;

  mc_syntax_node *sn = NULL, *tn;

  switch (cursor->pos->type) {
  case MC_SYNTAX_CODE_BLOCK: {
    mc_syntax_node_list *stn = cursor->pos->code_block.statement_list;

    for (a = 0; a < stn->count; ++a) {
      if (stn->items[a]->type == MC_SYNTAX_SWITCH_STATEMENT) {
        if (sn) {
          return -2;
        }
        sn = stn->items[a];
      }
    }
  } break;
  case MC_SYNTAX_FOR_STATEMENT: {
    tn = cursor->pos;
    cursor->pos = cursor->pos->for_statement.loop_statement;
    a = mcs_fc_move_into_only_switch_statement(cursor);
    if (a) {
      cursor->pos = tn;
      MCerror(4827, "HERE");
    }
    return 0;
  }
  default:
    MCerror(5201, "TODO : %s", get_mc_syntax_token_type_name(cursor->pos->type));
  }

  if (!sn) {
    // No Statement Found
    return -1;
  }

  cursor->pos = sn;
  return 0;
}

/*
 * @returns 0 for success, -1 for case label already exists.
 * @error 8528 if in invalid position
 */
int mcs_fc_add_switch_case_label(mc_function_code_cursor *cursor, const char *label)
{
  int a, b;
  mc_syntax_node *sn, *nn, *tn;
  mc_syntax_node_list *sl, *ll;

  switch (cursor->pos->type) {
  case MC_SYNTAX_SWITCH_STATEMENT: {
    // Check label does not already exist
    sl = cursor->pos->switch_statement.sections;
    for (a = 0; a < sl->count; ++a) {
      ll = sl->items[a]->switch_section.labels;
      for (b = 0; b < ll->count; ++b) {
        tn = ll->items[b];
        if (tn->type == MC_SYNTAX_SWITCH_DEFAULT_LABEL)
          continue;
        if (tn->type != MC_SYNTAX_SWITCH_CASE_LABEL) {
          // Don't think this is necessary
          MCerror(3783, "TODO");
        }
        sn = ll->items[b]->switch_case_label.constant;
        printf("sn=%p\n", sn);

        switch (sn->type) {
        case MC_TOKEN_IDENTIFIER: {
          if (!strcmp(sn->text, label)) {
            return -1;
          }
        } break;
        default:
          MCerror(4412, "TODO : %s", get_mc_syntax_token_type_name(sn->type));
        }
      }
    }

    // Insert a new label in a new section
    MCcall(mcs_construct_syntax_node(NULL, MC_SYNTAX_SWITCH_SECTION, NULL, cursor->pos, &sn));
    MCcall(append_to_collection((void ***)&sl->items, &sl->alloc, &sl->count, sn));

    MCcall(mcs_construct_syntax_node(NULL, MC_SYNTAX_SWITCH_CASE_LABEL, NULL, sn, &nn));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_CASE_KEYWORD, "case", NULL));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_SPACE_SEQUENCE, " ", NULL));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_IDENTIFIER, label, &nn->switch_case_label.constant));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_COLON, ":", NULL));
    MCcall(append_to_collection((void ***)&sn->switch_section.labels->items, &sn->switch_section.labels->alloc,
                                &sn->switch_section.labels->count, nn));

    MCcall(_mcs_attach_token(nn, MC_TOKEN_NEW_LINE, "\n", NULL));
    MCcall(_mcs_attach_token(sn, MC_TOKEN_SPACE_SEQUENCE, " ", NULL));

    MCcall(mcs_construct_syntax_node(NULL, MC_SYNTAX_CODE_BLOCK, NULL, sn, &nn));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_CURLY_OPENING_BRACKET, "{", NULL));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_NEW_LINE, "\n", NULL));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_CURLY_CLOSING_BRACKET, "}", NULL));
    MCcall(append_to_collection((void ***)&sn->switch_section.statement_list->items,
                                &sn->switch_section.statement_list->alloc, &sn->switch_section.statement_list->count,
                                nn));

    MCcall(_mcs_attach_token(sn, MC_TOKEN_NEW_LINE, "\n", NULL));
    MCcall(mcs_construct_syntax_node(NULL, MC_SYNTAX_BREAK_STATEMENT, NULL, sn, &nn));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_BREAK_KEYWORD, "break", NULL));
    MCcall(_mcs_attach_token(nn, MC_TOKEN_SEMI_COLON, ";", NULL));
    MCcall(append_to_collection((void ***)&sn->switch_section.statement_list->items,
                                &sn->switch_section.statement_list->alloc, &sn->switch_section.statement_list->count,
                                nn));

  } break;
  default:
    MCerror(8528, "TODO : %s", get_mc_syntax_token_type_name(cursor->pos->type));
  }

  return 0;
}

/*
 * @returns 0 for success, -1 for case label not found
 * @error 4256 if in invalid position
 */
int mcs_fc_move_into_switch_section(mc_function_code_cursor *cursor, const char *case_label)
{
  int a, b;
  mc_syntax_node *sn, *tn;
  mc_syntax_node_list *sl, *ll;

  switch (cursor->pos->type) {
  case MC_SYNTAX_SWITCH_STATEMENT: {
    sl = cursor->pos->switch_statement.sections;
    for (a = 0; a < sl->count; ++a) {
      ll = sl->items[a]->switch_section.labels;

      for (b = 0; b < ll->count; ++b) {
        tn = ll->items[b];

        if (tn->type == MC_SYNTAX_SWITCH_DEFAULT_LABEL)
          continue;
        if (tn->type != MC_SYNTAX_SWITCH_CASE_LABEL) {
          // Don't think this is necessary
          MCerror(3783, "TODO");
        }

        sn = ll->items[b]->switch_case_label.constant;
        // printf("sn=%p\n", sn);
        switch (sn->type) {
        case MC_TOKEN_IDENTIFIER: {
          if (!strcmp(sn->text, case_label)) {
            cursor->pos = sl->items[a];
            return 0;
          }
        } break;
        default:
          MCerror(4412, "TODO : %s", get_mc_syntax_token_type_name(sn->type));
        }
      }
    }

    return -1;

  } break;
  default:
    MCerror(4256, "TODO : %s", get_mc_syntax_token_type_name(cursor->pos->type));
  }

  return 0;
}

int mcs_fc_add_statement(mc_function_code_cursor *cursor, const char *statement)
{
  mc_syntax_node *st_ast, *sn, *sm;
  MCcall(mcs_parse_singular_statement(statement, &st_ast));

  switch (cursor->pos->type) {
  case MC_SYNTAX_SWITCH_SECTION: {
    sn = cursor->pos->switch_section.statement_list->items[cursor->pos->switch_section.statement_list->count - 1];
    if (sn->type == MC_SYNTAX_BREAK_STATEMENT) {
      sn = cursor->pos->switch_section.statement_list->items[cursor->pos->switch_section.statement_list->count - 2];
    }
    else {
      MCerror(7748, "TODO : %s", get_mc_syntax_token_type_name(sn->type));
    }

    if (sn->type == MC_SYNTAX_CODE_BLOCK) {
      // Place inside at the end
      MCcall(append_to_collection((void ***)&sn->code_block.statement_list->items,
                                  &sn->code_block.statement_list->alloc, &sn->code_block.statement_list->count,
                                  st_ast));

      MCcall(insert_in_collection((void ***)&sn->children->items, &sn->children->alloc, &sn->children->count,
                                  sn->children->count - 1, st_ast));
      break;
    }

    MCerror(4976, "TODO : %s", get_mc_syntax_token_type_name(sn->type));
  } break;
  default:
    MCerror(4897, "TODO : %s", get_mc_syntax_token_type_name(cursor->pos->type));
  }

  return 0;
}

// int debug_test_code_cursor()
// {
//   mc_function_code_cursor cursor;
//   MCcall(find_function_info("_mc_transcribe_segment_list", &cursor.function));
//   if (!cursor.function) {
//     MCerror(8527, "doe");
//   }

//   // printf("code:\n%s||\n", cursor.function->code);

//   MCcall(mcs_parse_code_block_to_syntax_tree(cursor.function->code, &cursor.root_block));
//   cursor.pos = cursor.root_block;
//   cursor.depth = 0;

//   // DO STUFF
//   MCcall(mcs_fc_move_into_only_for_loop(&cursor));
//   MCcall(mcs_fc_move_into_only_switch_statement(&cursor));
//   MCcall(mcs_fc_add_switch_case_label(&cursor, "MC_SOURCE_SEGMENT_SINGLE_LINE_COMMENT"));
//   MCcall(mcs_fc_move_into_switch_section(&cursor, "MC_SOURCE_SEGMENT_SINGLE_LINE_COMMENT"));
//   MCcall(mcs_fc_add_statement(&cursor, "MCcall(append_to_mc_str(str, (const char *)seg->data));"));

//   MCcall(print_syntax_node(cursor.pos, 0));

//   MCerror(7522, "PROGRESS");

//   return 0;
// }