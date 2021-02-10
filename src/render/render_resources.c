/* render_resources.c */

#include <stdio.h>
#include <string.h>

#include "stb/stb_truetype.h"

#include "core/midge_app.h"
#include "render/render_common.h"

int mcr_determine_text_display_dimensions(mcr_font_resource *font, const char *text, float *text_width,
                                          float *text_height)
{
  if (text == NULL || text[0] == '\0') {
    *text_width = 0;
    *text_height = 0;
    return 0;
  }

  midge_app_info *global_data;

  if (!font) {
    mc_obtain_midge_app_info(&global_data);

    // Use the global default font resource
    font = global_data->ui_state->default_font_resource;
  }

  *text_width = 0;
  *text_height = font->draw_vertical_offset;

  int text_length = strlen(text);
  for (int c = 0; c < text_length; ++c) {

    char letter = text[c];
    if (letter < 32 || letter > 127) {
      MCerror(7874, "TODO character '%i' not supported.\n", letter);
    }

    // Source texture bounds
    stbtt_aligned_quad q;

    // TODO -- newlines/tabs etc
    // printf("garbagein: %i %i %f %f %i\n", (int)font_image->width, (int)font_image->height, align_x, align_y, letter -
    // 32);

    stbtt_GetBakedQuad(font->char_data, 256, 256, letter - 32, text_width, text_height, &q, 1);
    // printf("char:'%c' w:%.2f h:%.2f\n", letter, *text_width, *text_height);
  }

  return 0;
}
