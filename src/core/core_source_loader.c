/* core_source_loader.c */

#include "midge_common.h"

int _mcl_obtain_core_source_information(void **p_core_source_info)
{
  const int STRUCT = 11;
  const int FUNCTION = 12;
  const int ENUM = 13;
  struct core_object_info {
    int type;
    const char *identity;
    const char *filepath;
  };

  const int MAX_OBJECTS_COUNT = 50;
  struct core_object_info *objects =
      (struct core_object_info *)malloc(sizeof(struct core_object_info) * MAX_OBJECTS_COUNT);
  int objects_index = 0;

  // Set
  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "struct_id";
  objects[objects_index++].filepath = "src/core/core_definitions.h";

  objects[objects_index].type = STRUCT;
  objects[objects_index].identity = "node";
  objects[objects_index++].filepath = "src/core/core_definitions.h";

  objects[objects_index].type = 0;
  if (objects_index >= MAX_OBJECTS_COUNT) {
    MCerror(17, "Increase Array Size");
  }
  *p_core_source_info = (void *)objects;

  return 0;
}

int _mcl_read_all_file_text(const char *filepath, char **contents)
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

int _mcl_find_sequence_in_text_ignoring_empty_text(const char *text, const char *first, const char *second,
                                                   const char *third, int *index)
{
  for (int i = 0;; ++i) {

    if (text[i] == '\0') {
      break;
    }

    // First
    int s;
    for (s = 0; first; ++s) {
      if (first[s] == '\0') {
        break;
      }
      if (text[i + s] != first[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    // Second
    int t = s;
    for (s = 0; second; ++s) {
      if (second[s] == '\0') {
        break;
      }
      if (text[i + t + s] != second[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    // Third
    t = t + s;
    for (s = 0; third; ++s) {
      if (third[s] == '\0') {
        break;
      }
      if (text[i + t + s] != third[s]) {
        s = -1;
        break;
      }
    }
    if (s < 0) {
      continue;
    }

    *index = i;
    return 0;
  }

  MCerror(60, "Could not find '%s'>'%s'>'%s'", first, second, third);
}

int _mcl_load_core_temp_struct(void *p_core_source_info, int source_index)
{
  const int STRUCT = 11;
  const int FUNCTION = 12;
  const int ENUM = 13;
  struct core_object_info {
    int type;
    const char *identity;
    const char *filepath;
  };
  struct core_object_info *core_objects = (struct core_object_info *)p_core_source_info;

  // Find and extract the definition text
  char *file_text;
  MCcall(_mcl_read_all_file_text(core_objects[source_index].filepath, &file_text));

  int index;
  MCcall(_mcl_find_sequence_in_text_ignoring_empty_text(file_text, "typedef struct ",
                                                        core_objects[source_index].identity, NULL, &index));

  int end_index;
  MCcall(_mcl_find_sequence_in_text_ignoring_empty_text(file_text, "} ", core_objects[source_index].identity, ";",
                                                        &end_index));
  end_index += 2 + strlen(core_objects[source_index].identity) + 1;

  c_str *definition;
  MCcall(init_c_str(&definition));
  MCcall(append_to_c_strn(definition, file_text + index, end_index - index));

  // Convert it
  for (int i = 0; i < definition->len; ++i) {
    for (int o = 0; o <= source_index; ++o) {
      int n = strlen(core_objects[o].identity);
      if (!strncmp(definition->text + i, core_objects[o].identity, n)) {
        MCcall(insert_into_c_str(definition, "mc_core_v_", i));
        i += 10 + n - 1;
      }
    }
  }

  // printf("newdef:\n%s||\n", definition->text);
  // MCerror(159, "next");

  clint_declare(definition->text);

  return 0;
}

int _mcl_load_core_temp_source(void *p_core_source_info)
{
  // const int STRUCT = 11;
  // const int FUNCTION = 12;
  // const int ENUM = 13;
  // struct core_object_info {
  //   int type;
  //   const char *identity;
  //   const char *filepath;
  // };
  // struct core_object_info *core_objects = (struct core_object_info *)p_core_source_info;

  // int objects_index = 0;
  // while (core_objects[objects_index].type) {
  //   printf("%i:%s:%s\n", core_objects[objects_index].type, core_objects[objects_index].filepath,
  //          core_objects[objects_index].identity);
  //   switch (core_objects[objects_index].type) {
  //   case STRUCT:
  //     MCcall(_mcl_load_core_temp_struct(p_core_source_info, objects_index));
  //     break;
  //   default:
  //     MCerror(60, "Unsupported:%i", core_objects[objects_index].type);
  //     break;
  //   }

  //   ++objects_index;
  // }

  const char *core_objects[] = {
      "node", "global_root_data", "function_info", "struct_info", "enumeration_info", NULL,
  };

  const char *source_files[] = {
      "src/core/core_definitions.h",
      NULL,
  };

  c_str *src;
  MCcall(init_c_str(&src));
  for (int a = 0; source_files[a]; ++a) {

    char *file_text;
    MCcall(_mcl_read_all_file_text(source_files[a], &file_text));
    MCcall(set_c_str(src, file_text));
    free(file_text);

    for (int i = 0; i < src->len; ++i) {
      for (int o = 0; core_objects[o]; ++o) {
        int n = strlen(core_objects[o]);
        if (!strncmp(src->text + i, core_objects[o], n)) {
          MCcall(insert_into_c_str(src, "mc_core_v_", i));
          i += 10 + n - 1;
        }
      }
    }

    printf("def:\n%s||\n", src->text);
    MCcall(clint_declare(src->text));
  }

  return 0;
}

int _mcl_init_core_data(void **p_global_root)
{
  char buf[6400];
  sprintf(buf,
          "{"
          "  mc_core_v_node *global = (mc_core_v_node *)calloc(sizeof(mc_core_v_node), 1);"
          "  global->type = NODE_TYPE_GLOBAL_ROOT;"
          "  global->name = \"global\";"
          "  global->parent = NULL;"
          "  global->children.alloc = 40;"
          "  global->children.count = 0;"
          "  global->children.items = (mc_core_v_node **)calloc(sizeof(mc_core_v_node *), global->children.alloc);"
          ""
          "  mc_core_v_global_root_data *global_root_data = (mc_core_v_global_root_data "
          "*)malloc(sizeof(mc_core_v_global_root_data ));"
          "  global->data = global_root_data;"
          ""
          "  global_root_data->functions.alloc = 100;"
          "  global_root_data->functions.count = 0;"
          "  global_root_data->functions.items = (mc_core_v_function_info **)calloc(sizeof(mc_core_v_function_info *),"
          "                                         global_root_data->functions.alloc);"
          ""
          "  global_root_data->structs.alloc = 30;"
          "  global_root_data->structs.count = 0;"
          "  global_root_data->structs.items = (mc_core_v_struct_info **)calloc(sizeof(mc_core_v_struct_info *),"
          "                                       global_root_data->structs.alloc);"
          ""
          "  global_root_data->enumerations.alloc = 20;"
          "  global_root_data->enumerations.count = 0;"
          "  global_root_data->enumerations.items = (mc_core_v_enumeration_info "
          "**)calloc(sizeof(mc_core_v_enumeration_info *), global_root_data->enumerations.alloc);"
          ""
          "  global_root_data->event_handlers.alloc = 0;"
          "  global_root_data->event_handlers.count = 0;"
          ""
          "  *(void **)(%p) = (void *)global_root_data;"
          "}",
          p_global_root);
  MCcall(clint_process(buf));
  printf("here-8\n");

  return 0;
}

int _mcl_load_core_mc_source(void *command_hub, void *p_core_source_info)
{
  const char *source_files[] = {
      "src/core/core_definitions.h",
      NULL,
  };

  char buf[512];
  for (int i = 0; source_files[i]; ++i) {
    sprintf(buf, "MCcall(mc_core_v_instantiate_all_definitions_from_file(\"%s\"));", source_files[i]);
    MCcall(clint_process(buf));
  }

  return 0;
}

int _mcl_load_app_mc_source(void *command_hub)
{
  const char *source_files[] = {
      "src/index_functions.c",
      NULL,
  };

  char buf[512];
  for (int i = 0; source_files[i]; ++i) {
    sprintf(buf, "MCcall(instantiate_all_definitions_from_file(\"%s\"));", source_files[i]);
    MCcall(clint_process(buf));
  }

  return 0;
}

int mcl_load_app_source(void **command_hub)
{
  void *p_core_source_info = NULL;
  // MCcall(_mcl_obtain_core_source_information(&p_core_source_info));
  MCcall(_mcl_load_core_temp_source(p_core_source_info));

  MCcall(_mcl_init_core_data(command_hub));

  MCcall(_mcl_load_core_mc_source(*command_hub, p_core_source_info));

  MCcall(_mcl_load_app_mc_source(*command_hub));

  return 0;
}