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

#include "modules/collections/hash_table.h"
// #include "modules/mc_io/mc_file.h"
// #include "modules/render_utilities/render_util.h"
#include "modules/welcome_window/welcome_window.h"
#include "modules/ui_elements/ui_elements.h"

#define PORT 8080

#define SERVER_RESPONSE_ITEM_CAPACITY 10

struct command_configuring;
typedef struct command_configuring {
  struct command_configuring *parent;
  char *command_text;

  struct {
    uint32_t count, capacity;
    char **items;
  } sequence;

} command_configuring;

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
      char cmd[512];
      struct {
        mc_str action_name, action_detail;
        float prob;
      } items[SERVER_RESPONSE_ITEM_CAPACITY];
    } server_responses;
   
  } icp_conn;
  
  command_configuring *configuring_command;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  mcu_panel *cmd_panel;
  mcu_textblock *status_block;
  mcu_textbox *cmd_textbox;

  mcu_panel *config_panel;
  mcu_textblock *config_textblock;

  hash_table_t basal_ops;

  char prompt_cmd[512];
  mcu_panel *prompt_panel;
  struct {
    unsigned int capacity, count;
    mcu_button **items;
  } options_buttons;

//   mc_node *context_viewer_node;

} commander_data;

typedef struct action_button_data {
  commander_data *cmdr_data;
  int index;
  float prob;
  mc_str suggested_action, action_detail;
} action_button_data;

void _mcm_cmdr_render_mod(image_render_details *irq, mc_node *node) {
  commander_data *data = (commander_data *)node->data;

  // mcr_issue_render_command_colored_quad(irq, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, 
  //                                        (unsigned int)node->layout->__bounds.width,
  //                                        (unsigned int)node->layout->__bounds.height, COLOR_OLIVE);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Input / UI Logic /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void _mcm_cmdr_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mcm_mo_handle_input\n");
  input_event->handled = true;
  // if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
  // }
}

int _mcm_cmdr_execute_process(commander_data *data, mci_input_event *ie, const char *process_name)
{
  if(!strcmp(process_name, "raise-create-project-dialog")) {
    printf("COMMAND RESPONSE: %s\n", "raise-create-project-dialog");

    mc_node *ww_node;
    MCcall(mca_find_hierarchy_node_any(data->node, "midge-root>welcome-window", &ww_node));
    if(!ww_node) {
      puts ("couldn't find welcome window ???? 58358");
    } else {

      mcm_wwn_raise_new_project_dialog(ww_node, NULL);
      puts("DONE raising dialog");
    }
  }
  else {
    printf("[2918] ERROR mcm-commander: Unrecoginized process:'%s'\n", process_name);
  }

  return 0;
}

int _mcm_cmdr_configure_command(commander_data *cd)
{
  // cd->configuring_command
  command_configuring *cmd_cfg = (command_configuring *)malloc(sizeof(command_configuring));
  cmd_cfg->parent = cd->configuring_command;
  cmd_cfg->command_text = strdup(cd->prompt_cmd);
  cmd_cfg->sequence.capacity = cmd_cfg->sequence.count = 0U;
    
  cd->configuring_command = cmd_cfg;

  // Update the display
  cd->config_panel->node->layout->visible = true;
  char buf[512];
  sprintf(buf, "Configuring '%s'...", cd->prompt_cmd);
  MCcall(mcu_set_textblock_text(cd->config_textblock, buf));
  MCcall(mca_set_node_requires_layout_update(cd->config_panel->node));

  return 0;
}

int _mcm_process_command(commander_data *cd, const char *cmd, bool *handled)
{
  // Basal Function Creation
  if(!strcmp(cmd, "create basal function")) {

    

    *handled = true;
  }
  
  puts("h525h2");

  *handled = false;
  return 0;
}

