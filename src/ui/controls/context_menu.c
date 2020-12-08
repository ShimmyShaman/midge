#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mcu_determine_context_menu_extents(mc_node *node, layout_extent_restraints restraints)
{
  mcu_context_menu *context_menu = (mcu_context_menu *)node->data;

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
    mcu_button *button = context_menu->_buttons.items[a];

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

void __mcu_update_context_menu_layout(mc_node *node, mc_rectf *available_area)
{
  mcu_context_menu *context_menu = (mcu_context_menu *)node->data;

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

void __mcu_render_context_menu_headless(render_thread_info *render_thread, mc_node *node)
{
  mcu_context_menu *context_menu = (mcu_context_menu *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) = (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
    }
  }
}

void __mcu_render_context_menu_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_context_menu *context_menu = (mcu_context_menu *)node->data;

  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, context_menu->background_color);

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_presentation)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_presentation(image_render_queue, child);
    }
  }
}

void mcu_init_context_menu(mc_node *parent, mcu_context_menu **p_context_menu)
{
  // // Node
  // mc_node *node;
  // mca_init_mc_node(parent, NODE_TYPE_MCU_CONTEXT_MENU, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mcu_determine_context_menu_extents;
  node->layout->update_layout = (void *)&__mcu_update_context_menu_layout;
  node->layout->render_headless = (void *)&__mcu_render_context_menu_headless;
  node->layout->render_present = (void *)&__mcu_render_context_menu_present;

  // Control
  mcu_context_menu *context_menu = (mcu_context_menu *)malloc(sizeof(mcu_context_menu));
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

void mcu_context_menu_clear_options(mcu_context_menu *context_menu)
{
  for (int i = 0; i < context_menu->_buttons.count; ++i) {
    set_mc_str(context_menu->_buttons.items[i]->str, "");
    context_menu->_buttons.items[i]->node->layout->visible = false;
  }
  context_menu->_buttons.count = 0;
}

void mcu_context_menu_option_clicked(mci_input_event *input_event, mcu_button *button)
{
  // Get the context menu
  mc_node *context_menu_node = (mc_node *)button->node->parent;
  mcu_context_menu *context_menu = (mcu_context_menu *)context_menu_node->data;

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

void mcu_context_menu_add_option(mcu_context_menu *menu, const char *option_text)
{
  if (menu->_buttons.count + 1 > menu->_buttons.alloc) {

    int prev_alloc = menu->_buttons.alloc;
    reallocate_collection((void ***)&menu->_buttons.items, &menu->_buttons.alloc, 0, 0);

    for (int a = prev_alloc; a < menu->_buttons.alloc; ++a) {
      mcu_init_button(menu->node, &menu->_buttons.items[a]);
      menu->_buttons.items[a]->node->layout->visible = false;

      menu->_buttons.items[a]->node->layout->padding = {2, 2, 2, 2};

      // printf("mcu_temp_TODO:%p\n", *((void **)mcu_temp_TODO));
      // printf("mcu_temp_TODO:%p\n", mcu_temp_TODO);
      // printf("mcu_temp_TODO:%p\n", &mcu_temp_TODO);
      // void *bb = (void *)&mcu_temp_TODO;
      // printf("mcu_temp_TODO:%p\n", (void *)&bb);

      // DEBUG TODO (has to be set with void for now)
      menu->_buttons.items[a]->left_click = &mcu_context_menu_option_clicked;

      // menu->_buttons.items[a]->element->layout->preferred_width = 118.f;
      // menu->_buttons.items[a]->element->layout->preferred_height = 28.f;
    }
  }

  set_mc_str(menu->_buttons.items[menu->_buttons.count]->str, option_text);
  menu->_buttons.items[menu->_buttons.count]->node->layout->visible = true;
  ++menu->_buttons.count;
}