/* mo_serialization.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include "core/mc_source.h"
#include "env/environment_definitions.h"
#include "mc_str.h"
#include "midge_error_handling.h"

#include "modules/modus_operandi/mo_serialization.h"

#define MC_MO_EOL_BUF_SIZE 256

int _mc_mo_parse_serialized_process(mc_mo_process_stack *process_stack, const char **serialization,
                                    mo_operational_process **p_process);

int mc_mo_parse_past_empty_space(const char **str)
{
  const char *c = *str;
  while (*c == ' ' || *c == '\t' || *c == '\n')
    ++c;

  *str = c;
  return 0;
}

int mc_mo_parse_past(const char **str, const char *expect)
{
  const char *c = *str;

  while (1) {
    if (*expect == '\0') {
      break;
    }

    if (*c != *expect) {
      MCerror(3735, "expected:'%c' got:'%c'", *expect, *c);
    }
    ++expect;
    ++c;
  }

  *str = c;
  return 0;
}

/*
 * Parses the rest of the line into dest, and advances str past the end-line character ('\n')
 * Error if permit_end_of_file is false and end-of-file occurs or size limit equals or exceeds MC_MO_EOL_BUF_SIZE.
 */
int mc_mo_parse_line(char *dest, const char **str, bool permit_end_of_file)
{
  const char *c = *str;

  while (*c != '\n') {
    if (*c == '\0') {
      if (permit_end_of_file)
        break;

      MCerror(6539, "Unexpected end-of-file");
    }

    ++c;
  }

  if (c - *str >= MC_MO_EOL_BUF_SIZE) {
    MCerror(6540, "parsed line exceeds defined bounds");
  }

  strncpy(dest, *str, c - *str);
  dest[c - *str] = '\0';

  *str = c + 1;
  return 0;
}

/*
 * Parses the rest of the line using the format _property_name=a value_ past the end-line character ('\n')
 * @dest_prop_name should be a buffer of size at least 64.
 * Error if end-of-file occurs or size limit equals or dest_prop_name exceeds 64 chars, or dest_value exceeds
 * MC_MO_EOL_BUF_SIZE.
 */
int _mc_mo_parse_property_line(char *dest_prop_name, char *dest_value, const char **str)
{
  mc_mo_parse_past_empty_space(str);

  const char *c = *str;

  if (!isalpha(*c) && *c != '_') {
    MCerror(6878, "property expected to begin with alphabet letter is:'%.14s'", c);
  }

  // Property name
  while (*c != '=') {
    if (*c == '\0' || *c == '\n') {
      MCerror(6893, "Unexpected end-of-file or new-line");
    }

    ++c;
  }

  if (c - *str >= 64) {
    MCerror(6540, "property name exceeds defined bounds size");
  }
  strncpy(dest_prop_name, *str, c - *str);
  dest_prop_name[c - *str] = '\0';

  // Property name
  *str = ++c;
  while (*c != '\n') {
    if (*c == '\0') {
      MCerror(6898, "Unexpected end-of-file or new-line");
    }

    ++c;
  }

  if (c - *str >= MC_MO_EOL_BUF_SIZE) {
    MCerror(6544, "property name exceeds defined bounds size");
  }
  strncpy(dest_value, *str, c - *str);
  dest_value[c - *str] = '\0';

  *str = c + 1;
  return 0;
}

int mc_mo_parse_serialized_process_parameter(mc_mo_process_stack *pstack, mo_operational_process *process,
                                             const char **serialization)
{
  const char *s = *serialization;

  char buf[MC_MO_EOL_BUF_SIZE];
  char prop_name[64];

  // Init
  mo_operational_process_parameter *param;
  MCcall(mc_grow_array((void **)&process->parameters, &process->nb_parameters, sizeof(mo_operational_process_parameter),
                       (void **)&param));

  // Preamble
  MCcall(mc_mo_parse_past(&s, "param"));
  MCcall(mc_mo_parse_past_empty_space(&s));
  MCcall(mc_mo_parse_past(&s, "{"));
  MCcall(mc_mo_parse_past_empty_space(&s));

  // Name
  MCcall(_mc_mo_parse_property_line(prop_name, buf, &s));
  if (strcmp(prop_name, "name")) {
    MCerror(3883, "process parameters are to begin with name={$param-name$}");
  }
  param->name = strdup(buf);
  MCcall(mc_mo_parse_past_empty_space(&s));

  // Obtain Sub-process
  MCcall(mc_mo_parse_past(&s, "obtain="));
  MCcall(_mc_mo_parse_serialized_process(pstack, &s, &param->obtain_value_subprocess));

  // Postamble
  MCcall(mc_mo_parse_past_empty_space(&s));
  MCcall(mc_mo_parse_past(&s, "}"));

  *serialization = s;
  return 0;
}

