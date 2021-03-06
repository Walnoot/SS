#include <config.h>

#include <stdio.h>
#include <string.h>

#include <sylvan.h>

#include <andl.h>
#include <andl-lexer.h>
#include <ss-andl-parser.h>
#include <util.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//to create our fancy CTL ast
#include "ctl.h"
#include "smc.h"

#include "state_space.h"

/**
 * Load the andl file in \p name.
 * \p andl_context: The user context available when paring the andl file.
 * \p name: the name of the andl file to parse.
 * \return: 0 on success, 1 on failure.
 */
int
load_andl(andl_context_t *andl_context, const char *name)
{
    int res;
    FILE *f = fopen(name, "r");
    if (f == NULL) {
        warn("Could not open file '%s'", name);
        res = 1;
    } else {
        // initialize the lexer
        yyscan_t scanner;
        andl_lex_init(&scanner);
        // make the lexer read the file f
        andl_set_in(f, scanner);

        // zero the andl_context
        memset(andl_context, 0, sizeof(andl_context_t));

        andl_context->place_buf_size = 16;
        andl_context->places = malloc(sizeof(place_t) * andl_context->place_buf_size);

        andl_context->transition_buf_size = 16;
        andl_context->transitions = malloc(sizeof(transition_t) * andl_context->transition_buf_size);

        // parse the andl file
        const int pres = andl_parse(scanner, andl_context);

        // destroy the lexer
        andl_lex_destroy(scanner);
        fclose(f);
        res = andl_context->error || pres;
    }

    return res;
}

/**
 * Initializes Sylvan. The number of lace workers will be automatically
 * detected. The size of the node table, and cache are set to sensible
 * defaults. We initialize the BDD package (not LDD, or MTBDD).
 */
void
init_sylvan()
{
    int n_workers = 1; // auto-detect
    lace_init(n_workers, 40960000);
    lace_startup(0, NULL, NULL);

    /* initialize the node table and cache with minimum size 2^20 entries, and
     * maximum 2^27 entries */
    sylvan_init_package(1LL<<20,1LL<<27,1LL<<20,1LL<<27);

    // initialize Sylvan's BDD sub system
    sylvan_init_bdd();

    // sylvan_gc_disable();
}

/**
 * Deinialize Sylvan. If Sylvan is compiled with
 * -DSYLVAN_STATS=ON, then statistics will be print,
 * such as the number of nodes in the node table.
 */
void
deinit_sylvan()
{
    sylvan_stats_report(stderr);
    sylvan_quit();
    lace_exit();
}

/**
 * Here you should implement whatever is required for the Software Science lab class.
 * \p andl_context: The user context that is used while parsing
 * the andl file.
 * The default implementation right now, is to print several
 * statistics of the parsed Petri net.
 */
void
do_ss_things(andl_context_t *andl_context)
{
    warn("The name of the Petri net is: %s", andl_context->name);
    warn("There are %d transitions", andl_context->num_transitions);
    warn("There are %d places", andl_context->num_places);
    warn("There are %d in arcs", andl_context->num_in_arcs);
    warn("There are %d out arcs", andl_context->num_out_arcs);
    // warn("Current transition: %s", andl_context->current_trans);

    LACE_ME;

    // generate the BDDs of the initial state and relations

    BDD init = generate_initial_state(andl_context);
    sylvan_protect(&init);

    BDD *relations = malloc(andl_context->num_transitions * sizeof(BDD));
    BDD *vars = malloc(andl_context->num_transitions * sizeof(BDD));

    for (int i = 0; i < andl_context->num_transitions; i++) {
        relations[i] = generate_relation(andl_context->transitions + i);
        vars[i] = generate_vars(andl_context->transitions + i);
    }

    // perform the BFS

    BDD map = generate_map(andl_context);
    sylvan_protect(&map);

    BDD vOld = sylvan_set_empty();
    BDD vNew = init;
    sylvan_protect(&vOld);
    sylvan_protect(&vNew);

    int bfs_counter = 0;

    while (vOld != vNew) {
        vOld = vNew;

        for (int i = 0; i < andl_context->num_transitions; i++) {
            BDD relprod = sylvan_exists(sylvan_and(vNew, relations[i]), vars[i]);
            relprod = sylvan_compose(relprod, map);

            vNew = sylvan_or(vNew, relprod);
        }

        bfs_counter++;
    }

    printf("Number of loops: %d\n", bfs_counter);

    free(relations);
    free(vars);

    int count = mtbdd_satcount(vNew, andl_context->num_places);
    printf("SAT count: %d\n", count);

    FILE *f = fopen("test.dot", "w+");
    sylvan_fprintdot(f, vNew);
    fclose(f);
}

