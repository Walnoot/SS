#include "smc.h"

#include <sylvan.h>

BDD check(smc_model_t *model, ctl_node_t *formula);

BDD check_BDD(smc_model_t *model, ctl_node_t *formula) {
	switch(formula->type) {
		case CTL_ATOM:
			return check_BDD_atom(model, formula);
		case CTL_NEGATION:
			return check_BDD_negation(model, formula);
		case CTL_CONJUNCTION:
			return check_BDD_conjunction(model, formula);
		case CTL_DISJUNCTION:
			// not stricty needed, but implementation is simple so it's not reduced in ctl.c
			return check_BDD_disjunction(model, formula);
		case CTL_EX:
			return check_BDD_EX(model, formula);
		case CTL_EU:
			return check_BDD_EU(model, formula);
		case CTL_EG:
			return check_BDD_EG(model, formula);
		default:
			printf("Unknown case in check_BDD\n");
			return (BDD) NULL;
	}
}

BDD check_BDD_atom(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;

	BDD result = sylvan_false;
	sylvan_protect(&result);

	for (int i = 0; i < formula->atom.num_transitions; i++) {
		// check precondition of transition
		// result = sylvan_or(sylvan_ithvar(formula->atom.transitions[i] * 2));
	}

	sylvan_unprotect(&result);
	return result;
}

BDD check_BDD_negation(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;

	return sylvan_not(check_BDD(model, formula->unary.child));
}

BDD check_BDD_conjunction(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;

	BDD left = check_BDD(model, formula->binary.left);
	BDD right = check_BDD(model, formula->binary.right);
	return sylvan_and(left, right);
}

BDD check_BDD_disjunction(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;
	
	BDD left = check_BDD(model, formula->binary.left);
	BDD right = check_BDD(model, formula->binary.right);
	return sylvan_or(left, right);
}

BDD check_BDD_EX(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;
	
	BDD relation; //TODO: construct combined R from R1, R2...Rn
	BDD vars;

	return sylvan_relprev(relation, check_BDD(model, formula->unary.child), vars);
}

BDD check_BDD_EU(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;
	
	BDD a = check_BDD(model, formula->binary.left);
	BDD b = check_BDD(model, formula->binary.right);

	sylvan_protect(&a);
	sylvan_protect(&b);

	BDD relation; //TODO: construct combined R from R1, R2...Rn
	BDD vars;

	BDD z = b;
	BDD old = (BDD) NULL; //assume BDD identifiers are never equal to NULL
	while (z != old) {
		old = z;
		z = sylvan_or(z, sylvan_and(a, sylvan_relprev(relation, z, vars)));
	}

	sylvan_unprotect(&a);
	sylvan_unprotect(&b);

	return z;
}

BDD check_BDD_EG(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;
	
	BDD a = check_BDD(model, formula->unary.child);

	sylvan_protect(&a);

	BDD relation; //TODO: construct combined R from R1, R2...Rn
	BDD vars;

	BDD z = a;
	BDD old = (BDD) NULL; //assume BDD identifiers are never equal to NULL
	while (z != old) {
		old = z;
		z = sylvan_or(z, sylvan_and(a, sylvan_relprev(relation, z, vars)));
	}

	sylvan_unprotect(&a);

	return z;
}