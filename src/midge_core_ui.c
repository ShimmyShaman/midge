#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

typedef struct core_entry {
  struct {
    uint utilized_count;
    uint size;
    core_entry **items;
  } children;

  int type;
  void *data;

  bool collapsed;
} core_entry;

typedef struct core_display_state {
  uint font_resource_uid;
  node *visual_node;

  struct {
    unsigned int x, y, width, height;
  } function_button_bounds;
  struct {
    uint utilized_count;
    uint size;
    node **items;
  } entry_visual_nodes;

  core_entry *global_core_entry;
  int entry_display_offset;
  bool entries_require_render_update;

} core_display_state;

int initialize_entry_visual_nodes(node *core_display, core_display_state *cdstate)
{
  register_midge_error_tag("build_entry_visual_nodes()");
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  cdstate->entry_visual_nodes.size = RENDERED_CORE_ENTRIES;
  cdstate->entry_visual_nodes.utilized_count = 0;
  cdstate->entry_visual_nodes.items = (mc_node_v1 **)malloc(sizeof(mc_node_v1 *) * cdstate->entry_visual_nodes.size);

  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  for (int a = 0; a < RENDERED_CORE_ENTRIES; ++a) {
    // printf("bce-1\n");
    mc_node_v1 *core_entry = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);
    cdstate->entry_visual_nodes.items[a] = core_entry;
    MCcall(append_to_collection((void ***)&core_display->children, &core_display->children_alloc,
                                &core_display->child_count, core_entry));

    // printf("bce-2\n");
    core_entry->parent = core_display;
    cprintf(core_entry->name, "core_entry%i", a);
    core_entry->type = NODE_TYPE_VISUAL;

    // printf("bce-3\n");
    core_entry->data.visual.bounds.x = 0;
    core_entry->data.visual.bounds.y = 0;
    core_entry->data.visual.bounds.x = core_display->data.visual.bounds.x + 2;
    core_entry->data.visual.bounds.y = core_display->data.visual.bounds.y + 2 + a * 26;
    core_entry->data.visual.bounds.width = 296;
    core_entry->data.visual.bounds.height = 26;
    core_entry->data.visual.image_resource_uid = 0;
    core_entry->data.visual.requires_render_update = true;
    core_entry->data.visual.render_delegate = NULL;
    core_entry->data.visual.visible = true;
    core_entry->data.visual.input_handler = NULL; //&core_display_entry_handle_input;

    // printf("bce-5\n");
    resource_command *command;

    // Obtain visual resources
    // Function Editor Image
    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &core_entry->data.visual.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = core_entry->data.visual.bounds.width;
    command->data.create_texture.height = core_entry->data.visual.bounds.height;
  }
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  // printf("bce-8\n");
  register_midge_error_tag("build_entry_visual_nodes(~)");
  return 0;
}

int mcu_render_core_entry(core_display_state *cdstate, core_entry *entry, int indent)
{
  // printf("mcu_rce-0\n");
  register_midge_error_tag("mcu_render_core_entry()");
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  image_render_queue *sequence;
  element_render_command *element_cmd;

  // printf("mrce-0\n");

  node *child = (node *)cdstate->entry_visual_nodes.items[cdstate->entry_visual_nodes.utilized_count++];
  child->extra = entry;
  child->data.visual.visible = true;

  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = child->data.visual.bounds.width;
  sequence->image_height = child->data.visual.bounds.height;
  sequence->clear_color = COLOR_NEARLY_BLACK;
  sequence->data.target_image.image_uid = child->data.visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  element_cmd->x = 6;
  element_cmd->y = 18;
  element_cmd->data.print_text.font_resource_uid = cdstate->font_resource_uid;

  int indent_len = indent * 1; // + (indent ? 1 : 0);
  char indent_str[indent_len + 1];
  for (int a = 0; a < indent_len; ++a) {
    indent_str[a] = ' ';
  }
  indent_str[indent_len] = '\0';

  // printf("mrce-2 indent_len:%i indent_str='%s'\n", indent_len, indent_str);
  // printf("mrce-2\n");
  switch (entry->type) {
  case CORE_ENTRY_FUNCTION: {
    function_info *func_info = (function_info *)entry->data;
    cprintf(element_cmd->data.print_text.text, "%s%s", indent_str, func_info->name);
    if (func_info->source) {
      element_cmd->data.print_text.color = COLOR_FUNCTION_GREEN;
    }
    else {
      element_cmd->data.print_text.color = COLOR_FUNCTION_RED;
    }
  } break;
  case CORE_ENTRY_STRUCT: {
    // printf("mrce-4\n");
    //     printf("mrce-struct_info->name:%s\n", ((struct_info *)entry->data)->name);
    //     printf("mrce-struct_info->source->code:%p\n%s||\n", ((struct_info *)entry->data)->source->code,
    //            ((struct_info *)entry->data)->source->code);
    struct_info *str_info = (struct_info *)entry->data;
    cprintf(element_cmd->data.print_text.text, "%s%s", indent_str, str_info->name);
    element_cmd->data.print_text.color = COLOR_LIGHT_YELLOW;
  } break;
  case CORE_ENTRY_CATEGORY_STRUCT: {
    if (entry->collapsed) {
      cprintf(element_cmd->data.print_text.text, "%s+%s", indent_str, "structs");
    }
    else {
      cprintf(element_cmd->data.print_text.text, "%s-%s", indent_str, "structs");
    }
    element_cmd->data.print_text.color = COLOR_BURLY_WOOD;
  } break;
  case CORE_ENTRY_CATEGORY_FUNCTION: {
    if (entry->collapsed) {
      cprintf(element_cmd->data.print_text.text, "%s+%s", indent_str, "functions");
    }
    else {
      cprintf(element_cmd->data.print_text.text, "%s-%s", indent_str, "functions");
    }
    element_cmd->data.print_text.color = COLOR_BURLY_WOOD;
  } break;
  case CORE_ENTRY_CATEGORY_CHILDREN: {
    if (entry->collapsed) {
      cprintf(element_cmd->data.print_text.text, "%s+%s", indent_str, "nodes");
    }
    else {
      cprintf(element_cmd->data.print_text.text, "%s-%s", indent_str, "nodes");
    }
    element_cmd->data.print_text.color = COLOR_BURLY_WOOD;
  } break;
  case CORE_ENTRY_NODE: {
    mc_node_v1 *node_data = (mc_node_v1 *)entry->data;
    // printf("nodename:%s\n", node_data->name);
    if (entry->children.utilized_count) {
      if (entry->collapsed) {
        cprintf(element_cmd->data.print_text.text, "%s+%s", indent_str, node_data->name);
      }
      else {
        cprintf(element_cmd->data.print_text.text, "%s-%s", indent_str, node_data->name);
      }
    }
    else {
      cprintf(element_cmd->data.print_text.text, "%s %s", indent_str, node_data->name);
    }
    element_cmd->data.print_text.color = COLOR_NODE_ORANGE;
  } break;
  default: {
    MCerror(278, "Unsupported type:%i", entry->type);
  }
  }

  // printf("mrce-6\n");
  if (!entry->collapsed) {
    for (int b = 0; b < entry->children.utilized_count &&
                    cdstate->entry_visual_nodes.utilized_count + 1 < cdstate->entry_visual_nodes.size;
         ++b) {
      // printf("cdstate->entry_visual_nodes.utilized_count:%i size:%i\n", cdstate->entry_visual_nodes.utilized_count,
      //        cdstate->entry_visual_nodes.size);
      core_entry *subentry = entry->children.items[b];
      MCcall(mcu_render_core_entry(cdstate, subentry, indent + 1));
    }
  }

  // printf("mcu_rce-9\n");
  register_midge_error_tag("mcu_render_core_entry()");
  return 0;
}

