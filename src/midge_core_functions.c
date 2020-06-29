#include "midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int find_function_info_v1(int argc, void **argv)
{
  if (argc != 3) {
    MCerror(-848, "Incorrect argument count");
  }

  function_info **func_info = (function_info **)argv[0];
  node *nodespace = *(node **)argv[1];
  char *function_name = *(char **)argv[2];

  *func_info = NULL;
  // printf("ffi-nodespace.name:%s\n", nodespace->name);
  for (int i = 0; i < nodespace->function_count; ++i) {

    // printf("ffi-3\n");
    // printf("dope\n");
    function_info *finfo = nodespace->functions[i];

    // printf("ffi-4a\n");
    // printf("findfunc-cmp: '%s'<>'%s'\n", finfo->name, function_name);
    // printf("ffi-4b\n");
    if (strcmp(finfo->name, function_name))
      continue;
    // printf("dwde\n");

    // printf("ffi-5\n");

    // Matches
    *func_info = finfo;
    // printf("find_function_info:set with '%s'\n", finfo->name);
    return 0;
  }
  // printf("dopu\n");

  // printf("ffi-2\n");

  if (nodespace->parent) {
    // Search in the parent nodespace
    void *mc_vargs[3];
    mc_vargs[0] = argv[0];
    mc_vargs[1] = (void *)&nodespace->parent;
    mc_vargs[2] = argv[2];
    MCcall(find_function_info(3, mc_vargs));
  }
  // printf("find_function_info: '%s' could not be found!\n", function_name);
  return 0;
}

int declare_function_pointer_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1
  printf("declare_function_pointer_v1()\n");
  char *name = *(char **)argv[0];
  char *return_type = *(char **)argv[1];

  // printf("dfp-name:%s\n", name);
  // printf("dfp-rett:%s\n", return_type);

  // TODO -- check
  // printf("dfp-0\n");

  // Fill in the function_info and attach to the nodespace
  // function_info *func_info = (function_info *)malloc(sizeof(function_info));
  function_info *func_info = (function_info *)malloc(sizeof(function_info));
  func_info->name = name;
  func_info->latest_iteration = 0U;
  func_info->return_type = return_type;
  func_info->parameter_count = (argc - 2) / 2;
  func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(void *) * func_info->parameter_count);
  func_info->variable_parameter_begin_index = -1;
  unsigned int struct_usage_alloc = 2;
  func_info->struct_usage = (mc_struct_info_v1 **)malloc(sizeof(mc_struct_info_v1 *) * struct_usage_alloc);
  func_info->struct_usage_count = 0;
  // printf("dfp-1\n");

  for (int i = 0; i < func_info->parameter_count; ++i) {
    // printf("dfp-2\n");
    mc_parameter_info_v1 *parameter_info = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    // printf("dfp>%p=%s\n", i, (void *)parameters[2 + i * 2 + 0], (char *)parameters[2 + i * 2 + 0]);
    char *type_name;
    allocate_and_copy_cstr(type_name, *(char **)argv[2 + i * 2 + 0]);

    // Strip the type name of dereference operators
    parameter_info->type_deref_count = 0;
    while (true) {
      int k = strlen(type_name) - 1;
      if (k < 0) {
        MCerror(108, "arg error");
      }

      if (type_name[k] == ' ') {
        // Do nothing
      }
      else if (type_name[k] == '*') {
        ++parameter_info->type_deref_count;
      }
      else
        break;

      // Strip of last character
      char *temp;
      allocate_and_copy_cstrn(temp, type_name, k);
      free(type_name);
      type_name = temp;
    }
    parameter_info->type_name = type_name;

    // printf("dfp-4a\n");
    // printf("chn:%p\n", command_hub->nodespace);
    // printf("ptn:%s\n", parameter_info->type_name);
    //  printf("dfp-4b\n");
    void *p_struct_info = NULL;
    MCcall(find_struct_info((void *)command_hub->nodespace, parameter_info->type_name, &p_struct_info));
    if (!strcmp(parameter_info->type_name, "node") && !p_struct_info) {
      MCerror(109, "Its not finding node in find_struct_info");
    }
    if (p_struct_info) {
      struct_info *sinfo = (struct_info *)p_struct_info;
      parameter_info->type_version = sinfo->version;

      int already_added = 0;
      for (int j = 0; j < func_info->struct_usage_count; ++j) {
        struct_info *existing = (struct_info *)func_info->struct_usage[j];

        if (!strcmp(parameter_info->type_name, existing->name)) {
          already_added = 1;
          break;
        }
      }
      if (!already_added) {
        MCcall(append_to_collection((void ***)&func_info->struct_usage, &struct_usage_alloc, &func_info->struct_usage_count,
                                    (void *)sinfo));
      }
      // printf("dfp-6\n");
    }
    else
      parameter_info->type_version = 0;

    char *param_name;
    allocate_and_copy_cstr(param_name, *(char **)argv[2 + i * 2 + 1]);
    parameter_info->name = param_name;

    func_info->parameters[i] = (mc_parameter_info_v1 *)parameter_info;
    // printf("dfp>set param[%i]=%s %s\n", i, parameter_info->type, parameter_info->name);
  }

  // Add the function info to the nodespace
  MCcall(append_to_collection((void ***)&command_hub->nodespace->functions, &command_hub->nodespace->functions_alloc,
                              &command_hub->nodespace->function_count, (void *)func_info));

  // Cleanup Parameters
  if (func_info->struct_usage_count != struct_usage_alloc) {
    if (func_info->struct_usage_count > 0) {
      struct_info **new_struct_usage = (struct_info **)malloc(sizeof(struct_info *) * func_info->struct_usage_count);
      memcpy((void *)new_struct_usage, func_info->struct_usage, sizeof(struct_info *) * func_info->struct_usage_count);

      free(func_info->struct_usage);
      func_info->struct_usage = new_struct_usage;
    }
    else {
      struct_usage_alloc = 0;
      free(func_info->struct_usage);
      func_info->struct_usage = NULL;
    }
  }

  // Declare with clint
  char buf[1024];
  strcpy(buf, "int (*");
  strcat(buf, name);
  strcat(buf, ")(int,void**);");
  printf("dfp>cling_declare:%s\n -- with %i parameters returning %s\n", buf, func_info->parameter_count, func_info->return_type);
  clint_declare(buf);
  // printf("dfp-concludes\n");
  return 0;
}

int cling_process(int argc, void **argv)
{
  // Parameters
  char *str = *(char **)argv[0];

  // printf("arg is:'%s'\n", str);

  return clint_process(str);
}

int init_void_collection(mc_void_collection_v1 **collection);

int obtain_item_from_collection(void **items, uint *allocated, uint *count, uint size_of_type, void **item)
{

  if (*allocated < *count + 1) {
    uint new_allocated = (*count + 1) + 4 + (*count + 1) / 4;
    void *new_ary = (void *)malloc(size_of_type * new_allocated);

    if (*allocated) {
      memcpy(new_ary, *items, size_of_type * (*count));
      free(*items);
    }
    *items = new_ary;
    *allocated = new_allocated;
  }

  *item = items[*count];
  ++(*count);
  return 0;
}

int obtain_resource_command(resource_queue *resource_queue, resource_command **p_command)
{
  // MCcall(obtain_item_from_collection((void **)resource_queue->commands, &resource_queue->allocated, &resource_queue->count,
  //                                    sizeof(resource_command), (void **)p_command));
  if (resource_queue->allocated < resource_queue->count + 1) {
    int new_allocated = (resource_queue->count + 1) + 4 + (resource_queue->count + 1) / 4;
    resource_command *new_ary = (resource_command *)malloc(sizeof(resource_command) * new_allocated);

    if (resource_queue->allocated) {
      memcpy(new_ary, resource_queue->commands, sizeof(resource_command) * resource_queue->count);
      free(resource_queue->commands);
    }
    resource_queue->commands = new_ary;
    resource_queue->allocated = new_allocated;
  }

  *p_command = &resource_queue->commands[resource_queue->count++];

  return 0;
}

