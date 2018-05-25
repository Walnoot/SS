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

//basic building blocks
state_property_t *makeTrue() {
    state_property_t *trueStateProperty = malloc(sizeof(state_property_t));
    trueStateProperty->stateKind    = S_ATOM;
    trueStateProperty->atom         = sylvan_true;
    return trueStateProperty;
}

state_property_t *makeFalse() {
    state_property_t *falseStateProperty = malloc(sizeof(state_property_t));
    falseStateProperty->stateKind    = S_ATOM;
    falseStateProperty->atom         = sylvan_false;
    return falseStateProperty;
}


//state properties
state_property_t *negate(state_property_t *formula) {
    state_property_t *negation  = malloc(sizeof(state_property_t));
    negation->stateKind         = S_NEGATION;
    negation->unary             = formula;
    return negation;
}

state_property_t *conjunction(state_property_t *formula1, state_property_t *formula2) {
    state_property_t *conj  = malloc(sizeof(state_property_t));
    conj->stateKind         = S_CONJUNCTION;
    conj->binary1           = formula1;
    conj->binary2           = formula2;
    return conj;
}

state_property_t *disjunction(state_property_t *formula1, state_property_t *formula2) {
    state_property_t *disj  = malloc(sizeof(state_property_t));
    disj->stateKind         = S_DISJUNCTION;
    disj->binary1           = formula1;
    disj->binary2           = formula2;
    return disj;
}

state_property_t *exists(path_property_t *pathProperty) {
    state_property_t *existential   = malloc(sizeof(state_property_t));
    existential->stateKind          = S_EXISTS;
    existential->pathProperty       = pathProperty;
    return existential;
}

state_property_t *forAll(path_property_t *pathProperty) {
    state_property_t *universal = malloc(sizeof(state_property_t));
    universal->stateKind        = S_FORALL;
    universal->pathProperty     = pathProperty;
    return universal;
}


//path properties
path_property_t *next(state_property_t *formula) {
    path_property_t *x  = malloc(sizeof(path_property_t));
    x->pathKind         = P_NEXT;
    x->unary            = formula;
    return x;
}

path_property_t *globally(state_property_t *formula) {
    path_property_t *g  = malloc(sizeof(path_property_t));
    g->pathKind         = P_GLOBALLY;
    g->unary            = formula;
    return g;
}

path_property_t *eventually(state_property_t *formula) {
    path_property_t *f  = malloc(sizeof(path_property_t));
    f->pathKind         = P_EVENTUALLY;
    f->unary            = formula;
    return f;
}

path_property_t *until(state_property_t *formula1, state_property_t *formula2) {
    path_property_t *u = malloc(sizeof(path_property_t));
    u->pathKind     = P_UNTIL;
    u->binary1      = formula1;
    u->binary2      = formula2;
    return u;
}

path_property_t *releases(state_property_t *formula1, state_property_t *formula2) {
    path_property_t *r = malloc(sizeof(path_property_t));
    r->pathKind     = P_RELEASES;
    r->binary1      = formula1;
    r->binary2      = formula2;
    return r;
}


//normalization cases
state_property_t *normalize_EF(state_property_t *ast) {
    path_property_t *pathProperty   = ast->pathProperty;
    state_property_t *innerFormula  = normalize(pathProperty->unary);

    free(pathProperty); //free F from the old EF formula
    free(ast);          //free E from the old EF formula

    return exists(until(makeTrue(), innerFormula));
}

state_property_t *normalize_AX(state_property_t *ast) {
    path_property_t *pathProperty   = ast->pathProperty;
    state_property_t *innerFormula  = normalize(pathProperty->unary);

    free(pathProperty);     //free X from the old AX formula
    free(ast);              //free A from the old AX formula

    return negate(exists(next(negate(innerFormula))));
}

state_property_t *normalize_AG(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    state_property_t *innerFormula = normalize(pathProperty->unary);

    free(pathProperty);     //free G from the old AG formula
    free(ast);              //free A from the old AG formula

    return negate(exists(eventually(negate(innerFormula))));
}

state_property_t *normalize_AF(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    state_property_t *innerFormula = pathProperty->unary;

    free(pathProperty);     //free F from the old AF formula
    free(ast);              //free A from the old AF formula

    return negate(exists(globally(negate(innerFormula))));
}

state_property_t *normalize_AR(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    state_property_t *innerOne = normalize(pathProperty->binary1);
    state_property_t *innerTwo = normalize(pathProperty->binary2);

    free(pathProperty);     //free R from the old AR formula
    free(ast);              //free A from the old AR formula

    return negate(exists(until(negate(innerOne), negate(innerTwo))));
}

state_property_t *normalize_AU(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    state_property_t *innerOne = normalize(pathProperty->binary1);
    state_property_t *innerTwo = normalize(pathProperty->binary2);

    free(pathProperty);     //free U from the old AU formula
    free(ast);              //free A from the old AU formula

    return negate(exists(releases(negate(innerOne), negate(innerTwo))));
}

state_property_t *normalize_ER(state_property_t *ast) {
    path_property_t *pathProperty = ast->pathProperty;
    state_property_t *innerOne = normalize(pathProperty->binary1);
    state_property_t *innerTwo = normalize(pathProperty->binary2);

    free(pathProperty);     //free R from the old ER formula
    free(ast);              //free E from the old ER formula

    return disjunction(exists(until(innerTwo, conjunction(innerOne, innerTwo))), exists(globally(innerTwo)));
}