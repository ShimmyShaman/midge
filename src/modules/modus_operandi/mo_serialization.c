/* mo_serialization.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mc_str.h"

#include "modules/modus_operandi/mo_serialization.h"

#define MC_MO_EOL_BUF_SIZE 256

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

  // Name
  MCcall(mc_mo_parse_property_line(prop_name, buf, &s));
  if (strcmp(prop_name, "name")) {
    MCerror(3883, "process parameters are to begin with name={$param-name$}");
  }
  param->name = strdup(buf);
  MCcall(mc_mo_parse_past_empty_space(&s));

  // Obtain Sub-process
  MCcall(mc_mo_parse_past(&s, "obtain="));
  MCcall(_mc_mo_parse_serialized_process(pstack, &s, &param->obtain_value_subprocess));

  *serialization = s;
  return 0;
}

int mc_mo_parse_serialized_process_step(mc_mo_process_stack *pstack, mo_operational_process *process,
                                        const char **serialization)
{
  const char *s = *serialization;

  MCerror(4444, "TODO");

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
  const char *s = serialization;
  char buf[MC_MO_EOL_BUF_SIZE];

  mo_operational_process *p = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  p->stack = process_stack;
  p->nb_parameters = 0U;
  p->first = NULL;

  // Process Name
  MCcall(mc_mo_parse_line(buf, &s, false));
  p->name = strdup(buf);

  MCcall(mc_mo_parse_past_empty_space(&s));

  mo_operational_step *step = NULL;
  while (1) {
    switch (*s) {
    case 'p': {
      MCcall(mc_mo_parse_past(&s, "param"));
      MCcall(mc_mo_parse_past_empty_space(&s));
      MCcall(mc_mo_parse_past(&s, "{"));
      MCcall(mc_mo_parse_past_empty_space(&s));

      MCcall(mc_mo_parse_serialized_process_parameter(process_stack, p, &s));

      MCcall(mc_mo_parse_past_empty_space(&s));
      MCcall(mc_mo_parse_past(&s, "}"));
    } break;
    case '>': {
      MCcall(mc_mo_parse_serialized_process_step(process_stack, p, &s));
    } break;
    default:
      MCerror(8137, "Unexpected char sequence beginning '%.20s'", s);
    }

    MCcall(mc_mo_parse_past_empty_space(&s));
  }

  MCerror(4442, "TODO");

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