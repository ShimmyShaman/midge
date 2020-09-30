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

void skip_space(const char **token)
{
  while ((*token)[0] == ' ' || (*token)[0] == '\t') {
    (*token)++;
  }
}

void skip_space_and_cr(const char **token)
{
  while ((*token)[0] == ' ' || (*token)[0] == '\t' || (*token)[0] == '\r') {
    (*token)++;
  }
}

int until_space(const char *token)
{
  const char *p = token;
  while (p[0] != '\0' && p[0] != ' ' && p[0] != '\t' && p[0] != '\r') {
    p++;
  }

  return (int)(p - token);
}

size_t length_until_newline(const char *token, size_t n)
{
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  for (len = 0; len < n - 1; len++) {
    if (token[len] == '\n') {
      break;
    }
    if ((token[len] == '\r') && ((len < (n - 2)) && (token[len + 1] != '\n'))) {
      break;
    }
  }

  return len;
}

size_t length_until_line_feed(const char *token, size_t n)
{
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  for (len = 0; len < n; len++) {
    if ((token[len] == '\n') || (token[len] == '\r')) {
      break;
    }
  }

  return len;
}

/* Make index zero-base, and also support relative index. */
int fixIndex(int idx, size_t n)
{
  if (idx > 0)
    return idx - 1;
  if (idx == 0)
    return 0;
  return (int)n + idx; /* negative value = relative */
}

/* http://stackoverflow.com/questions/5710091/how-does-atoi-function-in-c-work
 */
int my_atoi(const char *c)
{
  int value = 0;
  int sign = 1;
  if (*c == '+' || *c == '-') {
    if (*c == '-')
      sign = -1;
    c++;
  }
  while (((*c) >= '0') && ((*c) <= '9')) { /* isdigit(*c) */
    value *= 10;
    value += (int)(*c - '0');
    c++;
  }
  return value * sign;
}

size_t my_strnlen(const char *s, size_t n)
{
  const char *p = (const char *)memchr(s, 0, n);
  return p ? (size_t)(p - s) : n;
}

char *my_strdup(const char *s, size_t max_length)
{
  char *d;
  size_t len;

  if (s == NULL)
    return NULL;

  /* Do not consider CRLF line ending(#19) */
  len = length_until_line_feed(s, max_length);
  /* len = strlen(s); */

  /* trim line ending and append '\0' */
  d = (char *)malloc(len + 1); /* + '\0' */
  memcpy(d, s, (size_t)(len));
  d[len] = '\0';

  return d;
}

char *my_strndup(const char *s, size_t len)
{
  char *d;
  size_t slen;

  if (s == NULL)
    return NULL;
  if (len == 0)
    return NULL;

  slen = my_strnlen(s, len);
  d = (char *)malloc(slen + 1); /* + '\0' */
  if (!d) {
    return NULL;
  }
  memcpy(d, s, slen);
  d[slen] = '\0';

  return d;
}