int obtain_image_render_queue(render_queue *render_queue, image_render_queue **p_command)
{
  if (render_queue->allocated < render_queue->count + 1) {
    int new_allocated = render_queue->allocated + 4 + render_queue->allocated / 4;
    image_render_queue *new_ary = (image_render_queue *)malloc(sizeof(image_render_queue) * new_allocated);

    if (render_queue->allocated) {
      memcpy(new_ary, render_queue->image_renders, sizeof(image_render_queue) * render_queue->count);
      free(render_queue->image_renders);
    }
    for (int i = 0; i < new_allocated - render_queue->allocated; ++i) {
      new_ary[render_queue->allocated + i].commands_allocated = 4;
      new_ary[render_queue->allocated + i].command_count = 0;
      new_ary[render_queue->allocated + i].commands = (element_render_command *)malloc(
          sizeof(element_render_command) * new_ary[render_queue->allocated + i].commands_allocated);
    }

    render_queue->image_renders = new_ary;
    render_queue->allocated = new_allocated;
  }

  *p_command = &render_queue->image_renders[render_queue->count++];
  (*p_command)->command_count = 0;
  return 0;
}

int obtain_element_render_command(image_render_queue *image_queue, element_render_command **p_command)
{
  // MCcall(obtain_item_from_collection((void **)image_queue->commands, &image_queue->commands_allocated,
  //                                    &image_queue->command_count, sizeof(element_render_command), (void **)p_command));
  if (image_queue->commands_allocated < image_queue->command_count + 1) {
    int new_allocated = image_queue->commands_allocated + 4 + image_queue->commands_allocated / 4;
    element_render_command *new_ary = (element_render_command *)malloc(sizeof(element_render_command) * new_allocated);

    if (image_queue->commands_allocated) {
      memcpy(new_ary, image_queue->commands, sizeof(element_render_command) * image_queue->command_count);
      free(image_queue->commands);
    }
    image_queue->commands = new_ary;
    image_queue->commands_allocated = new_allocated;
  }

  *p_command = &image_queue->commands[image_queue->command_count++];

  return 0;
}

int force_render_update(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  // Obtain the resource command queue
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);

  // TODO -- REFACTOR
  resource_command *command;
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_TEXTURE;
  command->p_uid = &command_hub->ui_elements[0].resource_uid;
  command->data.load_texture.path = "res/texture.jpg";

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &command_hub->ui_elements[1].resource_uid;
  command->data.create_texture.width = 512;
  command->data.create_texture.height = 128;
  command->data.create_texture.use_as_render_target = true;

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &command_hub->ui_elements[2].resource_uid;
  command->data.font.path = "res/font/LiberationMono-Regular.ttf";
  command->data.font.height = 24.0f;

  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  // Wait for resources to load before proceeding
  while (command_hub->renderer.resource_queue->count) {
    usleep(100);
  }

  // Obtain the render command queue
  pthread_mutex_lock(&command_hub->renderer.render_queue->mutex);

  // Font Writing
  // image_render_queue *sequence;
  // MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  // sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  // sequence->image_width = 512;
  // sequence->image_height = 128;
  // sequence->data.target_image.image_uid = command_hub->ui_elements[1].resource_uid;

  // element_render_command *element_cmd;
  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 2;
  // element_cmd->y = 2;
  // element_cmd->data.colored_rect_info.width = 508;
  // element_cmd->data.colored_rect_info.height = 124;
  // element_cmd->data.colored_rect_info.color = teal;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 80;
  // element_cmd->y = 30;
  // element_cmd->data.colored_rect_info.width = 68;
  // element_cmd->data.colored_rect_info.height = 68;
  // element_cmd->data.colored_rect_info.color = dark_slate_gray;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  // element_cmd->x = 82;
  // element_cmd->y = 32 + 24;
  // element_cmd->data.print_text.text = "The quick brown fox jumps over the lazy dog!";
  // element_cmd->data.print_text.color = black;
  // element_cmd->data.print_text.font_resource_uid = command_hub->ui_elements[2].resource_uid;

  // For the global node (and whole screen)
  // MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  // sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  // sequence->image_width = 1024;
  // sequence->image_height = 640;
  // sequence->render_target = NODE_RENDER_TARGET_PRESENT;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 2;
  // element_cmd->y = 2;
  // element_cmd->data.colored_rect_info.width = 1020;
  // element_cmd->data.colored_rect_info.height = 636;
  // element_cmd->data.colored_rect_info.color = dark_slate_gray;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_SAMPLE_CUBE;
  // element_cmd->x = 0;
  // element_cmd->y = 0;
  // element_cmd->width = 1024;
  // element_cmd->height = 640;
  // element_cmd->data = NULL;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 244;
  // element_cmd->y = 52;
  // element_cmd->data.colored_rect_info.width = 536;
  // element_cmd->data.colored_rect_info.height = 536;
  // element_cmd->data.colored_rect_info.color = burlywood;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  // element_cmd->x = 256;
  // element_cmd->y = 64;
  // element_cmd->data.textured_rect_info.width = 512;
  // element_cmd->data.textured_rect_info.height = 128;
  // element_cmd->data.textured_rect_info.texture_uid = command_hub->ui_elements[1].resource_uid;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 300 - 8;
  // element_cmd->y = 640 - 40 - 8;
  // element_cmd->data.colored_rect_info.width = 300;
  // element_cmd->data.colored_rect_info.height = 40;
  // element_cmd->data.colored_rect_info.color = greenish;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 300 - 8 + 4;
  // element_cmd->y = 640 - 40 - 8 + 4;
  // element_cmd->data.colored_rect_info.width = 240;
  // element_cmd->data.colored_rect_info.height = 32;
  // element_cmd->data.colored_rect_info.color = ghost_white;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 300 - 8 + 4;
  // element_cmd->y = 640 - 40 - 8 + 4;
  // element_cmd->data.colored_rect_info.width = 2;
  // element_cmd->data.colored_rect_info.height = 32;
  // element_cmd->data.colored_rect_info.color = black;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 300 - 8 + 4;
  // element_cmd->y = 640 - 40 - 8 + 4;
  // element_cmd->data.colored_rect_info.width = 240;
  // element_cmd->data.colored_rect_info.height = 2;
  // element_cmd->data.colored_rect_info.color = black;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 60 - 8 + 4 - 2;
  // element_cmd->y = 640 - 40 - 8 + 4;
  // element_cmd->data.colored_rect_info.width = 2;
  // element_cmd->data.colored_rect_info.height = 32;
  // element_cmd->data.colored_rect_info.color = black;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 1024 - 300 - 8 + 4;
  // element_cmd->y = 640 - 8 - 4 - 2;
  // element_cmd->data.colored_rect_info.width = 240;
  // element_cmd->data.colored_rect_info.height = 2;
  // element_cmd->data.colored_rect_info.color = black;

  pthread_mutex_unlock(&command_hub->renderer.render_queue->mutex);

  return 0;
}

/* Conforms the given type_identity to a mc_type_identity if it is available. Searches first the given func_info, failing that
 * then searches the current nodespace.
 * @conformed_type_identity : (char **) Return Value.
 * @func_info : *(function_info **) The function info, may be NULL.
 * @type_identity : *(const char * const *) The type name to check for
 */
