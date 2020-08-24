#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

// typedef struct editor_panel_button_info {
//   struct {
//     uint x, y, actual_width, actual_height, max_width, max_height;
//   } bounds;
//   uint image_resource_uid;
//   char *button_text;
// } editor_panel_button_info;

int code_editor_render_fld_view_code(frame_time *elapsed, mc_node_v1 *visual_node)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // Entity States
  mc_code_editor_state_v1 *cestate = (mc_code_editor_state_v1 *)visual_node->extra;
  fld_view_state *fld_view = cestate->fld_view;

  image_render_queue *sequence;
  element_render_command *element_cmd;

  // printf ("######################\n");
  {
    const int EDITOR_LINE_STRIDE = 22;
    const float EDITOR_FONT_HORIZONTAL_STRIDE = 10.31f;
    int line_index = 0;
    int col_index = 0;

    // printf("fldr-0 fld_view->visual_code.count:%i\n", fld_view->visual_code.count);
    int previous_line_index = -1;
    int ce_offset_line_index;
    bool line_is_visible;
    for (int i = 0; i < fld_view->visual_code.count; ++i) {

      if (line_index != previous_line_index) {
        ce_offset_line_index = line_index - cestate->line_display_offset;
        line_is_visible = (ce_offset_line_index >= 0 && ce_offset_line_index < CODE_EDITOR_RENDERED_CODE_LINES);
        previous_line_index = line_index;

        // Render Line
        if (line_is_visible) {
          // printf("obtain_render_queue: ce_offset_line_index:%i\n", ce_offset_line_index);
          MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
          sequence->render_target = NODE_RENDER_TARGET_IMAGE;
          sequence->clear_color = COLOR_TRANSPARENT;
          sequence->image_width = cestate->render_lines[ce_offset_line_index]->width;
          sequence->image_height = cestate->render_lines[ce_offset_line_index]->height;
          sequence->data.target_image.image_uid = cestate->render_lines[ce_offset_line_index]->image_resource_uid;

          MCcall(set_c_str(cestate->render_lines[ce_offset_line_index]->rtf, ""));
        }
      }

      switch (fld_view->visual_code.items[i]->type) {
      case FLD_CODE_CSTRING: {
        char *text = (char *)fld_view->visual_code.items[i]->data;
        int c = 0;
        while (1) {
          int s = c;
          bool eos = false;
          for (;; ++c) {
            if (text[c] == '\0') {
              eos = true;
              break;
            }
            else if (text[c] == '\n') {
              break;
            }
          }

          if (line_index != previous_line_index) {
            ce_offset_line_index = line_index - cestate->line_display_offset;
            line_is_visible = (ce_offset_line_index >= 0 && ce_offset_line_index < CODE_EDITOR_RENDERED_CODE_LINES);
            previous_line_index = line_index;

            // Render Line
            if (line_is_visible) {
              // printf("obtain_render_queue: ce_offset_line_index:%i\n", ce_offset_line_index);
              MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
              sequence->render_target = NODE_RENDER_TARGET_IMAGE;
              sequence->clear_color = COLOR_TRANSPARENT;
              sequence->image_width = cestate->render_lines[ce_offset_line_index]->width;
              sequence->image_height = cestate->render_lines[ce_offset_line_index]->height;
              sequence->data.target_image.image_uid = cestate->render_lines[ce_offset_line_index]->image_resource_uid;

              MCcall(set_c_str(cestate->render_lines[ce_offset_line_index]->rtf, ""));
            }
          }

          // Print sequence
          if (line_is_visible && c - s > 0) {
            MCcall(obtain_element_render_command(sequence, &element_cmd));
            element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
            element_cmd->x = 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
            element_cmd->y = 2 + 12;
            element_cmd->data.print_text.font_resource_uid = cestate->font_resource_uid;
            allocate_and_copy_cstrn(element_cmd->data.print_text.text, text + s, c - s);
            element_cmd->data.print_text.color.a = 1.f;
            element_cmd->data.print_text.color.r = 0.61f;
            element_cmd->data.print_text.color.g = 0.86f;
            element_cmd->data.print_text.color.b = 0.99f;
            // printf("print_text_command: ce_offset_line_index:%i:'%s'[len:%i]\n", ce_offset_line_index,
            //        element_cmd->data.print_text.text, strlen(element_cmd->data.print_text.text));

            // printf("printing to :%i:'%s'\n", ce_offset_line_index, element_cmd->data.print_text.text);

            col_index += c - s;
          }

          if (eos) {
            break;
          }

          ++line_index;
          col_index = 0;
          ++c;
        }
      } break;
      case FLD_CODE_SNAPSHOT: {
        if (!line_is_visible) {
          break;
        }

        fld_variable_snapshot *snapshot = (fld_variable_snapshot *)fld_view->visual_code.items[i]->data;

        MCcall(obtain_element_render_command(sequence, &element_cmd));
        element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
        element_cmd->x = 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
        element_cmd->y = 2 + 12;
        element_cmd->data.print_text.font_resource_uid = cestate->font_resource_uid;
        allocate_and_copy_cstr(element_cmd->data.print_text.text, snapshot->value_text);
        element_cmd->data.print_text.color = COLOR_YELLOW;
        // printf("print_text_command: ce_offset_line_index:%i:'%s'[len:%i]\n", ce_offset_line_index,
        //        element_cmd->data.print_text.text, strlen(element_cmd->data.print_text.text));

        col_index += strlen(snapshot->value_text);
      } break;
      default: {
        MCerror(456, "TODO:%i", fld_view->visual_code.items[i]->type);
      }
      }
    }
  }
  return 0;
}

int mce_parse_past_integer(char *text, int *text_index, int *result)
{
  if (!isdigit(text[*text_index])) {
    MCerror(303, "Not an integer");
  }

  int n = 0;
  while (isdigit(text[*text_index])) {
    n *= 10;
    n += text[*text_index] - '0';
    ++*text_index;
  }

  *result = n;

  return 0;
}

int free_fld_visual_code_element(fld_visual_code_element **item)
{
  // Free the data
  switch ((*item)->type) {
  case FLD_CODE_CSTRING: {
    char *cstr = (char *)(*item)->data;
    free(cstr);
  } break;
  case FLD_CODE_SNAPSHOT: {
    fld_variable_snapshot *field = (fld_variable_snapshot *)(*item)->data;

    if (field->type) {
      free(field->type);
      field->type = NULL;
    }
    if (field->mc_declared_type) {
      free(field->mc_declared_type);
      field->mc_declared_type = NULL;
    }
    if (field->name) {
      free(field->name);
      field->name = NULL;
    }
    if (field->value_text) {
      free(field->value_text);
      field->value_text = NULL;
    }

    field->line_index = 0;
    field->type_deref_count = 0U;
  } break;
  default: {
    MCerror(54, "TODO:%i", (*item)->type);
  }
  }

  free(*item);
  (*item) = NULL;

  return 0;
}