/* Parse raw triples: i, i/j/k, i//k, i/j */
tinyobj_vertex_index_t parseRawTriple(const char **token)
{
  tinyobj_vertex_index_t vi;
  /* 0x80000000 = -2147483648 = invalid */
  vi.v_idx = (int)(0x80000000);
  vi.vn_idx = (int)(0x80000000);
  vi.vt_idx = (int)(0x80000000);

  vi.v_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' && (*token)[0] != '\t' &&
         (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }
  (*token)++;

  /* i//k */
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = my_atoi((*token));
    while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' && (*token)[0] != '\t' &&
           (*token)[0] != '\r') {
      (*token)++;
    }
    return vi;
  }

  /* i/j/k or i/j */
  vi.vt_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' && (*token)[0] != '\t' &&
         (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }

  /* i/j/k */
  (*token)++; /* skip '/' */
  vi.vn_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' && (*token)[0] != '\t' &&
         (*token)[0] != '\r') {
    (*token)++;
  }
  return vi;
}

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
int tryParseDouble(const char *s, const char *s_end, double *result)
{
  double mantissa = 0.0;
  /* This exponent is base 2 rather than 10.
   * However the exponent we parse is supposed to be one of ten,
   * thus we must take care to convert the exponent/and or the
   * mantissa to a * 2^E, where a is the mantissa and E is the
   * exponent.
   * To get the final double we will use ldexp, it requires the
   * exponent to be in base 2.
   */
  int exponent = 0;

  /* NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
   * TO JUMP OVER DEFINITIONS.
   */
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  /* How many characters were read in a loop. */
  int read = 0;
  /* Tells whether a loop terminated due to reaching s_end. */
  int end_not_reached = 0;

  /*
     BEGIN PARSING.
     */

  if (s >= s_end) {
    return 0; /* fail */
  }

  /* Find out what sign we've got. */
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
  }
  else if (isdigit(*curr)) { /* Pass through. */
  }
  else {
    goto fail;
  }

  /* Read the integer part. */
  end_not_reached = (curr != s_end);
  while (end_not_reached && isdigit(*curr)) {
    mantissa *= 10;
    mantissa += (int)(*curr - 0x30);
    curr++;
    read++;
    end_not_reached = (curr != s_end);
  }

  /* We must make sure we actually got something. */
  if (read == 0)
    goto fail;
  /* We allow numbers of form "#", "###" etc. */
  if (!end_not_reached)
    goto assemble;

  /* Read the decimal part. */
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && isdigit(*curr)) {
      /* pow(10.0, -read) */
      double frac_value = 1.0;
      int f;
      for (f = 0; f < read; f++) {
        frac_value *= 0.1;
      }
      mantissa += (int)(*curr - 0x30) * frac_value;
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  }
  else if (*curr == 'e' || *curr == 'E') {
  }
  else {
    goto assemble;
  }

  if (!end_not_reached)
    goto assemble;

  /* Read the exponent part. */
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    /* Figure out if a sign is present and if it is. */
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    }
    else if (isdigit(*curr)) { /* Pass through. */
    }
    else {
      /* Empty E is not allowed. */
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && isdigit(*curr)) {
      exponent *= 10;
      exponent += (int)(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    if (read == 0)
      goto fail;
  }

assemble :

{
  double a = 1.0; /* = pow(5.0, exponent); */
  double b = 1.0; /* = 2.0^exponent */
  int i;
  for (i = 0; i < exponent; i++) {
    a = a * 5.0;
  }

  for (i = 0; i < exponent; i++) {
    b = b * 2.0;
  }

  if (exp_sign == '-') {
    a = 1.0 / a;
    b = 1.0 / b;
  }

  *result =
      /* (sign == '+' ? 1 : -1) * ldexp(mantissa * pow(5.0, exponent),
         exponent); */
      (sign == '+' ? 1 : -1) * (mantissa * a * b);
}

  return 1;
fail:
  return 0;
}

float parseFloat(const char **token)
{
  const char *end;
  double val = 0.0;
  float f = 0.0f;
  skip_space(token);
  int len_until_space = until_space((*token));
  end = (*token) + len_until_space;
  val = 0.0;
  tryParseDouble((*token), end, &val);
  f = (float)(val);
  (*token) = end;
  return f;
}

void parseFloat2(float *x, float *y, const char **token)
{
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
}

void parseFloat3(float *x, float *y, float *z, const char **token)
{
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
  (*z) = parseFloat(token);
}

