#include "state_space.h"

BDD generate_initial_state(andl_context_t *andl_context) {
    LACE_ME;

    BDD init = sylvan_true;
    sylvan_protect(&init);

    for (int i = 0; i < andl_context->num_places; i++) {
        place_t *place = andl_context->places + i;

        // variable identifiers are 2*n for normal variables, and 2*n+1 for prime variables.

        if (place->initial_marking == 0) {
            init = sylvan_and(init, sylvan_nithvar(place->identifier * 2));
        } else {
            // assume initial marking is either 0 or 1, because 1-safe

            init = sylvan_and(init, sylvan_ithvar(place->identifier * 2));
        }
    }

    sylvan_unprotect(&init);
    return init;
}

BDD generate_relation(transition_t *transition) {
    LACE_ME;

    BDD relation = sylvan_true;
    sylvan_protect(&relation);

    for (int i = 0; i < transition->num_arcs; i++) {
        arc_t *arc = transition->arcs + i;

        if (arc->dir == ARC_IN) {
            // precondition
            relation = sylvan_and(relation, sylvan_ithvar(arc->place->identifier * 2));

            //postcondition
            relation = sylvan_and(relation, sylvan_nithvar(arc->place->identifier * 2 + 1));
        } else {
            // postcondition
            relation = sylvan_and(relation, sylvan_ithvar(arc->place->identifier * 2 + 1));
        }
    }

    sylvan_unprotect(&relation);
    return relation;
}

BDD generate_vars(transition_t *transition) {
    LACE_ME;

    BDD vars = sylvan_set_empty();
    sylvan_protect(&vars);


    for (int i = 0; i < transition->num_arcs; i++) {
        arc_t *arc = transition->arcs + i;
        vars = sylvan_set_add(vars, arc->place->identifier * 2);
    }

    sylvan_unprotect(&vars);
    return vars;
}

BDD generate_map(andl_context_t *andl_context) {
    LACE_ME;

    BDD map = sylvan_map_empty();
    sylvan_protect(&map);

    for (int i = 0; i < andl_context->num_places; i++) {
        place_t *place = andl_context->places + i;

        map = sylvan_map_add(map, place->identifier * 2 + 1, sylvan_ithvar(place->identifier * 2));
    }

    sylvan_unprotect(&map);
    return map;
}