int conform_type_identity_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // Parameters
  char **conformed_type_identity = (char **)argv[0];
  function_info *func_info = *(function_info **)argv[1];
  const char *const type_identity = *(const char *const *)argv[2];

  // if (!strcmp(type_identity, "node")) {
  //   allocate_and_copy_cstr((*conformed_type_identity), "mc_node_v1 *");
  //   return 0;
  // }

  char *finalized_identity;
  allocate_and_copy_cstr(finalized_identity, type_identity);

  // Strip the type identity of all deref operators
  int type_deref_count = 0;
  while (true) {
    int k = strlen(finalized_identity) - 1;
    if (k < 0) {
      MCerror(108, "arg error");
    }

    if (finalized_identity[k] == ' ') {
      // Do nothing
    }
    else if (finalized_identity[k] == '*') {
      ++type_deref_count;
    }
    else
      break;

    // Strip of last character
    char *temp;
    allocate_and_copy_cstrn(temp, finalized_identity, k);
    free(finalized_identity);
    finalized_identity = temp;
  }

  bool matched = false;
  if (func_info) {
    // printf("ctn-0  %s\n", func_info->name);
    // Check for utilized version in function info
    for (int i = 0; i < func_info->struct_usage_count; ++i) {
      if (!strcmp(finalized_identity, func_info->struct_usage[i]->name)) {
        // printf("ctn-1\n");
        // Match!
        matched = true;
        free(finalized_identity);
        allocate_and_copy_cstr(finalized_identity, func_info->struct_usage[i]->declared_mc_name);
        break;
      }
    }
  }

  if (!matched) {
    // Look for type in nodespace
    void *p_struct_info;
    MCcall(find_struct_info((void *)command_hub->nodespace, finalized_identity, &p_struct_info));
    if (p_struct_info) {
      struct_info *str_info = (struct_info *)p_struct_info;
      matched = true;

      // printf("ctn-3\n");
      // Change Name
      free(finalized_identity);
      allocate_and_copy_cstr(finalized_identity, str_info->declared_mc_name);

      // Add reference to function infos struct usages
      if (func_info) {
        struct_info **new_collection = (struct_info **)malloc(sizeof(struct_info *) * (func_info->struct_usage_count + 1));
        if (func_info->struct_usage_count)
          memcpy((void *)new_collection, func_info->struct_usage, sizeof(struct_info *) * func_info->struct_usage_count);
        new_collection[func_info->struct_usage_count] = str_info;
        if (func_info->struct_usage)
          free(func_info->struct_usage);

        // // printf("ctn-4\n");
        func_info->struct_usage_count = func_info->struct_usage_count + 1;
        func_info->struct_usage = new_collection;
      }
    }
  }

  // Re-add any deref operators
  if (type_deref_count) {
    char *temp = (char *)malloc(sizeof(char) * (strlen(finalized_identity) + 1 + type_deref_count + 1));
    strcpy(temp, finalized_identity);
    strcat(temp, " ");
    for (int i = 0; i < type_deref_count; ++i)
      strcat(temp, "*");

    free(finalized_identity);
    finalized_identity = temp;
  }

  // No modification, use the original value
  allocate_and_copy_cstr((*conformed_type_identity), finalized_identity);
  free(finalized_identity);

  return 0;
}

int instantiate_function_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/
  printf("~instantiate()\n");

  char *function_name = *(char **)argv[0];
  char *script = *(char **)argv[1];

  // Find the function info
  mc_function_info_v1 *func_info = NULL;
  {
    void *mc_vargs[3];
    mc_vargs[0] = (void *)&func_info;
    mc_vargs[1] = (void *)&command_hub->nodespace;
    mc_vargs[2] = (void *)&function_name;
    MCcall(find_function_info(3, mc_vargs));
    if (!func_info) {
      MCerror(184, "cannot find function info for function_name=%s", function_name);
    }
  }
  // printf("@ifv-0\n");

  // Translate the code-block from script into workable midge-cling C
  char *midge_c;
  {
    void *mc_vargs[3];
    mc_vargs[0] = (void *)&midge_c;
    mc_vargs[1] = (void *)&func_info;
    mc_vargs[2] = (void *)&script;
    MCcall(parse_script_to_mc(3, mc_vargs));
  }

  // printf("@ifv-1\n");

  // // const char *identity; const char *type; struct_info *struct_info;
  // struct {
  //   const char *var_name;
  //   const char *type;
  //   void *struct_info;
  // } declared_types[200];
  // int declared_type_count = 0;

  // Increment function iteration
  ++func_info->latest_iteration;

  // Construct the function identifier
  const char *function_identifier_format = "%s_v%u";
  char func_identity_buf[256];
  func_identity_buf[0] = '\0';
  sprintf(func_identity_buf, function_identifier_format, func_info->name, func_info->latest_iteration);

  // printf("@ifv-2\n");
  // Construct the function parameters
  char param_buf[4096];
  param_buf[0] = '\0';
  for (int i = 0; i < func_info->parameter_count; ++i) {
    // MC conformed type name
    char *conformed_type_name;
    {
      void *mc_vargs[3];
      mc_vargs[0] = (void *)&conformed_type_name;
      mc_vargs[1] = (void *)&func_info;
      mc_vargs[2] = (void *)&func_info->parameters[i]->type_name;
      // printf("@ifv-4\n");
      MCcall(conform_type_identity(3, mc_vargs));
      // printf("@ifv-5\n");
      //  printf("ifv:paramName:'%s' conformed_type_name:'%s'\n",func_info->parameters[i]->type_name, conformed_type_name);
    }

    // Deref
    char derefbuf[24];
    if (func_info->parameters[i]->type_deref_count) {
      derefbuf[0] = ' ';
      for (int j = 0; j < func_info->parameters[i]->type_deref_count; ++j)
        derefbuf[1 + j] = '*';
      derefbuf[1 + func_info->parameters[i]->type_deref_count] = '\0';
    }
    else
      derefbuf[func_info->parameters[i]->type_deref_count] = '\0';

    // Decl
    sprintf(param_buf + strlen(param_buf), "  %s%s%s = ", conformed_type_name, derefbuf, func_info->parameters[i]->name);

    // Deref + 1 (unless its the return value)
    derefbuf[0] = ' ';
    for (int j = 0; j < func_info->parameters[i]->type_deref_count; ++j)
      derefbuf[1 + j] = '*';
    derefbuf[1 + func_info->parameters[i]->type_deref_count] = '*';
    derefbuf[1 + func_info->parameters[i]->type_deref_count + 1] = '\0';

    // Assignment
    sprintf(param_buf + strlen(param_buf), "*(%s%s)argv[%i];\n", conformed_type_name, derefbuf, i);

    free(conformed_type_name);
  }

  // printf("@ifv-3\n");
  // Declare the function
  const char *function_declaration_format = "int %s(int argc, void **argv) {\n"
                                            "  // MidgeC Function Locals\n"
                                            "  void *mc_vargs[128];\n"
                                            "\n"
                                            "  // Function Parameters\n"
                                            "%s"
                                            "\n"
                                            "  // Function Code\n"
                                            "%s"
                                            "\n"
                                            "  return 0;\n"
                                            "}";
  int function_declaration_length = snprintf(NULL, 0, function_declaration_format, func_identity_buf, param_buf, midge_c);
  char *function_declaration = (char *)malloc(sizeof(char) * (function_declaration_length + 1));
  sprintf(function_declaration, function_declaration_format, func_identity_buf, param_buf, midge_c);

  // Declare the function
  printf("ifv>cling_declare:\n%s\n", function_declaration);
  clint_declare(function_declaration);

  // Set the method to the function pointer
  char decl_buf[1024];
  sprintf(decl_buf, "%s = &%s;", func_info->name, func_identity_buf);
  // printf("ifv>clint_process:\n%s\n", decl_buf);
  clint_process(decl_buf);

  free(function_declaration);
  free(midge_c);
  // printf("ifv-concludes\n");
  return 0;
}

int parse_past_conformed_type_identifier(function_info *func_info, char *code, int *i, char **conformed_type_identity)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  *conformed_type_identity = NULL;

  if (!isalpha(code[*i])) {
    MCerror(459, "Type must begin with alpha character was:'%c'", code[*i]);
  }

  char *type_identity;
  int type_end_index;
  for (type_end_index = *i;; ++type_end_index) {
    if (code[type_end_index] == ' ' || code[type_end_index] == '\0') {
      if (code[type_end_index] == ' ' && code[type_end_index + 1] == '*') {
        do {
          ++type_end_index;
        } while (code[type_end_index] == '*');
      }

      type_identity = (char *)malloc(sizeof(char) * (type_end_index - *i + 1));
      strncpy(type_identity, code + *i, type_end_index - *i);
      type_identity[type_end_index - *i] = '\0';
      *i = type_end_index;
      break;
    }
  }

  char *conformed_result;
  {
    void *mc_vargs[3];
    mc_vargs[0] = (void *)&conformed_result;
    mc_vargs[1] = (void *)&func_info;
    mc_vargs[2] = (void *)&type_identity;
    // printf("@ppcti-2\n");
    MCcall(conform_type_identity(3, mc_vargs));
    // printf("@ppcti-3\n");
    // printf("ppcti:paramName:'%s' conformed_type_name:'%s'\n", type_identity, conformed_result);
  }
  // printf("@ppcti-4\n");
  free(type_identity);

  *conformed_type_identity = conformed_result;
  return 0;
}