int fld_report_variable_snapshot(mc_code_editor_state_v1 *cestate, fld_variable_snapshot *field, void *p_value)
{
  if (!cestate->in_view_function_live_debugger) {
    return 0;
  }

  if (field->value_text) {
    free(field->value_text);
  }

  if (!field->mc_declared_type && !strcmp(field->type, "frame_time") && field->type_deref_count == 1) {
    frame_time *ft = (frame_time *)(*(void **)p_value);
    cprintf(field->value_text, "[%s:%lds %ldus]", field->name, ft->app_secs, ft->frame_nsecs / 1000);
  }
  else if (!field->mc_declared_type && !strcmp(field->type, "int") && field->type_deref_count == 0) {
    int val = (*(int *)p_value);
    cprintf(field->value_text, "[%s:%i]", field->name, val);
  }
  else {
    cprintf(field->value_text, "[%s:%s]", field->name, p_value ? field->type : "null");
  }

  // Notify of render update
  cestate->visual_node->data.visual.requires_render_update = true;

  return 0;
}

int fld_construct_variable_snapshot(const char *type_name, const char *mc_declared_type_name, uint type_deref_count,
                                    const char *variable_name, uint line_index, fld_variable_snapshot **field_snapshot)
{
  fld_variable_snapshot *field = (fld_variable_snapshot *)malloc(sizeof(fld_variable_snapshot));
  allocate_and_copy_cstr(field->type, type_name);
  allocate_and_copy_cstr(field->mc_declared_type, mc_declared_type_name);
  field->type_deref_count = type_deref_count;
  allocate_and_copy_cstr(field->name, variable_name);
  char buf[64];
  sprintf(buf, "%s:[--]", variable_name);
  allocate_and_copy_cstr(field->value_text, buf);
  field->line_index = line_index;

  *field_snapshot = field;

  return 0;
}

int fld_append_visual_code(fld_view_state *fld_view, const char *display_code)
{
  // Attempt to append to previous
  // if (fld_view->visual_code.count > 0 &&
  //     fld_view->visual_code.items[fld_view->visual_code.count - 1]->type == FLD_CODE_CSTRING) {

  //   char *new_string = (char *)malloc(
  //       sizeof(char) * (strlen((char *)fld_view->visual_code.items[fld_view->visual_code.count - 1]->data) +
  //                       strlen(display_code) + 1));
  //   strcpy(new_string, (char *)fld_view->visual_code.items[fld_view->visual_code.count - 1]->data);
  //   strcat(new_string, display_code);

  //   free(fld_view->visual_code.items[fld_view->visual_code.count - 1]->data);
  //   fld_view->visual_code.items[fld_view->visual_code.count - 1]->data = new_string;
  //   return 0;
  // }

  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_CSTRING;
  allocate_and_copy_cstr(element->data, display_code);

  MCcall(append_to_collection((void ***)&fld_view->visual_code.items, &fld_view->visual_code.alloc,
                              &fld_view->visual_code.count, element));

  return 0;
}

int fld_append_variable_snapshot(fld_view_state *fld_view, fld_variable_snapshot *field_snapshot)
{
  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_SNAPSHOT;
  element->data = field_snapshot;

  MCcall(append_to_collection((void ***)&fld_view->visual_code.items, &fld_view->visual_code.alloc,
                              &fld_view->visual_code.count, element));

  return 0;
}

int obtain_syntax_node_parsed_code(mc_syntax_node *syntax_node, c_str *cstr)
{
  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    if ((int)child->type > (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

      MCcall(obtain_syntax_node_parsed_code(child, cstr));
    }
    else {

      MCcall(append_to_c_str(cstr, child->text));
    }
  }

  return 0;
}

typedef struct fld_member_access_sequence {
  char *identity;
  fld_member_access_sequence *next;
  bool is_pointer_access;
} fld_member_access_sequence;

void release_fld_member_access_sequence(fld_member_access_sequence *mas)
{
  if (mas) {
    if (mas->next) {
      release_fld_member_access_sequence(mas->next);
    }

    if (mas->identity) {
      free(mas->identity);
    }

    free(mas);
  }
}

int fld_translate_identity_to_member_access_sequence(char *member_access_str,
                                                     fld_member_access_sequence **member_identity_sequence)
{
  fld_member_access_sequence *part = (fld_member_access_sequence *)malloc(sizeof(fld_member_access_sequence));

  int i = 0;
  while (isalnum(member_access_str[i]) || member_access_str[i] == '_') {
    ++i;
  }
  allocate_and_copy_cstrn(part->identity, member_access_str, i);

  if (i < 1) {
    MCerror(688, "str format error");
  }

  if (member_access_str[i] != '\0') {
    int offset = 0;
    if (member_access_str[i] == '.') {
      part->is_pointer_access = false;
      offset = 1;
    }
    else if (member_access_str[i] == '-' && member_access_str[i + 1] == '>') {
      part->is_pointer_access = true;
      offset = 2;
    }
    else {
      MCerror(683, "Incorrectly formatted string:'%s'", member_access_str);
    }

    MCcall(fld_translate_identity_to_member_access_sequence(member_access_str + i + offset, &part->next));
  }
  else {
    part->next = NULL;
    part->is_pointer_access = false;
  }

  *member_identity_sequence = part;

  return 0;
}

typedef struct fld_transcription_state {
  int scope_depth;
  mc_syntax_node_list locals;
} fld_transcription_state;

int fld_count_dereference_in_syntax_node(mc_syntax_node *syntax_node, uint *deref_count)
{
  *deref_count = 0;
  printf("here-42 %p\n", syntax_node);
  printf("here-43 %p\n", syntax_node->children);
  for (int a = 0; a < syntax_node->children->count; ++a) {
    printf("here-44\n");
    mc_syntax_node *child = syntax_node->children->items[a];
    printf("here-46\n");

    if ((mc_token_type)child->type == MC_TOKEN_STAR_CHARACTER) {
      printf("here-47\n");
      ++*deref_count;
    }
    printf("here-48\n");
  }

  printf("here-49\n");
  return 0;
}

typedef struct fld_type_info {
  bool is_mc_struct;
  union {
    struct_info *mc_struct_info;
    char *type_name;
  };
} fld_type_info;

void release_fld_type_info(fld_type_info *ptr)
{
  if (ptr) {
    if (!ptr->is_mc_struct) {
      if (ptr->type_name) {
        free(ptr->type_name);
      }
    }

    free(ptr);
  }
}