// convert a xml representation to the internal representation of the CTL formula.
ctl_node_t *parse_formula_to_ctl(xmlNode *node, andl_context_t *andl_context) {
    if (node == NULL) {
        warn("Invalid XML");
        return NULL;
    }
    else if (node -> type != XML_ELEMENT_NODE) {
        return parse_formula_to_ctl(xmlNextElementSibling(node), andl_context);
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "transition") == 0) {
        warn("Invalid XML - got transition node outside of an is-fireable node");
        return NULL;
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "is-fireable") == 0) {
        xmlNode *transitionNode = xmlFirstElementChild(node);

        ctl_node_t *atomNode = malloc(sizeof(ctl_node_t));
        atomNode->type = CTL_ATOM;
        atomNode->atom.num_transitions = 0;
        atomNode->atom.fireable_transitions = NULL;

        // keep track of the size of the array allocated for the transitions in this atom
        int buf_size = 0;

        while (transitionNode != NULL) {
            char* label = xmlNodeGetContent(transitionNode);

            //linear search the transitions for a transition with a matching name
            for (int i = 0; i < andl_context->num_transitions; i++) {
                transition_t *andl_transition = andl_context->transitions + i;

                if (strcmp(andl_transition->name, label) == 0) {
                    //we found the transition with the same name!

                    if (buf_size == 0) {
                        // allocate initial buffer size
                        buf_size = 8;
                        atomNode->atom.fireable_transitions = malloc(sizeof(transition_t) * buf_size);
                    } else if (buf_size == atomNode->atom.num_transitions - 1) {
                        // exponentially grow the buffer size
                        buf_size *= 2;
                        atomNode->atom.fireable_transitions = realloc(atomNode->atom.fireable_transitions,
                                sizeof(transition_t) * buf_size);
                    }

                    atomNode->atom.fireable_transitions[atomNode->atom.num_transitions] = *andl_transition;
                    atomNode->atom.num_transitions++;
                    break;
                }
            }

            transitionNode = xmlNextElementSibling(transitionNode);
        }

        return atomNode;
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "negation") == 0) {
        return negate(parse_formula_to_ctl(xmlFirstElementChild(node), andl_context));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "conjunction") == 0) {
        xmlNode *first = xmlFirstElementChild(node);
        xmlNode *second = xmlNextElementSibling(first);
        return conjunction(parse_formula_to_ctl(first, andl_context),
                           parse_formula_to_ctl(second, andl_context));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "disjunction") == 0) {
        xmlNode *first = xmlFirstElementChild(node);
        xmlNode *second = xmlNextElementSibling(first);
        return disjunction(parse_formula_to_ctl(first, andl_context),
                           parse_formula_to_ctl(second, andl_context));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "all-paths") == 0) {
        //ForAll cases
        xmlNode *child = xmlFirstElementChild(node);

        if (xmlStrcmp(child->name, (const xmlChar*) "globally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AG(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "finally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AF(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "next") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AX(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "until") == 0) {
            //parse Before and parse Reach
            xmlNode *beforeChild    = xmlFirstElementChild(child);
            xmlNode *reachChild     = xmlNextElementSibling(beforeChild);

            ctl_node_t *left = parse_formula_to_ctl(xmlFirstElementChild(beforeChild), andl_context);
            ctl_node_t *right = parse_formula_to_ctl(xmlFirstElementChild(reachChild), andl_context);

            return ctl_make_AU(left, right);
        }
    } //endif ForAll
    else if (xmlStrcmp(node->name, (const xmlChar*) "exists-path") == 0) {
        //Exists cases
        xmlNode *child = xmlFirstElementChild(node);

        if (xmlStrcmp(child->name, (const xmlChar*) "globally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EG(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "finally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EF(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "next") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EX(parse_formula_to_ctl(grandChild, andl_context));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "until") == 0) {
            //parse Before and parse Reach
            xmlNode *beforeChild    = xmlFirstElementChild(child);
            xmlNode *reachChild     = xmlNextElementSibling(beforeChild);

            ctl_node_t *left = parse_formula_to_ctl(xmlFirstElementChild(beforeChild), andl_context);
            ctl_node_t *right = parse_formula_to_ctl(xmlFirstElementChild(reachChild), andl_context);

            return ctl_make_EU(left, right);
        }
    } //endif Exists

    return NULL;
}

