#include "midge_error_handling.h"

/* app_modules.c */

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "tinycc/libtccinterp.h"

#include "core/core_definitions.h"
#include "core/mc_source.h"
#include "core/midge_app.h"
#include "mc_str.h"
#include "midge_error_handling.h"

int _mca_load_module(const char *base_path, const char *module_name) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("_mca_load_module", __FILE__, __LINE__, &midge_error_stack_index);

  midge_app_info * app_info;
  mc_obtain_midge_app_info(&app_info);

  char  buf[512];
  sprintf(buf, "%s/%s/init_%s.c", base_path, module_name, module_name);
  if (access(buf, F_OK)==-1) {
    MCerror(1999, "Within each module there must be a file named 'init_{%%module_name%%}.c' : This could not be accessed for "
            "module_name='%s'", module_name);
  }


  MCcall(mcs_interpret_file(app_info->itp_data->interpreter, buf));

  // Initialize the module
  sprintf(buf, "init_%s", module_name);
  int (*initialize_module)(mc_node *) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  if (!initialize_module) {
    MCerror(2000, "within each 'init_{%%module_name%%}.c' file there must be a method with signature 'int "
            "init_{%%module_name%%}(mc_node *)' : This was not found for module_name='%s'", module_name);
  }


  // TODO -- for some reason interpreting from this function loads the functions to address ranges I normally see
  // reserved for stack variables. Find out whats happenning -- that can't be good
  // printf("interpreter=%p\n",app_info->itp_data->interpreter);
  // printf("%s=%p\n", buf, initialize_module);
  MCcall(initialize_module(app_info->global_node));


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int mca_load_modules() {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mca_load_modules", __FILE__, __LINE__, &midge_error_stack_index);

  const char * module_directories[] = {
    "modus_operandi",
    NULL
  };

  for (int  d = 0  ; module_directories[d]  ; ++d  ) {
    MCcall(_mca_load_module("src/modules", module_directories[d]));
  }



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


void _mca_set_project_state(char *base_path, char *module_name) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("_mca_set_project_state", __FILE__, __LINE__, &midge_error_stack_index);



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return;
  }
}


int mca_load_open_projects() {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mca_load_open_projects", __FILE__, __LINE__, &midge_error_stack_index);

  char * open_list_text;
  read_file_text("projects/open_project_list", &open_list_text);

  printf("open_list_text:'%s'\n", open_list_text);

  char  buf[256];
  mc_str * str;
  init_mc_str(&str);

  int  i = 0,  s = 0;
  bool  eof = false;
  while (!eof  ) {
    s = i;

    for (    ; open_list_text[i]!='|'    ; ++i    )     if (open_list_text[i]=='\0') {
      eof = true;
      break;
    }


    if (i>s) {
      strncpy(buf, open_list_text + s, i - s);
      buf[i - s] = '\0';

      MCcall(_mca_load_module("projects", buf));
      _mca_set_project_state("projects", buf);
    }

  }



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}