int fld_obtain_member_variable_type(struct_info *root_type, fld_member_access_sequence *variable_sequence,
                                    fld_type_info **variable_type)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (!variable_sequence->next) {
    *variable_type = (fld_type_info *)malloc(sizeof(fld_type_info));
    (*variable_type)->is_mc_struct = true;
    (*variable_type)->mc_struct_info = root_type;
    return 0;
  }

  // Find the member in the struct info
  mc_parameter_info_v1 *field;
  for (int a = 0; a < root_type->field_count; ++a) {
    if (!strcmp(variable_sequence->next->identity, root_type->fields[a]->name)) {
      // Attempt to obtain the type
      char *member_primitive_type_name = NULL;
      if (!strcmp(root_type->fields[a]->type_name, "int")) {
        allocate_and_copy_cstr(member_primitive_type_name, "int");
      }
      if (member_primitive_type_name) {
        // Found
        *variable_type = (fld_type_info *)malloc(sizeof(fld_type_info));
        (*variable_type)->is_mc_struct = false;
        (*variable_type)->type_name = member_primitive_type_name;
        return 0;
      }

      mc_struct_info_v1 *member_type_info = NULL;
      {
        void *vargs[3];
        vargs[0] = &command_hub->nodespace;
        vargs[1] = &member_primitive_type_name;
        vargs[2] = &member_type_info;
        MCcall(find_struct_info(3, vargs));
      }
      if (member_type_info) {
        MCcall(fld_obtain_member_variable_type(member_type_info, variable_sequence->next, variable_type));
        return 0;
      }

      MCerror(795, "Unhandled:'%s'", root_type->fields[a]->type_name);
    }
  }

  MCerror(799, "couldn't find field for member:'%s'", variable_sequence->next->identity);
}

int fld_transcribe_syntax_node(mc_code_editor_state_v1 *cestate, c_str *debug_declaration,
                               fld_transcription_state *transcription_state, mc_syntax_node *syntax_node)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  fld_view_state *fld_view = cestate->fld_view;

  switch (syntax_node->type) {
  case MC_SYNTAX_BLOCK: {
    if (transcription_state->scope_depth > 0) {
      if ((int)syntax_node->children->items[0]->type != (int)MC_TOKEN_CURLY_OPENING_BRACKET) {
        MCerror(641, "AST Format Error");
      }

      MCcall(append_to_c_str(debug_declaration, "{\n"));
      MCcall(fld_append_visual_code(fld_view, "{\n"));
    }

    // Children
    {
      ++transcription_state->scope_depth;
      for (int a = 1; a < syntax_node->children->count - 1; ++a) {
        mc_syntax_node *child = syntax_node->children->items[a];

        if ((int)child->type > (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

          MCcall(fld_transcribe_syntax_node(cestate, debug_declaration, transcription_state, child));
        }
        else {
          // printf("ptr-text:%p\n", child->text);
          // printf("-text:'%s'\n", child->text);

          MCcall(append_to_c_str(debug_declaration, child->text));
          MCcall(fld_append_visual_code(fld_view, child->text));
        }
      }
      --transcription_state->scope_depth;
    }

    // MCcall(append_to_c_str(debug_declaration, "  printf(\"NO! no. this. <()>\\n\");\n  return 0;\n"));
    // MCcall(fld_append_visual_code(fld_view, "  printf(\"NO! no. this. <()>\\n\");\n  return 0;\n"));
    if (transcription_state->scope_depth > 0) {
      if ((int)syntax_node->children->items[syntax_node->children->count - 1]->type !=
          (int)MC_TOKEN_CURLY_CLOSING_BRACKET) {
        MCerror(671, "AST Format Error");
      }

      MCcall(append_to_c_str(debug_declaration, "}"));
      MCcall(fld_append_visual_code(fld_view, "}"));
    }
  } break;
  // case MC_SYNTAX_LOCAL_VA: {
  //   // Type
  //   if (syntax_node->local_declaration.mc_type) {
  //     MCcall(append_to_c_str(debug_declaration, syntax_node->local_declaration.mc_type->declared_mc_name));
  //   }
  //   else {
  //     MCcall(append_to_c_str(debug_declaration, syntax_node->local_declaration.type_identifier->text));
  //   }
  //   MCcall(fld_append_visual_code(fld_view, syntax_node->local_declaration.type_identifier->text));

  //   // Rest
  //   for (int a = 1; a < syntax_node->children->count; ++a) {
  //     mc_syntax_node *child = syntax_node->children->items[a];

  //     if ((int)child->type > (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

  //       MCcall(fld_transcribe_syntax_node(cestate, debug_declaration, transcription_state, child));
  //     }
  //     else {
  //       // printf("ptr-text:%p\n", child->text);
  //       // printf("-text:'%s'\n", child->text);

  //       MCcall(append_to_c_str(debug_declaration, child->text));
  //       MCcall(fld_append_visual_code(fld_view, child->text));
  //     }
  //   }

  //   // fld_local_variable *local_variable = (fld_local_variable *)malloc(sizeof(fld_local_variable));
  //   // local_variable->type_deref_count =
  //   // allocate_and_copy_cstr(local_variable->name, syntax_node->local_declaration.variable_name);
  //   MCcall(append_to_collection((void ***)&transcription_state->locals.items, &transcription_state->locals.alloc,
  //                               &transcription_state->locals.count, syntax_node));
  // } break;
  // // case MC_SYNTAX_ASSIGNMENT_STATEMENT: {
  //   // Variable
  //   {
  //     c_str *variable_full_identity;
  //     MCcall(init_c_str(&variable_full_identity));
  //     MCcall(obtain_syntax_node_parsed_code(syntax_node->assignment.variable, variable_full_identity));

  //     fld_variable_snapshot *variable_snapshot = NULL;
  //     {
  //       // Attempt to take variable snapshot
  //       fld_member_access_sequence *variable_sequence;
  //       MCcall(fld_translate_identity_to_member_access_sequence(variable_full_identity->text, &variable_sequence));

  //       mc_syntax_node *declarator;
  //       for (int a = 0; a < transcription_state->locals.count; ++a) {
  //         if (!strcmp(variable_sequence->identity,
  //                     transcription_state->locals.items[a]->local_declaration.variable_name->text)) {

  //           // Found local declaration
  //           declarator = transcription_state->locals.items[a];
  //           break;
  //         }
  //       }

  //       // printf("here-99\n");
  //       if (declarator) {
  //         // Found type of variable root
  //         uint deref_count = 0;
  //         if (declarator->local_declaration.type_dereference) {
  //           MCcall(fld_count_dereference_in_syntax_node(declarator->local_declaration.type_dereference,
  //           &deref_count));
  //         }

  //         if (deref_count) {
  //           MCerror(859, "TODO");
  //         }
  //         else {
  //           // Determine a type info actually exists
  //           fld_type_info *variable_type;
  //           MCcall(fld_obtain_member_variable_type(declarator->local_declaration.mc_type, variable_sequence,
  //                                                  &variable_type));

  //           // printf("here-bvt\n");
  //           if (variable_type) {
  //             if (variable_type->is_mc_struct) {
  //               MCcall(fld_construct_variable_snapshot(
  //                   variable_type->mc_struct_info->name, variable_type->mc_struct_info->declared_mc_name,
  //                   deref_count, variable_full_identity->text, syntax_node->begin.line, &variable_snapshot));
  //             }
  //             else {
  //               MCcall(fld_construct_variable_snapshot(variable_type->type_name, NULL, deref_count,
  //                                                      variable_full_identity->text, syntax_node->begin.line,
  //                                                      &variable_snapshot));
  //             }

  //             release_fld_type_info(variable_type);
  //           }
  //         }
  //       }

  //       release_fld_member_access_sequence(variable_sequence);
  //     }

  //     // Set Variable
  //     if (variable_snapshot) {
  //       MCcall(fld_append_variable_snapshot(fld_view, variable_snapshot));

  //       MCvacall(append_to_c_strf(debug_declaration,
  //                                 "  MCcall(fld_report_variable_snapshot((mc_code_editor_state_v1 *)%p, "
  //                                 "(fld_variable_snapshot *)%p, &%s));\n ",
  //                                 cestate, variable_snapshot, variable_full_identity->text));
  //     }
  //     else {
  //       // Just print text
  //       MCcall(fld_append_visual_code(fld_view, variable_full_identity->text));
  //     }

  //     MCcall(append_to_c_str(debug_declaration, variable_full_identity->text));
  //     MCcall(release_c_str(variable_full_identity));
  //   }

  //   // Rest
  //   for (int a = 1; a < syntax_node->children->count; ++a) {
  //     mc_syntax_node *child = syntax_node->children->items[a];

  //     if ((int)child->type > (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

  //       MCcall(fld_transcribe_syntax_node(cestate, debug_declaration, transcription_state, child));
  //     }
  //     else {

  //       MCcall(append_to_c_str(debug_declaration, child->text));
  //       MCcall(fld_append_visual_code(fld_view, child->text));
  //     }
  //   }
  // } break;
  case MC_SYNTAX_INVOCATION:
  case MC_SYNTAX_SUPERNUMERARY:
  case MC_SYNTAX_DEREFERENCE_SEQUENCE:
  case MC_SYNTAX_MEMBER_ACCESS_EXPRESSION: {
    for (int a = 0; a < syntax_node->children->count; ++a) {
      mc_syntax_node *child = syntax_node->children->items[a];

      if ((int)child->type > (int)MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
        MCcall(fld_transcribe_syntax_node(cestate, debug_declaration, transcription_state, child));
      }
      else {
        // printf("ptr-text:%p\n", child->text);
        // printf("-text:'%s'\n", child->text);

        MCcall(append_to_c_str(debug_declaration, child->text));
        MCcall(fld_append_visual_code(fld_view, child->text));
      }
    }
  } break;
  default: {
    MCerror(636, "Unsupported syntax_node.type:%i", syntax_node->type);
  }
  }

  return 0;
}