int mc_mo_parse_delegate_code(mc_mo_process_stack *process_stack, void **fptr, const char **str)
{
  const char *c = *str;

  // Read the delegate code
  int bcount = 1;
  while (bcount) {
    switch (*c) {
    case '"': {
      while (1) {
        ++c;
        if (*c == '"' && *(c - 1) != '\\') {
          ++c;
          break;
        }
      }
    } break;
    case '\'': {
      ++c;
      if (*c == '\\')
        ++c;
      ++c;
      if (*c != '\'') {
        MCerror(7672, "unexpected");
      }
      ++c;
    } break;
    case '{': {
      ++c;
      ++bcount;
    } break;
    case '}': {
      --bcount;
      if (bcount)
        ++c;
    } break;
    case '\0': {
      MCerror(6498, "Unexpected EOF");
    }
    default:
      ++c;
      break;
    }
  }

  char *fcode = strndup(*str, c - *str);
  // printf("fcode:\n%s||", fcode);

  char del_func_name[64];
  sprintf(del_func_name, "mo_delegate_function_%i", 4312);

  // Form the delegate function
  mc_str *fc;
  MCcall(init_mc_str(&fc));

  MCcall(append_to_mc_str(fc, "#include <stdio.h>\n"));
  MCcall(append_to_mc_str(fc, "#include <string.h>\n"));
  MCcall(append_char_to_mc_str(fc, '\n'));
  MCcall(append_to_mc_str(fc, "#include \"midge_error_handling.h\"\n"));
  MCcall(append_char_to_mc_str(fc, '\n'));
  MCcall(append_to_mc_str(fc, "#include \"modules/modus_operandi/mo_types.h\"\n"));
  MCcall(append_char_to_mc_str(fc, '\n'));
  MCcall(append_to_mc_strf(fc, "int %s() {\n", del_func_name));
  MCcall(append_to_mc_strf(fc, "  mc_mo_process_stack *process_stack = (mc_mo_process_stack *)%p;\n", process_stack));
  MCcall(append_to_mc_str(fc, "  hash_table_t *context = &process_stack->context_maps[process_stack->index];\n"));

  MCcall(append_char_to_mc_str(fc, '\n'));
  MCcall(append_to_mc_str(fc, fcode));
  free(fcode);

  MCcall(append_to_mc_str(fc, "\n  return 0;\n}"));

  // TODO -- do this in mc_source maybe?
  mc_app_itp_data *app_itp_data;
  MCcall(mc_obtain_app_itp_data(&app_itp_data));

  // -- Send the code to the interpreter
  {
    int mc_res = tcci_add_string(app_itp_data->interpreter, "modus_operandi_delegate_function", fc->text);
    if (mc_res) {
      printf("--"
             "tcci_add_string(app_itp_data->interpreter, filepath, code)"
             "line:%i:ERR:%i\n",
             __LINE__ - 5, mc_res);
      save_text_to_file("src/temp/todelete.h", fc->text);
      return mc_res;
    }

    *fptr = tcci_get_symbol(app_itp_data->interpreter, del_func_name);
    if (!*fptr) {
      MCerror(5621, "why?");
    }
  }

  // Cleanup
  release_mc_str(fc, true);

  *str = c;
  return 0;
}

int mc_mo_parse_context_arg(const char *str, mo_op_step_context_arg *dest)
{
  MCcall(mc_mo_parse_past(&str, "{"));

  const char *c = str;
  while (*c != ':') {
    ++c;
    if (*c == '\0') {
      MCerror(7427, "Incorrect format");
    }
  }

  if (!strncmp(str, "CSTR", c - str)) {
    dest->type = MO_OPPC_CSTR;

    str = c + 1;
    while (*c != '}') {
      ++c;
      if (*c == '\0') {
        MCerror(7489, "Incorrect format");
      }
    }

    dest->data = strndup(str, c - str);
    if (strlen(dest->data) == 0 || !strcmp(dest->data, "NULL") || !strcmp(dest->data, "null")) {
      free(dest->data);
      dest->data = NULL;
    }
  }
  else {
    MCerror(7434, "Unrecognized context arg type:'%s'", str);
  }

  MCcall(mc_mo_parse_past(&c, "}"));

  return 0;
}

