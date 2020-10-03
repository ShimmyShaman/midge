
#include "modules/obj_loader/wvf_obj_loader.h"
#include "render/render_common.h"

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

//     _wvf_obj_vertex_index_list f[TINYOBJ_MAX_FACES_PER_F_LINE];
//     token += 2;
//     skip_space(&token);

//     while (!IS_NEW_LINE(token[0])) {
//       _wvf_obj_vertex_index_list vi = wvf_obj_parse_raw_triple(&token);
//       skip_space_and_cr(&token);

//       f[num_f] = vi;
//       num_f++;
//     }

//     command->type = COMMAND_F;

//     if (triangulate) {
//       size_t k;
//       size_t n = 0;

//       _wvf_obj_vertex_index_list i0 = f[0];
//       _wvf_obj_vertex_index_list i1;
//       _wvf_obj_vertex_index_list i2 = f[1];

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

void wvf_obj_parse_obj_info(char *code, _wvf_obj_parsed_obj_info *file_index)
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
    if (_WVF_OBJ_IS_NEW_LINE(code[i])) {
      ++file_index->cmd_count;
    }
  }

  // Init
  _wvf_obj_cmd *cmds = (_wvf_obj_cmd *)malloc(sizeof(_wvf_obj_cmd) * (file_index->cmd_count + 1));
  file_index->cmds = cmds;

  int cmd_index = 0;
  i = 0;
  while (code[i] != '\0') {
    switch (code[i]) {
    case '#': {
      // Comment
      // Ignore
      for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'm': {
      /* load mtl */
      if ((0 == strncmp(code + i, "mtllib", 6)) && _WVF_OBJ_IS_SPACE(code[i + 6])) {
        /* By specification, `mtllib` should be appear only once in .obj */
        i += 7;

        // Comment
        // Ignore
        for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
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
      if ((0 == strncmp(code + i, "usemtl", 6)) && _WVF_OBJ_IS_SPACE(code[i + 6])) {
        /* By specification, `mtllib` should be appear only once in .obj */
        i += 7;

        // Comment
        // Ignore
        for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
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
      for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'g': {
      // Group

      // Ignore
      for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'f': {
      // Faces
      if (!_WVF_OBJ_IS_SPACE(code[i + 1])) {
        MCerror(9286, "Format Error");
      }

      // f
      cmds[cmd_index].type = _wvf_obj_cmd_F;
      cmds[cmd_index].begin = i;

      if (file_index->f_cmd_begin == UNASSIGNED) {
        file_index->f_cmd_begin = cmd_index;
      }
      ++cmd_index;
      ++file_index->num_faces;
      i += 2;

      // Add to triangle count
      int num_fv = 0;
      while (!_WVF_OBJ_IS_NEW_LINE(code[i])) {
        bool val = false;
        for (; isdigit(code[i]) || code[i] == '/'; ++i) {
          val = true;
        }
        if (val) {
          ++num_fv;
        }

        while (_WVF_OBJ_IS_SPACE(code[i])) {
          ++i;
        }
      }

      file_index->triangle_count += num_fv - 2;
    } break;
    case 's': {
      // Smooth Shading

      // Ignore
      for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
        // Empty -- TODO allow empty semi-colon blocks
      }
    } break;
    case 'v': {
      switch (code[i + 1]) {
      case ' ':
      case '\t': {
        // v
        cmds[cmd_index].type = _wvf_obj_cmd_V;
        cmds[cmd_index].begin = i;

        if (file_index->v_cmd_begin == UNASSIGNED) {
          file_index->v_cmd_begin = cmd_index;
        }
        ++cmd_index;

        // ++i;
      } break;
      case 't': {
        // vt
        cmds[cmd_index].type = _wvf_obj_cmd_VT;
        cmds[cmd_index].begin = i;

        if (file_index->vt_cmd_begin == UNASSIGNED) {
          file_index->vt_cmd_begin = cmd_index;
        }
        ++cmd_index;

        // ++i;
      } break;
      case 'n': {
        // vn
        cmds[cmd_index].type = _wvf_obj_cmd_VN;
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

      for (; !_WVF_OBJ_IS_NEW_LINE(code[i]); ++i) {
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
// TODO -- move this to a util module
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

// TODO -- move this to a util module
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
// TODO -- move this to a util module
int my_atoi(const char **c)
{
  int value = 0;
  int sign = 1;
  if (**c == '+' || **c == '-') {
    if (**c == '-')
      sign = -1;
    ++*c;
  }
  while (((**c) >= '0') && ((**c) <= '9')) { /* isdigit(*c) */
    value *= 10;
    value += (int)(**c - '0');
    ++*c;
  }
  return value * sign;
}

/* Parse raw triples: i, i/j/k, i//k, i/j */
_wvf_obj_vertex_index_list wvf_obj_parse_raw_triple(const char **token)
{
  // Subtracting 1 for 0-base-indexing

  _wvf_obj_vertex_index_list vi;
  /* 0x80000000 = -2147483648 = invalid */
  vi.v_idx = (int)(0x80000000);
  vi.vn_idx = (int)(0x80000000);
  vi.vt_idx = (int)(0x80000000);

  vi.v_idx = my_atoi(token);
  vi.v_idx -= 1;
  _WVF_OBJ_CPTR_SKIP_SPACE(*token);
  if ((*token)[0] != '/') {
    return vi;
  }
  // printf("tokenb:'%.16s'\n", *token);
  ++(*token);

  /* i//k */
  if ((*token)[0] == '/') {
    ++(*token);
    vi.vn_idx = my_atoi(token);
    vi.vn_idx -= 1;
    _WVF_OBJ_CPTR_SKIP_SPACE(*token);
    return vi;
  }

  /* i/j/k or i/j */
  vi.vt_idx = my_atoi(token);
  vi.vt_idx -= 1;
  _WVF_OBJ_CPTR_SKIP_SPACE(*token);
  // printf("tokenc:'%.16s'\n", *token);
  if ((*token)[0] != '/') {
    return vi;
  }

  /* i/j/k */
  ++(*token); /* skip '/' */
  vi.vn_idx = my_atoi(token);
  vi.vn_idx -= 1;
  _WVF_OBJ_CPTR_SKIP_SPACE(*token);
  // printf("tokene:'%.16s'\n", *token);
  return vi;
}

void wvf_obj_parse_vertex_info_to_data(char *file_text, _wvf_obj_parsed_obj_info *fli,
                                       _wvf_obj_vertex_index_list *vertex_indices, float *vertex_data, int *vi,
                                       unsigned int *index_data, int *ii)
{
  // Position
  _wvf_obj_cmd cmd = fli->cmds[fli->v_cmd_begin + vertex_indices->v_idx];
  // DEBUG
  if (cmd.type != _wvf_obj_cmd_V) {
    MCerror(9732, "TODO %i", cmd.type);
  }
  // DEBUG

  char *vc = file_text + cmd.begin + 2;
  parse_float(&vc, &vertex_data[(*vi)++]);
  // printf(" %.3f", vertex_data[(*vi) - 1]);
  _WVF_OBJ_CPTR_SKIP_SPACE(vc);
  parse_float(&vc, &vertex_data[(*vi)++]);
  // printf(" %.3f", vertex_data[(*vi) - 1]);
  _WVF_OBJ_CPTR_SKIP_SPACE(vc);
  parse_float(&vc, &vertex_data[(*vi)++]);
  // printf(" %.3f", vertex_data[(*vi) - 1]);
  // _WVF_OBJ_CPTR_SKIP_SPACE(vc);

  // Texture
  cmd = fli->cmds[fli->vt_cmd_begin + vertex_indices->vt_idx];
  // DEBUG
  if (cmd.type != _wvf_obj_cmd_VT) {
    MCerror(9749, "TODO %i", cmd.type);
  }
  // DEBUG

  vc = file_text + cmd.begin + 3;

  // "Flip" texture coords to accommodate vulkans right-hand-coordinate usage & objs left-hand-coordinate output
  float vt;
  parse_float(&vc, &vt);
  vertex_data[(*vi)++] = 1.f - vt;
  // parse_float(&vc, &vertex_data[(*vi)++]);
  // printf(" %.3f", vertex_data[(*vi) - 1]);
  _WVF_OBJ_CPTR_SKIP_SPACE(vc);
  parse_float(&vc, &vt);
  vertex_data[(*vi)++] = 1.f - vt;
  // parse_float(&vc, &vertex_data[(*vi)++]);
  // printf(" %.3f", vertex_data[(*vi) - 1]);
  // _WVF_OBJ_CPTR_SKIP_SPACE(vc);

  // printf(" [%i]\n", *vi);

  // printf("index_data[%u] = %u\n", *ii, *ii);
  index_data[*ii] = *ii;
  ++*ii;
}

void _wvf_obj_load_vert_index_data(const char *obj_path, float **vertices, unsigned int *vertex_count,
                                   unsigned int **indices, unsigned int *index_count)
{
  // TODO -- one day, maybe use https://github.com/thisistherk/fast_obj/blob/master/fast_obj.h
  char *file_text;
  read_file_text(obj_path, &file_text);

  _wvf_obj_parsed_obj_info fli;
  wvf_obj_parse_obj_info(file_text, &fli);

  // printf("triangle_count:%u %u %u\n", fli.triangle_count, fli.num_faces, fli.f_cmd_begin);

  // TODO -- optimise
  // 3 vertices for each triangle
  *vertex_count = fli.triangle_count * 3 * (3 + 2);
  *vertices = (float *)malloc(sizeof(float) * *vertex_count); // vert-tex
  *index_count = fli.triangle_count * 3;
  *indices = (unsigned int *)malloc(sizeof(unsigned int) * *index_count);

  // printf("allocated v:%i i:%i\n", *vertex_count, *index_count);

  int vi = 0, ii = 0;
  for (int f = 0; f < fli.num_faces; ++f) {
    int cmd_index = fli.f_cmd_begin + f;

    _wvf_obj_cmd cmd = fli.cmds[cmd_index];
    // DEBUG
    if (cmd.type != _wvf_obj_cmd_F) {
      MCerror(9416, "TODO %i", cmd.type);
    }
    // DEBUG

    char *code = file_text + cmd.begin + 2;

    size_t k;
    size_t n = 0;

    _WVF_OBJ_CPTR_SKIP_SPACE(code);
    _wvf_obj_vertex_index_list i0 = wvf_obj_parse_raw_triple(&code);
    _WVF_OBJ_CPTR_SKIP_SPACE(code);
    _wvf_obj_vertex_index_list i1;
    _wvf_obj_vertex_index_list i2 = wvf_obj_parse_raw_triple(&code);

    while (true) {
      _WVF_OBJ_CPTR_SKIP_SPACE(code);
      // printf("codea:'%.20s'\n", code);
      bool brk = _WVF_OBJ_IS_NEW_LINE(*code);
      if (brk)
        break;

      i1 = i2;
      i2 = wvf_obj_parse_raw_triple(&code);
      // printf("codeb:'%.20s'\n", code);

      // printf("%i %i\n", i0.v_idx, i0.vt_idx);
      wvf_obj_parse_vertex_info_to_data(file_text, &fli, &i0, *vertices, &vi, *indices, &ii);
      // printf("%i %i\n", i1.v_idx, i1.vt_idx);
      wvf_obj_parse_vertex_info_to_data(file_text, &fli, &i1, *vertices, &vi, *indices, &ii);
      // printf("%i %i\n", i2.v_idx, i2.vt_idx);
      wvf_obj_parse_vertex_info_to_data(file_text, &fli, &i2, *vertices, &vi, *indices, &ii);
    }
    // command->num_f = 3 * n;
    // command->num_f_num_verts = n;
  }

  // printf("vi=%i ii=%i\n", vi, ii);

  // float *vertex_data = (float *)malloc(sizeof(float) * (cmds->f_count *));
  // unsigned int *index_data =

  free(file_text);
  free(fli.cmds);
}

void mcr_load_wavefront_obj_model(const char *obj_path, mcr_model **loaded_model)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Load the obj data
  float *vertices;
  unsigned int *indices, vertex_count, index_count;
  _wvf_obj_load_vert_index_data(obj_path, &vertices, &vertex_count, &indices, &index_count);

  // Construct the model and load its resources
  *loaded_model = (mcr_model *)malloc(sizeof(mcr_model));
  (*loaded_model)->texture = 0;

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_MESH;
  command->p_uid = &(*loaded_model)->vertex_buffer;
  command->load_mesh.p_data = vertices;
  command->load_mesh.data_count = vertex_count;
  command->load_mesh.release_original_data_on_copy = false;

  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_INDEX_BUFFER;
  command->p_uid = &(*loaded_model)->index_buffer;
  command->load_indices.p_data = indices;
  command->load_indices.data_count = index_count;
  command->load_indices.release_original_data_on_copy = false;

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  mcr_load_texture_resource("res/cube/cube_diffuse.png", &(*loaded_model)->texture);
}

void mcr_render_model(image_render_details *image_render_queue, mcr_model *model)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_INDEXED_MESH;
  render_cmd->indexed_mesh.vertex_buffer = model->vertex_buffer;
  render_cmd->indexed_mesh.index_buffer = model->index_buffer;
  render_cmd->indexed_mesh.texture_uid = model->texture;
}