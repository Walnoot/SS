#include <config.h>

#include <stdio.h>

#include <sylvan.h>

#include <andl.h>
#include <andl-lexer.h>
#include <ss-andl-parser.h>
#include <util.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//to create our fancy CTL ast
#include "ctl.h"

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
    int n_workers = 0; // auto-detect
    lace_init(n_workers, 0);
    lace_startup(0, NULL, NULL);

    /* initialize the node table and cache with minimum size 2^20 entries, and
     * maximum 2^25 entries */
    sylvan_init_package(1LL<<20,1LL<<25,1LL<<20,1LL<<25);

    // initialize Sylvan's BDD sub system
    sylvan_init_bdd();
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

BDD generate_initial_state(andl_context_t *andl_context) {
    LACE_ME;

    BDD init = sylvan_true;
    sylvan_protect(&init);

    for (int i = 0; i < andl_context->num_places; i++) {
        place_t *place = andl_context->places + i;

        // variable identifiers are 2*n for normal variables, and 2*n+1 for prime variables.

        if (place->initial_marking == 0) {
            init = sylvan_and(init, sylvan_nithvar(place->identifier * 2));
        } else {
            // assume initial marking is either 0 or 1, because 1-safe

            init = sylvan_and(init, sylvan_ithvar(place->identifier * 2));
        }
    }

    sylvan_unprotect(&init);
    return init;
}

BDD generate_relation(transition_t *transition) {
    LACE_ME;

    BDD relation = sylvan_true;
    sylvan_protect(&relation);

    for (int i = 0; i < transition->num_arcs; i++) {
        arc_t *arc = transition->arcs + i;

        if (arc->dir == ARC_IN) {
            // precondition
            relation = sylvan_and(relation, sylvan_ithvar(arc->place->identifier * 2));

            //postcondition
            relation = sylvan_and(relation, sylvan_nithvar(arc->place->identifier * 2 + 1));
        } else {
            // postcondition
            relation = sylvan_and(relation, sylvan_ithvar(arc->place->identifier * 2 + 1));
        }
    }

    sylvan_unprotect(&relation);
    return relation;
}

BDD generate_vars(transition_t *transition) {
    LACE_ME;

    BDD vars = sylvan_set_empty();
    sylvan_protect(&vars);


    for (int i = 0; i < transition->num_arcs; i++) {
        arc_t *arc = transition->arcs + i;
        vars = sylvan_set_add(vars, arc->place->identifier * 2);
    }

    sylvan_unprotect(&vars);
    return vars;
}

