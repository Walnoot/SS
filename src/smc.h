#include <sylvan.h>
#include <ctl.h>
#include "andl.h"

#ifndef SMC_H
#define SMC_H

typedef struct
{
	int num_relations;
    BDD *relations;
    BDD *variables;
} smc_model_t;

int check(andl_context_t *model, ctl_node_t *formula);

BDD check_BDD(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_atom(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_negation(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_conjunction(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_disjunction(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EX(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EU(smc_model_t *model, ctl_node_t *formula);
BDD check_BDD_EG(smc_model_t *model, ctl_node_t *formula);

#endif