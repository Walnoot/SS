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
    P_STATE,
    P_NEGATION,
    P_CONJUNCTION,
    P_DISJUNCTION,
    P_NEXT,
    P_EVENTUALLY,
    P_GLOBALLY,
    P_UNTIL,
    P_RELEASES,
} path_kind_t;

typedef struct StateProperty {
    state_kind_t stateKind;
    union {
        struct {BDD atom;};                             //ATOM
        struct {state_property_t *unary;};              //UNARY     (negation)
        struct {state_property_t *binary1, *binary2;};  //BINARY    (conjunction, disjunction)
        struct {path_property_t *pathProperty;};        //PATH      (exists, forall)
    };
} state_property_t;

typedef struct PathProperty {
    path_kind_t pathKind;
    union {
        struct{state_property_t *stateProperty;};       //STATE
        struct{path_property_t *unary;};                //UNARY     (negation, next, eventually, globally)
        struct{path_property_t *binary1, *binary2;};    //BINARY    (conjunction, disjunction, until, releases)
    };
} path_property_t;

state_property_t normalize(state_property_t ast);

#endif