BDD generate_map(andl_context_t *andl_context) {
    LACE_ME;

    BDD map = sylvan_map_empty();
    sylvan_protect(&map);

    for (int i = 0; i < andl_context->num_places; i++) {
        place_t *place = andl_context->places + i;

        map = sylvan_map_add(map, place->identifier * 2 + 1, sylvan_ithvar(place->identifier * 2));
    }

    sylvan_unprotect(&map);
    return map;
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

    // for (int i = 0; i < andl_context->num_places; i++) {
    //     warn("place: %s = %d", andl_context->places[i].name, andl_context->places[i].initial_marking);
    // }

    // for (int i = 0; i < andl_context->num_transitions; i++) {
    //     transition_t *transition = andl_context->transitions + i;
    //     warn("transition: %s", transition->name);

    //     for (int j = 0; j < transition->num_arcs; j++) {
    //         arc_t *arc = transition->arcs + j;

    //         if (arc->dir == ARC_IN) {
    //             warn("\tarc: %s (IN)", arc->place->name);
    //         } else {
    //             warn("\tarc: %s (OUT)", arc->place->name);
    //         }
    //     }
    // }

    LACE_ME;

    BDD init = generate_initial_state(andl_context);
    sylvan_protect(&init);

    BDD *relations = malloc(andl_context->num_transitions * sizeof(BDD));
    BDD *vars = malloc(andl_context->num_transitions * sizeof(BDD));

    for (int i = 0; i < andl_context->num_transitions; i++) {
        relations[i] = generate_relation(andl_context->transitions + i);
        vars[i] = generate_vars(andl_context->transitions + i);
    }

    // bfs

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

ctl_node_t *parse_xml_to_ctl(xmlNode *node) {
    if (node == NULL) {
        warn("Invalid XML");
        return NULL;
    }
    else if (node -> type != XML_ELEMENT_NODE) {
        return parse_xml_to_ctl(xmlNextElementSibling(node));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "transition") == 0) {
        //TODO what to do here? I don't think this case is needed.
        return NULL;
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "is-fireable") == 0) {
        xmlNode *transitionNode = xmlFirstElementChild(node);

        while (transitionNode != NULL) {
            //TODO what to do with each transition?
            //TODO build up result.

            transitionNode = xmlNextElementSibling(transitionNode);
        }

        return NULL; //TODO return result
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "negation") == 0) {
        return negate(parse_xml_to_ctl(xmlFirstElementChild(node)));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "conjunction") == 0) {
        xmlNode *first = xmlFirstElementChild(node);
        xmlNode *second = xmlNextElementSibling(first);
        return conjunction(parse_xml_to_ctl(first), parse_xml_to_ctl(second));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "disjunction") == 0) {
        xmlNode *first = xmlFirstElementChild(node);
        xmlNode *second = xmlNextElementSibling(first);
        return disjunction(parse_xml_to_ctl(first), parse_xml_to_ctl(second));
    }
    else if (xmlStrcmp(node->name, (const xmlChar*) "all-paths") == 0) {
        //ForAll cases
        xmlNode *child = xmlFirstElementChild(node);

        if (xmlStrcmp(child->name, (const xmlChar*) "globally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AG(parse_xml_to_ctl(grandChild));
        }else if (xmlStrcmp(child->name, (const xmlChar*) "finally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AF(parse_xml_to_ctl(grandChild));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "next") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_AX(parse_xml_to_ctl(grandChild));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "until") == 0) {
            //parse Before and parse Reach
            xmlNode *beforeChild    = xmlFirstElementChild(child);
            xmlNode *reachChild     = xmlNextElementSibling(beforeChild);
            return ctl_make_AU(parse_xml_to_ctl(beforeChild), parse_xml_to_ctl(reachChild));
        }
    } //endif ForAll
    else if (xmlStrcmp(node->name, (const xmlChar*) "exists-path") == 0) {
        //Exists cases
        xmlNode *child = xmlFirstElementChild(node);

        if (xmlStrcmp(child->name, (const xmlChar*) "globally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EG(parse_xml_to_ctl(grandChild));
        }else if (xmlStrcmp(child->name, (const xmlChar*) "finally") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EF(parse_xml_to_ctl(grandChild));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "next") == 0) {
            xmlNode *grandChild = xmlFirstElementChild(child);
            return ctl_make_EX(parse_xml_to_ctl(grandChild));
        } else if (xmlStrcmp(child->name, (const xmlChar*) "until") == 0) {
            //parse Before and parse Reach
            xmlNode *beforeChild    = xmlFirstElementChild(child);
            xmlNode *reachChild     = xmlNextElementSibling(beforeChild);
            return ctl_make_EU(parse_xml_to_ctl(beforeChild), parse_xml_to_ctl(reachChild));
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

/**
 * \brief recursively parse the given XML node.
 */
static int
parse_xml(xmlNode *node)
{
    int res = 0;
    // first check if the node is not a NULL pointer.
    if (node == NULL) {
        res = 1;
        warn("Invalid XML");
    // only parse xml nodes, skip other parts of the XML file.
    } else if (node->type != XML_ELEMENT_NODE) res = parse_xml(xmlNextElementSibling(node));
    // parse property-set
    else if (xmlStrcmp(node->name, (const xmlChar*) "property-set") == 0) {
        // loop over all children that are property nodes
        for (xmlNode *property = xmlFirstElementChild(node);
                property != NULL && !res;
                property = xmlNextElementSibling(property)) {
            res = parse_xml(property);
        }
    // parse property
    } else if (xmlStrcmp(node->name, (const xmlChar*) "property") == 0) {
        warn("parsing property");
        res = parse_xml(xmlFirstElementChild(node));
    // parse id of property
    } else if (xmlStrcmp(node->name, (const xmlChar*) "id") == 0) {
        warn("Property id is: %s", xmlNodeGetContent(node));
        res = parse_xml(xmlNextElementSibling(node));
    // parse description of property
    } else if (xmlStrcmp(node->name, (const xmlChar*) "description") == 0) {
        warn("Property description is: %s", xmlNodeGetContent(node));
        res = parse_xml(xmlNextElementSibling(node));
    // parse the formula
    } else if (xmlStrcmp(node->name, (const xmlChar*) "formula") == 0) {
        warn("Parsing formula...");
        res = parse_formula(xmlFirstElementChild(node));
        printf("\n");
    // node not recognized
    } else {
        res = 1;
        warn("Invalid xml node '%s'", node->name);
    }

    return res;
}

/**
 * \brief parses the XML file name.
 *
 * \returns 0 on success, 1 on failure.
 */
static int
load_xml(const char* name)
{
    int res;

    LIBXML_TEST_VERSION
    warn("parsing formulas file: %s", name);
    xmlDoc *doc = xmlReadFile(name, NULL, 0);
    if (doc == NULL) res = 1;
    else {
        xmlNode *node = xmlDocGetRootElement(doc);
        res = parse_xml(node);
    }

    return res;
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
            warn("Successful parse of file '%s' :)", name);
            if (argc == 3) {
                const char *formulas = argv[2];
                res = load_xml(formulas);
                if (res) warn("Unable to load xml '%s'", formulas);
            }
            init_sylvan();
            // execute the main body of code
            do_ss_things(&andl_context);
            deinit_sylvan();
        }
    } else {
        warn("Usage: %s <petri-net>.andl [<CTL-formulas>.xml]", argv[0]);
        res = 1;
    }

    return res;
}

