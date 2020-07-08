#include "core/midge_core.h"

void function_editor_handle_input_v1(frame_time *elapsed, mc_node_v1 *fedit, mc_input_event_v1 *event)
{
  printf("function_editor_handle_input-begin\n");

  if (fedit->data.visual.hidden) {
    return;
  }

  function_editor_state *state = (function_editor_state *)fedit->extra;

  event->handled = true;

  printf("fehi-3\n");
  if (event->type == INPUT_EVENT_MOUSE_PRESS) {
    if (event->detail.mouse.button == MOUSE_BUTTON_SCROLL_DOWN) {
      ++state->line_display_offset;
    }
    else if (event->detail.mouse.button == MOUSE_BUTTON_SCROLL_UP) {
      --state->line_display_offset;
    }
  }
  else if (event->type == INPUT_EVENT_KEY_PRESS) {
    function_editor_handle_keyboard_input(fedit, event);
  }
  else {
    return;
  }

  printf("fehi-4\n");
  // Update all modified rendered lines
  for (int i = 0; i < FUNCTION_EDITOR_RENDERED_CODE_LINES; ++i) {
    if (i + state->line_display_offset >= state->text.lines_count) {

      printf("fehi-5\n");
      if (!state->render_lines[i].text) {
        continue;
      }

      // printf("was:'%s' now:NULL\n", state->render_lines[i].text);
      free(state->render_lines[i].text);
      state->render_lines[i].text = NULL;
    }
    else {
      printf("fehi-6\n");
      if (state->render_lines[i].text &&
          !strcmp(state->render_lines[i].text, state->text.lines[i + state->line_display_offset])) {
        continue;
      }

      // printf("was:'%s' now:'%s'\n", state->render_lines[i].text, state->text.lines[i + state->line_display_offset]);
      // Update
      if (state->render_lines[i].text) {
        free(state->render_lines[i].text);
      }
      allocate_and_copy_cstr(state->render_lines[i].text, state->text.lines[i + state->line_display_offset]);
    }

    printf("fehi-7\n");
    state->render_lines[i].requires_render_update = true;
    fedit->data.visual.requires_render_update = true;
  }

  return;
}