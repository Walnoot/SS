#ifndef ANDL_H
#define ANDL_H

/**
 * Stores information while parsing .andl files.
 * Feel free to modify this file as you like.
 */

/**
 * \brief The direction of an arc:
 *  - ARC_IN is a place to transition arc,
 *  - ARC_OUT is a transition to place arc.
 */
typedef enum {
    ARC_IN,
    ARC_OUT,
} arc_dir_t;

typedef struct {
    char *name;
    int identifier;
    int initial_marking;
} place_t;

typedef struct {
    place_t *place;
    arc_dir_t dir;
} arc_t;

typedef struct {
    char *name;

    arc_t *arcs;
    int num_arcs;
    int arcs_buf_size;
} transition_t;

/**
 * \brief A struct to store information while parsing
 * an andl file.
 */
typedef struct {
    // the name of the Petri net
    char *name;

    // the name of the current transition being parsed
    char *current_trans;

    // the number of places in the Petri net
    int num_places;

    // the number of transitions in the Petri net
    int num_transitions;

    // the number of place-transition arcs in the Petri net
    int num_in_arcs;

    // the number of transition-place arcs in the Petri net
    int num_out_arcs;

    // whether an error has occured during parsing
    int error;

    place_t *places;
    int place_buf_size;

    transition_t *transitions;
    int transition_buf_size;
} andl_context_t;

#endif