int parseLine(Command *command, const char *p, size_t p_len, int triangulate)
{
  char linebuf[4096];
  const char *token;
  MC_ASSERT(p_len < 4095, "TODO");

  memcpy(linebuf, p, p_len);
  linebuf[p_len] = '\0';

  token = linebuf;

  command->type = COMMAND_EMPTY;

  /* Skip leading space. */
  skip_space(&token);

  MC_ASSERT(token, "TODO");
  if (token[0] == '\0') { /* empty line */
    return 0;
  }

  if (token[0] == '#') { /* comment line */
    return 0;
  }

  /* vertex */
  if (token[0] == 'v' && IS_SPACE((token[1]))) {
    float x, y, z;
    token += 2;
    parseFloat3(&x, &y, &z, &token);
    command->vx = x;
    command->vy = y;
    command->vz = z;
    command->type = COMMAND_V;
    return 1;
  }

  /* normal */
  if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
    float x, y, z;
    token += 3;
    parseFloat3(&x, &y, &z, &token);
    command->nx = x;
    command->ny = y;
    command->nz = z;
    command->type = COMMAND_VN;
    return 1;
  }

  /* texcoord */
  if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
    float x, y;
    token += 3;
    parseFloat2(&x, &y, &token);
    command->tx = x;
    command->ty = y;
    command->type = COMMAND_VT;
    return 1;
  }

  /* face */
  if (token[0] == 'f' && IS_SPACE((token[1]))) {
    size_t num_f = 0;

    tinyobj_vertex_index_t f[TINYOBJ_MAX_FACES_PER_F_LINE];
    token += 2;
    skip_space(&token);

    while (!IS_NEW_LINE(token[0])) {
      tinyobj_vertex_index_t vi = parseRawTriple(&token);
      skip_space_and_cr(&token);

      f[num_f] = vi;
      num_f++;
    }

    command->type = COMMAND_F;

    if (triangulate) {
      size_t k;
      size_t n = 0;

      tinyobj_vertex_index_t i0 = f[0];
      tinyobj_vertex_index_t i1;
      tinyobj_vertex_index_t i2 = f[1];

      MC_ASSERT(3 * num_f < TINYOBJ_MAX_FACES_PER_F_LINE, "TODO");

      for (k = 2; k < num_f; k++) {
        i1 = i2;
        i2 = f[k];
        command->f[3 * n + 0] = i0;
        command->f[3 * n + 1] = i1;
        command->f[3 * n + 2] = i2;

        command->f_num_verts[n] = 3;
        n++;
      }
      command->num_f = 3 * n;
      command->num_f_num_verts = n;
    }
    else {
      size_t k = 0;
      MC_ASSERT(num_f < TINYOBJ_MAX_FACES_PER_F_LINE, "TODO");
      for (k = 0; k < num_f; k++) {
        command->f[k] = f[k];
      }

      command->num_f = num_f;
      command->f_num_verts[0] = (int)num_f;
      command->num_f_num_verts = 1;
    }

    return 1;
  }

  /* use mtl */
  if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
    token += 7;

    skip_space(&token);
    command->material_name = p + (token - linebuf);
    size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
    command->material_name_len = (unsigned int)lun;
    command->type = COMMAND_USEMTL;

    return 1;
  }

  /* load mtl */
  if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
    /* By specification, `mtllib` should be appear only once in .obj */
    token += 7;

    skip_space(&token);
    command->mtllib_name = p + (token - linebuf);
    size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
    command->mtllib_name_len = (unsigned int)lun;
    command->type = COMMAND_MTLLIB;

    return 1;
  }

  /* group name */
  if (token[0] == 'g' && IS_SPACE((token[1]))) {
    /* @todo { multiple group name. } */
    token += 2;

    command->group_name = p + (token - linebuf);
    size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
    command->group_name_len = (unsigned int)lun;
    command->type = COMMAND_G;

    return 1;
  }

  /* object name */
  if (token[0] == 'o' && IS_SPACE((token[1]))) {
    /* @todo { multiple object name? } */
    token += 2;

    command->object_name = p + (token - linebuf);
    size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
    command->object_name_len = (unsigned int)lun;
    command->type = COMMAND_O;

    return 1;
  }

  return 0;
}

int is_line_ending(const char *p, size_t i, size_t end_i)
{
  if (p[i] == '\0')
    return 1;
  if (p[i] == '\n')
    return 1; /* this includes \r\n */
  if (p[i] == '\r') {
    if (((i + 1) < end_i) && (p[i + 1] != '\n')) { /* detect only \r case */
      return 1;
    }
  }
  return 0;
}

/* Find '\n' and create line data. */
int get_line_infos(const char *buf, size_t buf_len, tinyobj_line_info_t **line_infos, size_t *num_lines)
{
  size_t i = 0;
  size_t end_idx = buf_len;
  size_t prev_pos = 0;
  size_t line_no = 0;
  size_t last_line_ending = 0;

  /* Count # of lines. */
  for (i = 0; i < end_idx; i++) {
    int is_le = is_line_ending(buf, i, end_idx);
    if (is_le) {
      (*num_lines)++;
      last_line_ending = i;
    }
  }
  /* The last char from the input may not be a line
   * ending character so add an extra line if there
   * are more characters after the last line ending
   * that was found. */
  if (end_idx - last_line_ending > 0) {
    (*num_lines)++;
  }
  //   printf("num_lines=%i\n", *num_lines);

  if (*num_lines == 0)
    return -1;

  *line_infos = (tinyobj_line_info_t *)malloc(sizeof(tinyobj_line_info_t) * (*num_lines));

  /* Fill line infos. */
  for (i = 0; i < end_idx; i++) {
    int is_le = is_line_ending(buf, i, end_idx);
    if (is_le) {
      (*line_infos)[line_no].pos = prev_pos;
      (*line_infos)[line_no].len = i - prev_pos;
      prev_pos = i + 1;
      line_no++;
    }
  }
  if (end_idx - last_line_ending > 0) {
    (*line_infos)[line_no].pos = prev_pos;
    (*line_infos)[line_no].len = end_idx - 1 - last_line_ending;
  }

  return 0;
}

