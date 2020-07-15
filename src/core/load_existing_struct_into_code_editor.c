/* load_existing_struct_into_code_editor.c */

#include "core/midge_core.h"

// [_mc_iteration=1]
void load_existing_struct_into_code_editor(mc_node_v1 *code_editor, mc_struct_info_v1 *p_struct_info)
{
  printf("load_existing_struct_into_code_editor()\n");

  // Set
  mc_code_editor_state_v1 *feState = (mc_code_editor_state_v1 *)code_editor->extra;
  feState->source_data_type = CODE_EDITOR_SOURCE_DATA_STRUCT;
  feState->source_data = (void *)p_struct_info;

  // Begin Writing into the Code Editor textbox
  for (int j = 0; j < feState->text->lines_count; ++j) {
    if (feState->text->lines[j]) {
      free(feState->text->lines[j]);
      feState->text->lines[j] = NULL;
    }
  }
  feState->text->lines_count = 0;

  // Prepare the collection
  if (feState->text->lines_count + 1 >= feState->text->lines_alloc) {
    uint new_alloc = feState->text->lines_alloc + 4 + feState->text->lines_alloc / 4;
    char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
    if (feState->text->lines_alloc) {
      memcpy(new_ary, feState->text->lines, feState->text->lines_alloc * sizeof(char *));
      free(feState->text->lines);
    }
    for (int i = feState->text->lines_alloc; i < new_alloc; ++i) {
      new_ary[i] = NULL;
    }

    feState->text->lines_alloc = new_alloc;
    feState->text->lines = new_ary;
  }

  // Line Alloc
  uint line_alloc = 2;
  char *line = (char *)malloc(sizeof(char) * line_alloc);
  line[0] = '\0';

  char buf[256];
  sprintf(buf, "struct %s {", p_struct_info->name);
  append_to_cstr(&line_alloc, &line, buf);
  feState->text->lines[feState->text->lines_count++] = line;

  for (int i = 0; i < p_struct_info->field_count; ++i) {
    line_alloc = 2;
    char *line = (char *)malloc(sizeof(char) * line_alloc);
    line[0] = '\0';

    mc_parameter_info_v1 *field = (mc_parameter_info_v1 *)p_struct_info->fields[i];
    append_to_cstr(&line_alloc, &line, "  ");
    append_to_cstr(&line_alloc, &line, field->type_name);
    append_to_cstr(&line_alloc, &line, " ");
    for (int j = 0; j < field->type_deref_count; ++j) {
      append_to_cstr(&line_alloc, &line, "*");
    }
    append_to_cstr(&line_alloc, &line, field->name);
    append_to_cstr(&line_alloc, &line, ";");

    // Add to the collection
    if (feState->text->lines_count + 1 >= feState->text->lines_alloc) {
      uint new_alloc = feState->text->lines_alloc + 4 + feState->text->lines_alloc / 4;
      char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
      if (feState->text->lines_alloc) {
        memcpy(new_ary, feState->text->lines, feState->text->lines_alloc * sizeof(char *));
        free(feState->text->lines);
      }
      for (int i = feState->text->lines_alloc; i < new_alloc; ++i) {
        new_ary[i] = NULL;
      }

      feState->text->lines_alloc = new_alloc;
      feState->text->lines = new_ary;
    }

    feState->text->lines[feState->text->lines_count++] = line;
  }

  line_alloc = 2;
  line = (char *)malloc(sizeof(char) * line_alloc);
  line[0] = '\0';
  append_to_cstr(&line_alloc, &line, "};");

  // Prepare the collection -- TODO refactor this code out (triplicated this function)
  if (feState->text->lines_count + 1 >= feState->text->lines_alloc) {
    uint new_alloc = feState->text->lines_alloc + 4 + feState->text->lines_alloc / 4;
    char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
    if (feState->text->lines_alloc) {
      memcpy(new_ary, feState->text->lines, feState->text->lines_alloc * sizeof(char *));
      free(feState->text->lines);
    }
    for (int i = feState->text->lines_alloc; i < new_alloc; ++i) {
      new_ary[i] = NULL;
    }

    feState->text->lines_alloc = new_alloc;
    feState->text->lines = new_ary;
  }

  feState->text->lines[feState->text->lines_count++] = line;

  {
    // Set for render update
    feState->line_display_offset = 0;
    for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
      if (feState->line_display_offset + i < feState->text->lines_count) {
        // printf("life-6a\n");
        if (feState->render_lines[i]->text) {
          // printf("life-6b\n");
          feState->render_lines[i]->requires_render_update =
              feState->render_lines[i]->requires_render_update ||
              strcmp(feState->render_lines[i]->text, feState->text->lines[feState->line_display_offset + i]);
          // printf("life-6c\n");
          free(feState->render_lines[i]->text);
          // printf("life-6d\n");
        }
        else {
          // printf("life-6e\n");
          // printf("dawn:%i %i\n", feState->line_display_offset + i, feState->text->lines_count);
          // printf("dawn:%i\n", feState->text->lines_alloc);
          // printf("dawn:%c\n", feState->text->lines[1][4]);
          // printf("dawn:%zu\n", strlen(feState->text->lines[feState->line_display_offset + i]));
          feState->render_lines[i]->requires_render_update =
              feState->render_lines[i]->requires_render_update ||
              !feState->text->lines[feState->line_display_offset + i] ||
              strlen(feState->text->lines[feState->line_display_offset + i]);
        }

        // printf("life-6f\n");
        // Assign
        allocate_and_copy_cstr(feState->render_lines[i]->text, feState->text->lines[feState->line_display_offset + i]);
        // printf("life-6g\n");
      }
      else {
        // printf("life-6h\n");
        if (feState->render_lines[i]->text) {
          // printf("life-6i\n");
          feState->render_lines[i]->requires_render_update = true;
          free(feState->render_lines[i]->text);
          // printf("life-6j\n");
          feState->render_lines[i]->text = NULL;
        }
      }
      // printf("life-6k\n");
    }

    // printf("life-7\n");
    feState->cursorLine = 1;
    feState->cursorCol = strlen(feState->text->lines[feState->cursorLine]);

    // printf("life-7a\n");
    code_editor->data.visual.hidden = false;
    code_editor->data.visual.requires_render_update = true;
  }

  printf("~load_existing_struct_into_code_editor()\n");
}