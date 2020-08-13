
#include "core/midge_core.h"

void add_notification_handler(uint event_type, int (**handler)(int, void **))
{

  event_handler_array *handler_array = NULL;
  for (int i = 0; i < command_hub->global_node->event_handlers.count; ++i) {
    if (command_hub->global_node->event_handlers.items[i]->event_type == event_type) {
      handler_array = command_hub->global_node->event_handlers.items[i];
      break;
    }
  }

  if (handler_array == NULL) {
    // Make a new one
    handler_array = (event_handler_array *)malloc(sizeof(event_handler_array));
    handler_array->alloc = 0;
    handler_array->count = 0;
    handler_array->event_type = event_type;
  }

  MCerror("TODO -- also handler/event data");
}

void notify_handlers_of_event(uint event_type, void *event_data)
{
  event_handler_array *handler_array = NULL;
  for (int i = 0; i < command_hub->global_node->event_handlers.count; ++i) {
    if (command_hub->global_node->event_handlers.items[i]->event_type == event_type) {
      handler_array = command_hub->global_node->event_handlers.items[i];
      break;
    }
  }

  if (handler_array == NULL) {
    return;
  }

  for (int i = 0; i < handler_array->count; ++i) {
    if (!(*handler_array->handlers[i])) {
      void *vargs[1];
      vargs[0] = &event_data;
      (*handler_array->handlers[i])(1, vargs);
    }
  }
}

void add_node_as_child(node *parent, node *child)
{
  append_to_collection((void ***)&parent->children, &parent->children_alloc, &parent->child_count, child);

  // Fire an event...
  uint event_type = ME_NODE_HIERARCHY_UPDATED;
  notify_handlers_of_event(event_type, NULL);
}