int parse_past_expression(function_info *func_info, void *nodespace, char *code, int *i, char **output)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/
  char *primary, *temp;

  if (isalpha(code[*i])) {

    MCcall(parse_past_identifier(code, i, &primary, false, true));

    if (!strcmp(primary, "command_hub")) {
      int len = snprintf(NULL, 0, "((mc_command_hub_v1 *)%p)", command_hub);
      free(primary);
      primary = (char *)malloc(sizeof(char) * (len + 1));
      sprintf(primary, "((mc_command_hub_v1 *)%p)", command_hub);
    }

    switch (code[*i]) {
    case '[': {
      for (int b = 1;; ++b) {
        // Parse past the '['
        ++*i;

        char *secondary;
        parse_past_expression(func_info, (void *)nodespace, code, i, &secondary);

        temp = (char *)malloc(sizeof(char) * (strlen(primary) + 1 + strlen(secondary) + b + 1));
        strcpy(temp, primary);
        strcat(temp, "[");
        strcat(temp, secondary);
        free(primary);
        free(secondary);
        primary = temp;

        if (code[*i] != '[') {
          int k = strlen(primary);
          for (int c = 0; c < b; ++c) {
            primary[k + c] = ']';
            ++*i;
          }
          primary[k + b] = '\0';

          switch (code[*i]) {
          case ' ':
          case '\0':
          case '\n':
            *output = primary;
            return 0;
            break;
          case '-': {
            MCcall(parse_past(code, i, "->"));
            MCcall(parse_past_identifier(code, i, &secondary, true, true));
            if (code[*i] != '[') {
              temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2 + strlen(secondary) + 1));
              strcpy(temp, primary);
              strcat(temp, "->");
              strcat(temp, secondary);
              free(primary);
              free(secondary);
              *output = temp;
              return 0;
            }
            MCerror(326, "TODO ");
          }
          default:
            MCerror(329, "TODO '%c'", code[*i]);
          }
        }
      }
    } break;
    case '-': {
      parse_past(code, i, "->");
      parse_past_expression(func_info, nodespace, code, i, &temp);
      int len = snprintf(NULL, 0, "%s->%s", primary, temp);
      *output = (char *)malloc(sizeof(char) * (len + 1));
      sprintf(*output, "%s->%s", primary, temp);
      // printf("primary:'%s'\n", primary);
      // printf("temp:'%s'\n", temp);
      free(primary);
      free(temp);
      return 0;
    }
    default: {
      *output = primary;
      return 0;
    }
    }
  }
  else
    switch (code[*i]) {
    case '"': {
      MCcall(mc_parse_past_literal_string(code, i, output));

      return 0;
    }
    case '\'': {
      MCcall(parse_past_character_literal(code, i, output));

      return 0;
    }
    case '&': {
      ++*i;
      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &primary));
      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2));
      strcpy(temp, "&");
      strcat(temp, primary);
      temp[strlen(primary) + 1] = '\0';

      free(primary);
      *output = temp;

      return 0;
    }
    case '(': {
      // Assume to be a casting - do containment other ways
      parse_past(code, i, "(");
      MCcall(parse_past_conformed_type_identifier(func_info, code, i, &primary));
      parse_past(code, i, ")");

      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &temp));

      *output = (char *)malloc(sizeof(char) * (2 + strlen(primary) + strlen(temp) + 1));
      sprintf(*output, "(%s)%s", primary, temp);
      free(primary);
      free(temp);

      return 0;
    }
    case '!': {
      ++*i;
      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &primary));
      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2));
      strcpy(temp, "!");
      strcat(temp, primary);
      temp[strlen(primary) + 1] = '\0';

      free(primary);
      *output = temp;

      return 0;
    }
    case '$': {
      ++*i;

      MCcall(parse_past_expression(func_info, nodespace, code, i, output));

      char *temp = (char *)malloc(sizeof(char) * (strlen(*output) + 2));
      sprintf(temp, "$%s", *output);
      free(*output);
      *output = temp;

      return 0;
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      MCcall(parse_past_number(code, i, output));

      return 0;
    }
    case '+':
    case '-':
    case '*':
    case '/':
    case '%': {
      char *oper = (char *)malloc(sizeof(char) * 2);
      oper[0] = code[*i];
      oper[1] = '\0';
      ++*i;
      if (code[*i] != ' ') {
        MCerror(404, "BackTODO");
      }
      MCcall(parse_past(code, i, " "));

      // left
      char *left;
      MCcall(parse_past_expression(func_info, nodespace, code, i, &left));
      MCcall(parse_past(code, i, " "));

      // right
      char *right;
      MCcall(parse_past_expression(func_info, nodespace, code, i, &right));

      *output = (char *)malloc(sizeof(char) * (strlen(oper) + 1 + strlen(left) + 1 + strlen(right) + 1));
      sprintf(*output, "%s %s %s", left, oper, right);

      free(left);
      free(oper);
      free(right);
    }
      return 0;
    default: {
      MCcall(print_parse_error(code, *i, "parse_past_expression", "first_char"));
      return -427;
    }
    }

  MCerror(432, "Incorrectly Handled");
}

int create_default_mc_struct_v1(int argc, void **argv)
{
  // printf("@cdms-1\n");
  // Arguments
  void **ptr_output = (void **)argv[0];
  char *type_name = *(char **)argv[1];

  if (!strcmp(type_name, "mc_node_v1 *")) {
    // printf("@cdms-2\n");
    mc_node_v1 *data = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
    data->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
    data->struct_id->identifier = "mc_node_v1";
    data->struct_id->version = 1U;
    data->name = NULL;
    data->parent = NULL;
    data->functions_alloc = 1;
    data->function_count = 0;
    data->functions = (mc_function_info_v1 **)malloc(sizeof(mc_function_info_v1 *) * data->functions_alloc);
    data->structs_alloc = 1;
    data->struct_count = 0;
    data->structs = (mc_struct_info_v1 **)malloc(sizeof(mc_struct_info_v1 *) * data->structs_alloc);
    data->children_alloc = 1;
    data->child_count = 0;
    data->children = (mc_node_v1 **)malloc(sizeof(mc_node_v1 *) * data->children_alloc);

    *ptr_output = (void *)data;
    // printf("@cdms-3\n");
    return 0;
  }
  MCerror(616, "Unrecognized type:%s", type_name);
}