int code_editor_begin_function_live_debug(mc_code_editor_state_v1 *cestate)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (cestate->source_data->type != SOURCE_DEFINITION_FUNCTION) {
    return 0;
  }

  function_info *function = cestate->source_data->func_info;

  // Begin
  printf("op01\n");
  printf("function '%s' code:\n%s\n", function->name, function->source->code);

  // Replace the function with the debug version && form the displayed code for the debug view
  c_str *debug_declaration;
  MCcall(init_c_str(&debug_declaration));
  // MCcall(append_to_c_strf(debug_declaration, "a %p %p %s c", debug_declaration, debug_declaration, "b"));
  // printf("cstr:'%s'\n", debug_declaration->text);
  // return 0;
  printf("op03\n");

  // -- Clear previous values
  for (int a = 0; a < cestate->fld_view->visual_code.count; ++a) {
    MCcall(free_fld_visual_code_element(&cestate->fld_view->visual_code.items[a]));
  }
  cestate->fld_view->visual_code.count = 0;

  // Debug function name
  char debug_function_name[128];
  sprintf(debug_function_name, "fld_%u_%s_v%u", cestate->fld_view->declare_incremental_uid++, function->name,
          function->latest_iteration);

  //
  printf("op04\n");
  MCvacall(append_to_c_strf(debug_declaration, "int %s(int argc, void **argv) {\n", debug_function_name));

  printf("op05\n");
  // printf("cstr: alloc=%u len=%u str=||%s||\n", debug_declaration->alloc, debug_declaration->len,
  //        debug_declaration->text);
  {
    char return_type_str[256];
    sprintf(return_type_str, "%s ", function->return_type.name);
    for (int i = 0; i < function->return_type.deref_count; ++i) {
      strcat(return_type_str, "*");
    }
    MCcall(fld_append_visual_code(cestate->fld_view, return_type_str));
    MCcall(fld_append_visual_code(cestate->fld_view, function->name));
    MCcall(fld_append_visual_code(cestate->fld_view, "("));
  }

  // Arguments
  {
    MCcall(append_to_c_str(debug_declaration, "  // Arguments\n"));
    for (int a = 0; a < function->parameter_count; ++a) {
      const char **p_utilized_type_name;
      if (function->parameters[a]->mc_declared_type) {
        p_utilized_type_name = &function->parameters[a]->mc_declared_type;
      }
      else {
        p_utilized_type_name = &function->parameters[a]->type_name;
      }
      char deref_buf[16];
      for (int d = 0; d < function->parameters[a]->type_deref_count; ++d) {
        deref_buf[d] = '*';
      }
      deref_buf[function->parameters[a]->type_deref_count] = '\0';

      MCvacall(append_to_c_strf(debug_declaration, "  %s %s%s = *(%s %s*)argv[%i];\n", *p_utilized_type_name, deref_buf,
                                function->parameters[a]->name, *p_utilized_type_name, deref_buf, a));
      printf("op03\n");

      // Parameter Type
      if (a > 0) {
        MCcall(fld_append_visual_code(cestate->fld_view, ", "));
      }
      MCcall(fld_append_visual_code(cestate->fld_view, function->parameters[a]->type_name));
      MCcall(fld_append_visual_code(cestate->fld_view, " "));
      MCcall(fld_append_visual_code(cestate->fld_view, deref_buf));

      // Live Argument
      fld_variable_snapshot *argument_snapshot;
      MCcall(fld_construct_variable_snapshot(
          function->parameters[a]->type_name, function->parameters[a]->mc_declared_type,
          function->parameters[a]->type_deref_count, function->parameters[a]->name, 0, &argument_snapshot));
      MCcall(fld_append_variable_snapshot(cestate->fld_view, argument_snapshot));

      printf("op06\n");
      // Report the value at call
      MCvacall(append_to_c_strf(debug_declaration,
                                "  MCcall(fld_report_variable_snapshot((mc_code_editor_state_v1 *)%p, "
                                "(fld_variable_snapshot *)%p, &%s));\n ",
                                cestate, argument_snapshot, function->parameters[a]->name));

      MCcall(append_to_collection((void ***)&cestate->fld_view->arguments.items, &cestate->fld_view->arguments.alloc,
                                  &cestate->fld_view->arguments.count, argument_snapshot));
      printf("op07\n");
    }

    MCcall(fld_append_visual_code(cestate->fld_view, ") {\n"));
    MCcall(append_to_c_str(debug_declaration, "\n"));
  }

  //
  fld_transcription_state transcription_state;
  transcription_state.scope_depth = 0;
  transcription_state.locals.alloc = 0;
  transcription_state.locals.count = 0;

  MCerror(1141, "Redo once finished");
  // MCcall(fld_transcribe_syntax_node(cestate, debug_declaration, &transcription_state,
  //                                   cestate->source_interpretation.function_ast));

  // MCcall(append_to_c_str(debug_declaration, "  printf(\"this instead!\\n\");\n"
  //                                             "\n"
  //                                             "  return 0;\n"
  //                                             "}"));
  MCcall(append_to_c_str(debug_declaration, "  return 0;\n}"));
  MCcall(fld_append_visual_code(cestate->fld_view, "}"));
  printf("lfild-func-decl:\n%s\n##########\n", debug_declaration->text);
  MCcall(clint_declare(debug_declaration->text));
  release_c_str(debug_declaration, true);

  char decl_buf[256];
  sprintf(decl_buf,
          "*((int (***)(int, void **))%p) = &%s;\n"
          "*((int (**)(int, void **))%p) = %s;\n"
          "%s = &%s;",
          &cestate->fld_view->ptr_function_ptr, function->name, &cestate->fld_view->previous_function_address,
          function->name, function->name, debug_function_name);
  MCcall(clint_process(decl_buf));

  // Re-render
  cestate->visual_node->data.visual.requires_render_update = true;

  return 0;
}

