
#include "core/midge_core.h"

void add_notification_handler(mc_node_v1 *apex_node, uint event_type, int (**handler)(int, void **))
{
  event_handler_array *handler_array = NULL;
  for (int i = 0; i < apex_node->event_handlers.count; ++i) {
    if (apex_node->event_handlers.items[i]->event_type == event_type) {
      handler_array = apex_node->event_handlers.items[i];
      break;
    }
  }

  if (handler_array == NULL) {
    // Make a new one
    handler_array = (event_handler_array *)malloc(sizeof(event_handler_array));
    handler_array->alloc = 0;
    handler_array->count = 0;
    handler_array->event_type = event_type;

    append_to_collection((void ***)&apex_node->event_handlers.items, &apex_node->event_handlers.alloc,
                         &apex_node->event_handlers.count, handler_array);
  }

  // printf("adding %p *->%p\n", handler, *handler);
  append_to_collection((void ***)&handler_array->handlers, &handler_array->alloc, &handler_array->count, handler);
}

void notify_handlers_of_event(uint event_type, void *event_data)
{
  // printf("notify_handlers_of_event\n");
  event_handler_array *handler_array = NULL;
  for (int i = 0; i < command_hub->global_node->event_handlers.count; ++i) {
    if (command_hub->global_node->event_handlers.items[i]->event_type == event_type) {
      handler_array = command_hub->global_node->event_handlers.items[i];
      break;
    }
  }

  if (handler_array == NULL) {
    printf("handler_array couldnt be found for:%i out of %i events handled for\n", event_type,
           command_hub->global_node->event_handlers.count);
    return;
  }

  // printf("hel %i [0]:%p\n", handler_array->count, handler_array->handlers[0]);

  for (int i = 0; i < handler_array->count; ++i) {
    if ((*handler_array->handlers[i])) {
      void *vargs[1];
      vargs[0] = &event_data;

      // printf("invoking [%i]:%p\n", i, (*handler_array->handlers[i]));
      (*handler_array->handlers[i])(1, vargs);
    }
  }
}

void add_node_to_heirarchy(node *parent_attachment, node *node_to_add)
{
  append_to_collection((void ***)&parent_attachment->children, &parent_attachment->children_alloc,
                       &parent_attachment->child_count, node_to_add);

  // Fire an event...
  uint event_type = ME_NODE_HIERARCHY_UPDATED;
  notify_handlers_of_event(event_type, NULL);

  // TODO -- maybe find a better place to do this
  switch (node_to_add->type) {
  case NODE_TYPE_CONSOLE_APP: {
    console_app_info *app_info = (console_app_info *)node_to_add->extra;
    if (app_info->initialize && (*app_info->initialize)) {
      void *vargs[1];
      vargs[0] = &node_to_add;
      (*app_info->initialize)(1, vargs);
    }
  } break;
  default:
    break;
  }
}

void exit_app(mc_node_v1 *node_scope, int result)
{
  switch (node_scope->type) {
  case NODE_TYPE_CONSOLE_APP: {
    console_app_info *app_info = (console_app_info *)node_scope->extra;
    if (node_scope->parent == NULL) {
      // Exit the whole application
      exit(result); // Maybe rename this or intercept if for use in midge??
    }
    else {
      // Remove this node from the heirarchy
      // remove_node_from_heirarchy() // ??? -- or keep it and show it has exited...?
    }
  } break;
  default:
    break;
  }
}

size_t save_text_to_file(char *filepath, char *text);

void export_node_to_application(mc_node_v1 *node, char *path)
{
  // Generate the source?
  const char *main_c = "#include <stdio.h>\n"
                       "\n"
                       "int main()\n"
                       "{\n"
                       "printf(\"Hello World! TorchSaltCar\");\n"
                       "\n"
                       "return 0;\n"
                       "}\n";

  save_text_to_file("home/jason/midge/test/main.c", main_c);

  // Compile

  //

  // char* args[] = {"2", "1"};
  // if(execvp("/home/jason/cling/inst/bin/clang", args) == -1) {
  //     printf("\nfailed connection\n");
  // }
}