int parse_script_to_mc_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  char **output = (char **)argv[0];
  function_info *func_info = *(function_info **)argv[1];
  char *code = *(char **)argv[2];

  // Parse the script one statement at a time
  unsigned int translation_alloc = 80;
  char *translation = (char *)malloc(sizeof(char) * translation_alloc);
  translation[0] = '\0';
  char buf[2048];
  int i = 0;
  int code_len = strlen(code);
  while (i <= code_len) {
    switch (code[i]) {
    case 'a': {
      // ass / asi
      MCcall(parse_past(code, &i, "ass"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      char *dest_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &dest_expr));
      MCcall(parse_past(code, &i, " "));

      char *src_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &src_expr));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4829, "expected statement end");
      }

      sprintf(buf, "%s = %s;\n", dest_expr, src_expr);
      MCcall(append_to_cstr(&translation_alloc, &translation, buf));

      // free(var_identifier);
      // free(left);
    } break;
    case 'c': {
      // cpy - deep copy of known types
      MCcall(parse_past(code, &i, "cpy"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Type Identifier
      char *type_identifier;
      MCcall(parse_past_conformed_type_identifier(func_info, code, &i, &type_identifier));
      MCcall(parse_past(code, &i, " "));

      // Destination Name
      char *dest_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &dest_expr));
      MCcall(parse_past(code, &i, " "));

      // Source Name
      char *src_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &src_expr));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(589, "expected end of statement");
      }

      if (!strcmp(type_identifier, "char *")) {
        sprintf(buf,
                "{\n"
                "  char *mc_tmp_constchar = (char *)malloc(sizeof(char) * (strlen(%s) + 1));\n"
                "  strcpy(mc_tmp_constchar, %s);\n"
                "  %s = mc_tmp_constchar;\n"
                "}\n",
                src_expr, src_expr, dest_expr);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      }
      else {
        MCerror(727, "Unsupported deep copy type");
      }

      free(type_identifier);
      free(dest_expr);
      free(src_expr);
    } break;
    case 'd': {
      MCcall(parse_past(code, &i, "dc"));
      if (code[i] == 'd') {
        // dcd - initialization of configured mc_structures to default values
        MCcall(parse_past(code, &i, "d"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // parse_past(code, &i, "'node *' child");
        // break;
        // Identifier
        char *type_identifier;
        MCcall(parse_past_conformed_type_identifier(func_info, code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));
        // parse_past(code, &i, "child");
        // break;

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(589, "expected end of statement");
        }

        // Normal Declaration
        sprintf(buf, "%s %s;\n", type_identifier, var_name);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // Call for default allocation
        sprintf(buf,
                "{\n"
                // "printf(\"@innercdmscall-1\\n\");"
                "  void *mc_vargs[2];\n"
                "  mc_vargs[0] = (void *)&%s;\n"
                "  const char *mc_type_name = \"%s\";\n"
                "  mc_vargs[1] = (void *)&mc_type_name;\n"
                "  MCcall(create_default_mc_struct(2, mc_vargs));\n"
                "}\n",
                var_name, type_identifier);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        free(var_name);
        free(type_identifier);
      }
      else if (code[i] == 'l') {
        MCerror(576, "TODO");
        // // dcl
        // MCcall(parse_past(code, &i, "l"));
        // MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // // Identifier
        // char *type_identifier;
        // MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        // MCcall(parse_past(code, &i, " "));

        // // Variable Name
        // char *var_name;
        // MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        // if (code[i] != '\n' && code[i] != '\0') {
        //   // Array decl
        //   MCcall(parse_past(code, &i, "["));

        //   char *left;
        //   MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
        //   MCcall(parse_past(code, &i, "]"));
        //   if (code[i] != '\n' && code[i] != '\0') {
        //     MCerror(-4829, "expected statement end");
        //   }

        //   // Add deref to type
        //   char *new_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier) + 2));
        //   strcpy(new_type_identifier, type_identifier);
        //   strcat(new_type_identifier, "*");
        //   new_type_identifier[strlen(type_identifier) + 1] = '\0';

        //   // Normal Declaration
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count,
        //   script,
        //                                      buf, local_scope_depth, new_type_identifier, var_name));
        //   // printf("dcl-here-0\n");
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   // printf("dcl-here-1\n");

        //   // Allocate the array
        //   char *replace_var_name;
        //   MCcall(
        //       mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        //       &replace_var_name));
        //   // printf("dcl-here-2\n");

        //   sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_var_name, new_type_identifier, type_identifier, left);
        //   // printf("dcl-here-3\n");
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   // printf("dcl-here-4\n");

        //   free(left);
        //   // printf("dcl-here-5\n");
        //   free(new_type_identifier);
        //   // printf("dcl-here-6\n");
        //   // printf("Translation:\n%s\n", translation);
        //   // printf("dcl-here-7\n");
        // }
        // else {
        //   // Normal Declaration
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count,
        //   script,
        //                                      buf, local_scope_depth, type_identifier, var_name));
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }
        // free(var_name);
        // free(type_identifier);
      }
      else if (code[i] == 's') {
        MCerror(641, "TODO");
        //   // dcs
        //   MCcall(parse_past(code, &i, "s"));
        //   MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        //   // Identifier
        //   char *type_identifier;
        //   MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        //   MCcall(parse_past(code, &i, " "));

        //   // Variable Name
        //   char *var_name;
        //   MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        //   MCcall(parse_past(code, &i, " "));

        //   // Set Value
        //   char *left;
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        //   if (code[i] != '\n' && code[i] != '\0') {
        //     MCerror(-4829, "expected statement end");
        //   }

        //   // Generate Local
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count,
        //   script,
        //                                      buf, local_scope_depth, type_identifier, var_name));
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        //   // Assign
        //   char *replace_var_name;
        //   MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        //   &replace_var_name));

        //   sprintf(buf, "%s = %s;\n", replace_var_name, left);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        //   free(type_identifier);
        //   free(var_name);
        //   free(left);
        // }
        // else {
        //   MCerror(678, "TODO");
        // }
      }
    } break;
    case 'n': {
      MCcall(parse_past(code, &i, "nv"));
      if (code[i] == 'i') {
        MCerror(286, "TODO");
        // // nvi
        // MCcall(parse_past(code, &i, "i"));
        // MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // // Type Identifier
        // char *type_identifier;
        // MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        // MCcall(parse_past(code, &i, " "));

        // // Variable Name
        // char *var_name;
        // MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        // MCcall(parse_past(code, &i, " "));

        // MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
        //                                    buf, local_scope_depth, type_identifier, var_name));
        // append_to_cstr(&translation_alloc, &translation, buf);

        // char *replace_name;
        // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));
        // // printf("nvi gen replace_name:%s=%s\n", var_name, replace_name);

        // // Invoke
        // // Function Name
        // char *function_name;
        // MCcall(parse_past_identifier(code, &i, &function_name, true, false));

        // // printf("dopeh\n");
        // function_info_v1 *func_info;
        // {
        //   void *mc_vargs[3];
        //   mc_vargs[0] = (void *)&func_info;
        //   mc_vargs[1] = (void *)&nodespace;
        //   mc_vargs[2] = (void *)&function_name;
        //   find_function_info(3, mc_vargs);
        // }
        // // printf("dopey\n");
        // if (func_info) {
        //   if (!strcmp(func_info->return_type, "void")) {
        //     MCerror(-1002, "compile error: cannot assign from a void function!");
        //   }
        //   else {
        //     sprintf(buf,
        //             "{\n"
        //             "  mc_vargs[0] = (void *)&%s;\n",
        //             replace_name);
        //     MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   }
        // }
        // else {
        //   sprintf(buf, "%s = %s(", replace_name, function_name);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }

        // int arg_index = 0;
        // while (code[i] != '\n' && code[i] != '\0') {
        //   MCcall(parse_past(code, &i, " "));

        //   char *argument;
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &argument));
        //   // MCcall(parse_past_identifier(code, &i, &argument, true, true));
        //   // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, argument,
        //   // &replace_name)); char *arg_entry = argument; if (replace_name)
        //   //   arg_entry = replace_name;
        //   if (func_info) {
        //     if (argument[0] == '$') {
        //       sprintf(buf,
        //               "  char *mc_context_data_%i;\n"
        //               "  MCcall(get_process_contextual_data(script_instance->contextual_action, \"%s\", (void "
        //               "**)&mc_context_data_%i));\n"
        //               "  mc_vargs[%i] = (void *)&mc_context_data_%i;\n",
        //               arg_index + 1, argument + 1, arg_index + 1, arg_index + 1, arg_index + 1);
        //     }
        //     else {
        //       sprintf(buf, "mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, argument);
        //     }
        //   }
        //   else {
        //     if (argument[0] == '$') {
        //       MCerror(4829, "NOT YET IMPLEMENTED");
        //     }
        //     sprintf(buf, "%s%s", arg_index ? ", " : "", argument);
        //   }
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   ++arg_index;
        //   free(argument);
        // }

        // if (func_info) {

        //   // append_to_cstr(&translation_alloc, &translation, "  printf(\"here-22  =%s\\n\", (char *)mc_vargs[2]);\n");
        //   sprintf(buf, "  %s(%i, mc_vargs", function_name, arg_index + 1);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }

        // MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
        // if (func_info)
        //   MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));
        // // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-31\\n\");\n"));

        // free(function_name);
      }
      else if (code[i] == 'k') {
        // nvk
        MCcall(parse_past(code, &i, "k"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Invoke
        // Function
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));
        MCcall(append_to_cstr(&translation_alloc, &translation, function_name));
        MCcall(append_to_cstr(&translation_alloc, &translation, "("));

        function_info *func_info;
        {
          void *mc_vargs[3];
          mc_vargs[0] = (void *)&func_info;
          mc_vargs[1] = (void *)&command_hub->nodespace;
          mc_vargs[2] = (void *)&function_name;
          find_function_info(3, mc_vargs);
        }
        if (func_info) {
          // Return Parameter
          sprintf(buf, "{\n"
                       "  mc_vargs[0] = NULL;\n");
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }
        else {
          sprintf(buf, "%s(", function_name);
        }

        // Arguments
        int arg_index = 0;
        while (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " "));

          char *argument;
          MCcall(parse_past_expression(func_info, (void *)command_hub->nodespace, code, &i, &argument));

          // printf("argument='%s'\n", argument);

          if (func_info) {
            sprintf(buf, "  mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, argument);
          }
          else {
            sprintf(buf, "%s%s", arg_index ? ", " : "", argument);
          }
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          ++arg_index;
          free(argument);
        }

        if (func_info) {
          // append_to_cstr(&translation_alloc, &translation, "  printf(\"here-22  =%s\\n\", (char *)mc_vargs[2]);\n");
          sprintf(buf, "  %s(%i, mc_vargs", func_info->name, arg_index + 1);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }

        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
        if (func_info)
          MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));
        // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-31\\n\");\n"));

        free(function_name);
      }
      else {
        MCerror(742, "TODO");
      }
    } break;
    case '\n': {
      MCcall(parse_past(code, &i, "\n"));
    } break;
    case ' ': {
      MCcall(parse_past(code, &i, " "));
    } break;
    case '\t': {
      MCcall(parse_past(code, &i, "\t"));
    } break;
    case '\0': {
      // Trim translation
      *output = translation;
      return 0;
    }
    default: {
      // printf("\ntranslation:\n%s\n\n", translation);
      MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "UnhandledStatement"));
      return 285;
    }
    }
  }

  MCerror(279, "Improperly terminated script argument");
}

