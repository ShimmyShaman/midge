/* midge_core.c */

#include <stdlib.h>
#include <string.h>

typedef void **struct_definition;
typedef void **midgeo;
typedef unsigned int uint;

/*
 * @field a (void **) variable to store the created value in.
 */
#define allocate_from_intv(field, val)                                                          \
    *field = (void *)malloc(sizeof(int) * 1);                                                   \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(int **)field = val;

#define allocate_from_uintv(field, val)                                                         \
    *field = (void *)malloc(sizeof(uint) * 1);                                                  \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(uint **)field = val;

#define allocate_from_cstringv(field, cstr)                                                     \
    *field = (void *)malloc(sizeof(char) * (strlen(cstr) + 1));                                 \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    strcpy((char *)(*field), cstr);

#define MCcall(function)                     \
    res = function;                          \
    if (res)                                 \
    {                                        \
        printf("--" #function ":%i\n", res); \
        return res;                          \
    }

int clint_process(const char *str);
int clint_declare(const char *str);
int clint_loadfile(const char *path);
int clint_loadheader(const char *path);

int (*allocate_struct_id)(int, void **);
int (*allocate_midge_field_info)(int, void **);
int (*define_struct)(int, void **);
int (*allocate_from_definition)(int, void **);
int (*declare_function_pointer)(int, void **);
int (*obtain_from_index)(int, void **);

int print_struct_id(int argc, void **argv)
{
    midgeo struct_id = (midgeo)argv;

    printf("[%s (version:%i)]\n", (char *)struct_id[0], *(uint *)struct_id[1]);

    return 0;
}

int print_struct_definition(int argc, void **argv)
{
    struct_definition definition = (struct_definition)argv;

    print_struct_id(1, (void **)definition[0]);
    int field_count = *(int *)definition[1];
    char buf[12];
    for (int i = 0; i < field_count; ++i)
    {
        int deref_count = *(int *)((void **)definition[2 + i])[1];
        for (int j = 0; j < deref_count && j < 11; ++j)
            buf[j] = '*';
        buf[deref_count] = '\0';
        printf("--%s %s%s;\n", (char *)((void **)definition[2 + i])[0], buf, (char *)((void **)definition[2 + i])[2]);
    }

    return 0;
}

int allocate_struct_id_v1(int argc, void **argv)
{
    midgeo out_field = (midgeo)argv[0];
    const char *struct_name = (char *)argv[1];
    uint version = *(uint *)argv[2];

    midgeo field_data = (midgeo)malloc(sizeof(void *) * 2);
    if (!field_data)
    {
        printf("allocate_struct_id(): failed to allocate memory for field_data.\n");
        return -1;
    }

    allocate_from_cstringv(&field_data[0], struct_name);
    allocate_from_uintv(&field_data[1], version);
    *out_field = (void *)field_data;
    return 0;
}

int allocate_midge_field_info_v1(int argc, void **argv)
{
    midgeo out_field = (midgeo)argv[0];
    const char *type = (char *)argv[1];
    int pointer_count = *(int *)argv[2];
    const char *name = (char *)argv[3];

    midgeo field_data = (midgeo)malloc(sizeof(void *) * 3);
    allocate_from_cstringv(&field_data[0], type);
    allocate_from_intv(&field_data[1], pointer_count);
    allocate_from_cstringv(&field_data[2], name);
    // printf("pointer_count:%i, *field_data[1]=%i\n", pointer_count, *(int *)field_data[1]);
    *out_field = (void *)field_data;
    return 0;
}

int define_struct_v1(int argc, void **argv)
{
    struct_definition *stdef = (struct_definition *)argv[0];
    const char *name = (char *)argv[1];
    uint version = *(uint *)argv[2];
    int field_count = *(int *)argv[3];
    midgeo fields = (midgeo)argv[4];

    int res;

    struct_definition field_data = (struct_definition)malloc(sizeof(void *) * (2 + field_count));
    if (!field_data)
    {
        printf("define_struct(): failed to allocate memory for field_data.\n");
        return -1;
    }

    void *vargs[5];
    vargs[0] = (void *)&field_data[0];
    vargs[1] = (void *)name;
    vargs[2] = (void *)&version;
    MCcall(allocate_struct_id(3, vargs));

    allocate_from_uintv(&field_data[1], field_count);
    for (int i = 0; i < field_count; ++i)
        field_data[2 + i] = fields[i];

    *stdef = field_data;
    return 0;
}

int allocate_from_definition_v1(int argc, void **argv)
{
    midgeo *allocation = (midgeo *)argv[0];
    struct_definition definition = (struct_definition)argv[1];

    int res;

    // printf("allocating %i void*\n", *(int *)definition[1]);
    midgeo alloc = (midgeo)calloc(sizeof(void *), (1 + *(int *)definition[1]));
    if (!alloc)
    {
        printf("allocate_from_definition(): failed to allocate memory for alloc.\n");
        return -1;
    }

    // print_struct_definition(1, definition);
    // print_struct_id(1, (void **)definition[0]);
    // printf("d:%s c:%u\n", (char *)((void **)definition[0])[0], *(uint *)((void **)definition[0])[1]);
    void *vargs[5];
    vargs[0] = (void *)&alloc[0];
    vargs[1] = (char *)((void **)definition[0])[0];
    vargs[2] = (uint *)((void **)definition[0])[1];
    MCcall(allocate_struct_id(3, vargs));

    *allocation = alloc;
    return 0;
}

int obtain_from_index_v1(int argc, void **argv)
{
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

    midgeo *out_var = (midgeo *)argv[0];
    midgeo index_object = (midgeo)argv[1];
    char *name = (char *)argv[2];

    *out_var = NULL;
    while (index_object != NULL)
    {
        // item is 1 : index 2 is left : 3 is right
        int r = strcmp(name, (char *)index_object[1]);
        if (r > 0)
        {
            index_object = (midgeo)index_object[2];
        }
        else if (r < 0)
        {
            index_object = (midgeo)index_object[3];
        }
        else
        {
            // Match!
            *out_var = index_object;
            return 0;
        }
    }

    return 0;
}

int obtain_struct_info_from_index_v1(int argc, void **argv)
{
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

    midgeo *out_var = (midgeo *)argv[0];
    midgeo node = (midgeo)argv[1];
    char *name = (char *)argv[2];

    *out_var = NULL;
    while (node != NULL)
    {
        midgeo index_object = (midgeo)node[4];

        void *vargs[3];
        vargs[0] = argv[0];
        vargs[1] = (void *)index_object;
        vargs[2] = argv[2];
        obtain_from_index(3, vargs);

        if (out_var != NULL)
            return 0;

        // Set node to contextual parent
        node = (midgeo)node[2];
        continue;
    }

    return 0;
}

int declare_function_pointer_v1(int argc, void **argv)
{
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

    midgeo *out_var = (midgeo *)argv[0];
    midgeo parent = (midgeo)argv[1];
    char *name = (char *)argv[2];
    int parameter_count = *(int *)argv[3];
    midgeo parameters = (midgeo)argv[4];

    int res;

    // Obtain the global function_info definition
    midgeo global = parent;
    while (parent[2] != NULL)
        global = (midgeo)parent[2];
    struct_definition function_info_definition;

    void *vargs[3];
    vargs[0] = (void *)&function_info_definition;
    vargs[1] = (void *)global;
    const char *function_info_name = "function_info";
    vargs[2] = (void *)function_info_name;
    MCcall(obtain_struct_info_from_index_v1(3, vargs));

    vargs[0] = (void *)out_var;
    vargs[1] = (void *)function_info_definition;
    MCcall(allocate_from_definition(2, vargs));

    // Set Allocated function_info
    allocate_from_cstringv(&(*out_var)[1], "declare_function_pointer");
    (*out_var)[2] = NULL;
    (*out_var)[3] = NULL;
    allocate_from_intv(&(*out_var)[4], parameter_count);
    (*out_var)[5] = *parameters;

    // Declare with clint
    char buf[1024];
    strcpy(buf, "int (*");
    strcat(buf, name);
    strcat(buf, ")(int,void**);");
    clint_declare(buf);
    return 0;
}

int mcqck_temp_declare_function_pointer(midgeo *out_var, midgeo parent, const char *name, int parameter_count, midgeo parameters)
{
    // SAME AS declare_function_pointer_v1 without the clint declaration
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1
    int res;

    // Obtain the global function_info definition
    midgeo global = parent;
    while (parent[2] != NULL)
        global = (midgeo)parent[2];
    struct_definition function_info_definition;

    void *vargs[3];
    vargs[0] = (void *)&function_info_definition;
    vargs[1] = (void *)global;
    const char *function_info_id = "function_info";
    vargs[2] = (void *)function_info_id;
    MCcall(obtain_from_index(3, vargs));
    if (function_info_definition == NULL)
    {
        printf("Could not find 'function_info' definition in provided nodes global root!\n");
        return -1;
    }

    vargs[0] = (void *)out_var;
    vargs[1] = (void *)function_info_definition;
    MCcall(allocate_from_definition(2, vargs));

    // Set Allocated function_info
    allocate_from_cstringv(&(*out_var)[1], "declare_function_pointer");
    (*out_var)[2] = NULL;
    (*out_var)[3] = NULL;
    allocate_from_intv(&(*out_var)[4], parameter_count);
    (*out_var)[5] = *parameters;

    return 0;
}

int mcqck_temp_allocate_struct_id(midgeo out_field, const char *struct_name, uint version)
{
    int res;
    void *vargs[3];
    vargs[0] = (void *)out_field;
    vargs[1] = (void *)struct_name;
    vargs[2] = (void *)&version;
    MCcall(allocate_struct_id(3, vargs));
    return 0;
}

int mcqck_temp_allocate_field(void **fields, const char *type, int deref_count, const char *name)
{
    int res;
    void *vargs[4];
    vargs[0] = (void *)&fields[0];
    vargs[1] = (void *)type;
    vargs[2] = (void *)&deref_count;
    vargs[3] = (void *)name;
    MCcall(allocate_midge_field_info(4, vargs));
    return 0;
}

int mcqck_temp_define_struct(struct_definition *definition, const char *name, uint version, int field_count, midgeo fields)
{
    int res;
    void *vargs[5];
    vargs[0] = (void *)definition;
    vargs[1] = (void *)name;
    vargs[2] = (void *)&version;
    vargs[3] = (void *)&field_count;
    vargs[4] = (void *)fields;
    MCcall(define_struct(5, vargs));
    return 0;
}

int mcqck_temp_allocate_from_definition(midgeo *allocation, struct_definition definition)
{
    int res;
    void *vargs[2];
    vargs[0] = (void *)allocation;
    vargs[1] = (void *)definition;

    MCcall(allocate_from_definition(2, vargs));
    return 0;
}

#define PROCESS_HANDLE_INTERACTION 1
#define PROCESS_HANDLE_BRANCH 2
#define PROCESS_HANDLE_INVOKE 3
#define PROCESS_BRANCH_THROUGH 4
#define PROCESS_BRANCH_SAVE_AND_THROUGH 5
#define INTERACTION_CONTEXT_BLANK 1
#define INTERACTION_CONTEXT_PROCESS 2
int mcqck_temp_create_process_declare_function_pointer(midgeo *process_unit)
{
    midgeo data_collection = (midgeo)calloc(sizeof(void *), 20);
    allocate_from_intv(&data_collection[0], 20);
    allocate_from_intv(&data_collection[1], 0);

    midgeo process_unit_invoke = (midgeo)malloc(sizeof(void *) * 4);
    midgeo process_unit_type = (midgeo)malloc(sizeof(void *) * 4);
    midgeo process_unit_name = (midgeo)malloc(sizeof(void *) * 4);
    midgeo process_unit_function_name = (midgeo)malloc(sizeof(void *) * 4);

    // Function Invoke
    allocate_from_intv(&process_unit_invoke[0], PROCESS_HANDLE_INVOKE);
    process_unit_invoke[1] = (void *)&declare_function_pointer_v1;
    process_unit_invoke[2] = data_collection;
    process_unit_invoke[3] = NULL;

    // process_unit_name
    allocate_from_intv(&process_unit_name[0], PROCESS_HANDLE_INTERACTION);
    allocate_from_cstringv(&process_unit_name[1], "Parameter Name:");
    process_unit_name[2] = data_collection;
    process_unit_name[3] = process_unit_type;

    // process_unit_type
    allocate_from_intv(&process_unit_type[0], PROCESS_HANDLE_BRANCH);
    allocate_from_cstringv(&process_unit_type[1], "Parameter Type:");
    process_unit_type[2] = NULL;
    midgeo array = (midgeo)malloc(sizeof(void *) * 3);
    process_unit_type[3] = array;
    allocate_from_intv(&array[0], 2);

    midgeo branch = (midgeo)malloc(sizeof(void *) * 4);
    allocate_from_cstringv(&branch[0], "end");
    allocate_from_intv(&branch[1], PROCESS_BRANCH_THROUGH);
    branch[2] = NULL;
    branch[3] = process_unit_invoke;
    array[1] = branch;

    branch = (midgeo)malloc(sizeof(void *) * 4);
    allocate_from_cstringv(&branch[0], "");
    allocate_from_intv(&branch[1], PROCESS_BRANCH_SAVE_AND_THROUGH);
    branch[2] = data_collection;
    branch[3] = process_unit_name;
    array[2] = branch;

    // name of the function
    allocate_from_intv(&process_unit_function_name[0], PROCESS_HANDLE_INTERACTION);
    allocate_from_cstringv(&process_unit_function_name[1], "Function Name:");
    process_unit_function_name[2] = data_collection;
    process_unit_function_name[3] = process_unit_type;

    *process_unit = process_unit_function_name;
    return 0;
}

int process_command(int argc, void **argsv);
int mc_main(int argc, const char *const *argv)
{
    int sizeof_void_ptr = sizeof(void *);
    if (sizeof_void_ptr != sizeof(int *) || sizeof_void_ptr != sizeof(char *) || sizeof_void_ptr != sizeof(uint *) || sizeof_void_ptr != sizeof(const char *) ||
        sizeof_void_ptr != sizeof(void **) || sizeof_void_ptr != sizeof(allocate_struct_id) || sizeof_void_ptr != sizeof(&allocate_struct_id) || sizeof_void_ptr != sizeof(int *))
    {
        printf("pointer sizes aren't equal!!!\n");
        return -1;
    }

    int res;

    // Function Pointer Setting
    allocate_struct_id = &allocate_struct_id_v1;
    allocate_midge_field_info = &allocate_midge_field_info_v1;
    define_struct = &define_struct_v1;
    allocate_from_definition = &allocate_from_definition_v1;
    declare_function_pointer = &declare_function_pointer_v1;
    obtain_from_index = &obtain_from_index_v1;

    // DEFINE: field_definition
    struct_definition field_definition_v1;
    midgeo field_definition_v1_fields = (midgeo)malloc(sizeof(void *) * (3));
    MCcall(mcqck_temp_allocate_field(&field_definition_v1_fields[0], "char", 1, "type"));
    MCcall(mcqck_temp_allocate_field(&field_definition_v1_fields[1], "int", 0, "deref_count"));
    MCcall(mcqck_temp_allocate_field(&field_definition_v1_fields[2], "char", 1, "identifier"));
    MCcall(mcqck_temp_define_struct(&field_definition_v1, "field_info", 1U, 3, field_definition_v1_fields));

    // DEFINE: struct_definition
    struct_definition struct_definition_v1;
    midgeo struct_definition_v1_fields = (midgeo)malloc(sizeof(void *) * (3));
    MCcall(mcqck_temp_allocate_field(&struct_definition_v1_fields[0], "struct_id", 1, "id"));
    MCcall(mcqck_temp_allocate_field(&struct_definition_v1_fields[1], "int", 0, "field_count"));
    MCcall(mcqck_temp_allocate_field(&struct_definition_v1_fields[2], "field_info", 1, "fields"));
    MCcall(mcqck_temp_define_struct(&struct_definition_v1, "struct_info", 1U, 3, struct_definition_v1_fields));

    // DEFINE: index_node_definition
    struct_definition index_node_definition_v1;
    midgeo index_node_definition_v1_fields = (midgeo)malloc(sizeof(void *) * (4));
    MCcall(mcqck_temp_allocate_field(&index_node_definition_v1_fields[0], "char", 1, "name"));
    MCcall(mcqck_temp_allocate_field(&index_node_definition_v1_fields[1], "void", 1, "item"));
    MCcall(mcqck_temp_allocate_field(&index_node_definition_v1_fields[2], "void", 1, "left"));
    MCcall(mcqck_temp_allocate_field(&index_node_definition_v1_fields[3], "void", 1, "right"));
    MCcall(mcqck_temp_define_struct(&index_node_definition_v1, "field_info", 1U, 4, index_node_definition_v1_fields));

    // DEFINE: function_info
    struct_definition function_definition_v1;
    midgeo function_definition_v1_fields = (midgeo)malloc(sizeof(void *) * (4));
    MCcall(mcqck_temp_allocate_field(&function_definition_v1_fields[0], "char", 1, "name"));
    MCcall(mcqck_temp_allocate_field(&function_definition_v1_fields[1], "char", 1, "return_type"));
    MCcall(mcqck_temp_allocate_field(&function_definition_v1_fields[2], "int", 0, "parameter_count"));
    MCcall(mcqck_temp_allocate_field(&function_definition_v1_fields[3], "parameter_info", 1, "parameters"));
    MCcall(mcqck_temp_define_struct(&function_definition_v1, "function_info", 1U, 4, function_definition_v1_fields));
    // MCcall(print_struct_definition(1, function_definition_v1));
    // printf("--%s (%i*)%s;\n", (char *)((void **)function_definition_v1[3])[0], *(int *)((void **)function_definition_v1[3])[1],
    //        (char *)((void **)function_definition_v1[3])[2]);
    free(function_definition_v1_fields);
    // MCcall(print_struct_definition(1, function_definition_v1));

    // DEFINE: node
    struct_definition node_definition_v1;
    midgeo node_definition_v1_fields = (midgeo)malloc(sizeof(void *) * (6));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[0], "char", 1, "name"));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[1], "node", 1, "parent"));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[2], "function_info", 1, "function_index"));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[3], "structure_info", 1, "structure_index"));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[4], "int", 0, "children_count"));
    MCcall(mcqck_temp_allocate_field(&node_definition_v1_fields[5], "node", 1, "children"));
    MCcall(mcqck_temp_define_struct(&node_definition_v1, "node", 1U, 6, node_definition_v1_fields));
    // MCcall(print_struct_definition(1, node_definition_v1));
    free(node_definition_v1_fields);

    // TODO -- add 2 previous defined structures to global structure index
    // midgeo global_structure_index;
    // MCcall(mcqck_temp_allocate_from_definition(&global_structure_index, index_node_definition_v1));
    // global_structure_index[1] = function_definition_v1;
    // global_structure_index[2] = NULL;
    // global_structure_index[3] = NULL;

    // Instantiate and declare the global::declare_function_pointer method
    // midgeo dfpi;
    // midgeo fields = (midgeo)malloc(sizeof(void *) * 5);
    // MCcall(mcqck_temp_allocate_field(&fields[0], "function_info", 2, "out_var"));
    // MCcall(mcqck_temp_allocate_field(&fields[1], "node", 1, "parent"));
    // MCcall(mcqck_temp_allocate_field(&fields[2], "char", 1, "name"));
    // MCcall(mcqck_temp_allocate_field(&fields[3], "int", 0, "parameter_count"));
    // MCcall(mcqck_temp_allocate_field(&fields[4], "parameter_info", 1, "parameters"));
    // MCcall(mcqck_temp_declare_function_pointer(&dfpi, global, "declare_function_pointer", 5, fields));
    // clint_declare("int (*global_declare_function_pointer)(int argc, void **argv);");

    // midgeo global_function_index;
    // MCcall(mcqck_temp_allocate_from_definition(&global_function_index, index_node_definition_v1));
    // global_structure_index[1] = function_definition_v1;
    // global_structure_index[2] = NULL;
    // global_structure_index[3] = NULL;

    // Instantiate: node global;
    midgeo global;
    MCcall(mcqck_temp_allocate_from_definition(&global, node_definition_v1));
    allocate_from_cstringv(&global[1], "global");
    global[2] = NULL;
    global[3] = NULL; // global_function_index
    global[4] = NULL; // global_structure_index;
    allocate_from_intv(&global[5], 0);
    global[6] = NULL;

    // TODO -- Instantiate version 2 of declare_function_pointer (with struct usage)

    // Execute commands
    midgeo function_index = NULL;
    midgeo process_matrix = (midgeo)malloc(sizeof(void *) * 20);
    allocate_from_intv(&process_matrix[0], 20);
    allocate_from_intv(&process_matrix[1], 0);
    midgeo interaction_context = (midgeo)malloc(sizeof_void_ptr * 2);
    allocate_from_intv(&interaction_context[0], INTERACTION_CONTEXT_BLANK);

    midgeo process_dfp = (midgeo)malloc(sizeof_void_ptr * 3);
    allocate_from_cstringv(&process_dfp[0], "invoke declare_function_pointer");
    MCcall(mcqck_temp_create_process_declare_function_pointer((midgeo *)&process_dfp[1]));
    allocate_from_intv(&process_matrix[1], 1);
    process_matrix[2] = process_dfp;

    // TODO...

    const char *commands =
        "invoke declare_function_pointer\n"
        // What is the name of the function?
        "construct_and_attach_child_node\n"
        // Parameter 0 type:
        "char *\n"
        // Parameter 0 identifier:
        "node_name"
        // Parameter 1 type:
        "end\n"
        "invoke initialize_function\n"
        // What is the name of the function you wish to initialize?
        "construct_and_attach_child_node\n"
        // construct_and_attach_child_node(char *node_name)
        // write code:
        "TODO\n"
        "end\n"

        "create function\n"
        // Uncertain response: Type another command or type demobegin to demostrate it
        "demobegin\n"
        "demoend\n"
        "create node\n"
        "construct_and_attach_child_node\n";

    int n = strlen(commands);
    int s = 0;
    char cstr[2048];
    void *vargs[12]; // TODO -- count
    for (int i = 0; i < n; ++i)
    {
        if (commands[i] != '\n')
            continue;
        strncpy(cstr, commands + s, i - s);
        cstr[i - s] = '\0';
        s = i + 1;

        char *reply;
        int a = 0;
        vargs[a++] = function_index;
        vargs[a++] = process_matrix;
        vargs[a++] = interaction_context;
        vargs[a++] = global;
        vargs[a++] = (void *)cstr;
        vargs[a++] = (void *)&reply;

        printf("%s\n", cstr);
        MCcall(process_command(a, vargs));

        if (reply != NULL)
        {
            printf("%s", reply);
        }
    }

    return 0;
}

