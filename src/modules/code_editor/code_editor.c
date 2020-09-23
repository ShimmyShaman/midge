

typedef struct mcce_code_editor_state {

} mcce_code_editor_state;

typedef struct mcce_code_editor_pool {
  unsigned int max_size;

  unsigned int size;
  mcce_code_editor_state **instances;
} mcce_code_editor_pool;

void mca_init_code_editor_pool()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mcce_code_editor_pool *code_editor_pool = (mcce_code_editor_pool *)malloc(sizeof(mcce_code_editor_pool));
  global_data->ui_state->code_editor_pool = code_editor_pool;

  code_editor_pool->max_instances = 25;
  code_editor_pool->size = 0;
}

void _mcce_obtain_code_editor_instance(mcce_code_editor_pool *code_editor_pool, mcce_code_editor_state **code_editor)
{

  for (int i = 0; i < code_editor_pool->size; ++i) {
    if (!code_editor_pool->instances[i]->node->layout->visible) {
      *code_editor = code_editor_pool->instances[i];
    }
  }

  MCerror
}

void mca_edit_code_definition(source_definition *definition)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mca_code_editor_pool *code_editor_pool = (mca_code_editor_pool *)global_data->ui_state->code_editor_pool;

  code_editor_state *code_editor;
  _mcce_obtain_code_editor_instance(code_editor_pool, &code_editor);

  _mcce_set_definition_to_code_editor(code_editor, definition;
}