// int build_initial_workspace_v1(int argc, void **argv)
// {
//   /*mcfuncreplace*/
//   mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
//                                   // find_struct_info/find_function_info and do the same there.
//   /*mcfuncreplace*/

//   // Build the function editor window
//   mc_node_v1 *visual_hierarchy_node = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
//   visual_hierarchy_node->name = "visual_hierarchy";
//   visual_hierarchy_node->parent = NULL;
//   visual_hierarchy_node->functions_alloc = 40;
//   visual_hierarchy_node->functions =
//       (mc_function_info_v1 **)calloc(sizeof(mc_function_info_v1 *), visual_hierarchy_node->functions_alloc);
//   visual_hierarchy_node->function_count = 0;
//   visual_hierarchy_node->structs_alloc = 40;
//   visual_hierarchy_node->structs =
//       (mc_struct_info_v1 **)calloc(sizeof(mc_struct_info_v1 *), visual_hierarchy_node->structs_alloc);
//   visual_hierarchy_node->struct_count = 0;
//   visual_hierarchy_node->children_alloc = 40;
//   visual_hierarchy_node->children = (mc_node_v1 **)calloc(sizeof(mc_node_v1 *), visual_hierarchy_node->children_alloc);
//   visual_hierarchy_node->child_count = 0;

//   MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
//                               &command_hub->global_node->child_count, visual_hierarchy_node));

//   // Visuals
//   visual_hierarchy_node->data.visual.image_resource_uid = 0;
//   visual_hierarchy_node->data.visual.requires_render_update = true;
//   visual_hierarchy_node->data.visual.bounds = (rectf){42, 24, 128, 512};
//   visual_hierarchy_node->data.visual.background = (mc_visual_element_v1)

//   // Obtain the resource command queue
//   pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);

//   resource_command *command;
//   MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
//   command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
//   command->p_uid = &visual_hierarchy_node->data.visual.image_resource_uid;
//   command->data.create_texture.width = visual_hierarchy_node->data.visual.bounds.width;
//   command->data.create_texture.height = visual_hierarchy_node->data.visual.bounds.height;
//   command->data.create_texture.use_as_render_target = true;
//   pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

// #define COLOR_BLACK (render_color) { 0.f, 0.f, 0.f, 1.f}

//   // MCcall(add_colored_rect)

//   // void * data = {0, 0, 1};
//   // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
//   // element_cmd->x = 1024 - 60 - 8 + 4 - 2;
//   // element_cmd->y = 640 - 40 - 8 + 4;
//   // element_cmd->data.colored_rect_info.width = 2;
//   // element_cmd->data.colored_rect_info.height = 32;
//   // element_cmd->data.colored_rect_info.color = COLOR_BLACK;

//   return 0;
// }

int update_interactive_console_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  frame_time *elapsed = (frame_time *)argv[0];
  const char *commands[1] = {// Create invoke function script
                             ".beginNewFunction"};

  if (command_hub->interactive_console->logic.action_count < 1 && elapsed->app_sec >= 4) {
    // Enter text into the textbox
    ++command_hub->interactive_console->logic.action_count;

    command_hub->interactive_console->input_line.text = commands[0];
    command_hub->interactive_console->input_line.requires_render_update = true;
    command_hub->interactive_console->visual.requires_render_update = true;
  }
  else if (command_hub->interactive_console->logic.action_count < 2 && elapsed->app_sec >= 5) {
    // Submit the text in the textbox
    ++command_hub->interactive_console->logic.action_count;

    ((mc_node_v1 *)command_hub->global_node->children[0])->data.visual.hidden = false;
    ((mc_node_v1 *)command_hub->global_node->children[0])->data.visual.requires_render_update = true;

    command_hub->interactive_console->input_line.text = "...";
    command_hub->interactive_console->input_line.requires_render_update = true;
    command_hub->interactive_console->visual.requires_render_update = true;
  }

  // const char *commands =
  //     // Create invoke function script
  //     ".createScript\n"
  //     "nvi 'function_info *' finfo find_function_info nodespace @function_to_invoke\n"
  //     "ifs !finfo\n"
  //     "err 2455 \"Could not find function_info for specified function\"\n"
  //     "end\n"
  //     ""
  //     "dcs int rind 0\n"
  //     "dcl 'char *' responses[32]\n"
  //     ""
  //     "dcs int linit finfo->parameter_count\n"
  //     "ifs finfo->variable_parameter_begin_index >= 0\n"
  //     "ass linit finfo->variable_parameter_begin_index\n"
  //     "end\n"
  //     "for i 0 linit\n"
  //     "dcl char provocation[512]\n"
  //     "nvk strcpy provocation finfo->parameters[i]->name\n"
  //     "nvk strcat provocation \": \"\n"
  //     "$pi responses[rind] provocation\n"
  //     "ass rind + rind 1\n"
  //     "end for\n"
  //     // "nvk printf \"func_name:%s\\n\" finfo->name\n"
  //     "ifs finfo->variable_parameter_begin_index >= 0\n"
  //     "dcs int pind finfo->variable_parameter_begin_index\n"
  //     "whl 1\n"
  //     "dcl char provocation[512]\n"
  //     "nvk strcpy provocation finfo->parameters[pind]->name\n"
  //     "nvk strcat provocation \": \"\n"
  //     "$pi responses[rind] provocation\n"
  //     "nvi bool end_it strcmp responses[rind] \"finish\"\n"
  //     "ifs !end_it\n"
  //     "brk\n"
  //     "end\n"
  //     // "nvk printf \"responses[1]='%s'\\n\" responses[1]\n"
  //     "ass rind + rind 1\n"
  //     "ass pind + pind 1\n"
  //     "ass pind % pind finfo->parameter_count\n"
  //     "ifs pind < finfo->variable_parameter_begin_index\n"
  //     "ass pind finfo->variable_parameter_begin_index\n"
  //     "end\n"
  //     "end\n"
  //     "end\n"
  //     "$nv @function_to_invoke $ya rind responses\n"
  //     "|"
  //     "invoke_function_with_args_script|"
  //     "demo|"
  //     "invoke @function_to_invoke|"
  //     "mc_dummy_function|"
  //     ".runScript invoke_function_with_args_script|"
  //     "enddemo|"
  //     // // "demo|"
  //     // // "call dummy thrice|"
  //     // // "invoke mc_dummy_function|"
  //     // // "invoke mc_dummy_function|"
  //     // // "invoke mc_dummy_function|"
  //     // // "enddemo|"
  //     "demo|"
  //     "create function @create_function_name|"
  //     "construct_and_attach_child_node|"
  //     "invoke declare_function_pointer|"
  //     // ---- SCRIPT SEQUENCE ----
  //     // ---- void declare_function_pointer(char *function_name, char *return_type, [char *parameter_type,
  //     // ---- char *parameter_name]...);
  //     // > function_name:
  //     "@create_function_name|"
  //     // > return_type:
  //     "void|"
  //     // > parameter_type:
  //     "const char *|"
  //     // > parameter_name:
  //     "node_name|"
  //     // > Parameter 1 type:
  //     "finish|"
  //     // ---- END SCRIPT SEQUENCE ----
  //     // ---- SCRIPT SEQUENCE ----
  //     // ---- void instantiate_function(char *function_name, char *mc_script);
  //     "invoke instantiate_function|"
  //     "@create_function_name|"
  //     // "nvk printf \"got here, node_name=%s\\n\" node_name\n"
  //     "dcd node * child\n"
  //     "cpy char * child->name node_name\n"
  //     "ass child->parent command_hub->nodespace\n"
  //     "nvk append_to_collection (void ***)&child->parent->children &child->parent->children_alloc &child->parent->child_count "
  //     "(void *)child\n"
  //     "|"
  //     "enddemo|"
  //     // // -- END DEMO create function $create_function_name
  //     // "invoke force_render_update|"
  //     "invoke construct_and_attach_child_node|"
  //     "command_interface_node|"
  //     // "invoke set_nodespace|"
  //     // "command_interface_node|"

  //     // "create function print_word|"
  //     // "@create_function_name|"
  //     // "void|"
  //     // "char *|"
  //     // "word|"
  //     // "finish|"
  //     // "@create_function_name|"
  //     // "nvk printf \"\\n\\nThe %s is the Word!!!\\n\" word\n"
  //     // "|"
  //     // "invoke print_word|"
  //     // "===========$================$===============$============$============|"
  //     // clint->declare("void updateUI(mthread_info *p_render_thread) { int ms = 0; while(ms < 40000 &&"
  //     //                " !p_render_thread->has_concluded) { ++ms; usleep(1000); } }");

  //     // clint->process("mthread_info *rthr;");
  //     // // printf("process(begin)\n");
  //     // clint->process("begin_mthread(midge_render_thread, &rthr, (void *)&rthr);");
  //     // printf("process(updateUI)\n");
  //     // clint->process("updateUI(rthr);");
  //     // printf("process(end)\n");
  //     // clint->process("end_mthread(rthr);");
  //     // printf("\n! MIDGE COMPLETE !\n");
  //     "midgequit|";

  // // MCerror(2553, "TODO ?? have to reuse @create_function_name variable in processes...");

  // // Command Loop
  // printf("\n:> ");
  // int n = strlen(commands);
  // int s = 0;
  // char cstr[2048];
  // mc_process_action_v1 *suggestion = NULL;
  // void *vargs[12]; // TODO -- count
  // for (int i = 0; i < n; ++i) {
  //   if (commands[i] != '|')
  //     continue;
  //   strncpy(cstr, commands + s, i - s);
  //   cstr[i - s] = '\0';
  //   s = i + 1;

  //   vargs[0] = (void *)command_hub;
  //   vargs[4] = (void *)cstr;
  //   vargs[6] = (void *)&suggestion;

  //   if (!strcmp(cstr, "midgequit")) {
  //     printf("midgequit\n");
  //     break;
  //   }

  //   // printf("========================================\n");
  //   if (suggestion) {
  //     printf("%s]%s\n>: ", get_action_type_string(suggestion->type), suggestion->dialogue);
  //     release_process_action(suggestion);
  //     suggestion = NULL;
  //   }
  //   MCcall(submit_user_command(12, vargs));

  //   // if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BROKEN)
  //   // {
  //   //   printf("\nUNHANDLED_COMMAND_SEQUENCE\n");
  //   //   break;
  //   // }
  //   // if (reply != NULL)
  //   // {
  //   //   printf("%s", reply);
  //   // }
  // }

  // if (global->child_count > 0) {
  //   mc_node_v1 *child = (mc_node_v1 *)global->children[0];
  //   printf("\n>> global has a child named %s!\n", child->name);
  // }
  // else {
  //   printf("\n>> global has no children\n");
  // }
  return 0;
}

