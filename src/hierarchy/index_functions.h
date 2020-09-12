#ifndef INDEX_FUNCTIONS_H
#define INDEX_FUNCTIONS_H

#include "core/core_definitions.h"

extern "C" {
// void add_notification_handler(mc_node *apex_node, unsigned int event_type, int (**handler)(int, void **));
// void notify_handlers_of_event(unsigned int event_type, void *event_data);
void attach_node_to_hierarchy(mc_node *hierarchy_node, mc_node *node_to_attach);
// void attach_definition_to_hierarchy(mc_node *parent_attachment, char *definition);
void exit_app(mc_node *node_scope, int result);
}

#endif // INDEX_FUNCTIONS_H