
#include "core/midge_core.h"

void ensure_core_entry_has_child_alloc(core_entry *entry, int size)
{
  // register_midge_error_tag("ensure_core_entry_has_child_alloc()");
  if (entry->children.size >= size) {
    register_midge_error_tag("ensure_core_entry_has_child_alloc(~enough)");
    return;
  }

  core_entry **new_ary = (core_entry **)malloc(sizeof(core_entry *) * size);

  if (entry->children.size) {
    memcpy(new_ary, entry->children.items, sizeof(core_entry *) * entry->children.size);
    free(entry->children.items);
  }
  for (int i = entry->children.size; i < size; ++i) {
    core_entry *child = (core_entry *)malloc(sizeof(core_entry));
    child->type = 0;
    child->data = NULL;
    child->collapsed = true;
    child->children.size = 0;
    child->children.utilized_count = 0;

    new_ary[i] = child;
  }

  entry->children.items = new_ary;
  entry->children.size = size;

  register_midge_error_tag("ensure_core_entry_has_child_alloc(~)");
}

void update_nodes_core_entry(core_display_state *cdstate, core_entry *entry)
{
  if (entry->type != CORE_ENTRY_NODE) {
    MCerror(184, "TODO");
  }

  mc_node_v1 *node_data = (mc_node_v1 *)entry->data;

  // Update each of its children
  // -- Allocate
  ensure_core_entry_has_child_alloc(entry, 3);
  entry->children.utilized_count = 0;

  // -- Structs
  // register_midge_error_tag("update_nodes_core_entry-2");
  if (node_data->struct_count) {
    core_entry *category_entry = entry->children.items[entry->children.utilized_count++];
    category_entry->type = CORE_ENTRY_CATEGORY_STRUCT;

    ensure_core_entry_has_child_alloc(category_entry, node_data->struct_count);
    category_entry->children.utilized_count = node_data->struct_count;
    // printf("category_entry:%p size:%i util%i\n", category_entry, category_entry->children.size,
    //        category_entry->children.utilized_count);

    // Set directly
    for (int i = 0; i < node_data->struct_count; ++i) {
      if (category_entry->children.items[i]->type != CORE_ENTRY_STRUCT ||
          category_entry->children.items[i]->data != node_data->structs[i]) {
        // Initialize
        cdstate->entries_require_render_update = true;
        cdstate->visual_node->data.visual.requires_render_update = true;

        category_entry->children.items[i]->type = CORE_ENTRY_STRUCT;
        category_entry->children.items[i]->data = node_data->structs[i];
        category_entry->children.items[i]->children.utilized_count = 0;
      }
    }
  }
  // register_midge_error_tag("update_nodes_core_entry-3");

  // -- Functions
  if (node_data->function_count) {
    core_entry *category_entry = entry->children.items[entry->children.utilized_count++];
    category_entry->type = CORE_ENTRY_CATEGORY_FUNCTION;

    ensure_core_entry_has_child_alloc(category_entry, node_data->function_count);
    category_entry->children.utilized_count = node_data->function_count;

    // Set directly
    for (int i = 0; i < node_data->function_count; ++i) {
      if (category_entry->children.items[i]->type != CORE_ENTRY_FUNCTION ||
          category_entry->children.items[i]->data != node_data->functions[i]) {
        // Initialize
        cdstate->entries_require_render_update = true;
        cdstate->visual_node->data.visual.requires_render_update = true;

        category_entry->children.items[i]->type = CORE_ENTRY_FUNCTION;
        category_entry->children.items[i]->data = node_data->functions[i];
        category_entry->children.items[i]->children.utilized_count = 0;
      }
    }
  }
  // register_midge_error_tag("update_nodes_core_entry-4");

  // -- Nodes
  if (node_data->child_count) {
    core_entry *category_entry = entry->children.items[entry->children.utilized_count++];
    if (category_entry->type != CORE_ENTRY_CATEGORY_CHILDREN) {
      category_entry->type = CORE_ENTRY_CATEGORY_CHILDREN;
      category_entry->collapsed = false;
    }

    ensure_core_entry_has_child_alloc(category_entry, node_data->child_count);
    category_entry->children.utilized_count = node_data->child_count;

    // Set directly
    for (int i = 0; i < node_data->child_count; ++i) {
      if (category_entry->children.items[i]->type != CORE_ENTRY_NODE ||
          category_entry->children.items[i]->data != node_data->children[i]) {
        // Initialize
        cdstate->entries_require_render_update = true;
        cdstate->visual_node->data.visual.requires_render_update = true;

        category_entry->children.items[i]->type = CORE_ENTRY_NODE;
        category_entry->children.items[i]->data = node_data->children[i];
      }

      // Update its children
      update_nodes_core_entry(cdstate, category_entry->children.items[i]);
    }
  }
  register_midge_error_tag("update_nodes_core_entry(~)");
}

