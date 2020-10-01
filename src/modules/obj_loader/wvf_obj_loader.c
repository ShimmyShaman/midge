
typedef enum obj_cmd_type {
  OBJ_CMD_NULL = 0,
  OBJ_CMD_EMPTY,
  OBJ_CMD_V,
  OBJ_CMD_VN,
  OBJ_CMD_VT,
  OBJ_CMD_F,
  OBJ_CMD_G,
  OBJ_CMD_O,
  OBJ_CMD_USEMTL,
  OBJ_CMD_MTLLIB
} obj_cmd_type;

typedef struct tinyobj_vertex_index_t {
  int v_idx, vt_idx, vn_idx;
} tinyobj_vertex_index_t;

#define OBJ_IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define OBJ_CPTR_SKIP_SPACE(x)                              \
  while ((*(x) == ' ') || (*(x) == '\t') || (*(x) == '\r')) \
    ++(x);
#define OBJ_IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

// int parseLine(Command *command, const char *p, size_t p_len, int triangulate)
// {
//   char linebuf[4096];
//   const char *token;
//   MC_ASSERT(p_len < 4095, "TODO");

//   memcpy(linebuf, p, p_len);
//   linebuf[p_len] = '\0';

//   token = linebuf;

//   command->type = COMMAND_EMPTY;

//   /* Skip leading space. */
//   skip_space(&token);

//   MC_ASSERT(token, "TODO");
//   if (token[0] == '\0') { /* empty line */
//     return 0;
//   }

//   if (token[0] == '#') { /* comment line */
//     return 0;
//   }

//   /* vertex */
//   if (token[0] == 'v' && IS_SPACE((token[1]))) {
//     float x, y, z;
//     token += 2;
//     parse_float3(&x, &y, &z, &token);
//     command->vx = x;
//     command->vy = y;
//     command->vz = z;
//     command->type = COMMAND_V;
//     return 1;
//   }

//   /* normal */
//   if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
//     float x, y, z;
//     token += 3;
//     parse_float3(&x, &y, &z, &token);
//     command->nx = x;
//     command->ny = y;
//     command->nz = z;
//     command->type = COMMAND_VN;
//     return 1;
//   }

//   /* texcoord */
//   if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
//     float x, y;
//     token += 3;
//     parse_float2(&x, &y, &token);
//     command->tx = x;
//     command->ty = y;
//     command->type = COMMAND_VT;
//     return 1;
//   }

//   /* face */
//   if (token[0] == 'f' && IS_SPACE((token[1]))) {
//     size_t num_f = 0;

//     tinyobj_vertex_index_t f[TINYOBJ_MAX_FACES_PER_F_LINE];
//     token += 2;
//     skip_space(&token);

//     while (!IS_NEW_LINE(token[0])) {
//       tinyobj_vertex_index_t vi = parse_raw_triple(&token);
//       skip_space_and_cr(&token);

//       f[num_f] = vi;
//       num_f++;
//     }

//     command->type = COMMAND_F;

//     if (triangulate) {
//       size_t k;
//       size_t n = 0;

//       tinyobj_vertex_index_t i0 = f[0];
//       tinyobj_vertex_index_t i1;
//       tinyobj_vertex_index_t i2 = f[1];

//       MC_ASSERT(3 * num_f < TINYOBJ_MAX_FACES_PER_F_LINE, "TODO");

//       for (k = 2; k < num_f; k++) {
//         i1 = i2;
//         i2 = f[k];
//         command->f[3 * n + 0] = i0;
//         command->f[3 * n + 1] = i1;
//         command->f[3 * n + 2] = i2;

//         command->f_num_verts[n] = 3;
//         n++;
//       }
//       command->num_f = 3 * n;
//       command->num_f_num_verts = n;
//     }
//     else {
//       size_t k = 0;
//       MC_ASSERT(num_f < TINYOBJ_MAX_FACES_PER_F_LINE, "TODO");
//       for (k = 0; k < num_f; k++) {
//         command->f[k] = f[k];
//       }

//       command->num_f = num_f;
//       command->f_num_verts[0] = (int)num_f;
//       command->num_f_num_verts = 1;
//     }

//     return 1;
//   }

//   /* use mtl */
//   if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
//     token += 7;

//     skip_space(&token);
//     command->material_name = p + (token - linebuf);
//     size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
//     command->material_name_len = (unsigned int)lun;
//     command->type = COMMAND_USEMTL;

//     return 1;
//   }

//   /* load mtl */
//   if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
//     /* By specification, `mtllib` should be appear only once in .obj */
//     token += 7;

