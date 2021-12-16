/* commander.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #include <dirent.h>
#include <unistd.h>

#include "core/midge_app.h"
#include "mc_error_handling.h"
#include "render/render_common.h"

// #include "modules/collections/hash_table.h"
// #include "modules/mc_io/mc_file.h"
// #include "modules/render_utilities/render_util.h"
#include "modules/welcome_window/welcome_window.h"
#include "modules/ui_elements/ui_elements.h"

#define PORT 8080

#define SERVER_RESPONSE_ITEM_CAPACITY 10

typedef struct commander_data {
  mc_node *node;

  struct {
    mthread_info *thr_info;
    pthread_mutex_t thr_lock;

    bool connected, status_updated;
    mc_str cmd_str, status_str;

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    struct {
      unsigned int query_id, count;
      struct {
        mc_str action_name, action_detail;
        float prob;
      } items[SERVER_RESPONSE_ITEM_CAPACITY];
    } server_responses;
   
  } icp_conn;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  mcu_textblock *status_block;
  mcu_textbox *cmd_textbox;

//   mc_create_process_dialog_data *create_process_dialog;
//   mc_process_step_dialog_data *create_step_dialog;

//   mc_mo_process_stack process_stack;

//   mo_operational_process_list all_processes;

//   mcu_textbox *search_textbox;
//   struct {
//     unsigned int capacity, count;
//     mcu_button **items;
//   } options_buttons;

//   mc_node *context_viewer_node;

} commander_data;

void _mcm_cmdr_render_mod(image_render_details *irq, mc_node *node) {
  commander_data *data = (commander_data *)node->data;

  mcr_issue_render_command_colored_quad(irq, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, 
                                         (unsigned int)node->layout->__bounds.width,
                                         (unsigned int)node->layout->__bounds.height, COLOR_OLIVE);

  // Children
  // printf("childcount:%i\n", node->children->count);
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }
}

void _mcm_cmdr_render_mod_headless(render_thread_info *render_thread, mc_node *node)
{
  commander_data *data = (commander_data *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) =
          (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
    }
  }

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_GRAPE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = data->render_target.width;   // TODO
  irq->image_height = data->render_target.height; // TODO
  irq->data.target_image.image = data->render_target.image;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  _mcm_cmdr_render_mod(irq, node);

  mcr_submit_image_render_request(global_data->render_thread, irq);
}

void _mcm_cmdr_render_present(image_render_details *irq, mc_node *node)
{
  commander_data *data = (commander_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, data->render_target.width,
  //                                        data->render_target.height, data->render_target.image);
  _mcm_cmdr_render_mod(irq, node);
}

void _mcm_cmdr_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mcm_mo_handle_input\n");
  input_event->handled = true;
  // if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
  // }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Client Server Connection ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

int __mcm_cmdr_set_connection_status(commander_data *data, bool connected, mc_str *str, const char *text) {
  pthread_mutex_lock(&data->icp_conn.thr_lock);

  MCcall(mc_set_str(str, text));
  data->icp_conn.connected = connected;
  data->icp_conn.status_updated = true;

  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

void *__mcm_cmdr_async_connect_to_server(void *state)
{
  midge_error_set_thread_name("mcm_cmdr_connection_thread");

  commander_data *data = (commander_data *)state;

  // socket create and verification
  data->icp_conn.sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (data->icp_conn.sockfd == -1) {
    __mcm_cmdr_set_connection_status(data, false, &data->icp_conn.status_str, "Socket creation failed.");
  }
  else {
    __mcm_cmdr_set_connection_status(data, true, &data->icp_conn.status_str, "Socket successfully created.");
  }
  bzero(&data->icp_conn.servaddr, sizeof(data->icp_conn.servaddr));
   
  // assign IP, PORT
  data->icp_conn.servaddr.sin_family = AF_INET;
  data->icp_conn.servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  data->icp_conn.servaddr.sin_port = htons(PORT);
   
  // connect the client socket to server socket
  if (connect(data->icp_conn.sockfd, (struct sockaddr*)&data->icp_conn.servaddr, sizeof(data->icp_conn.servaddr)) != 0) {
    __mcm_cmdr_set_connection_status(data, false, &data->icp_conn.status_str, "Connection with the server failed.");

    // close the socket
    shutdown(data->icp_conn.sockfd, SHUT_RDWR);
  }
  else {
    __mcm_cmdr_set_connection_status(data, true, &data->icp_conn.status_str, "Connected to the server.");
  }

  return NULL;
}

void *__mcm_cmdr_async_shutdown_connection(void *state)
{
  midge_error_set_thread_name("mcm_cmdr_connection_thread");

  commander_data *data = (commander_data *)state;

  // close the socket
  shutdown(data->icp_conn.sockfd, SHUT_RDWR);
  
  __mcm_cmdr_set_connection_status(data, true, &data->icp_conn.status_str, "Server shutdown.");

  return NULL;
}

void * __mcm_cmdr_async_icpconn_send(void *state) {
  commander_data *data = (commander_data *)state;
  
  pthread_mutex_lock(&data->icp_conn.thr_lock);
  if(!data->icp_conn.connected || !data->icp_conn.cmd_str.len) {
    pthread_mutex_unlock(&data->icp_conn.thr_lock);
    return NULL;
  }

  // Reset previous query cache
  ++data->icp_conn.server_responses.query_id;
  data->icp_conn.server_responses.count = 0;

  // function for chat
  char buf[512];
  ssize_t res;
  printf("Sending to server:'%s'\n", data->icp_conn.cmd_str.text);
  res = send(data->icp_conn.sockfd, data->icp_conn.cmd_str.text, sizeof(char) * data->icp_conn.cmd_str.len, 0);
  if(res == -1) {
    perror("send");
  }
  else {
    printf("Sent to server: %zu bytes\n", res);

    bzero(buf, sizeof(buf));
    res = recv(data->icp_conn.sockfd, buf, sizeof(buf), 0);
    if(res == -1) {
      perror("recv");
    }
    else {
      printf("recv: %zu bytes From Server:'%s'\n", res, buf);

      char *c = buf;
      unsigned int *i = &data->icp_conn.server_responses.count;
      while(1) {
        if(*c == '\0')
          break;
        
        // Get the action name
        char *a = c;
        while(*a != ':')
          ++a;
        mc_set_strn(&data->icp_conn.server_responses.items[*i].action_name, c, a - c);
        printf("aname:'%s'\n", data->icp_conn.server_responses.items[*i].action_name.text);
        
        // Get the probability
        a = c = ++a;
        while(*a != ':')
          ++a;
        char fbuf[16];
        strncpy(fbuf, c, a - c);
        data->icp_conn.server_responses.items[*i].prob = (float)atof(fbuf);
        printf("aprob:'%f'\n", data->icp_conn.server_responses.items[*i].prob);
        
        // Get the action name
        a = c = ++a;
        while(*a != ';')
          ++a;
        mc_set_strn(&data->icp_conn.server_responses.items[*i].action_detail, c, a - c);
        printf("adet:'%s'\n", data->icp_conn.server_responses.items[*i].action_detail.text);

        c = ++a;
        ++*i;
      }
      // char *str = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
      // strcpy(str, buf);
      // append_to_collection((void ***)&data->icp_conn.server_responses.items, &data->icp_conn.server_responses.capacity,
        // &data->icp_conn.server_responses.count, str);
    }
  }

  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return NULL;
}

int _mcm_cmdr_send_command(commander_data *data, const char *command) {
  pthread_mutex_lock(&data->icp_conn.thr_lock);
  
  MCcall(mc_set_str(&data->icp_conn.cmd_str, command));

  MCcall(begin_mthread(&__mcm_cmdr_async_icpconn_send, &data->icp_conn.thr_info, data));
  
  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

// int __mcm_cmdr_update_connection_callback() {
//   return 0;
// }

int _mcm_cmdr_update_connection(frame_time *ft, void *state) {
  commander_data *data = (commander_data *)state;

  pthread_mutex_lock(&data->icp_conn.thr_lock);
  if(data->icp_conn.status_updated) {
    MCcall(mcu_set_textblock_text(data->status_block, data->icp_conn.status_str.text));

    data->icp_conn.status_updated = false;
  }

  // printf("cc:%u\n", data->icp_conn.server_responses.count);
  while (data->icp_conn.server_responses.count) {

    printf("server responded with %u possible actions. Highest prob: %i\n", data->icp_conn.server_responses.count,
      (int)(data->icp_conn.server_responses.items[0].prob * 100));
    data->icp_conn.server_responses.count = 0;
    
    // if(!strncmp(rsp, "raise-create-project-dialog", 27)) {
    //   printf("COMMAND RESPONSE: %s\n", "raise-create-project-dialog");

    //   mc_node *ww_node;
    //   MCcall(mca_find_hierarchy_node_any(data->node, "midge-root>welcome-window", &ww_node));
    //   if(!ww_node) {
    //     puts ("couldn't find welcome window ???? 58358");
    //   } else {
    //     puts("raising dialog");
    //     mcm_wwn_raise_new_project_dialog(ww_node, NULL);
    //     puts("DONE raising dialog");
    //   }
    // } else if(!strncmp(rsp, "add-panel", 27)) {
    //   printf("COMMAND RESPONSE: %s\n", "add-panel");

    //   midge_app_info *mapp_info;
    //   mc_obtain_midge_app_info(&mapp_info);
      
    //   mc_node *ww_node;
    //   MCcall(mca_find_hierarchy_node_any(data->node, "midge-root>welcome-window", &ww_node));
    //   if(!ww_node) {
    //     puts ("couldn't find welcome window ???? 58358");
    //   } else {
    //     puts("raising dialog");
    //     mcm_wwn_raise_new_project_dialog(ww_node, NULL);
    //     puts("DONE raising dialog");
    //   }
    // }

    // free(rsp);
  }
  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// Initialization /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void __mcm_cmd_textbox_submit(mci_input_event *event, mcu_textbox *textbox) {
  commander_data *data = (commander_data *)textbox->node->parent->data;

  if(!textbox->contents->len) {
    // Ignore
    return;
  }
  
  _mcm_cmdr_send_command(data, textbox->contents->text);

  // Reset text
  mcu_set_textbox_text(data->cmd_textbox, "");
}

int mcm_cmdr_init_ui(mc_node *module_node)
{
  commander_data *data = (commander_data *)module_node->data;

  MCcall(mcu_init_textblock(module_node, &data->status_block));
  MCcall(mcu_set_textblock_text(data->status_block, "Default Status"));
  data->status_block->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  // data->status_block->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

  MCcall(mcu_init_textbox(module_node, &data->cmd_textbox));
  data->cmd_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  data->cmd_textbox->node->layout->padding = (mc_paddingf){6, 8, 42, 2};
  data->cmd_textbox->submit = &__mcm_cmd_textbox_submit;

//   MCcall(mcu_init_textbox(module_node, &mod->search_textbox));
//   mod->search_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   mod->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

//   char buf[64];
//   mcu_button *button;

//   // Visible Options Buttons
//   for (int a = 0; a < 12; ++a) {
//     MCcall(mcu_alloc_button(module_node, &button));

//     if (button->node->name) {
//       free(button->node->name);
//       button->node->name = NULL;
//     }
//     sprintf(buf, "mo-options-button-%i", a);
//     button->node->name = strdup(buf);
//     button->text_align.horizontal = HORIZONTAL_ALIGNMENT_LEFT;

//     button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//     button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
//     button->node->layout->max_width = 0U;
//     button->node->layout->visible = false;

//     button->left_click = (void *)&_mcm_mo_operational_process_selected;

//     MCcall(mc_set_str(&button->str, "button"));

//     MCcall(append_to_collection((void ***)&mod->options_buttons.items, &mod->options_buttons.capacity,
//                                 &mod->options_buttons.count, button));
//   }

//   // Add process button
//   MCcall(mcu_alloc_button(module_node, &button));
//   if (button->node->name) {
//     free(button->node->name);
//     button->node->name = NULL;
//   }
//   button->node->name = strdup("mo-create-process-button");

//   button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
//   button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   button->node->layout->padding = (mc_paddingf){6, 2, 6, 2};
//   button->node->layout->max_width = 16U;
//   button->node->layout->max_height = 16U;
//   button->tag = mod;

//   button->left_click = (void *)&_mcm_mo_create_process_clicked;

//   MCcall(mc_set_str(&button->str, "+"));

//   // Show Context Viewer button
//   MCcall(mcu_alloc_button(module_node, &button));
//   if (button->node->name) {
//     free(button->node->name);
//     button->node->name = NULL;
//   }
//   button->node->name = strdup("mo-show-context-viewer-button");

//   button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
//   button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
//   button->node->layout->padding = (mc_paddingf){6, 2, 6, 2};
//   button->node->layout->max_width = 18U;
//   button->node->layout->max_height = 18U;
//   button->tag = mod;

//   button->left_click = (void *)&_mcm_mo_toggle_context_viewer_clicked;

//   MCcall(mc_set_str(&button->str, "oo"));

  return 0;
}

int mc_cmdr_load_resources(mc_node *module_node)
{
  int a;

  // Initialize
  commander_data *data = (commander_data *)malloc(sizeof(commander_data));
  module_node->data = data;
  data->node = module_node;
  
  pthread_mutex_init(&data->icp_conn.thr_lock, NULL);
  data->icp_conn.server_responses.count = 0;
  data->icp_conn.server_responses.query_id = 0;
  for(int i = 0; i < SERVER_RESPONSE_ITEM_CAPACITY; ++i) {
    MCcall(mc_init_str(&data->icp_conn.server_responses.items[i].action_name, 16));
    MCcall(mc_init_str(&data->icp_conn.server_responses.items[i].action_detail, 16));
  }

//   mod->options_buttons.capacity = mod->options_buttons.count = 0U;
//   mod->all_processes.capacity = mod->all_processes.count = 0U;

//   mod->process_stack.index = -1;
//   mod->process_stack.state_arg = (void *)mod;
//   mod->process_stack.all_processes = &mod->all_processes;
//   MCcall(init_hash_table(64, &mod->process_stack.global_context));
//   MCcall(init_hash_table(4, &mod->process_stack.project_contexts));
//   for (a = 0; a < MO_OP_PROCESS_STACK_SIZE; ++a) {
//     MCcall(init_hash_table(8, &mod->process_stack.context_maps[a]));
//     mod->process_stack.processes[a] = NULL;
//     mod->process_stack.steps[a] = NULL;
//   }

  // data->render_target.image = NULL;
  // data->render_target.width = module_node->layout->preferred_width;
  // data->render_target.height = module_node->layout->preferred_height;
  // MCcall(mcr_create_texture_resource(data->render_target.width, data->render_target.height,
  //                                    MVK_IMAGE_USAGE_RENDER_TARGET_2D, &data->render_target.image));

  return 0;
}

int init_commander_system(mc_node *app_root) {

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_MODULE_ROOT, "commander-root", &node);
  mca_init_node_layout(&node->layout);

  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 349;
  node->layout->preferred_height = 66;

  node->layout->padding.left = 2;
  node->layout->padding.bottom = 2;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  // node->layout->render_headless = (void *)&_mcm_cmdr_render_mod_headless;
  node->layout->render_present = (void *)&_mcm_cmdr_render_present;
  node->layout->handle_input_event = (void *)&_mcm_cmdr_handle_input;

  // TODO
  // node->layout->visible = false;
  // TODO

  MCcall(mc_cmdr_load_resources(node));
  commander_data *data = (commander_data *)node->data;

  MCcall(mcm_cmdr_init_ui(node));

//   MCcall(_mcm_mo_load_operations(node));

//   // Create Process Step Dialog
//   MCcall(mc_mo_init_create_process_dialog(app_root, &mod->create_process_dialog));
//   MCcall(mc_mocsd_init_process_step_dialog(app_root, &mod->create_step_dialog));

  // Event Registers
//   MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_STRUCTURE_CREATION, &_mcm_mo_project_created, node->data));
//   MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_LOADED, &_mcm_mo_project_loaded, node->data));

  // ML Client Server Connection Thread
  MCcall(mc_init_str(&data->icp_conn.cmd_str, 256));
  data->icp_conn.connected = false;
  MCcall(mc_init_str(&data->icp_conn.status_str, 256));
  MCcall(begin_mthread(&__mcm_cmdr_async_connect_to_server, &data->icp_conn.thr_info, data));
  long one_hundred_milliseconds = 100000L;
  MCcall(mca_register_update_timer(one_hundred_milliseconds, true, data, &_mcm_cmdr_update_connection));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!data->render_target.image) {
    // puts("wait");
    usleep(100);
  }
  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  MCcall(mca_focus_node(data->cmd_textbox->node));

  return 0;
}