void mcd_on_hierarchy_update(void *event_data)
{
  printf("hierarchy_update!\n");

  mc_node_v1 *hierarchy_display;
  MCcall(obtain_subnode_with_name(command_hub->global_node, CORE_OBJECTS_DISPLAY_NAME, &hierarchy_display));
  if (!hierarchy_display) {
    MCerror(1860, "Could not find hierarchy_display");
  }

  core_display_state *cdstate = (core_display_state *)hierarchy_display->extra;

  update_nodes_core_entry(cdstate, cdstate->global_core_entry);
  // register_midge_error_tag("mcd_on_hierarchy_update(~)");
}

void core_display_handle_input(frame_time *elapsed, mc_node_v1 *core_display, mc_input_event_v1 *event)
{
  if (!core_display->data.visual.visible) {
    return;
  }

  // printf("cdhi-1\n");

  core_display_state *cdstate = (core_display_state *)core_display->extra;

  // Function Button
  // if (event->detail.mouse.x >= cdstate->function_button_bounds.x &&
  //     event->detail.mouse.y >= cdstate->function_button_bounds.y &&
  //     event->detail.mouse.x < cdstate->function_button_bounds.x + cdstate->function_button_bounds.width &&
  //     event->detail.mouse.y < cdstate->function_button_bounds.y + cdstate->function_button_bounds.height) {
  //   MCcall(build_core_entry(core_display, "unnamed"));
  //   MCcall(update_core_entries(core_display));

  //   core_display->data.visual.requires_render_update = true;
  //   return 0;
  // }

  // Find the core entry being clicked on

  for (int i = 0; !event->handled && i < cdstate->entry_visual_nodes.size; ++i) {
    // printf("cdhi-2 %p\n", cdstate->entry_visual_nodes.items[i]);
    node *entry_node = cdstate->entry_visual_nodes.items[i];

    if (!entry_node->data.visual.visible) {
      continue;
    }

    // Check is visual and has input handler and mouse event is within bounds
    // if (!*child->data.visual.input_handler)
    //   continue;
    if (event->detail.mouse.x < entry_node->data.visual.bounds.x ||
        event->detail.mouse.y < entry_node->data.visual.bounds.y ||
        event->detail.mouse.x >= entry_node->data.visual.bounds.x + entry_node->data.visual.bounds.width ||
        event->detail.mouse.y >= entry_node->data.visual.bounds.y + entry_node->data.visual.bounds.height) {
      // printf("cdhi-2a\n");
      continue;
    }
    if (event->detail.mouse.button == 1) {
      printf("LeftClick> x:%u y:%u\n", event->detail.mouse.x, event->detail.mouse.y);
    }

    switch (event->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      // Left-Click
      if (event->detail.mouse.button == 1) {
        // Find the entry this node represents
        int ei = 0;
        core_entry *entry = (core_entry *)entry_node->extra;

        if (!entry) {
          continue;
        }
        event->handled = true;

        switch (entry->type) {
        case CORE_ENTRY_FUNCTION: {
          // printf("babel\n");
          // printf("entry->data:%i\n", ((mc_source_definition_v1 *)entry->data)->type);
          void *vargs[1];
          vargs[0] = (void **)&entry->data;
          load_existing_function_into_code_editor(1, vargs);
          // printf("fish\n");
        } break;
        case CORE_ENTRY_STRUCT: {
          // printf("cdhi-struct_info->name:%s\n", ((struct_info *)entry->data)->name);
          // printf("cdhi-struct_info->source->code:%p\n%s||\n", ((struct_info *)entry->data)->source->code,
          //        ((struct_info *)entry->data)->source->code);

          load_existing_struct_into_code_editor(&entry->data);
        } break;
        case CORE_ENTRY_NODE:
        case CORE_ENTRY_CATEGORY_STRUCT:
        case CORE_ENTRY_CATEGORY_FUNCTION:
        case CORE_ENTRY_CATEGORY_CHILDREN: {
          entry->collapsed = !entry->collapsed;
          cdstate->entries_require_render_update = true;
          core_display->data.visual.requires_render_update = true;
        } break;
        default: {
          MCerror(513, "Unsupported type:%i", entry->type);
        }
        }
      }
    } break;
    default: {
      break;
    }
    }
  }

  // printf("cdhi-9\n");
  return;
}

