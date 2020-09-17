#include "render/render_thread.h"
#include "ui/ui_definitions.h"
#include <string.h>

void mui_init_button(mc_node *parent, mui_button **p_button)
{
  //   printf("mui-ib-0\n");
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // UI Element
  mui_ui_element *element;
  mui_init_ui_element(parent, UI_ELEMENT_BUTTON, &element);

  // Text Block
  mui_button *button = (mui_button *)malloc(sizeof(mui_button));
  button->element = element;
  element->data = button;

  //   printf("mui-ib-3\n");
  button->tag = NULL;
  init_c_str(&button->str);
  button->font_resource_uid = 0;
  button->font_color = COLOR_GHOST_WHITE;

  button->background_color = COLOR_DIM_GRAY;

  // Set to out pointer
  *p_button = button;
}

void mui_render_button(image_render_queue *render_queue, mc_node *visual_node)
{
  // printf("mui_render_button\n");
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_button *button = (mui_button *)element->data;

  // Background
  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)element->layout->__bounds.x,
                                        (unsigned int)element->layout->__bounds.y,
                                        (unsigned int)element->layout->__bounds.width,
                                        (unsigned int)element->layout->__bounds.height, button->background_color);

  // Text
  printf("renderbutton- %u %u %s %u\n", (unsigned int)element->layout->__bounds.x,
         (unsigned int)element->layout->__bounds.y, button->str->text, button->font_resource_uid);
  mcr_issue_render_command_text(render_queue, (unsigned int)element->layout->__bounds.x,
                                (unsigned int)element->layout->__bounds.y, button->str->text, button->font_resource_uid,
                                button->font_color);
}