void tinyobj_attrib_init(tinyobj_attrib_t *attrib)
{
  attrib->vertices = NULL;
  attrib->num_vertices = 0;
  attrib->normals = NULL;
  attrib->num_normals = 0;
  attrib->texcoords = NULL;
  attrib->num_texcoords = 0;
  attrib->faces = NULL;
  attrib->num_faces = 0;
  attrib->face_num_verts = NULL;
  attrib->num_face_num_verts = 0;
  attrib->material_ids = NULL;
}

void tinyobj_attrib_free(tinyobj_attrib_t *attrib)
{
  if (attrib->vertices)
    free(attrib->vertices);
  if (attrib->normals)
    free(attrib->normals);
  if (attrib->texcoords)
    free(attrib->texcoords);
  if (attrib->faces)
    free(attrib->faces);
  if (attrib->face_num_verts)
    free(attrib->face_num_verts);
  if (attrib->material_ids)
    free(attrib->material_ids);
}

int tinyobj_parse_obj(const char *file_name, tinyobj_obj **obj)
{
  int TINYOBJ_FLAG_TRIANGULATE = 1 << 4;
  int flags = TINYOBJ_FLAG_TRIANGULATE;

  tinyobj_line_info_t *line_infos = NULL;
  Command *commands = NULL;
  size_t num_lines = 0;

  size_t num_v = 0;
  size_t num_vn = 0;
  size_t num_vt = 0;
  size_t num_f = 0;
  size_t num_faces = 0;

  int mtllib_line_index = -1;

  tinyobj_material_t *materials = NULL;
  size_t num_materials = 0;

  hash_table_t material_table;

  char *buf = NULL;
  printf("tpo-0\n");
  read_file_text(file_name, &buf);
  printf("tpo-1\n");
  size_t len = strlen(buf);
  // TODO -- make a read_file_text_with_len function

  if (len < 1) {
    MCerror(9159, "TINYOBJ_ERROR_INVALID_PARAMETER");
  }

  tinyobj_obj *parsed_obj = (tinyobj_obj *)malloc(sizeof(tinyobj_obj));

  // if (attrib == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;
  // if (shapes == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;
  // if (num_shapes == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;
  // if (buf == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;
  // if (materials_out == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;
  // if (num_materials_out == NULL)
  //   return TINYOBJ_ERROR_INVALID_PARAMETER;

  tinyobj_attrib_init(&parsed_obj->attrib);

  /* 1. create line data */
  int res = get_line_infos(buf, len, &line_infos, &num_lines);
  if (res) {
    MCerror(9181, "TINYOBJ_ERROR_EMPTY");
  }

  commands = (Command *)malloc(sizeof(Command) * num_lines);

  create_hash_table(10, &material_table);

  /* 2. parse each line */
  {
    size_t i = 0;
    for (i = 0; i < num_lines; i++) {
      int ret = parseLine(&commands[i], &buf[line_infos[i].pos], line_infos[i].len, flags & TINYOBJ_FLAG_TRIANGULATE);
      if (ret) {
        if (commands[i].type == COMMAND_V) {
          num_v++;
        }
        else if (commands[i].type == COMMAND_VN) {
          num_vn++;
        }
        else if (commands[i].type == COMMAND_VT) {
          num_vt++;
        }
        else if (commands[i].type == COMMAND_F) {
          num_f += commands[i].num_f;
          num_faces += commands[i].num_f_num_verts;
        }

        if (commands[i].type == COMMAND_MTLLIB) {
          mtllib_line_index = (int)i;
        }
      }
    }
  }
  free(buf);

  /* line_infos are not used anymore. Release memory. */
  if (line_infos) {
    free(line_infos);
  }

  /* Load material(if exits) */
  if (mtllib_line_index >= 0 && commands[mtllib_line_index].mtllib_name &&
      commands[mtllib_line_index].mtllib_name_len > 0) {
    MCerror(8722, "TODO -- Materials");
    // char *filename = my_strndup(commands[mtllib_line_index].mtllib_name,
    // commands[mtllib_line_index].mtllib_name_len);

    // int ret = tinyobj_parse_and_index_mtl_file(&materials, &num_materials, filename, file_reader, &material_table);

    // if (ret != TINYOBJ_SUCCESS) {
    //   /* warning. */
    //   fprintf(stderr, "TINYOBJ: Failed to parse material file '%s': %d\n", filename, ret);
    // }

    // free(filename);
  }

  /* Construct attributes */

  {
    size_t v_count = 0;
    size_t n_count = 0;
    size_t t_count = 0;
    size_t f_count = 0;
    size_t face_count = 0;
    int material_id = -1; /* -1 = default unknown material. */
    size_t i = 0;

    parsed_obj->attrib.vertices = (float *)malloc(sizeof(float) * num_v * 3);
    parsed_obj->attrib.num_vertices = (unsigned int)num_v;
    parsed_obj->attrib.normals = (float *)malloc(sizeof(float) * num_vn * 3);
    parsed_obj->attrib.num_normals = (unsigned int)num_vn;
    parsed_obj->attrib.texcoords = (float *)malloc(sizeof(float) * num_vt * 2);
    parsed_obj->attrib.num_texcoords = (unsigned int)num_vt;
    parsed_obj->attrib.faces = (tinyobj_vertex_index_t *)malloc(sizeof(tinyobj_vertex_index_t) * num_f);
    parsed_obj->attrib.num_faces = (unsigned int)num_f;
    parsed_obj->attrib.face_num_verts = (int *)malloc(sizeof(int) * num_faces);
    parsed_obj->attrib.material_ids = (int *)malloc(sizeof(int) * num_faces);
    parsed_obj->attrib.num_face_num_verts = (unsigned int)num_faces;

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_EMPTY) {
        continue;
      }
      else if (commands[i].type == COMMAND_USEMTL) {
        /* @todo
           if (commands[t][i].material_name &&
           commands[t][i].material_name_len > 0) {
           std::string material_name(commands[t][i].material_name,
           commands[t][i].material_name_len);

           if (material_map.find(material_name) != material_map.end()) {
           material_id = material_map[material_name];
           } else {
        // Assign invalid material ID
        material_id = -1;
        }
        }
        */
        if (commands[i].material_name && commands[i].material_name_len > 0) {
          /* Create a null terminated string */
          char *material_name_null_term = (char *)malloc(commands[i].material_name_len + 1);
          memcpy((void *)material_name_null_term, (const void *)commands[i].material_name,
                 commands[i].material_name_len);
          material_name_null_term[commands[i].material_name_len] = 0;

          int hte = hash_table_exists(material_name_null_term, &material_table);
          if (hte) {
            long ht_get = hash_table_get(material_name_null_term, &material_table);
            material_id = (int)ht_get;
          }
          else
            material_id = -1;

          free(material_name_null_term);
        }
      }
      else if (commands[i].type == COMMAND_V) {
        parsed_obj->attrib.vertices[3 * v_count + 0] = commands[i].vx;
        parsed_obj->attrib.vertices[3 * v_count + 1] = commands[i].vy;
        parsed_obj->attrib.vertices[3 * v_count + 2] = commands[i].vz;
        v_count++;
      }
      else if (commands[i].type == COMMAND_VN) {
        parsed_obj->attrib.normals[3 * n_count + 0] = commands[i].nx;
        parsed_obj->attrib.normals[3 * n_count + 1] = commands[i].ny;
        parsed_obj->attrib.normals[3 * n_count + 2] = commands[i].nz;
        n_count++;
      }
      else if (commands[i].type == COMMAND_VT) {
        parsed_obj->attrib.texcoords[2 * t_count + 0] = commands[i].tx;
        parsed_obj->attrib.texcoords[2 * t_count + 1] = commands[i].ty;
        t_count++;
      }
      else if (commands[i].type == COMMAND_F) {
        size_t k = 0;
        for (k = 0; k < commands[i].num_f; k++) {
          tinyobj_vertex_index_t vi = commands[i].f[k];
          int v_idx = fixIndex(vi.v_idx, v_count);
          int vn_idx = fixIndex(vi.vn_idx, n_count);
          int vt_idx = fixIndex(vi.vt_idx, t_count);
          parsed_obj->attrib.faces[f_count + k].v_idx = v_idx;
          parsed_obj->attrib.faces[f_count + k].vn_idx = vn_idx;
          parsed_obj->attrib.faces[f_count + k].vt_idx = vt_idx;
        }

        for (k = 0; k < commands[i].num_f_num_verts; k++) {
          parsed_obj->attrib.material_ids[face_count + k] = material_id;
          parsed_obj->attrib.face_num_verts[face_count + k] = commands[i].f_num_verts[k];
        }

        f_count += commands[i].num_f;
        face_count += commands[i].num_f_num_verts;
      }
    }
  }

  /* 5. Construct shape information. */
  {
    unsigned int face_count = 0;
    size_t i = 0;
    size_t n = 0;
    size_t shape_idx = 0;

    const char *shape_name = NULL;
    unsigned int shape_name_len = 0;
    const char *prev_shape_name = NULL;
    unsigned int prev_shape_name_len = 0;
    unsigned int prev_shape_face_offset = 0;
    unsigned int prev_face_offset = 0;
    tinyobj_shape_t prev_shape = {NULL, 0, 0};

    /* Find the number of shapes in .obj */
    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        n++;
      }
    }

    /* Allocate array of shapes with maximum possible size(+1 for unnamed
     * group/object).
     * Actual # of shapes found in .obj is determined in the later */
    parsed_obj->shapes = (tinyobj_shape_t *)malloc(sizeof(tinyobj_shape_t) * (n + 1));

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        if (commands[i].type == COMMAND_O) {
          shape_name = commands[i].object_name;
          shape_name_len = commands[i].object_name_len;
        }
        else {
          shape_name = commands[i].group_name;
          shape_name_len = commands[i].group_name_len;
        }

        if (face_count == 0) {
          /* 'o' or 'g' appears before any 'f' */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
          prev_face_offset = face_count;
        }
        else {
          if (shape_idx == 0) {
            /* 'o' or 'g' after some 'v' lines. */
            parsed_obj->shapes[shape_idx].name = my_strndup(prev_shape_name, prev_shape_name_len); /* may be NULL */
            parsed_obj->shapes[shape_idx].face_offset = prev_shape.face_offset;
            parsed_obj->shapes[shape_idx].length = face_count - prev_face_offset;
            shape_idx++;

            prev_face_offset = face_count;
          }
          else {
            if ((face_count - prev_face_offset) > 0) {
              parsed_obj->shapes[shape_idx].name = my_strndup(prev_shape_name, prev_shape_name_len);
              parsed_obj->shapes[shape_idx].face_offset = prev_face_offset;
              parsed_obj->shapes[shape_idx].length = face_count - prev_face_offset;
              shape_idx++;
              prev_face_offset = face_count;
            }
          }

          /* Record shape info for succeeding 'o' or 'g' command. */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
        }
      }
      if (commands[i].type == COMMAND_F) {
        face_count++;
      }
    }

    if ((face_count - prev_face_offset) > 0) {
      size_t length = face_count - prev_shape_face_offset;
      if (length > 0) {
        parsed_obj->shapes[shape_idx].name = my_strndup(prev_shape_name, prev_shape_name_len);
        parsed_obj->shapes[shape_idx].face_offset = prev_face_offset;
        parsed_obj->shapes[shape_idx].length = face_count - prev_face_offset;
        shape_idx++;
      }
    }
    else {
      /* Guess no 'v' line occurrence after 'o' or 'g', so discards current
       * shape information. */
    }

    parsed_obj->num_shapes = shape_idx;
  }

  if (commands) {
    free(commands);
  }

  destroy_hash_table(&material_table);

  parsed_obj->materials = materials;
  parsed_obj->num_materials = num_materials;

  // // Make indices useful to me
  // parsed_obj->num_indices = parsed_obj->num_faces;
  // parsed_obj->indices = (unsigned int *)malloc(sizeof(unsigned int) * parsed_obj->num_indices);
  // for(int i = 0; i < )

  // Set to output
  *obj = parsed_obj;

  return 0;
}