void build_core_display()
{
  // register_midge_error_tag("build_core_display_v1()");

  // printf("build_core_display_v1\n");
  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *core_objects_display = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);
  core_objects_display->name = CORE_OBJECTS_DISPLAY_NAME;
  core_objects_display->parent = command_hub->global_node;
  core_objects_display->type = NODE_TYPE_VISUAL;

  core_objects_display->child_count = 0;
  core_objects_display->children_alloc = 4;
  core_objects_display->children = (node **)malloc(sizeof(void *) * core_objects_display->children_alloc);

  core_objects_display->data.visual.bounds.x = 0;
  core_objects_display->data.visual.bounds.y = 0;
  core_objects_display->data.visual.bounds.width = 300;
  core_objects_display->data.visual.bounds.height = 900;
  core_objects_display->data.visual.image_resource_uid = 0;
  core_objects_display->data.visual.requires_render_update = true;
  core_objects_display->data.visual.render_delegate = &core_display_render;
  core_objects_display->data.visual.visible = true;
  core_objects_display->data.visual.input_handler = &core_display_handle_input;

  core_display_state *cdstate = (core_display_state *)malloc(sizeof(core_display_state));
  cdstate->visual_node = core_objects_display;
  core_objects_display->extra = cdstate;
  cdstate->entry_display_offset = 0;
  cdstate->font_resource_uid = 0;
  cdstate->global_core_entry = (core_entry *)malloc(sizeof(core_entry));
  cdstate->global_core_entry->type = CORE_ENTRY_NODE;
  cdstate->global_core_entry->data = command_hub->global_node;
  cdstate->global_core_entry->collapsed = false;
  cdstate->global_core_entry->children.size = 0;
  cdstate->global_core_entry->children.utilized_count = 0;

  append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                       &command_hub->global_node->child_count, core_objects_display);

  int event_type = ME_NODE_HIERARCHY_UPDATED;
  // printf("&mcd_on_hierarchy_update=%p\n", &mcd_on_hierarchy_update);
  add_notification_handler(command_hub->global_node, event_type, &mcd_on_hierarchy_update);

  // printf("bcd-0\n");
  initialize_entry_visual_nodes(core_objects_display, cdstate);

  update_nodes_core_entry(cdstate, cdstate->global_core_entry);
  // printf("bcd-2\n");

  // cdstate->function_button_bounds.x = 2;
  // cdstate->function_button_bounds.y = core_objects_display->data.visual.bounds.height - 26;
  // cdstate->function_button_bounds.width = 80;
  // cdstate->function_button_bounds.height = 24;

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Function Editor Image
  obtain_resource_command(command_hub->renderer.resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &core_objects_display->data.visual.image_resource_uid;
  command->create_texture.use_as_render_target = true;
  command->create_texture.width = core_objects_display->data.visual.bounds.width;
  command->create_texture.height = core_objects_display->data.visual.bounds.height;

  // Font
  obtain_resource_command(command_hub->renderer.resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &cdstate->font_resource_uid;
  command->font.height = 18;
  command->font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  // register_midge_error_tag("build_core_display_v1(~)");
}