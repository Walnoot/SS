#include <sylvan.h>

#ifndef CTL_H
#define CTL_H

typedef struct StateProperty state_property_t;
typedef struct PathProperty path_property_t;

typedef enum StateKind {
    S_ATOM,
    S_NEGATION,
    S_CONJUNCTION,
    S_DISJUNCTION,
    S_EXISTS,
    S_FORALL
} state_kind_t;

typedef enum PathKind {
    //P_STATE,
    //P_NEGATION,     //Not strictly needed for CTL
    //P_CONJUNCTION,  //Not strictly needed for CTL
    //P_DISJUNCTION,  //NOT strictly needed for CTL
    P_NEXT,
    P_EVENTUALLY,
    P_GLOBALLY,
    P_UNTIL,
    P_RELEASES,
} path_kind_t;

typedef struct StateProperty {
    state_kind_t stateKind;
    union {
        struct {BDD atom;};                             //ATOM      //TODO is BDD the correct type here? should it be a pointer to a BDD? should it be something else entirely?
        struct {state_property_t *unary;};              //UNARY     (negation)
        struct {state_property_t *binary1, *binary2;};  //BINARY    (conjunction, disjunction)
        struct {path_property_t *pathProperty;};        //PATH      (exists, forall)
    };
} state_property_t;

typedef struct PathProperty {
    path_kind_t pathKind;
    union {
        /* struct{state_property_t *stateProperty;};       //STATE
         *
         * I removed this^ case and changed *unary, *binary1 and *binary2 to type state_property_t. (orignally was path_property_t) -- Jan
         * This change is allowed because of the shape of CTL formulas
         */
        struct{state_property_t *unary;};                //UNARY     (next, eventually, globally)       not implemented: negation
        struct{state_property_t *binary1, *binary2;};    //BINARY    (until, releases)                  not implemented: conjunction, disjunction
    };
} path_property_t;

/**
 * Normalizes a CTL formula to only contain EU, EG and EX clauses.
 * This function will free any pointers to subformulas that are rewritten.
 *
 * @param ast a pointer to the old CTL formula
 * @return a pointer to the new CTL formula
 */
state_property_t *normalize(state_property_t *ast);

#endif