//     skip_space(&token);
//     command->mtllib_name = p + (token - linebuf);
//     size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
//     command->mtllib_name_len = (unsigned int)lun;
//     command->type = COMMAND_MTLLIB;

//     return 1;
//   }

//   /* group name */
//   if (token[0] == 'g' && IS_SPACE((token[1]))) {
//     /* @todo { multiple group name. } */
//     token += 2;

//     command->group_name = p + (token - linebuf);
//     size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
//     command->group_name_len = (unsigned int)lun;
//     command->type = COMMAND_G;

//     return 1;
//   }

//   return 0;
// }

typedef struct obj_cmd {
  obj_cmd_type type;
  int begin;
} obj_cmd;

typedef struct file_index_list {
  obj_cmd *cmds;
  unsigned int cmd_count;

  unsigned int f_cmd_begin;
  unsigned int v_cmd_begin;
  unsigned int vt_cmd_begin;
  unsigned int vn_cmd_begin;

  unsigned int num_faces;

  unsigned int triangle_count;
} file_index_list;

void parse_file(char *code, file_index_list *file_index)
{
  const unsigned int UNASSIGNED = 24892428U;

  file_index->cmd_count = 0;
  file_index->triangle_count = 0;
  file_index->f_cmd_begin = UNASSIGNED;
  file_index->v_cmd_begin = UNASSIGNED;
  file_index->vt_cmd_begin = UNASSIGNED;
  file_index->vn_cmd_begin = UNASSIGNED;
  file_index->num_faces = 0;

  int i;
  for (i = 0; code[i] != '\0'; ++i) {
    if (OBJ_IS_NEW_LINE(code[i])) {
      ++file_index->cmd_count;
    }
  }

  // Init
  obj_cmd *cmds = (obj_cmd *)malloc(sizeof(obj_cmd) * (file_index->cmd_count + 1));
  file_index->cmds = cmds;

  int cmd_index = 0;
  i = 0;
  while (code[i] != '\0') {
    switch (code[i]) {
    case '#': {
      // Comment
      // Ignore
      for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'm': {
      /* load mtl */
      if ((0 == strncmp(code + i, "mtllib", 6)) && OBJ_IS_SPACE(code[i + 6])) {
        /* By specification, `mtllib` should be appear only once in .obj */
        i += 7;

        // Comment
        // Ignore
        for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
          // Empty -- TODO allow empty semi-colon blocks
        }

        //         // skip_space(&i);
        //         // command->mtllib_name = p + (token - linebuf);
        //         // size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
        //         // command->mtllib_name_len = (unsigned int)lun;
        //         // command->type = COMMAND_MTLLIB;

        break;
      }

      MCerror(8239, "ObjLoader:NotSupported:'%c'", code[i]);
    } break;
    case 'u': {
      /* use mtl */
      if ((0 == strncmp(code + i, "usemtl", 6)) && OBJ_IS_SPACE(code[i + 6])) {
        /* By specification, `mtllib` should be appear only once in .obj */
        i += 7;

        // Comment
        // Ignore
        for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
          // Empty -- TODO allow empty semi-colon blocks
        }

        //         // skip_space(&i);
        //         // command->mtllib_name = p + (token - linebuf);
        //         // size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
        //         // command->mtllib_name_len = (unsigned int)lun;
        //         // command->type = COMMAND_MTLLIB;

        break;
      }

      MCerror(8239, "ObjLoader:NotSupported:'%c'", code[i]);
    } break;
    case 'o': {
      //   Object name
      //       //   if (token[0] == 'o' && IS_SPACE((token[1]))) {
      //       //     /* @todo { multiple object name? } */
      //       //     token += 2;

      //       //     command->object_name = p + (token - linebuf);
      //       //     size_t lun = length_until_newline(token, (p_len - (size_t)(token - linebuf)) + 1);
      //       //     command->object_name_len = (unsigned int)lun;
      //       //     command->type = COMMAND_O;

      //       //     return 1;
      //       //   }
      // Ignore
      for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'g': {
      // Group

      // Ignore
      for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'f': {
      // Faces
      if (!OBJ_IS_SPACE(code[i + 1])) {
        MCerror(9286, "Format Error");
      }

      // f
      cmds[cmd_index].type = OBJ_CMD_F;
      cmds[cmd_index].begin = i;

      if (file_index->f_cmd_begin == UNASSIGNED) {
        file_index->f_cmd_begin = cmd_index;
      }
      ++cmd_index;
      ++file_index->num_faces;
      i += 2;

      // Add to triangle count
      int num_fv = 0;
      while (!OBJ_IS_NEW_LINE(code[i])) {
        bool val = false;
        for (; isdigit(code[i]) || code[i] == '/'; ++i) {
          val = true;
        }
        if (val) {
          ++num_fv;
        }

        while (OBJ_IS_SPACE(code[i])) {
          ++i;
        }
      }

      file_index->triangle_count += num_fv - 2;
    } break;
    case 's': {
      // Smooth Shading

      // Ignore
      for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'v': {
      switch (code[i + 1]) {
      case ' ':
      case '\t': {
        // v
        cmds[cmd_index].type = OBJ_CMD_V;
        cmds[cmd_index].begin = i;

        if (file_index->v_cmd_begin == UNASSIGNED) {
          file_index->v_cmd_begin = cmd_index;
        }
        ++cmd_index;

        // ++i;
      } break;
      case 't': {
        // vt
        cmds[cmd_index].type = OBJ_CMD_VT;
        cmds[cmd_index].begin = i;

        if (file_index->vt_cmd_begin == UNASSIGNED) {
          file_index->vt_cmd_begin = cmd_index;
        }
        ++cmd_index;

        // ++i;
      } break;
      case 'n': {
        // vn
        cmds[cmd_index].type = OBJ_CMD_VN;
        cmds[cmd_index].begin = i;

        if (file_index->vn_cmd_begin == UNASSIGNED) {
          file_index->vn_cmd_begin = cmd_index;
        }
        ++cmd_index;

        // ++i;
      } break;
      default:
        MCerror(8246, "ObjLoader:NotSupported:v>'%c'", code[i + 1]);
      }

      for (; !OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case ' ':
    case '\t':
    case '\r':
    case '\n': {
      ++i;
    } break;
    default:
      MCerror(8205, "ObjLoader:NotSupported:'%c'", code[i]);
    }
  }
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
int try_parse_double(const char *s, const char *s_end, double *result)
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
    return -5465; /* fail */
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

  return 0;
fail:
  return 6579;
}

int until_space(const char *token)
{
  const char *p = token;
  while (p[0] != '\0' && p[0] != ' ' && p[0] != '\t' && p[0] != '\r') {
    p++;
  }

  return (int)(p - token);
}

int parse_float(const char **token, float *result)
{
  const char *end;
  double val = 0.0;
  int len_until_space = until_space((*token));
  end = (*token) + len_until_space;
  val = 0.0;
  int res = try_parse_double((*token), end, &val);
  if (res) {
    MCerror(7600, "couldn't parse double");
  }

  *result = (float)(val);
  (*token) = end;
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

/* Parse raw triples: i, i/j/k, i//k, i/j */
tinyobj_vertex_index_t parse_raw_triple(const char **token)
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

void mcr_load_wavefront_obj_model(const char *obj_path, mcr_model **loaded_model)
{
  char *file_text;
  read_file_text(obj_path, &file_text);

  file_index_list fli;
  parse_file(file_text, &fli);

  printf("triangle_count:%u %u %u\n", fli.triangle_count, fli.num_faces, fli.f_cmd_begin);

  // TODO -- optimise
  // 3 vertices for each triangle
  float *vertices = (float *)malloc(sizeof(float) * fli.triangle_count * 3 * (3 + 2)); // vert-tex
  unsigned int *indices = (unsigned int *)malloc(sizeof(unsigned int) * fli.triangle_count * 3);

  int vi = 0, ii = 0;
  for (int f = 0; f < fli.num_faces; ++f) {
    int cmd_index = fli.f_cmd_begin + f;

    obj_cmd cmd = fli.cmds[cmd_index];
    if (cmd.type != OBJ_CMD_F) {
      MCerror(9416, "TODO %i", cmd.type);
    }

    char *code = file_text + cmd.begin + 2;

    size_t k;
    size_t n = 0;

    OBJ_CPTR_SKIP_SPACE(code);
    tinyobj_vertex_index_t i0 = parse_raw_triple(&code);
    OBJ_CPTR_SKIP_SPACE(code);
    tinyobj_vertex_index_t i1;
    tinyobj_vertex_index_t i2 = parse_raw_triple(&code);
    OBJ_CPTR_SKIP_SPACE(code);

    while (true) {
      bool brk = !OBJ_IS_NEW_LINE(*code);
      if (brk)
        break;

      i1 = i2;
      i2 = parse_raw_triple(&code);
      printf("%i %i\n", i2.v_idx, i2.vt_idx);
    }
    // command->num_f = 3 * n;
    // command->num_f_num_verts = n;
  }

  // float *vertex_data = (float *)malloc(sizeof(float) * (cmds->f_count *));
  // unsigned int *index_data =

  free(fli.cmds);
}