/**
 * \brief An in-order parser of the given XML node.
 *
 * The default implementation is to print the temporal logic formula
 * on stderr.
 */
static int
parse_formula(xmlNode *node)
{
    int res = 0;
    // first check if the node is not a NULL pointer.
    if (node == NULL) {
        res = 1;
        warn("Invalid XML");
    // only parse xml nodes, skip other parts of the XML file.
    } else if (node->type != XML_ELEMENT_NODE) res = parse_formula(xmlNextElementSibling(node));
    // parse forAll
    else if (xmlStrcmp(node->name, (const xmlChar*) "all-paths") == 0) {
        fprintf(stderr, "A ");
        res = parse_formula(xmlFirstElementChild(node));
    // parse Exists
    } else if (xmlStrcmp(node->name, (const xmlChar*) "exists-path") == 0) {
        fprintf(stderr, "E ");
        res = parse_formula(xmlFirstElementChild(node));
    // parse Globally
    } else if (xmlStrcmp(node->name, (const xmlChar*) "globally") == 0) {
        fprintf(stderr, "G ");
        res = parse_formula(xmlFirstElementChild(node));
    // parse Finally
    } else if (xmlStrcmp(node->name, (const xmlChar*) "finally") == 0) {
        fprintf(stderr, "F ");
        res = parse_formula(xmlFirstElementChild(node));
    // parse neXt
    } else if (xmlStrcmp(node->name, (const xmlChar*) "next") == 0) {
        fprintf(stderr, "X ");
        res = parse_formula(xmlFirstElementChild(node));
    // parse Until
    } else if (xmlStrcmp(node->name, (const xmlChar*) "until") == 0) {
        fprintf(stderr, "(");
        res = parse_formula(xmlFirstElementChild(node));
        fprintf(stderr, ") U (");
        res |= parse_formula(xmlNextElementSibling(xmlFirstElementChild(node)));
        fprintf(stderr, ")");
    // parse before
    } else if (xmlStrcmp(node->name, (const xmlChar*) "before") == 0) {
        res = parse_formula(xmlFirstElementChild(node));
    // parse reach
    } else if (xmlStrcmp(node->name, (const xmlChar*) "reach") == 0) {
        res = parse_formula(xmlFirstElementChild(node));
    // parse negation
    } else if (xmlStrcmp(node->name, (const xmlChar*) "negation") == 0) {
        fprintf(stderr, "!(");
        res = parse_formula(xmlFirstElementChild(node));
        fprintf(stderr, ")");
    // parse conjunction
    } else if (xmlStrcmp(node->name, (const xmlChar*) "conjunction") == 0) {
        fprintf(stderr, "(");
        res = parse_formula(xmlFirstElementChild(node));
        fprintf(stderr, ") && (");
        res |= parse_formula(xmlNextElementSibling(xmlFirstElementChild(node)));
        fprintf(stderr, ")");
    // parse disjunction
    } else if (xmlStrcmp(node->name, (const xmlChar*) "disjunction") == 0) {
        fprintf(stderr, "(");
        res = parse_formula(xmlFirstElementChild(node));
        fprintf(stderr, ") || (");
        res |= parse_formula(xmlNextElementSibling(xmlFirstElementChild(node)));
        fprintf(stderr, ")");
    // parse is-fireable: atomic predicate!
    } else if (xmlStrcmp(node->name, (const xmlChar*) "is-fireable") == 0) {
        fprintf(stderr, "is-fireable(");
        res = parse_formula(xmlFirstElementChild(node));
        fprintf(stderr, ")");
    // parse transition (part of the atomic predicate)
    } else if (xmlStrcmp(node->name, (const xmlChar*) "transition") == 0) {
        for (xmlNode *transition = node; transition != NULL;
                transition = xmlNextElementSibling(transition)) {
            fprintf(stderr, "%s,", xmlNodeGetContent(transition));
        }
    } else {
        res = 1;
        warn("Invalid xml node '%s'", node->name);
    }
    return res;
}