int code_editor_evaluate_syntax(mc_code_editor_state_v1 *cestate)
{
  // printf("cees-0\n");
  char *cstr;
  MCcall(read_editor_text_into_cstr(cestate, &cstr));

  if (cestate->source_data->type != SOURCE_DEFINITION_FUNCTION) {
    return 0;
  }
  // printf("cees-1\n");

  // Free the current syntax tree
  // cestate->syntax_tree

  // // Move to the code block
  // int i;
  // for (i = 0;; ++i) {
  //   if (cstr[i] == '\0') {
  //     MCerror(957, "TODO");
  //   }
  //   if (cstr[i] == '{') {
  //     break;
  //   }
  // }

  // if (cestate->edit_ast) {
  //   // print_syntax_node(cestate->source_interpretation.function_ast, 0);
  //   release_syntax_node(cestate->source_interpretation.function_ast);
  //   cestate->source_interpretation.function_ast = NULL;
  // }

  mc_syntax_node *code_syntax;
  int result = parse_mc_to_syntax_tree(cstr, &code_syntax, false);
  // MCcall(parse_mc_to_syntax_tree(cstr, &cestate->source_interpretation.function_ast));
  // printf("cees-2\n");
  if (result) {
    //   // printf("cees-3\n");
    //   release_syntax_node(cestate->source_interpretation.function_ast);
    //   cestate->source_interpretation.function_ast = NULL;

    //   // printf("cees-4\n");
    cprintf(cestate->status_bar.message, "ERR[%i]: read console output", result);
    cestate->status_bar.requires_render_update = true;
    //   // printf("cees-5\n");
  }
  else {

    if (cestate->code.syntax) {
      release_syntax_node(cestate->code.syntax);
    }
    cestate->code.syntax = code_syntax;

    allocate_and_copy_cstr(cestate->status_bar.message, "");
    cestate->status_bar.requires_render_update = true;
  }
  // printf("cees-7\n");

  return 0;
}

int code_editor_toggle_view_v1(mc_code_editor_state_v1 *state)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (state->source_data->type != SOURCE_DEFINITION_FUNCTION) {
    return 0;
  }

  if (state->in_view_function_live_debugger) {
    state->in_view_function_live_debugger = false;

    *state->fld_view->ptr_function_ptr = state->fld_view->previous_function_address;
    state->fld_view->ptr_function_ptr = NULL;
    state->fld_view->previous_function_address = NULL;

    // MCcall(code_editor_clear_function_live_debug_view(state->fld_view));
    state->visual_node->data.visual.requires_render_update = true;
  }
  else {
    state->in_view_function_live_debugger = true;

    MCcall(code_editor_evaluate_syntax(state));
    MCcall(code_editor_begin_function_live_debug(state));
    state->visual_node->data.visual.requires_render_update = true;
  }

  return 0;
}

int ce_update_syntax_rendered_lines(mc_code_editor_state_v1 *cestate)
{
  char *code = cestate->code.rtf->text;
  for (int i = 0;; ++i) {
    // Next char
    bool eof = false;
    switch (code[i]) {
    case '\0': {
      eof = true;
    } break;
    default:
      continue;
    }

    if (eof) {
      break;
    }
  }

  return 0;
}

