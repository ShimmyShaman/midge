#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mui_determine_context_menu_extents(mc_node *node, layout_extent_restraints restraints)
{
  mui_context_menu *context_menu = (mui_context_menu *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->determine_layout_extents) {
      // TODO fptr casting
      void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
          (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
      determine_layout_extents(child, restraints);
    }
  }

  // Determine
  float max_child_width = 0, cumulative_height = 0;
  for (int a = 0; a < context_menu->_buttons.count; ++a) {
    mui_button *button = context_menu->_buttons.items[a];

    if (button->node->layout->__bounds.width > max_child_width) {
      max_child_width = button->node->layout->padding.left + button->node->layout->__bounds.width +
                        button->node->layout->padding.right;
    }

    cumulative_height += button->node->layout->padding.top + button->node->layout->__bounds.height +
                         button->node->layout->padding.bottom;
  }

  mc_rectf new_bounds = node->layout->__bounds;
  if (node->layout->preferred_width) {
    new_bounds.width = node->layout->preferred_width;
  }
  else {
    new_bounds.width = max_child_width;
  }
  if (node->layout->preferred_height) {
    new_bounds.height = node->layout->preferred_height;
  }
  else {
    new_bounds.height = cumulative_height;
  }

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_layout_update(node);
  }
}

void __mui_update_context_menu_layout(mc_node *node, mc_rectf *available_area)
{
  mui_context_menu *context_menu = (mui_context_menu *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;
  new_bounds.x = available_area->x + node->layout->padding.left;
  new_bounds.y = available_area->y + node->layout->padding.top;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_rerender(node);
  }

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->update_layout) {
      // TODO fptr casting
      void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
      update_layout(child, &node->layout->__bounds);
    }
  }

  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
}

void __mui_render_context_menu_headless(mc_node *node)
{
  mui_context_menu *context_menu = (mui_context_menu *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }
}

void __mui_render_context_menu_present(image_render_request *render_queue, mc_node *node)
{
  mui_context_menu *context_menu = (mui_context_menu *)node->data;

  mcr_issue_render_command_colored_quad(render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, context_menu->background_color);

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_presentation)(image_render_request *, mc_node *) =
          (void (*)(image_render_request *, mc_node *))child->layout->render_present;
      render_node_presentation(render_queue, child);
    }
  }
}

void mui_init_context_menu(mc_node *parent, mui_context_menu **p_context_menu)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MUI_CONTEXT_MENU, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mui_determine_context_menu_extents;
  node->layout->update_layout = (void *)&__mui_update_context_menu_layout;
  node->layout->render_headless = (void *)&__mui_render_context_menu_headless;
  node->layout->render_present = (void *)&__mui_render_context_menu_present;

  // Control
  mui_context_menu *context_menu = (mui_context_menu *)malloc(sizeof(mui_context_menu));
  context_menu->node = node;
  node->data = context_menu;

  context_menu->background_color = COLOR_BURLY_WOOD;

  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->alloc = 0;
  node->children->count = 0;

  context_menu->_buttons.alloc = 0;
  context_menu->_buttons.count = 0;

  // Set to out pointer
  *p_context_menu = context_menu;
}

void mui_context_menu_clear_options(mui_context_menu *context_menu)
{
  for (int i = 0; i < context_menu->_buttons.count; ++i) {
    set_c_str(context_menu->_buttons.items[i]->str, "");
    context_menu->_buttons.items[i]->node->layout->visible = false;
  }
  context_menu->_buttons.count = 0;
}

void mui_context_menu_option_clicked(mui_button *button, mc_point click_point)
{
  // Get the context menu
  mc_node *context_menu_node = (mc_node *)button->node->parent;
  mui_context_menu *context_menu = (mui_context_menu *)context_menu_node->data;

  if (context_menu->option_selected) {
    // Fire the event handler
    // TODO fptr casting
    void (*option_selected_event)(const char *) = (void (*)(const char *))context_menu->option_selected;
    option_selected_event(button->str->text);
  }

  // Hide the context menu
  context_menu_node->layout->visible = false;
  mca_set_node_requires_rerender(context_menu_node);
}

void mui_context_menu_add_option(mui_context_menu *menu, const char *option_text)
{
  if (menu->_buttons.count + 1 > menu->_buttons.alloc) {

    int prev_alloc = menu->_buttons.alloc;
    reallocate_collection((void ***)&menu->_buttons.items, &menu->_buttons.alloc, 0, 0);

    for (int a = prev_alloc; a < menu->_buttons.alloc; ++a) {
      mui_init_button(menu->node, &menu->_buttons.items[a]);
      menu->_buttons.items[a]->node->layout->visible = false;

      menu->_buttons.items[a]->node->layout->padding = {2, 2, 2, 2};

      // printf("mui_temp_TODO:%p\n", *((void **)mui_temp_TODO));
      // printf("mui_temp_TODO:%p\n", mui_temp_TODO);
      // printf("mui_temp_TODO:%p\n", &mui_temp_TODO);
      // void *bb = (void *)&mui_temp_TODO;
      // printf("mui_temp_TODO:%p\n", (void *)&bb);

      // DEBUG TODO (has to be set with void for now)
      menu->_buttons.items[a]->left_click = (void *)&mui_context_menu_option_clicked;

      // menu->_buttons.items[a]->element->layout->preferred_width = 118.f;
      // menu->_buttons.items[a]->element->layout->preferred_height = 28.f;
    }
  }

  set_c_str(menu->_buttons.items[menu->_buttons.count]->str, option_text);
  menu->_buttons.items[menu->_buttons.count]->node->layout->visible = true;
  ++menu->_buttons.count;
}