static ctl_node_t *parse_xml_property(xmlNode *node, andl_context_t *andl_context) {
    ctl_node_t *res = NULL;

    if (xmlStrcmp(node->name, (const xmlChar*) "property") == 0) {
        warn("parsing property");
        res = parse_xml_property(xmlFirstElementChild(node), andl_context);
    // parse id of property
    } else if (xmlStrcmp(node->name, (const xmlChar*) "id") == 0) {
        warn("Property id is: %s", xmlNodeGetContent(node));
        res = parse_xml_property(xmlNextElementSibling(node), andl_context);
    // parse description of property
    } else if (xmlStrcmp(node->name, (const xmlChar*) "description") == 0) {
        warn("Property description is: %s", xmlNodeGetContent(node));
        res = parse_xml_property(xmlNextElementSibling(node), andl_context);
    // parse the formula
    } else if (xmlStrcmp(node->name, (const xmlChar*) "formula") == 0) {
        warn("Parsing formula...");
        res = parse_formula_to_ctl(xmlFirstElementChild(node), andl_context);
        printf("\n");
    // node not recognized
    } else {
        res = NULL;
        warn("Invalid xml node '%s'", node->name);
    }

    return res;
}

/**
 * \brief recursively parse the given XML node.
 */
static ctl_node_t **parse_xml(xmlNode *node, andl_context_t *andl_context)
{
    // construct a list of pointers to the CTL formulas
    ctl_node_t **res = NULL;
    int buf_size = 0;
    int num_formulas = 0;

    // first check if the node is not a NULL pointer.
    if (node == NULL) {
        warn("Invalid XML");
    // only parse xml nodes, skip other parts of the XML file.
    } else if (node->type != XML_ELEMENT_NODE) res = parse_xml(xmlNextElementSibling(node), andl_context);
    // parse property-set
    else if (xmlStrcmp(node->name, (const xmlChar*) "property-set") == 0) {
        // loop over all children that are property nodes
        for (xmlNode *property = xmlFirstElementChild(node);
                property != NULL;
                property = xmlNextElementSibling(property)) {
            ctl_node_t *formula = parse_xml_property(property, andl_context);

            if(buf_size == 0) {
                buf_size = 8;
                res = malloc(sizeof(ctl_node_t*) * buf_size);
            } else if (num_formulas - 1 == buf_size) {
                buf_size *= 2;

                res = realloc(res, sizeof(ctl_node_t*) * buf_size);
            }

            res[num_formulas] = formula;
            num_formulas++;
        }
    }

    // add NULL terminator
    res[num_formulas] = NULL;

    return res;
}

/**
 * \brief parses the XML file name.
 *
 * \returns a list of CTL formulas
 */
static ctl_node_t **load_xml(const char* name, andl_context_t *andl_context)
{
    int res;

    LIBXML_TEST_VERSION
    warn("parsing formulas file: %s", name);
    xmlDoc *doc = xmlReadFile(name, NULL, 0);
    if (doc == NULL) {
        warn("unable to read xml file: %s", name);
        return NULL;
    }
    else {
        xmlNode *node = xmlDocGetRootElement(doc);
        return parse_xml(node, andl_context);
    }
}

/**
 * \brief main. First parse the .andl file is parsed. And optionally parse the
 * XML file next.
 *
 * \returns 0 on success, 1 on failure.
 */
int main(int argc, char** argv)
{
    int res;
    if (argc >= 2) {
        andl_context_t andl_context;

        const char *name = argv[1];
        res = load_andl(&andl_context, name);
        if (res) warn("Unable to parse file '%s'", name);
        else {
            init_sylvan();

            warn("Successful parse of file '%s' :)", name);
            if (argc == 3) {
                const char *formulas = argv[2];
                // load all formulas from the XML file
                ctl_node_t **ctl_formulas = load_xml(formulas, &andl_context);

                // the list of ctl formulas is NULL terminated
                for (int i = 0; ctl_formulas[i] != NULL; i++) {
                    ctl_node_t *formula = ctl_formulas[i];

                    printf("\nformula %d\n\n", i);

                    print_ctl(formula);

                    ctl_node_t *normalized = normalize(formula);

                    printf("\nnormalized %d\n\n", i);

                    print_ctl(normalized);

                    printf("\nSMC outcome for formula %d: %s\n\n", i, check(&andl_context, normalized) ? "T" : "F");
                }
            }

            // generate the whole state space and print the SAT count
            do_ss_things(&andl_context);

            deinit_sylvan();
        }
    } else {
        warn("Usage: %s <petri-net>.andl [<CTL-formulas>.xml]", argv[0]);
        res = 1;
    }

    return res;
}