int ce_update_txt_rendered_lines(mc_code_editor_state_v1 *cestate)
{
  c_str *line_text;
  MCcall(init_c_str(&line_text));

  // const char *code = cestate->editor_text->text;
  // while (1) {
  //   // Move through the text to identify the end of any line (or the text)
  //   int s = i;
  //   bool eof = false;
  //   while (1) {
  //     if (code[i] == '\0') {
  //       eof = true;
  //       break;
  //     }
  //     else if (code[i] == '\n') {
  //       break;
  //     }
  //     ++i;
  //   }

  //   if (i - s) {
  //   }
  // }

  // for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
  //   if (cestate->line_display_offset + i < cestate->text->lines_count) {
  //     // printf("life-6a\n");
  //     if (cestate->render_lines[i]->text) {
  //       // printf("life-6b\n");
  //       cestate->render_lines[i]->requires_render_update =
  //           cestate->render_lines[i]->requires_render_update ||
  //           strcmp(cestate->render_lines[i]->text, cestate->text->lines[cestate->line_display_offset + i]);
  //       // printf("life-6c\n");
  //       free(cestate->render_lines[i]->text);
  //       // printf("life-6d\n");
  //     }
  //     else {
  //       // printf("life-6e\n");
  //       // printf("dawn:%i %i\n", cestate->line_display_offset + i, cestate->text->lines_count);
  //       // printf("dawn:%i\n", cestate->text->lines_alloc);
  //       // printf("dawn:%c\n", cestate->text->lines[1][4]);
  //       // printf("dawn:%zu\n", strlen(cestate->text->lines[cestate->line_display_offset + i]));
  //       cestate->render_lines[i]->requires_render_update =
  //           cestate->render_lines[i]->requires_render_update ||
  //           !cestate->text->lines[cestate->line_display_offset + i] ||
  //           strlen(cestate->text->lines[cestate->line_display_offset + i]);
  //     }

  //     // printf("life-6f\n");
  //     // Assign
  //     allocate_and_copy_cstr(cestate->render_lines[i]->text, cestate->text->lines[cestate->line_display_offset +
  //     i]);
  //     // printf("life-6g\n");
  //   }
  //   else {
  //     // printf("life-6h\n");
  //     if (cestate->render_lines[i]->text) {
  //       // printf("life-6i\n");
  //       cestate->render_lines[i]->requires_render_update = true;
  //       free(cestate->render_lines[i]->text);
  //       // printf("life-6j\n");
  //       cestate->render_lines[i]->text = NULL;
  //     }
  //   }
  //   // printf("life-6k\n");
  // }

  return 0;
}

int obtain_context_node_for_cursor(mc_syntax_node *syntax_node, mc_code_editor_state_v1 *cestate,
                                   mc_syntax_node **context_node)
{
  if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {
    *context_node = syntax_node->parent;
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
    *context_node = NULL;
    return 0;
  }

  MCcall(obtain_context_node_for_cursor(child_floor, cestate, context_node));

  return 0;
}

int update_code_editor_suggestion(mc_code_editor_state_v1 *cestate)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // Get the complete word surrounding the cursor
  char *code = cestate->code.rtf->text;
  int s = cestate->cursor.rtf_index;
  while (s > 0) {
    --s;
    bool brk = false;
    switch (code[s]) {
    case ' ':
    case '\n':
    case ';':
    case '[':
    case ')':
    case ',': {
      brk = true;
      ++s;
    } break;
    case ']': {
      // Discover whether it is a rtf attribute or code access operator close
      bool rtf_attribute = false;
      // for(int r = s - 1; r >= 0; --r) {
      //   if(code[r] == '[') {
      //     rtf_attribute = code[r - 1] != '[';
      //   }
      // }
      // if(rtf_attribute) {

      // }
      brk = true;
      ++s;
    } break;
    default:
      break;
    }
    if (brk) {
      break;
    }
  }

  char *whole_word;
  allocate_and_copy_cstrn(whole_word, code + s, cestate->cursor.rtf_index - s);
  int whole_word_len = strlen(whole_word);

  cestate->suggestion_box.visible = false;
  for (int i = 0; i < cestate->suggestion_box.entries.count; ++i) {
    if (cestate->suggestion_box.entries.items[i]) {
      free(cestate->suggestion_box.entries.items[i]);
    }
  }
  cestate->suggestion_box.entries.count = 0;

  if (whole_word_len < 1) {
    return 0;
  }
  printf("whole_word:'%s'\n", whole_word);

  // Search for it in midge structs
  for (int a = 0; a < command_hub->global_node->struct_count; ++a) {
    if (cestate->suggestion_box.entries.count > 6) {
      break;
    }

    bool match = true;

    int w = 0;
    char *struct_name = command_hub->global_node->structs[a]->name;
    int struct_name_len = strlen(struct_name);
    for (int s = 0; s < struct_name_len && w < whole_word_len; ++s) {
      if (whole_word[w] == struct_name[s]) {
        ++w;
      }
    }
    if (w < whole_word_len) {
      continue;
    }

    cestate->suggestion_box.visible = true;
    cestate->suggestion_box.requires_render_update = true;
    allocate_and_copy_cstr(cestate->suggestion_box.entries.items[cestate->suggestion_box.entries.count], struct_name);
    ++cestate->suggestion_box.entries.count;

    // if (!strcmp(struct_name, "special_data")) {
    //   // Context
    //   mc_syntax_node *context_node;
    //   MCcall(obtain_context_node_for_cursor(cestate->code.syntax, cestate, &context_node));

    //   printf("context:%s\n", get_mc_syntax_token_type_name(context_node->type));

    //   if (context_node && (context_node->type == MC_SYNTAX_STATEMENT_LIST || context_node->type == MC_SYNTAX_BLOCK))
    //   {

    //     cestate->suggestion_box.visible = true;
    //     cestate->suggestion_box.requires_render_update = true;
    //     cprintf(cestate->suggestion_box.entries.items[cestate->suggestion_box.entries.count], "~init %s",
    //     struct_name);
    //     ++cestate->suggestion_box.entries.count;
    //   }
    // }

    // printf("struct:'%s':%s\n", command_hub->global_node->structs[a]->name, "match");
  }

  return 0;
}

int update_code_editor_cursor_line_and_column(mc_code_editor_state_v1 *cestate)
{
  cestate->cursor.line = 0;
  cestate->cursor.col = 0;

  char *code = cestate->code.rtf->text;
  for (int i = 0; code[i] != '\0' && i < cestate->cursor.rtf_index; ++i) {
    if (code[i] == '\n') {
      ++cestate->cursor.line;
      cestate->cursor.col = 0;
      // print_parse_error(cestate->code.rtf->text, cestate->cursor.rtf_index, "uceclac", "beets");
      // printf("line:%i col:%i i:%i\n", cestate->cursor.line, cestate->cursor.col, i);
    }
    else if (code[i] == '[') {
      if (code[i + 1] == '[') {
        ++i;
        ++cestate->cursor.col;
      }
      else {
        for (;; ++i) {
          if (code[i] == '\0') {
            MCerror(1452, "TODO");
          }
          else if (code[i] == ']') {
            break;
          }
        }
      }
    }
    else {
      // Any char
      ++cestate->cursor.col;
    }
  }

  // print_parse_error(cestate->code.rtf->text, cestate->cursor.rtf_index, "uceclac", "");
  // printf("line:%i col:%i index:%i\n", cestate->cursor.line, cestate->cursor.col, cestate->cursor.rtf_index);

  return 0;
}