int interactive_console_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  window_input_event *event = (window_input_event *)argv[0];

  if (event->type == INPUT_EVENT_KEY_RELEASE)
    printf("ic_input:%i\n", event->code);

  return 0;
}

int render_interactive_console_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // printf("command_hub->interactive_console->visual.image_resource_uid=%u\n",
  //        command_hub->interactive_console->visual.image_resource_uid);
  image_render_queue *sequence;
  element_render_command *element_cmd;
  // Input Line
  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->clear_color = (render_color){0.13f, 0.13f, 0.13f, 1.f};
  sequence->image_width = command_hub->interactive_console->input_line.width;
  sequence->image_height = command_hub->interactive_console->input_line.height;
  sequence->data.target_image.image_uid = command_hub->interactive_console->input_line.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  element_cmd->x = 2;
  element_cmd->y = 2 + 18;
  element_cmd->data.print_text.text = command_hub->interactive_console->input_line.text;
  element_cmd->data.print_text.font_resource_uid = command_hub->interactive_console->font_resource_uid;
  element_cmd->data.print_text.color = (render_color){0.61f, 0.86f, 0.99f, 1.f};

  // Render
  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = command_hub->interactive_console->bounds.width;
  sequence->image_height = command_hub->interactive_console->bounds.height;
  sequence->clear_color = COLOR_CORNFLOWER_BLUE;
  sequence->data.target_image.image_uid = command_hub->interactive_console->visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 2;
  element_cmd->y = 2;
  element_cmd->data.colored_rect_info.width = command_hub->interactive_console->bounds.width - 4;
  element_cmd->data.colored_rect_info.height = command_hub->interactive_console->bounds.height - 4;
  element_cmd->data.colored_rect_info.color = (render_color){0.05f, 0.05f, 0.05f, 1.f};

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 0;
  element_cmd->y = command_hub->interactive_console->bounds.height - 28;
  element_cmd->data.colored_rect_info.width = command_hub->interactive_console->bounds.width;
  element_cmd->data.colored_rect_info.height = 28;
  element_cmd->data.colored_rect_info.color = COLOR_TEAL;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  element_cmd->x = 2;
  element_cmd->y = command_hub->interactive_console->bounds.height - 24 - 2;
  element_cmd->data.textured_rect_info.width = command_hub->interactive_console->input_line.width;
  element_cmd->data.textured_rect_info.height = command_hub->interactive_console->input_line.height;
  element_cmd->data.textured_rect_info.texture_uid = command_hub->interactive_console->input_line.image_resource_uid;

  return 0;
}

int build_interactive_console_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // Build the interactive console
  mc_interactive_console_v1 *console = (mc_interactive_console_v1 *)malloc(sizeof(mc_interactive_console_v1));
  command_hub->interactive_console = console;

  // interactive_console_node->functions_alloc = 40;
  // interactive_console_node->functions =
  //     (mc_function_info_v1 **)calloc(sizeof(mc_function_info_v1 *), interactive_console_node->functions_alloc);
  // interactive_console_node->function_count = 0;
  // interactive_console_node->structs_alloc = 40;
  // interactive_console_node->structs =
  //     (mc_struct_info_v1 **)calloc(sizeof(mc_struct_info_v1 *), interactive_console_node->structs_alloc);
  // interactive_console_node->struct_count = 0;
  // interactive_console_node->children_alloc = 40;
  // interactive_console_node->children = (mc_node_v1 **)calloc(sizeof(mc_node_v1 *), interactive_console_node->children_alloc);
  // interactive_console_node->child_count = 0;
  console->logic_delegate = &update_interactive_console_v1;
  console->handle_input_delegate = &interactive_console_handle_input_v1;

  // Logic
  command_hub->interactive_console->logic.action_count = 0;

  // Visuals
  console->visual.image_resource_uid = 0;
  console->visual.requires_render_update = true;
  console->bounds.x = 668;
  console->bounds.y = 510 - 200;
  console->bounds.width = 768;
  console->bounds.height = 384;
  console->visual.render_delegate = &render_interactive_console_v1;
  // interactive_console_node->data.visual.background = (mc_visual_element_v1)

  console->input_line.requires_render_update = true;
  console->input_line.text = "...";
  console->input_line.width = console->bounds.width - 4;
  console->input_line.height = 24;

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);

  resource_command *command;
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &console->visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = console->bounds.width;
  command->data.create_texture.height = console->bounds.height;
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &console->input_line.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = console->input_line.width;
  command->data.create_texture.height = console->input_line.height;
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &console->font_resource_uid;
  command->data.font.height = 20;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  return 0;
}

