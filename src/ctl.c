#include <sylvan.h>

#include "ctl.h"

ctl_node_t *normalize_EF(ctl_node_t *ast);
ctl_node_t *normalize_AX(ctl_node_t *ast);
ctl_node_t *normalize_AG(ctl_node_t *ast);
ctl_node_t *normalize_AF(ctl_node_t *ast);
ctl_node_t *normalize_AR(ctl_node_t *ast);
ctl_node_t *normalize_AU(ctl_node_t *ast);
ctl_node_t *normalize_ER(ctl_node_t *ast);


ctl_node_t *normalize(ctl_node_t *node) {
    switch(node->type) {
	    case CTL_ATOM:
	    	return node;
	    case CTL_NEGATION:
	    case CTL_EX:
	    case CTL_EG:
	    	node->unary.child = normalize(node->unary.child);
	    	return node;
	    case CTL_CONJUNCTION:
	    case CTL_DISJUNCTION:
	    case CTL_EU:
	    	node->binary.left = normalize(node->binary.left);
	    	node->binary.right = normalize(node->binary.right);
	    	return node;
	    case CTL_EF:
	    	return normalize_EF(node);
	    case CTL_ER:
	    	return normalize_ER(node);
	    case CTL_AX:
	    	return normalize_AX(node);
	    case CTL_AF:
	    	return normalize_AF(node);
	    case CTL_AG:
	    	return normalize_AG(node);
	    case CTL_AU:
	    	return normalize_AU(node);
	    case CTL_AR:
	    	return normalize_AR(node);
	    default:
	    	printf("Unhandled case in normalize\n");
	    	return NULL;
    }

    return node;
}

//basic building blocks
ctl_node_t *makeTrue() {
    ctl_node_t *trueStateProperty = malloc(sizeof(ctl_node_t));
    trueStateProperty->type = CTL_ATOM;
    trueStateProperty->atom.num_transitions = 0;
    return trueStateProperty;
}

ctl_node_t *negate(ctl_node_t *node) {
	ctl_node_t *negation = malloc(sizeof(ctl_node_t));
	negation->type = CTL_NEGATION;
	negation->unary.child = node;
	return negation;
}

ctl_node_t *conjunction(ctl_node_t *formula1, ctl_node_t *formula2) {
    ctl_node_t *conj  = malloc(sizeof(ctl_node_t));
    conj->type = CTL_CONJUNCTION;
    conj->binary.left = formula1;
    conj->binary.right = formula2;
    return conj;
}

ctl_node_t *disjunction(ctl_node_t *formula1, ctl_node_t *formula2) {
    ctl_node_t *disj  = malloc(sizeof(ctl_node_t));
    disj->type = CTL_DISJUNCTION;
    disj->binary.left = formula1;
    disj->binary.right = formula2;
    return disj;
}

//normalization cases

ctl_node_t *normalize_EF(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_EU;
	result->binary.left = makeTrue();
	result->binary.right = normalize(node->unary.child);

	free(node);

    return result;
}

ctl_node_t *normalize_AX(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_EX;
	result->unary.child = negate(normalize(node->unary.child));

	free(node);

    return negate(result);
}

ctl_node_t *normalize_AG(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_EF;
	result->unary.child = negate(normalize(node->unary.child));

	free(node);

    return negate(result);
}

ctl_node_t *normalize_AF(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_EG;
	result->unary.child = negate(normalize(node->unary.child));

	free(node);

    return negate(result);
}

ctl_node_t *normalize_AR(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_EU;
	result->binary.left = negate(normalize(node->binary.left));
	result->binary.right = negate(normalize(node->binary.right));

	free(node);

    return negate(result);
}

ctl_node_t *normalize_AU(ctl_node_t *node) {
	ctl_node_t *result = malloc(sizeof(ctl_node_t));

	result->type = CTL_ER;
	result->binary.left = negate(normalize(node->binary.left));
	result->binary.right = negate(normalize(node->binary.right));

	free(node);

    return negate(result);
}

ctl_node_t *normalize_ER(ctl_node_t *node) {
	ctl_node_t *left = malloc(sizeof(ctl_node_t));

	left->type = CTL_EU;

	ctl_node_t *child_right = normalize(node->binary.right);
	ctl_node_t *child_left = normalze(node->binary.left);

	left->binary.left = child_right;
	left->binary.right = conjunction(child_left, child_right);

	ctl_node_t *right = malloc(sizeof(ctl_node_t));

	right->type = CTL_EG;
	right->unary.child = child_right;

	free(node);

    return disjunction(left, right);
}