int _mcm_cmdr_action_option_selected(mci_input_event *ie, mcu_button *button)
{
  ie->handled = true;
  ie->focus_successor = NULL;

  action_button_data *abd = (action_button_data *) button->tag;
  commander_data *cd = (commander_data *)abd->cmdr_data;

  if(abd->suggested_action.len) {
    MCcall(_mcm_cmdr_execute_process(abd->cmdr_data, ie, abd->suggested_action.text));
  } else {
    // Increase configuration depth
    MCcall(_mcm_cmdr_configure_command(cd));

    cd->config_panel->node->layout->visible = true;
    MCcall(mca_set_node_requires_layout_update(cd->config_panel->node));
  }

  // Hide the prompt panel
  cd->prompt_panel->node->layout->visible = false;
  MCcall(mca_set_node_requires_layout_update(cd->prompt_panel->node));

  return 0;
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

  // Function for chat
  char buf[1024];
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

      // Obtain the processed command
      char *a = c;
      while(*a != ';' && *a != '\0') {
        ++a;
      }
      // TODO -- no checking the string at all
      strncpy(data->icp_conn.server_responses.cmd, c, a - c);
      data->icp_conn.server_responses.cmd[a - c] = '\0';

      *i = 0;
      while(*i < SERVER_RESPONSE_ITEM_CAPACITY) {
        if(*a == '\0')
          break;
        
        // Get the action name
        a = c = ++a;
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
  
  MCcall(mc_set_str(&data->icp_conn.cmd_str, "c]"));
  MCcall(mc_append_to_str(&data->icp_conn.cmd_str, command));

  MCcall(begin_mthread(&__mcm_cmdr_async_icpconn_send, &data->icp_conn.thr_info, data));
  
  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

int _mcm_cmdr_send_sample(commander_data *data, const char *command, const char *expected_action) {
  pthread_mutex_lock(&data->icp_conn.thr_lock);
  
  MCcall(mc_set_str(&data->icp_conn.cmd_str, "s]"));
  MCcall(mc_append_to_str(&data->icp_conn.cmd_str, command));
  MCcall(mc_append_to_str(&data->icp_conn.cmd_str, "|=|"));
  MCcall(mc_append_to_str(&data->icp_conn.cmd_str, expected_action));

  MCcall(begin_mthread(&__mcm_cmdr_async_icpconn_send, &data->icp_conn.thr_info, data));
  
  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

int _mcm_cmdr_update_connection(frame_time *ft, void *state) {
  commander_data *data = (commander_data *)state;

  pthread_mutex_lock(&data->icp_conn.thr_lock);
  if(data->icp_conn.status_updated) {
    MCcall(mcu_set_textblock_text(data->status_block, data->icp_conn.status_str.text));

    data->icp_conn.status_updated = false;
  }

  // printf("cc:%u\n", data->icp_conn.server_responses.count);
  if (data->icp_conn.server_responses.count) {
    printf("server responded with %u possible actions for cmd:'%s'. Highest prob: %i\n", data->icp_conn.server_responses.count,
      data->icp_conn.server_responses.cmd, (int)(data->icp_conn.server_responses.items[0].prob * 100));

    // No longer hide
    strcpy(data->prompt_cmd, data->icp_conn.server_responses.cmd);
    data->prompt_panel->node->layout->visible = true;

    // Show me the way
    action_button_data *abd = (action_button_data *)data->options_buttons.items[0]->tag;
    MCcall(mcu_set_button_text(data->options_buttons.items[0], "Configure Command"));
    MCcall(mc_set_str(&abd->suggested_action, ""));

    // Fill the buttons
    char buf[256];
    int a;
    for(a = 0; a < data->icp_conn.server_responses.count && a + 1 < data->options_buttons.count; ++a) {
      action_button_data *abd = (action_button_data *)data->options_buttons.items[a + 1]->tag;

      sprintf(buf, "%i%%", (int)(data->icp_conn.server_responses.items[a].prob * 100.f));
      while(strlen(buf) < 4)
        strcat(buf, " ");
      strcat(buf, "- ");
      strcat(buf, data->icp_conn.server_responses.items[a].action_name.text);
      MCcall(mcu_set_button_text(data->options_buttons.items[a + 1], buf));

      MCcall(mc_set_str(&abd->suggested_action, data->icp_conn.server_responses.items[a].action_name.text));
      MCcall(mc_set_str(&abd->action_detail, data->icp_conn.server_responses.items[a].action_detail.text));
      abd->prob = data->icp_conn.server_responses.items[a].prob;
    }
    for(; a + 1 < data->options_buttons.count; ++a) {
      data->options_buttons.items[a + 1]->node->layout->visible = false;
      MCcall(mca_set_node_requires_layout_update(data->options_buttons.items[a + 1]->node));
    }
    
    data->icp_conn.server_responses.count = 0;
  }
  pthread_mutex_unlock(&data->icp_conn.thr_lock);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// Initialization /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void __mcm_cmd_textbox_submit(mci_input_event *event, mcu_textbox *textbox) {
  commander_data *data = (commander_data *)textbox->node->parent->parent->data;

  if(!textbox->contents->len) {
    // Ignore
    return;
  }
  
  // // Obtain (& reset) text
  // char cmd[512];
  // int n = snprintf(cmd, 511, "%s", textbox->contents->text);
  // if(n >= 511) {
  //   puts("[8811] Warning! Entered Command Too Long");
  // }

  // Attempt to process the command internally, otherwise send it to the server
  bool handled;
  _mcm_process_command(data, textbox->contents->text, &handled);
  if(!handled) {
    _mcm_cmdr_send_command(data, textbox->contents->text);
  }

  mcu_set_textbox_text(data->cmd_textbox, "");
}

int mcm_cmdr_init_ui(mc_node *module_node)
{
  commander_data *data = (commander_data *)module_node->data;

  mcu_panel *panel;
  MCcall(mcu_init_panel(module_node, &panel));
  data->cmd_panel = panel;
  panel->background_color = COLOR_BURLY_WOOD;
  panel->node->layout->preferred_width = 349;
  panel->node->layout->preferred_height = 66;
  panel->node->layout->padding = (mc_paddingf){2, 2, 2, 2};
  panel->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  panel->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  MCcall(mcu_init_textblock(panel->node, &data->status_block));
  MCcall(mcu_set_textblock_text(data->status_block, "Default Status"));
  data->status_block->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  // data->status_block->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

  MCcall(mcu_init_textbox(panel->node, &data->cmd_textbox));
  data->cmd_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  data->cmd_textbox->node->layout->padding = (mc_paddingf){6, 8, 42, 2};
  data->cmd_textbox->submit = &__mcm_cmd_textbox_submit;

//   MCcall(mcu_init_textbox(module_node, &mod->search_textbox));
//   mod->search_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   mod->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

  MCcall(mcu_init_panel(module_node, &panel));
  data->prompt_panel = panel;
  panel->background_color = COLOR_DARK_GREEN;
  panel->node->layout->preferred_width = 349;
  panel->node->layout->preferred_height = 200;
  panel->node->layout->padding = (mc_paddingf){2, 2, 2, 2 + 66 + 2};
  panel->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  panel->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  panel->node->layout->visible = false;

  // Visible Options Buttons
  char buf[64];
  mcu_button *button;
  for (int a = 0; a < 4; ++a) {
    MCcall(mcu_alloc_button(panel->node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "mcm-cmdr-action-option-button-%i", a);
    button->node->name = strdup(buf);
    button->text_align.horizontal = HORIZONTAL_ALIGNMENT_LEFT;

    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
    button->node->layout->max_width = 0U;
    // button->node->layout->visible = false;
    
    action_button_data *abd = (action_button_data *)malloc(sizeof(action_button_data));
    abd->cmdr_data = data;
    abd->index = a;
    MCcall(mc_init_str(&abd->suggested_action, 128));
    MCcall(mc_init_str(&abd->action_detail, 64));
    button->tag = abd;

    button->left_click = (void *)&_mcm_cmdr_action_option_selected;

    MCcall(mc_set_str(&button->str, "button"));

    MCcall(append_to_collection((void ***)&data->options_buttons.items, &data->options_buttons.capacity,
                                &data->options_buttons.count, button));
  }

  // Configuration Panel
  MCcall(mcu_init_panel(module_node, &panel));
  data->config_panel = panel;
  panel->background_color = COLOR_DARK_GREEN;
  panel->node->layout->preferred_width = 309;
  panel->node->layout->preferred_height = 40;
  panel->node->layout->padding = (mc_paddingf){20, 2, 20, 270 + 2};
  panel->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  panel->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  panel->node->layout->visible = false;

  MCcall(mcu_init_textblock(panel->node, &data->config_textblock));
  MCcall(mcu_set_textblock_text(data->config_textblock, "NULL"));
  // data->config_textblock->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

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

  data->configuring_command = NULL;
  MCcall(init_hash_table(32, &data->basal_ops));
  // hash_table_set("create basal function", (void *)project_context, &data->basal_ops);

  data->options_buttons.capacity = data->options_buttons.count = 0U;
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

  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  node->layout->padding.left = 2;
  node->layout->padding.bottom = 2;

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