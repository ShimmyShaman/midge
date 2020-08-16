/* core_source_loader.c */

#include "midge_common.h"

int _mcl_obtain_core_source_information(void **p_core_source_info)
{
  const int STRUCT = 11;
  const int FUNCTION = 12;
  const int ENUM = 13;
  struct core_structure_info {
    int type;
    const char *identity;
    const char *filepath;
  };

  const int MAX_OBJECTS_COUNT = 50;
  struct core_structure_info *objects =
      (struct core_structure_info *)malloc(sizeof(struct core_structure_info) * MAX_OBJECTS_COUNT);
  int objects_index = 0;

  // Set
  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "struct_id";
  objects[objects_index++].identity = "core/core_definitions.h";

  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "node";
  objects[objects_index++].identity = "core/core_definitions.h";

  objects[objects_index].type = 0;
  if (objects_index >= MAX_OBJECTS_COUNT) {
    MCerror(17, "Increase Array Size");
  }
  *p_core_source_info = (void *)objects;

  return 0;
}

int _mcl_read_all_file_text(char *filepath, char **contents)
{
  // Load the text from the core functions directory
  FILE *f = fopen(filepath, "rb");
  if (!f) {
    MCerror(44, "Could not open filepath:'%s'", filepath);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  *contents = (char *)malloc(fsize + 1);
  fread(*contents, sizeof(char), fsize, f);
  fclose(f);

  return 0;
}

// int _mcl_find_sequence_in_text_ignoring_empty_text(const char *text, const char *prefix, const char *middle,
//                                                    const char *postfix, int *index)
// {

//   for (int i = 0;; ++i) {

//     // Prefix
//     int s;
//     for (s = 0;; ++s) {
//       if(prefix[s] == '\0'){
//         break;
//       }
//       if(text[i + s] != prefix[s]) {
//         s = -1;
//         break;
//       }
//     }
//     if(s < 0){
//       continue;
//     }
//     while(text[i + ])
//   }

//   MCerror(60, "Could not find '%s'>'%s'>'%s'", prefix, middle, postfix);
// }

int _mcl_load_core_temp_struct(const char *filepath, const char *identity)
{
  char *file_text;
  MCcall(_mcl_read_all_file_text(filepath, &file_text));

  int index;
  MCcall(_mcl_find_sequence_in_text_ignoring_empty_text(file_text, "typedef struct", identity, "", &index));
}

int _mcl_load_core_temp_source(void *p_core_source_info)
{
  const int STRUCT = 11;
  const int FUNCTION = 12;
  const int ENUM = 13;
  struct core_structure_info {
    int type;
    const char *identity;
    const char *filepath;
  };

  struct core_structure_info *core_source_objects = (struct core_structure_info *)p_core_source_info;

  int objects_index = 0;
  while (core_source_objects[objects_index].type) {
    switch (core_source_objects[objects_index].type) {
    case STRUCT:
      MCcall(_mcl_load_core_temp_struct(core_source_objects[objects_index].filepath,
                                        core_source_objects[objects_index].identity));
      break;
    default:
      MCerror(60, "Unsupported:%i", core_source_objects[objects_index].type);
      break;
    }

    ++objects_index;
  }

  return 0;
}

int _mcl_init_core_data(void **p_command_hub)
{
  char buf[1280];
  sprintf(buf,
          "{"
          "  mc_node_v1 *global = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);"
          "  global->name = \"global\";"
          "  global->parent = NULL;"
          "  global->functions.alloc = 40;"
          "  global->functions.count = 0;"
          "  global->functions.items = (mc_function_info_v1 **)calloc(sizeof(mc_function_info_v1 *), "
          "                             global->functions_alloc);"
          "  global->structs.alloc = 40;"
          "  global->struct.count = 0;"
          "  global->structs.items = (mc_struct_info_v1 **)calloc(sizeof(mc_struct_info_v1 *), global->structs_alloc);"
          "  global->children.alloc = 40;"
          "  global->children.count = 0;"
          "  global->children.items = (mc_node_v1 **)calloc(sizeof(mc_node_v1 *), global->children_alloc);"
          "  global->event_handlers.alloc = 0;"
          "  global->event_handlers.count = 0;"
          ""
          "  mc_command_hub_v1 *command_hub = (mc_command_hub_v1 *)calloc(sizeof(mc_command_hub_v1), 1);"
          "  command_hub->global_node = global;"
          "  command_hub->nodespace = global;"
          "  command_hub->uid_counter = 2000;"
          "  command_hub->source_files.alloc = 0;"
          "  command_hub->source_files.count = 0;"
          "  allocate_and_copy_cstr(command_hub->clipboard_text, "
          ");"

          "  *(void **)(%p) = (void *)command_hub;"
          "}",
          p_command_hub);
  clint_process(buf);

  return 0;
}

int _mcl_load_core_mc_source(void *command_hub, void *p_core_source_info) { return 0; }

int _mcl_load_app_mc_source(void *command_hub) { return 0; }

int mcl_load_app_source(void **command_hub)
{
  void *p_core_source_info;
  MCcall(_mcl_obtain_core_source_information(&p_core_source_info));
  MCcall(_mcl_load_core_temp_source(p_core_source_info));

  MCcall(_mcl_init_core_data(command_hub));

  MCcall(_mcl_load_core_mc_source(*command_hub, p_core_source_info));

  MCcall(_mcl_load_app_mc_source(*command_hub));

  return 0;
}