int process_command(int argc, void **argsv)
{
    midgeo process_matrix = (midgeo)argsv[1];
    midgeo interaction_context = (midgeo)argsv[2];
    // History:
    // [0] -- linked list cstr : most recent set
    char *command = (char *)argsv[4];
    char **reply = (char **)argsv[5];

    if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BLANK)
    {
        // No real context
        // User Submits Commmands without prompt

        int n = *(int *)process_matrix[1];
        for (int i = 0; i < n; ++i)
        {
            midgeo process = (midgeo)process_matrix[2 + i];
            if (!strcmp((char *)process[0], command))
            {
                int *ic = (int *)interaction_context[0];
                *ic = INTERACTION_CONTEXT_PROCESS;
                interaction_context[1] = process;

                midgeo process_unit = (midgeo)process[1];
                if (*(int *)process_unit[0] != PROCESS_HANDLE_INTERACTION)
                    return -2;

                *reply = (char *)process_unit[1];

                return 0;
            }
        }
        // else if (!strcmp("demobegin", command))
        // {
        //     *reply = "[Demo:\n";
        //     strcpy(*reply, "[Demo: \"");

        //     history[0] strcat(*reply, (char *)history) return 0;
        // }
        strcpy(*reply, "TODO\n");
        return 0;
    }
    else if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_PROCESS)
    {
        // In process...
    }

    return 0;
}