int mce_update_rendered_text(mc_code_editor_state_v1 *cestate)
{
  register_midge_error_tag("mce_update_rendered_text()");
  // Copies the rtf text to the rendered lines
  // printf("urt-0\n");
  char *code = cestate->code.rtf->text;
  int i = 0;
  int s = i;
  int line_index = 0;
  int max_line_index = cestate->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES;
  char *color_from_previous_line = NULL;
  char *color_at_end_of_current_line = NULL;
  for (; i < cestate->code.rtf->len && line_index < max_line_index;) {
    // Obtain the line rtf
    bool eof = false;
    // printf("urt-1\n");
    for (;; ++i) {
      if (code[i] == '\0') {
        eof = true;
        break;
      }
      else if (code[i] == '\n') {
        break;
      }
      else if (code[i] == '[') {
        if (code[i + 1] == '[') {
          ++i;
          continue;
        }
        else if (!strncmp(code + i, "[color=", 7)) {
          int j = i + 7;
          for (;; ++j) {
            if (code[j] == ']') {
              ++j;
              break;
            }
          }
          if (color_at_end_of_current_line) {
            free(color_at_end_of_current_line);
          }
          allocate_and_copy_cstrn(color_at_end_of_current_line, code + i, j - i);
          i = j - 1;
          continue;
        }
        print_parse_error(code, i, "mce_update_rendered_text", "Unhandled rtf?");
        MCerror(1465, "Unhandled");
      }
    }
    // register_midge_error_tag("mce_update_rendered_text-4");
    // printf("urt-4\n");

    if (line_index >= cestate->line_display_offset) {
      rendered_code_line *rendered_code_line = cestate->render_lines[line_index - cestate->line_display_offset];
      rendered_code_line->visible = true;

      bool text_differs;
      if (color_from_previous_line) {
        int potential_line_len = (color_from_previous_line ? strlen(color_from_previous_line) : 0) + i - s;
        text_differs =
            rendered_code_line->rtf->len != potential_line_len ||
            strncmp(rendered_code_line->rtf->text, color_from_previous_line, strlen(color_from_previous_line)) ||
            strncmp(rendered_code_line->rtf->text + strlen(color_from_previous_line), code + s, s - i);
      }
      else {
        text_differs =
            rendered_code_line->rtf->len != (i - s) || strncmp(rendered_code_line->rtf->text, code + s, s - i);
      }

      if (text_differs) {
        // printf("urt-5a\n");
        // printf("line-%i was:'%s'\n", line_index - cestate->line_display_offset, rendered_code_line->rtf->text);

        // Set previous rtf settings
        MCcall(set_c_str(rendered_code_line->rtf, ""));
        if (color_from_previous_line) {
          MCcall(append_to_c_str(rendered_code_line->rtf, color_from_previous_line));
        }

        // printf("urt-6\n");
        // Set the line text
        MCcall(append_to_c_strn(rendered_code_line->rtf, code + s, i - s));
        rendered_code_line->requires_render_update = true;
        cestate->visual_node->data.visual.requires_render_update = true;

        // printf("line-%i now:'%s'\n", line_index - cestate->line_display_offset, rendered_code_line->rtf->text);
      }
    }
    // register_midge_error_tag("mce_update_rendered_text-7");
    // printf("urt-8\n");

    ++line_index;
    if (eof) {
      break;
    }
    ++i; // Past the new-line
    s = i;

    if (color_at_end_of_current_line) {
      if (color_from_previous_line) {
        free(color_from_previous_line);
      }
      color_from_previous_line = color_at_end_of_current_line;
      color_at_end_of_current_line = NULL;
    }
    // printf("urt-9\n");
  }

  // Turn off visibility of the rest of the lines
  if (line_index - cestate->line_display_offset < 0) {
    line_index -= line_index - cestate->line_display_offset;
  }
  for (; line_index - cestate->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES; ++line_index) {
    rendered_code_line *rendered_code_line = cestate->render_lines[line_index - cestate->line_display_offset];
    rendered_code_line->visible = false;
  }

  register_midge_error_tag("mce_update_rendered_text(~)");
  return 0;
}

int _mce_convert_syntax_node_to_rtf(c_str *rtf, mc_syntax_node *syntax_node)
{
  if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

    // RTF - prepend
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_LINE_COMMENT:
      MCcall(append_to_c_str(rtf, "[color=22,162,18]"));
      break;
    default:
      break;
    }

    // Content
    int s = 0;
    for (int i = 0;; ++i) {
      if (syntax_node->text[i] == '\0') {
        if (i - s) {
          MCcall(append_to_c_strn(rtf, syntax_node->text + s, i - s));
        }
        break;
      }
      else if (syntax_node->text[i] == '[') {
        // Copy to this point
        MCcall(append_to_c_strn(rtf, syntax_node->text + s, i - s));
        // MCcall(append_to_c_str(rtf, "["));
        MCcall(append_to_c_str(rtf, "[["));
        s = i + 1;
      }
    }

    // RTF - append
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_LINE_COMMENT:
      MCcall(append_to_c_str(rtf, "[color=156,219,253]"));
      break;
    default:
      break;
    }

    return 0;
  }

  // Syntax Node -- convert each child
  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    MCcall(_mce_convert_syntax_node_to_rtf(rtf, child));
  }

  return 0;
}

int mce_convert_syntax_to_rtf(c_str *code_rtf, mc_syntax_node *syntax_node)
{
  register_midge_error_tag("mce_convert_syntax_to_rtf()");
  const char *DEFAULT_CODE_COLOR = "[color=156,219,253]";

  MCcall(set_c_str(code_rtf, DEFAULT_CODE_COLOR));
  MCcall(_mce_convert_syntax_node_to_rtf(code_rtf, syntax_node));

  register_midge_error_tag("mce_convert_syntax_to_rtf(~)");
  return 0;
}

