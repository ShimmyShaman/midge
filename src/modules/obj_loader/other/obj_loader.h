/*
   The MIT License (MIT)

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   */

// Derived from code by Syoyo Fujita and other many contributors at https://github.com/syoyo/tinyobjloader-c

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

#define TINYOBJ_MAX_FACES_PER_F_LINE (16)

typedef struct tinyobj_material_t {
  char *name;

  float ambient[3];
  float diffuse[3];
  float specular[3];
  float transmittance[3];
  float emission[3];
  float shininess;
  float ior;      /* index of refraction */
  float dissolve; /* 1 == opaque; 0 == fully transparent */
  /* illumination model (see http://www.fileformat.info/format/material/) */
  int illum;

  int pad0;

  char *ambient_texname;            /* map_Ka */
  char *diffuse_texname;            /* map_Kd */
  char *specular_texname;           /* map_Ks */
  char *specular_highlight_texname; /* map_Ns */
  char *bump_texname;               /* map_bump, bump */
  char *displacement_texname;       /* disp */
  char *alpha_texname;              /* map_d */
} tinyobj_material_t;

typedef struct tinyobj_shape_t {
  char *name; /* group name or object name. */
  unsigned int face_offset;
  unsigned int length;
} tinyobj_shape_t;

typedef struct tinyobj_vertex_index_t {
  int v_idx, vt_idx, vn_idx;
} tinyobj_vertex_index_t;

typedef struct tinyobj_attrib_t {
  unsigned int num_vertices;
  unsigned int num_normals;
  unsigned int num_texcoords;
  unsigned int num_faces;
  unsigned int num_face_num_verts;

  int pad0;

  float *vertices;
  float *normals;
  float *texcoords;
  tinyobj_vertex_index_t *faces;
  int *face_num_verts;
  int *material_ids;

  unsigned int num_indices;
  unsigned int *indices;
} tinyobj_attrib_t;

typedef struct tinyobj_line_info_t {
  size_t pos;
  size_t len;
} tinyobj_line_info_t;

typedef enum CommandType {
  COMMAND_EMPTY,
  COMMAND_V,
  COMMAND_VN,
  COMMAND_VT,
  COMMAND_F,
  COMMAND_G,
  COMMAND_O,
  COMMAND_USEMTL,
  COMMAND_MTLLIB

} CommandType;

typedef struct Command {
  float vx, vy, vz;
  float nx, ny, nz;
  float tx, ty;

  /* @todo { Use dynamic array } */
  tinyobj_vertex_index_t f[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f;

  int f_num_verts[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f_num_verts;

  const char *group_name;
  unsigned int group_name_len;
  int pad0;

  const char *object_name;
  unsigned int object_name_len;
  int pad1;

  const char *material_name;
  unsigned int material_name_len;
  int pad2;

  const char *mtllib_name;
  unsigned int mtllib_name_len;

  CommandType type;
} Command;

// TODO -- removal of struct name crashes parser
typedef struct tinyobj_obj {
  tinyobj_attrib_t attrib;
  tinyobj_shape_t *shapes;
  size_t num_shapes;
  tinyobj_material_t *materials;
  size_t num_materials;
} tinyobj_obj;

void skip_space(const char **token);
void skip_space_and_cr(const char **token);
int until_space(const char *token);
size_t length_until_newline(const char *token, size_t n);
size_t length_until_line_feed(const char *token, size_t n);

/* Make index zero-base, and also support relative index. */
int fixIndex(int idx, size_t n);

/* http://stackoverflow.com/questions/5710091/how-does-atoi-function-in-c-work
 */
int my_atoi(const char *c);

size_t my_strnlen(const char *s, size_t n);
char *my_strdup(const char *s, size_t max_length);
char *my_strndup(const char *s, size_t len);

/* Parse raw triples: i, i/j/k, i//k, i/j */
tinyobj_vertex_index_t parseRawTriple(const char **token);
/*
 * Tries to parse a floating point number located at s.
 *
 * s_end should be a location in the string where reading should absolutely
 * stop. For example at the end of the string, to prevent buffer overflows.
 *
 * Parses the following EBNF grammar:
 *   sign    = "+" | "-" ;
 *   END     = ? anything not in digit ?
 *   digit   = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
 *   integer = [sign] , digit , {digit} ;
 *   decimal = integer , ["." , integer] ;
 *   float   = ( decimal , END ) | ( decimal , ("E" | "e") , integer , END ) ;
 *
 *  Valid strings are for example:
 *   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41   11e2
 *
 * If the parsing is a success, result is set to the parsed value and true
 * is returned.
 *
 * The function is greedy and will parse until any of the following happens:
 *  - a non-conforming character is encountered.
 *  - s_end is reached.
 *
 * The following situations triggers a failure:
 *  - s >= s_end.
 *  - parse failure.
 */
int tryParseDouble(const char *s, const char *s_end, double *result);
float parseFloat(const char **token);
void parseFloat2(float *x, float *y, const char **token);
void parseFloat3(float *x, float *y, float *z, const char **token);
int parseLine(Command *command, const char *p, size_t p_len, int triangulate);
int is_line_ending(const char *p, size_t i, size_t end_i);

/* Find '\n' and create line data. */
int get_line_infos(const char *buf, size_t buf_len, tinyobj_line_info_t **line_infos, size_t *num_lines);
void tinyobj_attrib_init(tinyobj_attrib_t *attrib);
void tinyobj_attrib_free(tinyobj_attrib_t *attrib);
int tinyobj_parse_obj(const char *file_name, tinyobj_obj **obj);