int render_global_node_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // For the global node (and whole screen)
  image_render_queue *sequence;
  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->image_width = 1440;
  sequence->image_height = 900;
  sequence->clear_color = COLOR_DARK_SLATE_GRAY;
  sequence->render_target = NODE_RENDER_TARGET_PRESENT;

  element_render_command *element_cmd;

  for (int i = 0; i < command_hub->global_node->child_count; ++i) {
    mc_node_v1 *child = (mc_node_v1 *)command_hub->global_node->children[i];
    if (child->type == NODE_TYPE_VISUAL && !child->data.visual.hidden && child->data.visual.image_resource_uid) {
      MCcall(obtain_element_render_command(sequence, &element_cmd));
      element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
      element_cmd->x = child->data.visual.bounds.x;
      element_cmd->y = child->data.visual.bounds.y;
      element_cmd->data.textured_rect_info.width = child->data.visual.bounds.width;
      element_cmd->data.textured_rect_info.height = child->data.visual.bounds.height;
      element_cmd->data.textured_rect_info.texture_uid = child->data.visual.image_resource_uid;
    }
  }

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  element_cmd->x = command_hub->interactive_console->bounds.x;
  element_cmd->y = command_hub->interactive_console->bounds.y;
  element_cmd->data.textured_rect_info.width = command_hub->interactive_console->bounds.width;
  element_cmd->data.textured_rect_info.height = command_hub->interactive_console->bounds.height;
  element_cmd->data.textured_rect_info.texture_uid = command_hub->interactive_console->visual.image_resource_uid;

  return 0;
}

#define CODE_LINE_COUNT 16
#define MAX_LINE_WIDTH 120
typedef struct code_line {
  uint index;
  bool requires_render_update;
  uint image_resource_uid;
  char text[MAX_LINE_WIDTH];
  uint width, height;
} code_line;
typedef struct function_edit_info {
  code_line lines[CODE_LINE_COUNT];
  uint cursorCol, cursorLine;
} function_edit_info;

int render_function_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // frame_time const *const elapsed = (frame_time const *const)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  // printf("command_hub->interactive_console->visual.image_resource_uid=%u\n",
  //        command_hub->interactive_console->visual.image_resource_uid);
  image_render_queue *sequence;
  element_render_command *element_cmd;
  // Lines
  function_edit_info *state = (function_edit_info *)visual_node->extra;
  code_line *lines = state->lines;
  for (int i = 0; i < CODE_LINE_COUNT; ++i) {
    if (lines[i].requires_render_update) {
      lines[i].requires_render_update = false;

      MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
      sequence->render_target = NODE_RENDER_TARGET_IMAGE;
      sequence->clear_color = (render_color){0.13f, 0.13f, 0.13f, 1.f};
      sequence->image_width = lines[i].width;
      sequence->image_height = lines[i].height;
      sequence->data.target_image.image_uid = lines[i].image_resource_uid;

      if (lines[i].text && strlen(lines[i].text)) {
        MCcall(obtain_element_render_command(sequence, &element_cmd));
        element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
        element_cmd->x = 4;
        element_cmd->y = 2 + 18;
        element_cmd->data.print_text.text = lines[i].text;
        element_cmd->data.print_text.font_resource_uid = command_hub->interactive_console->font_resource_uid;
        element_cmd->data.print_text.color = (render_color){0.61f, 0.86f, 0.99f, 1.f};
      }
    }
  }

  // Render
  // printf("OIRS: w:%u h:%u uid:%u\n", visual_node->data.visual.bounds.width, visual_node->data.visual.bounds.height,
  //        visual_node->data.visual.image_resource_uid);
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
  element_cmd->data.colored_rect_info.color = (render_color){0.13f, 0.13f, 0.13f, 1.f};

  for (int i = 0; i < CODE_LINE_COUNT; ++i) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
    element_cmd->x = 2;
    element_cmd->y = 8 + i * 24;
    element_cmd->data.textured_rect_info.width = lines[i].width;
    element_cmd->data.textured_rect_info.height = lines[i].height;
    element_cmd->data.textured_rect_info.texture_uid = lines[i].image_resource_uid;
  }

  return 0;
}

int update_function_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  frame_time const *const elapsed = (frame_time const *const)argv[0];
  mc_node_v1 *fedit = (mc_node_v1 *)argv[1];

  function_edit_info *state = (function_edit_info *)fedit->extra;

  // bool shouldCursorBeVisible= elapsed->app_nsec > 500000000L;
  // if(state->cursorVisible != shouldCursorBeVisible){

  // }

  return 0;
}

int function_editor_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  frame_time const *const elapsed = (frame_time const *const)argv[0];
  mc_node_v1 *fedit = (mc_node_v1 *)argv[1];
  mc_input_event_v1 *event = (mc_input_event_v1 *)argv[2];

  function_edit_info *state = (function_edit_info *)fedit->extra;

  // bool shouldCursorBeVisible= elapsed->app_nsec > 500000000L;
  // if(state->cursorVisible != shouldCursorBeVisible){

  printf("fehi-Input:%i\n", event->event->code);
  char c = '\0';
  int res = get_key_input_code_char(false, event->event->code, &c);
  if (res)
    return 0; // TODO

  printf("char c:'%c'\n", c);
  printf("state:'%p'\n", state);
  // Update the text
  for (int i = state->cursorCol + 1; i < MAX_LINE_WIDTH; ++i) {
  printf("fehi-i:%i\n", i);
    state->lines[state->cursorLine].text[i] = state->lines[state->cursorLine].text[i - 1];
  }
  printf("fehi-0\n");
  state->lines[state->cursorLine].text[state->cursorCol] = c;
  printf("fehi-1\n");
  state->lines[state->cursorLine].requires_render_update = true;
  printf("fehi-2\n");
  fedit->data.visual.requires_render_update = true;
  printf("fehi-3\n");

  return 0;
}

int register_update_timer(int (*fnptr_update_callback)(int, void **), uint usecs_period, void *state)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  bool reset_timer_on_update = true; // future argument

  update_callback_timer *callback_timer;
  MCcall(obtain_item_from_collection((void **)&command_hub->update_timers.callbacks, &command_hub->update_timers.allocated,
                                     &command_hub->update_timers.count, sizeof(update_callback_timer), (void **)&callback_timer));

  callback_timer->last_update = (struct timespec){0L, 0L};
  callback_timer->reset_timer_on_update = true;
  callback_timer->update_delegate = &update_function_editor_v1;
  callback_timer->state = state;

  return 0;
}

int build_function_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/
  printf("bfe-a\n");

  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *fedit = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  fedit->name = "function_editor";
  fedit->parent = command_hub->global_node;
  fedit->type = NODE_TYPE_VISUAL;

  fedit->data.visual.bounds.x = 20;
  fedit->data.visual.bounds.y = 120;
  fedit->data.visual.bounds.width = 640;
  fedit->data.visual.bounds.height = 400;
  fedit->data.visual.image_resource_uid = 0;
  fedit->data.visual.requires_render_update = true;
  fedit->data.visual.render_delegate = &render_function_editor_v1;
  fedit->data.visual.hidden = true;
  fedit->data.visual.input_handler = &function_editor_handle_input_v1;

  register_update_timer(&update_function_editor_v1, 500 * 1000, (void *)fedit);

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fedit));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Code Lines
  function_edit_info *state = (function_edit_info *)malloc(sizeof(function_edit_info));
  printf("state:'%p'\n", state);
  state->cursorLine = 0;
  state->cursorCol = 0;
  code_line *code_lines = state->lines;
  for (int i = 0; i < CODE_LINE_COUNT; ++i) {

    code_lines[i].index = i;
    code_lines[i].requires_render_update = true;
    code_lines[i].text[0] = '\0';
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! ";

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &code_lines[i].image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = code_lines[i].width = fedit->data.visual.bounds.width - 4;
    command->data.create_texture.height = code_lines[i].height = 28;
  }
  fedit->extra = state;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &fedit->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = fedit->data.visual.bounds.width;
  command->data.create_texture.height = fedit->data.visual.bounds.height;
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  // MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  // command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  // command->p_uid = &console->input_line.image_resource_uid;
  // command->data.create_texture.use_as_render_target = true;
  // command->data.create_texture.width = console->input_line.width;
  // command->data.create_texture.height = console->input_line.height;
  // pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  // MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  // command->type = RESOURCE_COMMAND_LOAD_FONT;
  // command->p_uid = &console->font_resource_uid;
  // command->data.font.height = 20;
  // command->data.font.path = "res/font/DroidSansMono.ttf";
  // pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}