// Define the node structure
// struct_definition node_v1;
// midgeo node_v1_fields = (midgeo)malloc(sizeof(void *) * (4));

// vargs[0] = &node_v1_fields[0];
// const char *_mtl_0 = "char";
// vargs[1] = (void *)_mtl_0;
// int _mtl_1 = 1;
// vargs[2] = (void *)&_mtl_1;
// const char *_mtl_2 = "name";
// vargs[3] = (void *)_mtl_2;
// MCcall(allocate_midge_field_info(4, vargs));

// vargs[0] = &node_v1_fields[1];
// const char *_mtl_6 = "node";
// vargs[1] = (void *)_mtl_6;
// int _mtl_7 = 1;
// vargs[2] = (void *)&_mtl_7;
// const char *_mtl_8 = "parent";
// vargs[3] = (void *)_mtl_8;
// MCcall(allocate_midge_field_info(4, vargs));

// vargs[0] = &node_v1_fields[2];
// const char *_mtl_3 = "int";
// vargs[1] = (void *)_mtl_3;
// int _mtl_4 = 0;
// vargs[2] = (void *)&_mtl_4;
// const char *_mtl_5 = "child_count";
// vargs[3] = (void *)_mtl_5;
// MCcall(allocate_midge_field_info(4, vargs));

