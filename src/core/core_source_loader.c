/* core_source_loader.c */

#include <stdio.h>

typedef struct _csl_c_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} _csl_c_str;

typedef int booelan;

int init__csl_c_str(_csl_c_str **ptr)
{
  (*ptr) = (_csl_c_str *)malloc(sizeof(_csl_c_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int append_to__csl_c_str(_csl_c_str *cstr, const char *text)
{
  int len = strlen(text);
  if (cstr->len + len + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + len + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atcs-a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
  // printf("atcs-b text:'%s'\n", text);
  // memcpy(cstr->text + cstr->len, text, sizeof(char) * len);
  strcpy(cstr->text + cstr->len, text);
  // printf("atcs-c cstrtext:'%s' len:%u end:'%c'\n", cstr->text + cstr->len - 2, cstr->len, cstr->text[cstr->len]);
  cstr->len += len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int set__csl_c_str(_csl_c_str *cstr, const char *src)
{
  cstr->len = 0;
  cstr->text[0] = '\0';
  append_to__csl_c_str(cstr, src);

  return 0;
}

int release__csl_c_str(_csl_c_str *ptr, booelan free_char_string_also)
{
  if (ptr->alloc > 0 && free_char_string_also && ptr->text) {
    free(ptr->text);
  }

  free(ptr);

  return 0;
}

int insert_into__csl_c_str(_csl_c_str *cstr, const char *text, int index)
{
  if (index > cstr->len) {
    MCerror(667, "TODO");
  }

  int n = strlen(text);
  if (cstr->len + n + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + n + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    if (index) {
      memcpy(newptr, cstr->text, sizeof(char) * index);
    }
    memcpy(newptr + index, text, sizeof(char) * n);
    if (cstr->len - index) {
      memcpy(newptr + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
    }
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    cstr->len += n;
    cstr->text[cstr->len] = '\0';
    // printf("atc-8\n");
    return 0;
  }

  memmove(cstr->text + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
  memcpy(cstr->text + index, text, sizeof(char) * n);
  cstr->len += n;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int remove_from__csl_c_str(_csl_c_str *cstr, int start_index, int len)
{
  if (start_index > cstr->len || len == 0)
    return 0;

  if (start_index + len == cstr->len) {
    cstr->len = start_index;
    return 0;
  }

  int a;
  for (a = 0; start_index + len + a < cstr->len; ++a) {
    cstr->text[start_index + a] = cstr->text[start_index + len + a];
  }
  cstr->len -= len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

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

  (*contents)[fsize] = '\0';

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

const char *_mcl_core_objects[] = {
    "node",
    "global_root_data",
    "source_file_info",
    "function_info",
    "struct_info",
    "enumeration_info",
    "obtain_midge_global_root",
    "init_c_str",
    "release_c_str",
    "set_c_str",
    "set_c_strn",
    "append_to_c_str",
    "append_to_c_strn",
    "append_to_c_strf",
    "insert_into_c_str",
    "print_syntax_node",
    // "find_function_info",
    // "copy_syntax_to_node",

    // And everything here before -------------------------------------------------------------
    "instantiate_all_definitions_from_file",
    NULL,
};

const char *_mcl_source_files[] = {
    "src/midge_common.h",
    "src/core/core_definitions.h",
    "src/core/c_parser_lexer.h",
    "src/core/mc_code_transcription.h",
    "src/core/core_definitions.c",
    "src/core/c_parser_lexer.c",
    "src/core/mc_code_transcription.c",
    // And everything here before -------------------------------------------------------------
    "src/core/mc_source.c",
    NULL,
};

int _mcl_load_core_temp_source(void *p_core_source_info)
{

  _csl_c_str *src;
  MCcall(init__csl_c_str(&src));

  // set__csl_c_str(src, "abcdefghijklmnopqrstuvwxyz");
  // printf("%s\n", src->text);
  // MCcall(remove_from__csl_c_str(src, 3, 13));
  // printf("%s\n", src->text);
  // return 0;

  for (int a = 0; _mcl_source_files[a]; ++a) {

    char *file_text;
    MCcall(_mcl_read_all_file_text(_mcl_source_files[a], &file_text));
    MCcall(set__csl_c_str(src, file_text));
    free(file_text);

    for (int i = 0; i < src->len; ++i) {
      // Skip Includes
      if (src->text[i] == '#') {
        if (!strncmp(src->text + i, "#include \"", 10)) {

          int s = i, e = i;
          while (src->text[e] != '\n') {
            ++e;
          }

          // printf("removing ...'");
          // for (int a = 0; a < e - s; ++a)
          //   printf("%c", src->text[s + a]);
          // printf("'");
          // printf("-'%i' chars\n", e - s);

          // Delete all includes
          MCcall(remove_from__csl_c_str(src, s, e - s));
          --i;
          // if (!strcmp(_mcl_source_files[a], "src/midge_common.h"))
          //   printf("def:\n%s||\n", src->text);
          //   return 0;
          continue;
        }
      }

      // Exceptions
      switch (src->text[i - 1]) {
      case '_':
      case '"':
        continue;
      default: {
        if (isalpha(src->text[i - 1]))
          continue;
        break;
      }
      }

      // Check against each object identity
      for (int o = 0; _mcl_core_objects[o]; ++o) {
        int n = strlen(_mcl_core_objects[o]);
        if (!strncmp(src->text + i, _mcl_core_objects[o], n)) {
          // Exceptions
          switch (src->text[i + n]) {
          case '_':
          case '"':
            continue;
          default: {
            if (isalpha(src->text[i + n]))
              continue;
            break;
          }
          }

          // Insert
          MCcall(insert_into__csl_c_str(src, "mc_core_v_", i));
          i += 10 + n - 1;
        }
      }
    }

    // if (!strcmp(_mcl_source_files[a], "src/midge_common.h"))
    //   printf("def:\n%s||\n", src->text);
    printf("declaring file:'%s'\n", _mcl_source_files[a]);
    MCcall(clint_declare(src->text));

    // MCcall(clint_process("printf(\"%p\", &mc_core_v_init_c_str);//{c_str *str; mc_core_v_init_c_str(&str);}"));
  }

  return 0;
}

int _mcl_init_core_data(void **p_global_root)
{
  char buf[3200];
  sprintf(buf,
          "{"
          "  mc_core_v_node *global = (mc_core_v_node *)calloc(sizeof(mc_core_v_node), 1);"
          "  global->type = NODE_TYPE_GLOBAL_ROOT;"
          "  allocate_and_copy_cstr(global->name, \"global\");"
          "  global->parent = NULL;"
          "  global->children.alloc = 40;"
          "  global->children.count = 0;"
          "  global->children.items = (mc_core_v_node **)calloc(sizeof(mc_core_v_node *), global->children.alloc);"
          ""
          "  mc_core_v_global_root_data *global_root_data = (mc_core_v_global_root_data "
          "*)malloc(sizeof(mc_core_v_global_root_data ));"
          "  global->data = global_root_data;"
          "  global_root_data->global_node = global;"
          ""
          "  global_root_data->source_files.alloc = 100;"
          "  global_root_data->source_files.count = 0;"
          "  global_root_data->source_files.items = (mc_core_v_source_file_info "
          "**)calloc(sizeof(mc_core_v_source_file_info *), global_root_data->source_files.alloc);"
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

  sprintf(buf,
          "int mc_core_v_obtain_midge_global_root(mc_core_v_global_root_data **root_data) {\n"
          "  *root_data = (mc_core_v_global_root_data  *)(%p);\n"
          "  return 0;\n"
          "}",
          *p_global_root);
  MCcall(clint_declare(buf));

  return 0;
}

// extern "C" {
// struct mc_core_v_global_root_data;
// struct mc_core_v_source_file_info;
// int mc_core_v_instantiate_all_definitions_from_file(struct mc_core_v_global_root_data *global_data,
//                                                     const char *file_name,
//                                                     struct mc_core_v_source_file_info *source_file);
// int mc_core_v_obtain_midge_global_root(struct mc_core_v_global_root_data **root_data);
// }
int _mcl_load_core_mc_source(void *command_hub, void *p_core_source_info)
{
  // for (int i = 0; _mcl_source_files[i]; ++i) {
  //   printf("instantiate file:'%s'\n", _mcl_source_files[i]);
  //   struct mc_core_v_global_root_data *global_data;
  //   MCcall(mc_core_v_obtain_midge_global_root(&global_data));
  //   MCcall(mc_core_v_instantiate_all_definitions_from_file(global_data, _mcl_source_files[i], NULL));
  // }

  char buf[512];
  for (int i = 0; _mcl_source_files[i]; ++i) {
    printf("instantiate file:'%s'\n", _mcl_source_files[i]);
    int result;
    sprintf(buf,
            "{\n"
            "  mc_core_v_global_root_data *global_data;\n"
            "  MCcall(mc_core_v_obtain_midge_global_root(&global_data));\n"
            "  int result = mc_core_v_instantiate_all_definitions_from_file(global_data->global_node, (char *)\"%s\","
            "                 NULL);\n"
            "  if (result) {\n"
            "    printf(\"--mc_core_v_instantiate_all_definitions_from_file #in - clint_process\\n\");\n"
            "    *(int *)(%p) = result;\n"
            "  }\n"
            "}",
            _mcl_source_files[i], &result);

    MCcall(clint_process(buf));

    if (result != 0) {
      return result;
    }
  }

  return 0;
}

int _mcl_load_app_mc_source(void *command_hub)
{
  const char *_mcl_source_files[] = {
      // And everything here before -------------------------------------------------------------
      "src/core/midge_app.h",
      NULL,
  };

  char buf[512];
  for (int i = 0; _mcl_source_files[i]; ++i) {
    sprintf(buf,
            "{\n"
            "  node *global_node;\n"
            "  MCcall(obtain_midge_global_node(&global_node));\n"
            // "  char *file_name;\n"
            // "  allocate_and_copy_cstr(file_name, \"%s\");\n"
            "  MCcall(instantiate_all_definitions_from_file(global_node, (char *)\"%s\", NULL));\n"
            // "  free"
            "}",
            _mcl_source_files[i]);
    MCcall(clint_process(buf));
  }

  return 0;
}

int mcl_load_app_source(void **command_hub)
{
  void *p_core_source_info = NULL;
  // MCcall(_mcl_obtain_core_source_information(&p_core_source_info));
  printf("[_mcl_load_core_temp_source]\n");
  MCcall(_mcl_load_core_temp_source(p_core_source_info));

  printf("[_mcl_init_core_data]\n");
  MCcall(_mcl_init_core_data(command_hub));

  printf("[_mcl_load_core_mc_source]\n");
  MCcall(_mcl_load_core_mc_source(*command_hub, p_core_source_info));

  printf("[_mcl_load_app_mc_source]\n");
  MCcall(_mcl_load_app_mc_source(*command_hub));

  return 0;
}