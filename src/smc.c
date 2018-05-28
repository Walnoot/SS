#include "smc.h"

BDD check(smc_model_t *model, ctl_node_t *formula);

BDD check_BDD(smc_model_t *model, ctl_node_t *formula);

BDD check_BDD_atom(smc_model_t *model, ctl_node_t *formula) {
	BDD result = sylvan_false;
	sylvan_protect(&result);

	for (int i = 0; i < formula->atom.num_transitions; i++) {
		
	}

	sylvan_unprotect(&result);
	return result;
}


BDD check_BDD_negation(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_conjunction(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EX(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EU(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EG(smc_model_t *model, ctl_node_t *formula);