int code_editor_load_function(mc_code_editor_state_v1 *cestate, function_info *function)
{
  register_midge_error_tag("code_editor_load_function()");
  if (cestate->source_data->type != SOURCE_DEFINITION_FUNCTION) {
    MCerror(704, "TODO? :%i", cestate->source_data->type);
  }

  mc_syntax_node *code_syntax;
  int result = parse_mc_to_syntax_tree(function->source->code, &code_syntax, true);
  if (result) {
    // printf("cees-3  %i\n", result);
    //   release_syntax_node(cestate->source_interpretation.function_ast);
    //   cestate->source_interpretation.function_ast = NULL;

    // printf("cees-4\n");
    cprintf(cestate->status_bar.message, "ERR[%i]: read console output", result);
    cestate->status_bar.requires_render_update = true;
    // printf("cees-5\n");
    return 0;
  }
  else {
    // printf("cees-6\n");

    if (cestate->code.syntax) {
      printf("cees-6a\n");
      printf("cestate->code.syntax=%p\n", cestate->code.syntax);
      release_syntax_node(cestate->code.syntax);
    }
    cestate->code.syntax = code_syntax;
    // printf("cees-6b\n");

    cprintf(cestate->status_bar.message, "loaded %s(...)", function->name);
    cestate->status_bar.requires_render_update = true;
  }
  // printf("cees-7\n");

  // Set to cestate
  MCcall(mce_convert_syntax_to_rtf(cestate->code.rtf, cestate->code.syntax));
  cestate->code.syntax_updated = true;
  // printf("code.rtf:\n%s||\n", cestate->code.rtf->text);

  MCcall(mce_update_rendered_text(cestate));

  // printf("%p\n", cestate);
  // printf("code.rtf:\n%s||\n", cestate->code.rtf);

  // function_info *function =cestate->source_data->func_info;

  // for (int j = 0; j < cestate->text->lines_count; ++j) {
  //   free(cestate->text->lines[j]);
  //   allocate_and_copy_cstr(cestate->text->lines[j], "");
  // }
  // cestate->text->lines_count = 0;

  // // Line Alloc
  // uint line_alloc = 32;
  // char *line = (char *)malloc(sizeof(char) * line_alloc);
  // line[0] = '\0';

  // // Write the current signature
  // append_to_cstr(&line_alloc, &line, function->return_type.name);
  // append_to_cstr(&line_alloc, &line, " ");
  // for (int i = 0; i < function->return_type.deref_count; ++i) {
  //   append_to_cstr(&line_alloc, &line, "*");
  // }

  // append_to_cstr(&line_alloc, &line, function->name);
  // append_to_cstr(&line_alloc, &line, "(");

  // for (int i = 0; i < function->parameter_count; ++i) {
  //   if (i > 0) {
  //     append_to_cstr(&line_alloc, &line, ", ");
  //   }
  //   append_to_cstr(&line_alloc, &line, function->parameters[i]->type_name);
  //   append_to_cstr(&line_alloc, &line, " ");
  //   for (int j = 0; j < function->parameters[i]->type_deref_count; ++j)
  //     append_to_cstr(&line_alloc, &line, "*");
  //   append_to_cstr(&line_alloc, &line, function->parameters[i]->name);
  // }
  // append_to_cstr(&line_alloc, &line, ") {");
  // cestate->text->lines[cestate->text->lines_count++] = line;

  // Code Block
  // set_c_str(cestate->editor_text, cestate->source_data->code);
  // char *source_code = cestate->source_data->code;
  // {
  //   uint line_alloc = 1;
  //   char *line = (char *)malloc(sizeof(char) * line_alloc);
  //   line[0] = '\0';

  //   // Write the code in
  //   // Search for each line
  //   int i = 0;
  //   bool loop = true;
  //   bool wrapped_block = false;
  //   int s = i;
  //   for (; loop || !wrapped_block; ++i) {

  //     bool copy_line = false;
  //     switch (source_code[i]) {
  //     case '\0': {
  //       copy_line = true;
  //       loop = false;
  //     } break;
  //     case '\n': {
  //       copy_line = true;
  //     } break;
  //     default:
  //       break;
  //     }

  //     // printf("i-s=%i\n", i - s);
  //     if (copy_line || !loop) {
  //       // Transfer text to the buffer line
  //       if (i - s > 0) {
  //         append_to_cstrn(&line_alloc, &line, source_code + s, i - s);
  //       }
  //       else if (!loop && !strlen(line)) {
  //         wrapped_block = true;
  //         // append_to_cstr(&line_alloc, &line, "}");
  //       }

  //       // Add to the collection
  //       if (cestate->text->lines_count + 1 >= cestate->text->lines_alloc) {
  //         uint new_alloc = cestate->text->lines_alloc + 4 + cestate->text->lines_alloc / 4;
  //         char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
  //         if (cestate->text->lines_alloc) {
  //           memcpy(new_ary, cestate->text->lines, cestate->text->lines_alloc * sizeof(char *));
  //           free(cestate->text->lines);
  //         }
  //         for (int i = cestate->text->lines_alloc; i < new_alloc; ++i) {
  //           new_ary[i] = NULL;
  //         }

  //         cestate->text->lines_alloc = new_alloc;
  //         cestate->text->lines = new_ary;
  //       }

  //       cestate->text->lines[cestate->text->lines_count++] = line;
  //       // printf("Line:(%i:%i)>'%s'\n", cestate->text->lines_count - 1, i - s,
  //       // cestate->text->lines[cestate->text->lines_count - 1]); printf("strlen:%zu\n",
  //       // strlen(cestate->text->lines[cestate->text->lines_count - 1])); if (cestate->text->lines_count > 1)
  //       //   printf("dawn:%c\n", cestate->text->lines[1][4]);

  //       // Reset
  //       s = i + 1;
  //       line_alloc = 1;
  //       line = (char *)malloc(sizeof(char) * line_alloc);
  //       line[0] = '\0';
  //     }
  //   }
  // }
  // printf("life-6\n");

  // Set for render update
  // MCcall(update_rendered_code_lines(cestate));

  register_midge_error_tag("code_editor_load_function(~)");
  return 0;
}

int load_existing_function_into_code_editor_v1(int argc, void **argv)
{
  printf("life-begin\n");
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  function_info *function = *(function_info **)argv[0];

  // printf("life-begin\n");

  mc_node_v1 *code_editor;
  MCcall(obtain_subnode_with_name(command_hub->global_node, "code_editor", &code_editor));
  if (!code_editor) {
    MCerror(1860, "Could not find code editor");
  }

  // Begin Writing into the Function Editor textbox
  mc_code_editor_state_v1 *feState = (mc_code_editor_state_v1 *)code_editor->extra;
  feState->source_data = function->source;
  // printf("lefice:\n%s||\n", feState->source_data->code);

  feState->line_display_offset = 0;
  MCcall(code_editor_load_function(feState, function));

  // MCcall(code_editor_evaluate_syntax(feState));

  // printf("life-7\n");
  feState->cursor.line = 0; // min(feState->text->lines_count - 1, feState->cursor.line);
  feState->cursor.col = 0;  // min(feState->cursor.col, strlen(feState->text->lines[feState->cursor.line]));

  feState->selection_exists = false;

  // printf("life-7a\n");
  code_editor->data.visual.visible = true;
  code_editor->data.visual.requires_render_update = true;

  // process_editor_load(feState);

  // printf("ohfohe\n");

  return 0;
}