int core_display_render_v1(int argc, void **argv)
{
  register_midge_error_tag("core_display_render_v1()");
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("core_display_render_v1()\n");
  frame_time *elapsed = *(frame_time **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  if (!visual_node->data.visual.visible)
    return 0;
  core_display_state *cdstate = (core_display_state *)visual_node->extra;

  image_render_queue *sequence;
  element_render_command *element_cmd;

  {
    cdstate->entry_visual_nodes.utilized_count = 0;
    MCcall(mcu_render_core_entry(cdstate, cdstate->global_core_entry, 0));

    // Hide the rest
    for (int a = cdstate->entry_visual_nodes.utilized_count; a < cdstate->entry_visual_nodes.size; ++a) {
      cdstate->entry_visual_nodes.items[a]->data.visual.visible = false;
    }

    cdstate->entries_require_render_update = false;
  }

  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = visual_node->data.visual.bounds.width;
  sequence->image_height = visual_node->data.visual.bounds.height;
  sequence->clear_color = COLOR_GHOST_WHITE;
  sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 2;
  element_cmd->y = 2;
  element_cmd->data.colored_rect_info.width = visual_node->data.visual.bounds.width - 4;
  element_cmd->data.colored_rect_info.height = visual_node->data.visual.bounds.height - 4;
  element_cmd->data.colored_rect_info.color = COLOR_NEARLY_BLACK;

  for (int i = 0; i < visual_node->child_count; ++i) {
    node *child = (node *)visual_node->children[i];

    if (!child->data.visual.visible) {
      continue;
    }

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
    element_cmd->x = child->data.visual.bounds.x;
    element_cmd->y = child->data.visual.bounds.y;
    element_cmd->data.textured_rect_info.width = child->data.visual.bounds.width;
    element_cmd->data.textured_rect_info.height = child->data.visual.bounds.height;
    element_cmd->data.textured_rect_info.texture_uid = child->data.visual.image_resource_uid;
  }

  // Function Button
  {
    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    // element_cmd->x = cdstate->function_button_bounds.x;
    // element_cmd->y = cdstate->function_button_bounds.y;
    // element_cmd->data.colored_rect_info.width = cdstate->function_button_bounds.width;
    // element_cmd->data.colored_rect_info.height = cdstate->function_button_bounds.height;
    // element_cmd->data.colored_rect_info.color = COLOR_PURPLE;

    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    // element_cmd->x = cdstate->function_button_bounds.x + 2;
    // element_cmd->y = cdstate->function_button_bounds.y + 2;
    // element_cmd->data.colored_rect_info.width = cdstate->function_button_bounds.width - 4;
    // element_cmd->data.colored_rect_info.height = cdstate->function_button_bounds.height - 4;
    // element_cmd->data.colored_rect_info.color = (render_color){0.03f, 0.33f, 0.03f, 1.f};

    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
    // element_cmd->x = cdstate->function_button_bounds.x + 2;
    // element_cmd->y = cdstate->function_button_bounds.y + 18;
    // element_cmd->data.print_text.font_resource_uid = cdstate->font_resource_uid;
    // const char *function_text = "function";
    // element_cmd->data.print_text.text = &function_text;
    // element_cmd->data.print_text.color = COLOR_GHOST_WHITE;
  }

  register_midge_error_tag("core_display_render_v1(~)");
  return 0;
}
