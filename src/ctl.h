#include <sylvan.h>
#include "andl.h"

#ifndef CTL_H
#define CTL_H

typedef enum {
    CTL_ATOM,
    CTL_NEGATION,
    CTL_CONJUNCTION,
    CTL_DISJUNCTION,
    CTL_EX,
    CTL_EF,
    CTL_EG,
    CTL_EU,
    CTL_ER,
    CTL_AX,
    CTL_AF,
    CTL_AG,
    CTL_AU,
    CTL_AR,
} ctl_type_t;

typedef struct ctl_node_t ctl_node_t;

typedef struct ctl_node_t
{
    ctl_type_t type;
    union {
        struct
        {
            ctl_node_t *left;
            ctl_node_t *right;
        } binary;

        struct
        {
            ctl_node_t *child;
        } unary;

        struct
        {
            transition_t *fireable_transitions;

            //value of -1 represents true value
            int num_transitions;
        } atom;
    };
} ctl_node_t;

//CTL formula constructors

//basic building blocks
ctl_node_t *makeTrue();

//state properties
ctl_node_t *negate(ctl_node_t *formula);
ctl_node_t *conjunction(ctl_node_t *formula1, ctl_node_t *formula2);
ctl_node_t *disjunction(ctl_node_t *formula1, ctl_node_t *formula2);

//temporal operators
ctl_node_t *ctl_make_EX(ctl_node_t *inner);
ctl_node_t *ctl_make_EG(ctl_node_t *inner);
ctl_node_t *ctl_make_EF(ctl_node_t *inner);
ctl_node_t *ctl_make_EU(ctl_node_t *innerOne, ctl_node_t *innerTwo);
ctl_node_t *ctl_make_ER(ctl_node_t *innerOne, ctl_node_t *innerTwo);
ctl_node_t *ctl_make_AX(ctl_node_t *inner);
ctl_node_t *ctl_make_AG(ctl_node_t *inner);
ctl_node_t *ctl_make_AF(ctl_node_t *inner);
ctl_node_t *ctl_make_AU(ctl_node_t *innerOne, ctl_node_t *innerTwo);
ctl_node_t *ctl_make_AR(ctl_node_t *innerOne, ctl_node_t *innerTwo);


/**
 * Normalizes a CTL formula to only contain temporal logic clauses EU, EG and EX.
 * Naturally, regular negations, conjunctions and disjunctions can be part of the resulting formula.
 * This function will free any pointers to subformulas that are rewritten.
 *
 * @param ast a pointer to the old CTL formula
 * @return a pointer to the new CTL formula
 */
ctl_node_t *normalize(ctl_node_t *ast);

void print_ctl(ctl_node_t *ast);

#endif