#include "andl.h"
#include <sylvan.h>

#ifndef STATE_SPACE_H
#define STATE_SPACE_H

BDD generate_initial_state(andl_context_t *andl_context);

BDD generate_relation(transition_t *transition);

BDD generate_vars(transition_t *transition);

BDD generate_map(andl_context_t *andl_context);

#endif