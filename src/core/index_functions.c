
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

void add_node_as_child(node *parent, node *child)
{
  append_to_collection((void ***)&parent->children, &parent->children_alloc, &parent->child_count, child);

  // Fire an event...
  uint event_type = ME_NODE_HIERARCHY_UPDATED;
  notify_handlers_of_event(event_type, NULL);
}