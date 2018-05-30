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

    return NULL;
}

// print this CTL formula to stdout, useful for debugging
void print_ctl_rec(ctl_node_t *node) {
	if (node == NULL) {
		printf("NULL");
		return;
	}

    switch(node->type) {
	    case CTL_ATOM:
	    	if (node->atom.num_transitions == -1) {
	    		printf("TRUE");
	    	} else {
		    	printf("ATOM ");

		    	for (int i = 0; i < node->atom.num_transitions; i++) {
		    		printf("%s ", node->atom.fireable_transitions[i].name);
		    	}
	    	}

	    	break;
	    case CTL_NEGATION:
	    	printf("NOT( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_CONJUNCTION:
	    	printf("( ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" AND ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" )");
	    	break;
	    case CTL_DISJUNCTION:
	    	printf("( ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" OR ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" )");
	    	break;
	    case CTL_EX:
	    	printf("EX ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_EF:
	    	printf("EF ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_EG:
	    	printf("EG ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_EU:
	    	printf("E[ ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" U ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" ]");
	    	break;
	    case CTL_ER:
	    	printf("E[ ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" R ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" ]");
	    	break;
	    case CTL_AX:
	    	printf("AX ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_AF:
	    	printf("AF ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_AG:
	    	printf("AG ( ");
	    	print_ctl_rec(node->unary.child);
	    	printf(" )");
	    	break;
	    case CTL_AU:
	    	printf("A[ ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" U ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" ]");
	    	break;
	    case CTL_AR:
	    	printf("A[ ");
	    	print_ctl_rec(node->binary.left);
	    	printf(" R ");
	    	print_ctl_rec(node->binary.right);
	    	printf(" ]");
	    	break;
    }
}


void print_ctl(ctl_node_t *node) {
	print_ctl_rec(node);
	printf("\n");
}

//basic building blocks
ctl_node_t *makeTrue() {
    ctl_node_t *trueStateProperty = malloc(sizeof(ctl_node_t));
    trueStateProperty->type = CTL_ATOM;
    trueStateProperty->atom.num_transitions = -1;
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


//TL constructors
//Exists
ctl_node_t *ctl_make_EX(ctl_node_t *inner) {
    ctl_node_t *ex  = malloc(sizeof(ctl_node_t));
    ex->type = CTL_EX;
    ex->unary.child = inner;
    return ex;
}

ctl_node_t *ctl_make_EG(ctl_node_t *inner) {
    ctl_node_t *eg = malloc(sizeof(ctl_node_t));
    eg->type = CTL_EG;
    eg->unary.child = inner;
    return eg;
}

ctl_node_t *ctl_make_EF(ctl_node_t *inner) {
    ctl_node_t *ef = malloc(sizeof(ctl_node_t));
    ef->type = CTL_EF;
    ef->unary.child = inner;
    return ef;
}

ctl_node_t *ctl_make_EU(ctl_node_t *innerOne, ctl_node_t *innerTwo) {
    ctl_node_t *eu  = malloc(sizeof(ctl_node_t));
    eu->type = CTL_EU;
    eu->binary.left = innerOne;
    eu->binary.right = innerTwo;
    return eu;
}

ctl_node_t *ctl_make_ER(ctl_node_t *innerOne, ctl_node_t *innerTwo) {
    ctl_node_t *er  = malloc(sizeof(ctl_node_t));
    er->type = CTL_ER;
    er->binary.left = innerOne;
    er->binary.right = innerTwo;
    return er;
}
//ForAll
ctl_node_t *ctl_make_AX(ctl_node_t *inner) {
    ctl_node_t *ax  = malloc(sizeof(ctl_node_t));
    ax->type = CTL_AX;
    ax->unary.child = inner;
    return ax;
}

ctl_node_t *ctl_make_AG(ctl_node_t *inner) {
    ctl_node_t *ag = malloc(sizeof(ctl_node_t));
    ag->type = CTL_AG;
    ag->unary.child = inner;
    return ag;
}

ctl_node_t *ctl_make_AF(ctl_node_t *inner) {
    ctl_node_t *af = malloc(sizeof(ctl_node_t));
    af->type = CTL_AF;
    af->unary.child = inner;
    return af;
}

ctl_node_t *ctl_make_AU(ctl_node_t *innerOne, ctl_node_t *innerTwo) {
    ctl_node_t *au  = malloc(sizeof(ctl_node_t));
    au->type = CTL_AU;
    au->binary.left = innerOne;
    au->binary.right = innerTwo;
    return au;
}

ctl_node_t *ctl_make_AR(ctl_node_t *innerOne, ctl_node_t *innerTwo) {
    ctl_node_t *ar  = malloc(sizeof(ctl_node_t));
    ar->type = CTL_AR;
    ar->binary.left = innerOne;
    ar->binary.right = innerTwo;
    return ar;
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

    return negate(normalize(result));
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

    return negate(normalize(result));
}

ctl_node_t *normalize_ER(ctl_node_t *node) {
	ctl_node_t *left = malloc(sizeof(ctl_node_t));

	left->type = CTL_EU;

	ctl_node_t *child_right = normalize(node->binary.right);
	ctl_node_t *child_left = normalize(node->binary.left);

	left->binary.left = child_right;
	left->binary.right = conjunction(child_left, child_right);

	ctl_node_t *right = malloc(sizeof(ctl_node_t));

	right->type = CTL_EG;
	right->unary.child = child_right;

	free(node);

    return disjunction(left, right);
}