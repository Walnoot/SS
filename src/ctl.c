#include <sylvan.h>

#include "ctl.h"

state_property_t *normalize_EF(state_property_t *ast);
state_property_t *normalize_AX(state_property_t *ast);
state_property_t *normalize_AG(state_property_t *ast);
state_property_t *normalize_AF(state_property_t *ast);
state_property_t *normalize_AR(state_property_t *ast);
state_property_t *normalize_AU(state_property_t *ast);
state_property_t *normalize_ER(state_property_t *ast);


state_property_t *normalize(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    path_kind_t pathKind = pathProperty->pathKind;

    if (ast->stateKind == S_EXISTS) {
        switch(pathKind) {
            case P_EVENTUALLY:
                return normalize(normalize_EF(ast));
            case P_RELEASES:
                return normalize(normalize_ER(ast));
        }
    }
    else if (ast->stateKind == S_FORALL) {
        switch(pathKind) {
            case P_NEXT:
                return normalize(normalize_AX(ast));
            case P_GLOBALLY:
                return normalize(normalize_AG(ast));
            case P_EVENTUALLY:
                return normalize(normalize_AF(ast));
            case P_RELEASES:
                return normalize(normalize_AR(ast));
            case P_UNTIL:
                return normalize(normalize_AU(ast));
        }
    }

    return ast;
}

state_property_t *normalize_EF(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    path_property_t *inner_formula = pathProperty->unary;

    state_property_t *trueStateProperty = malloc(sizeof(state_property_t));
    trueStateProperty->stateKind = S_ATOM;
    trueStateProperty->atom = sylvan_true;
    path_property_t *truePathProperty = malloc(sizeof(pathProperty));
    truePathProperty->pathKind = P_STATE;
    truePathProperty->stateProperty = trueStateProperty;

    path_property_t *newPathProperty = malloc(sizeof(path_property_t));
    newPathProperty->pathKind = P_UNTIL;
    newPathProperty->binary1 = truePathProperty;        //pointer assignment
    newPathProperty->binary2 = inner_formula;           //pointer assignment

    state_property_t *newStateProperty = malloc(sizeof(state_property_t));
    newStateProperty->stateKind = S_EXISTS;
    newStateProperty->pathProperty = newPathProperty;   //pointer assignment

    free(pathProperty); //free F from the old EF formula
    free(ast);          //free E from the old EF formula

    return newStateProperty;
}

state_property_t *normalize_AX(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    path_property_t *inner_formula = pathProperty->unary;

    //TODO
    return ast;
}

state_property_t *normalize_AG(state_property_t *ast) {
    //TODO
    return ast;
}

state_property_t *normalize_AF(state_property_t *ast) {
    //TODO
    return ast;
}

state_property_t *normalize_AR(state_property_t *ast) {
    //TODO
    return ast;
}

state_property_t *normalize_AU(state_property_t *ast) {
    //TODO
    return ast;
}

state_property_t *normalize_ER(state_property_t *ast) {
    //TODO
    return ast;
}