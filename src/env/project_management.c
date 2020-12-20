// #include "core/core_definitions.h"
// #include "env/environment_definitions.h"
// #include "render/render_common.h"
// #include "ui/ui_definitions.h"

// void mca_visual_project_create_add_button(mc_node *context_node, mc_point context_location, const char *selected_text)
// {
//   mcu_button *button;
//   mcu_init_button(context_node, &button);

//   visual_project_data *visual_project = (visual_project_data *)context_node->data;

//   button->element->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
//   button->element->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   button->element->layout->padding = {(float)(context_location.x - (int)visual_project->layout->padding.left),
//                                       (float)(context_location.y - (int)visual_project->layout->padding.top), 0.f, 0.f};
// }

// void mca_init_visual_project_context_menu_options()
// {
//   // TODO this void * casting business
//   void *arg = (void *)&mca_visual_project_create_add_button;
//   mca_global_context_menu_add_option_to_node_context(NODE_TYPE_VISUAL_PROJECT, "add button", arg);
// }

// void mca_init_visual_project_management() { mca_init_visual_project_context_menu_options(); }

// void mca_init_visual_project_container(mc_node *project_node)
// {
//   visual_project_data *project = (visual_project_data *)project_node->data;

//   if (project->children->count > 0) {
//     MCerror(1158, "Invalid State: must initialize container with zero children");
//   }

//   // Panel
//   mcu_panel *panel;
//   mcu_init_panel(project_node, &panel);
//   panel->background_color = COLOR_MIDNIGHT_EXPRESS;

//   panel->element->layout->padding = {0, 0, 0, 0};
//   panel->element->layout->preferred_width = (float)project->screen.width;
//   panel->element->layout->preferred_height = (float)project->screen.height;

//   // Set container to seperate field and clear children
//   project->editor_container = panel->element->visual_node;
//   project->children->items[0] = NULL;
//   project->children->count = 0;
// }

// void mca_create_new_visual_project(const char *project_name)
// {
//   mc_global_data *global_data;
//   obtain_midge_global_root(&global_data);

//   // Node
//   mc_node *project_node;
//   mca_init_mc_node(global_data->global_node, NODE_TYPE_VISUAL_PROJECT, &project_node);

//   // Layout
//   mca_init_node_layout(&node->layout);
//   // node->layout->determine_layout_extents = (void *)&__mcu_determine_panel_extents;
//   // node->layout->update_layout = (void *)&__mcu_update_panel_layout;
//   // node->layout->render_headless = (void *)&__mcu_render_panel_headless;
//   // node->layout->render_present = (void *)&__mcu_render_panel_present;

//   // Project
//   visual_project_data *project = (visual_project_data *)malloc(sizeof(visual_project_data));
//   project_node->data = project;

//   project->name = strdup(project_name);

//   project->screen.width = 600;
//   project->screen.height = 480;
//   project->screen.offset_x = 400;
//   project->screen.offset_y = 200;
//   mca_init_node_layout(&project->layout);

//   project->children = (mc_node_list *)malloc(sizeof(mc_node_list));
//   project->children->alloc = 0;
//   project->children->count = 0;

//   project->present_image_resource_uid = 0;
//   mcu_initialize_ui_state(&project->ui_state);

//   // Resources
//   pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);

//   mcr_create_texture_resource(global_data->render_thread->resource_queue, (unsigned int)project->screen.width,
//                               (unsigned int)project->screen.height, true, &project->present_image_resource_uid);

//   pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);

//   // Initialize the project visual container (what will be replaced by the OS window)
//   mca_init_visual_project_container(project_node);

//   // Ensure initial update is triggered
//   mca_set_node_requires_layout_update(project_node);

//   printf("visual app created\n");
// }

// void mca_update_visual_project(mc_node *project_node)
// {
//   // TODO
//   // visual_project_data *project = (visual_project_data *)project_node->data;

//   // {
//   //   // TEMPORARY -- FIX When UI UPDATE is fixed
//   //   mcu_ui_element *element = (mcu_ui_element *)project->editor_container->data;
//   //   if (element->requires_layout_update) {
//   //     element->requires_layout_update = false;

//   //     // Trigger rerender
//   //     mca_set_node_requires_rerender(element->visual_node);
//   //   }
//   // }

//   // mca_update_node_list(project->children);

//   // // Projects always require updating
//   // project->requires_update = true;
// }

// void mca_render_project_headless(render_thread_info *render_thread, mc_node *project_node)
// {
//   visual_project_data *project = (visual_project_data *)project_node->data;

//   mca_render_node_list_headless(project->children);
// }

// void mca_render_project_present(image_render_details *image_render_queue, mc_node *project_node)
// {
//   visual_project_data *project = (visual_project_data *)project_node->data;

//   mcu_render_panel(image_render_queue, project->editor_container);

//   mca_render_node_list_present(image_render_queue, project->children);
// }