// vargs[0] = &node_v1_fields[3];
// const char *_mtl_9 = "node";
// vargs[1] = (void *)_mtl_9;
// int _mtl_10 = 1;
// vargs[2] = (void *)&_mtl_10;
// const char *_mtl_11 = "children";
// vargs[3] = (void *)_mtl_11;
// MCcall(allocate_midge_field_info(4, vargs));

// vargs[0] = &node_v1;
// const char *_mtl_12 = "node";
// vargs[1] = (void *)_mtl_12;
// uint _mtl_13 = 1U;
// vargs[2] = (void *)&_mtl_13;
// vargs[3] = node_v1_fields;
// int _mtl_14 = 4;
// vargs[4] = (void *)&_mtl_14;
// MCcall(define_struct(5, vargs));
// free(node_v1_fields);

// // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v1[2])[0], *(uint *)((void **)node_v1[2])[1], (char *)((void **)node_v1[2])[2]);
// // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v1[3])[0], *(uint *)((void **)node_v1[3])[1], (char *)((void **)node_v1[3])[2]);
// // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v1[4])[0], *(uint *)((void **)node_v1[4])[1], (char *)((void **)node_v1[4])[2]);
// // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v1[5])[0], *(uint *)((void **)node_v1[5])[1], (char *)((void **)node_v1[5])[2]);

// // node global
// midgeo global;
// vargs[0] = &global;
// vargs[1] = node_v1;
// MCcall(allocate_from_definition(2, vargs));
// allocate_from_cstringv(&global[1], "global");
// allocate_from_intv(&global[2], 0);
// global[3] = &global;
// midgeo children = (midgeo)malloc(sizeof(void *) * (0));
// global[4] = NULL;

// // printf("global:%p\n", global);
// // printf("struct_id:%s - %u\n", (char *)((void **)global[0])[0], *(uint *)((void **)global[0])[1]);
// // printf("global:%s\n", (char *)global[1]);
// // printf("global:%i\n", *(int *)global[2]);
// // printf("global:%p\n", global[3]);
// // printf("global:%p\n", global[4]);