int mc_mo_parse_serialized_process_step(mc_mo_process_stack *pstack, mo_operational_process *process,
                                        const char **serialization)
{
  const char *s = *serialization, *c;

  char prop_name[64], prop_value[MC_MO_EOL_BUF_SIZE];

  // Preamble
  MCcall(mc_mo_parse_past(&s, ">"));
  MCcall(mc_mo_parse_past_empty_space(&s));

  // Initialize
  mo_operational_step *step = (mo_operational_step *)malloc(sizeof(mo_operational_step));
  step->next = NULL;

  // Type
  if (!strncmp(s, "FILE_DIALOG", 11)) {
    s += 11;
    step->action = MO_OPPA_FILE_DIALOG;

    MCcall(mc_mo_parse_past_empty_space(&s));
    MCcall(mc_mo_parse_past(&s, "{"));
    MCcall(mc_mo_parse_past(&s, "\n"));
    MCcall(mc_mo_parse_past_empty_space(&s));

    while (*s != '}') {
      MCcall(_mc_mo_parse_property_line(prop_name, prop_value, &s));

      if (!strcmp(prop_name, "message")) {
        step->file_dialog.message = strdup(prop_value);
      }
      else if (!strcmp(prop_name, "initial_filename")) {
        MCcall(mc_mo_parse_context_arg(prop_value, &step->file_dialog.initial_filename));
      }
      else if (!strcmp(prop_name, "initial_folder")) {
        MCcall(mc_mo_parse_context_arg(prop_value, &step->file_dialog.initial_folder));
      }
      else if (!strcmp(prop_name, "target_context_property")) {
        step->file_dialog.target_context_property = strdup(prop_value);
      }
      else {
        MCerror(4563, "unhandled property name:'%s'", prop_name);
      }

      MCcall(mc_mo_parse_past_empty_space(&s));
    }
    MCcall(mc_mo_parse_past(&s, "}"));
  }
  else if (!strncmp(s, "FUNCTION", 8)) {
    s += 8;
    step->action = MO_OPPA_DELEGATE_FUNCTION;

    MCcall(mc_mo_parse_past_empty_space(&s));
    MCcall(mc_mo_parse_past(&s, "{"));
    MCcall(mc_mo_parse_past(&s, "\n"));

    MCcall(mc_mo_parse_delegate_code(pstack, &step->delegate.fptr, &s));

    MCcall(mc_mo_parse_past_empty_space(&s));
    MCcall(mc_mo_parse_past(&s, "}"));
  }
  else {
    MCerror(4933, "Unhandled type %.18s...", s);
  }

  // c = s;
  // while (*c != ' ' && *c != '{') {
  //   if (*c == '\0' || *c == '\n') {
  //     MCerror(6897, "Unexpected end-of-file or new-line");
  //   }
  //   ++c;
  // }
  // if(c - s >= 63) {
  //   MCerror(2938, "incorrect type name '%.20s...'", s);
  // }

  // Append the step to the process
  mo_operational_step *latest = process->first;
  if (!latest)
    process->first = step;
  else {
    while (latest->next)
      latest = latest->next;
    latest->next = step;
  }

  *serialization = s;
  return 0;
}

int mc_mo_serialize_process(mo_operational_process *process, const char **serialization)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // TODO
  MCcall(append_to_mc_str(str, process->name));
  MCerror(4466, "TODO");

  *serialization = str->text;
  release_mc_str(str, false);
  return 0;
}

int _mc_mo_parse_serialized_process(mc_mo_process_stack *process_stack, const char **serialization,
                                    mo_operational_process **p_process)
{
  const char *s = *serialization;
  char buf[MC_MO_EOL_BUF_SIZE];

  mo_operational_process *p = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  p->stack = process_stack;
  p->nb_parameters = 0U;
  p->first = NULL;

  // Process Name
  MCcall(mc_mo_parse_line(buf, &s, false));
  p->name = strdup(buf);

  mo_operational_step *step = NULL;
  int loop = 1;
  while (loop) {
    MCcall(mc_mo_parse_past_empty_space(&s));

    switch (*s) {
    case 'p': {
      MCcall(mc_mo_parse_serialized_process_parameter(process_stack, p, &s));
    } break;
    case '>': {
      MCcall(mc_mo_parse_serialized_process_step(process_stack, p, &s));
    } break;
    case '}':
    case '\0':
      loop = 0;
      // End
      break;
    default:
      MCerror(8137, "Unexpected char sequence beginning '%.20s'", s);
    }
  }

  *serialization = s;
  *p_process = p;
  return 0;
}

int mc_mo_parse_serialized_process(mc_mo_process_stack *process_stack, const char *serialization,
                                   mo_operational_process **p_process)
{
  const char *s = serialization;
  MCcall(_mc_mo_parse_serialized_process(process_stack, &s, p_process));

  return 0;
}