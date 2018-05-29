#include "smc.h"

#include <sylvan.h>

#include "state_space.h"
int check(andl_context_t *andl_context, ctl_node_t *formula) {
	LACE_ME;
	
	BDD intial_state = generate_initial_state(andl_context);
	BDD relation = sylvan_false;
	BDD vars = sylvan_set_empty();

	sylvan_protect(&intial_state);
	sylvan_protect(&relation);
	sylvan_protect(&vars);

	for (int i = 0; i < andl_context->num_transitions; i++) {
		relation = sylvan_or(relation, generate_relation(andl_context->transitions + i));
		BDD variables = generate_vars(andl_context->transitions + i);
		sylvan_protect(&variables);

		while(!sylvan_set_isempty(variables)) {
			vars = sylvan_set_add(vars, sylvan_set_first(variables));
			variables = sylvan_set_next(variables);
		}

		sylvan_unprotect(&variables);
	}
	
	smc_model_t *model = malloc(sizeof(smc_model_t));
	model->relation = relation;
	model->variables = vars;

	BDD state_space = check_BDD(model, formula);
	sylvan_protect(&state_space);

	int result = sylvan_and(intial_state, sylvan_not(state_space)) == sylvan_false;

	sylvan_unprotect(&intial_state);
	sylvan_unprotect(&state_space);
	sylvan_unprotect(&relation);
	sylvan_unprotect(&vars);

	return result;
}

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

		BDD transition_pre = sylvan_true;
		sylvan_protect(&transition_pre);

		transition_t *transition = formula->atom.fireable_transitions + i;
		for (int j = 0; j < transition->num_arcs; j++) {
			arc_t *arc = transition->arcs + j;

			if (arc->dir == ARC_IN) {
				int identifier = arc->place->identifier;
				transition_pre = sylvan_and(transition_pre, sylvan_ithvar(2 * identifier));
			}
		}

		sylvan_unprotect(&transition_pre);
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

	BDD z = b;
	BDD old = (BDD) NULL; //assume BDD identifiers are never equal to NULL
	while (z != old) {
		old = z;
		z = sylvan_or(z, sylvan_and(a, sylvan_relprev(model->relation, z, model->vars)));
	}

	sylvan_unprotect(&a);
	sylvan_unprotect(&b);

	return z;
}

BDD check_BDD_EG(smc_model_t *model, ctl_node_t *formula) {
	LACE_ME;
	
	BDD a = check_BDD(model, formula->unary.child);

	sylvan_protect(&a);

	BDD z = a;
	BDD old = (BDD) NULL; //assume BDD identifiers are never equal to NULL
	while (z != old) {
		old = z;
		z = sylvan_or(z, sylvan_and(a, sylvan_relprev(model->relation, z, model->variables)));
	}

	sylvan_unprotect(&a);

	return z;
}