/*
typedef struct midge_str
{
    int length;
    char *text;
} midge_str;

typedef struct midge_struct_id
{
    midge_str name;
    unsigned int version;
} midge_struct_id;

typedef struct midge_field_info
{
    char *type;
    int pointer_count;
    char *name;
} midge_field_info;

struct _struct
{
    int field_count;
    midge_field_info *fields;
};

struct node
{
    char *name;
    int *child_count;
    struct node *parent;
    struct node *children;
};*/

typedef void **struct_definition;
typedef void **struct_allocation;

int allocate_from_int(void **out_field, int value)
{
    int *alloc = (int *)malloc(sizeof(int) * 1);
    *alloc = value;
    *out_field = (void *)alloc;
    return 0;
}

int allocate_from_uint(void **out_field, unsigned int value)
{
    unsigned int *alloc = (unsigned int *)malloc(sizeof(unsigned int) * 1);
    *alloc = value;
    *out_field = (void *)alloc;
    return 0;
}

int allocate_from_cstring(void **out_field, const char *cstr)
{
    char *alloc = (char *)malloc(sizeof(char) * strlen(cstr));
    strcpy(alloc, cstr);
    *out_field = (void *)alloc;
    return 0;
}

int allocate_struct_id(void **out_field, const char *struct_name, unsigned int version)
{
    void **field_data = (void **)malloc(sizeof(void *) * 2);
    allocate_from_cstring(&field_data[0], struct_name);
    allocate_from_int(&field_data[1], version);
    *out_field = (void *)field_data;
    return 0;
}

int allocate_midge_field_info(void **out_field, const char *type, int pointer_count, const char *name)
{
    void **field_data = (void **)malloc(sizeof(void *) * 3);
    allocate_from_cstring(&field_data[0], type);
    allocate_from_int(&field_data[1], pointer_count);
    allocate_from_cstring(&field_data[2], name);
    *out_field = (void *)field_data;
    return 0;
}

int define_struct(struct_definition *stdef, char *name, unsigned int version, midge_fields fields, int field_count)
{
    struct_definition field_data = (struct_definition)malloc(sizeof(void *) * (2 + field_count));
    allocate_struct_id(&field_data[0], name, version);
    allocate_from_uint(&field_data[1], field_count);


    *stdef = field_data;
    return 0;
}

#define mto_int(container) *(int *)container
#define mto_uint(container) *(unsigned int *)container

int allocate_from_definition(struct_allocation *allocation, struct_definition *struct_definition)
{
    struct_allocation alloc = (struct_allocation)malloc(sizeof(void *) * mto_int(struct_definition[1]));
    allocate_struct_id(&(*alloc)[0], (char *)struct_definition[0], mto_uint(struct_definition[1]));

    *allocation = alloc;
    return 0;
}

typedef void **midge_fields;

// int free_void_array(void **)

int mc_main(int argc, const char *const *argv)
{
    // int a = 3;
    // int *b = &a;
    // void *c = &b;
    // printf("a:%i b:%p c:%p afc:%i\n", a, b, c, **((int **)c));
    // return 1;
    // char *name;
    // int *child_count;
    // struct node *parent;
    // struct node *children;

    // Define the node structure
    struct_definition node_v0;
    midge_fields node_v0_fields = (midge_fields)malloc(sizeof(void *) * (4));
    allocate_midge_field_info(&node_v0_fields[0], "char", 1, "name");
    allocate_midge_field_info(&node_v0_fields[1], "int", 0, "child_count");
    allocate_midge_field_info(&node_v0_fields[2], "node", 1, "parent");
    allocate_midge_field_info(&node_v0_fields[3], "node", 1, "children");
    define_struct(&node_v0, "node", 0, &node_v0_fields, 4);
    free(node_v0_fields);

    struct_definition node_v1;
    redefine_struct_append(&node_v0, node_v0_fields, 4);
    define_append_field(&node_v0, "char", 1, "name");
    expand_structdefine_struct_remove_field("name");

    void **node_structure = (void **)malloc(sizeof(void *) * (2 + 1 + 4));
    allocate_from_cstring(&node_structure[0], "node");
    allocate_from_uint(&node_structure[1], 0);
    allocate_from_int(&node_structure[2], 4); // field_count

    // define_struct("struct_id")
    // define_struct("node", 0, 4);

    // node global;
    void **global;
    allocate_from_definition(&global, node_structure);

    void **struct_id = (void **)global[0];

    return 0;
}