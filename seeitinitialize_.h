/* initialize_0Ê‚(ŠU.h */

#ifndef INITIALIZE_SEEIT_H
#define INITIALIZE_SEEIT_H

#include "core/core_definitions.h"

typedef struct seeit_data {
  mc_node *app_root;
} seeit_data;

/* seeit-Initiation */
int initialize_seeit(mc_node *app_root);

#endif // INITIALIZE_SEEIT_H