/* The ANDL grammer, found in:
@TECHREPORT { SRH16,
    AUTHOR = { M Schwarick and C Rohr and M Heiner },
    TITLE = { {MARCIE Manual} },
    INSTITUTION = { Brandenburg University of Technology Cottbus, Department of Computer Science },
    YEAR = { 2016 },
    NUMBER = { 02-16 },
    MONTH = { December },
    URL = { https://opus4.kobv.de/opus4-btu/frontdoor/index/index/docId/4056 },
    PDF = { http://www-dssz.informatik.tu-cottbus.de/track/download.php?id=187 },
}
*/
%pure-parser
%locations
%defines
%lex-param { void *scanner }
%parse-param { void *scanner} { andl_context_t *andl_context }
%define parse.error verbose
%define api.prefix {andl_}
%code requires {
#include <config.h>
#include <stdio.h>
#include <andl.h>
#include <string.h>
#ifdef YYDEBUG
#undef YYDEBUG
#define YYDEBUG 1
#endif
}
%code{
#include <util.h>
#include <andl-lexer.h>

    void yy_fatal_error(yyconst char* msg , yyscan_t yyscanner) {
        warn("Fatal error: %s", msg);
        (void) yyscanner;
    }

    void yyerror(YYLTYPE *loc, void *scanner, andl_context_t *andl_context, const char* c) {
        warn("Parse error on line %d: %s", loc->first_line, c);
        andl_context->error = 1;
        (void) scanner;
    }
}

%union {
    char *text;
    int number;
    arc_dir_t dir;
}

%token PN
%token <text> IDENT
%token LBRAC
%token RBRAC
%token LCURLY
%token RCURLY
%token COLON
%token CONSTANTS
%token PLACES
%token DISCRETE
%token SEMICOLON
%token ASSIGN
%token <number> NUMBER
%token <dir> PLUS
%token <dir> MIN
%token AMP
%token TRANSITIONS

%type <dir> op
%type <number> const_function

%%

/* top rule, parse the Petri net declaration. */
pn
    :   PN LBRAC IDENT RBRAC LCURLY items RCURLY {
            andl_context->name = strdup($3);
            if (andl_context->name == NULL) {
                warn("out of memory");
                YYABORT;
            }
            free($3);
        }
    ;

items
    :   item
    |   items item
    ;

item
    :   constants
    |   places
    |   discrete
    |   transitions
    ;

constants
    :   CONSTANTS COLON
    ;

places
    :   PLACES COLON
    ;

discrete
    :   DISCRETE COLON pdecs
    ;

transitions
    :   TRANSITIONS COLON tdecs
    ;

pdecs
    :   pdecs pdec
    |   /* empty */
    ;

/* single place declaration */
pdec
    :   IDENT ASSIGN NUMBER SEMICOLON {
            place_t p;
            p.name = strdup($1);
            p.identifier = andl_context->num_places;
            p.initial_marking = $3;

            if (andl_context->num_places >= andl_context->place_buf_size - 1) {
                andl_context->place_buf_size *= 2;
                andl_context->places = realloc(andl_context->places, andl_context->place_buf_size * sizeof(place_t));
            }

            andl_context->places[andl_context->num_places] = p;
            andl_context->num_places++;
            free($1);
        }
    |   IDENT error SEMICOLON {
            warn("Something went wrong with place %s on line %d", $1, @1.first_line);
            andl_context->error = 1;
        }
    ;

tdecs
    :   tdecs tdec
    |   /* empty */
    ;

/* single transition declaration */
tdec
    :   IDENT COLON conditions COLON {
            /* free the string of the previous transition */
            if (andl_context->current_trans != NULL) {
                free(andl_context->current_trans);
            }
            /* copy the name of the current transition */
            andl_context->current_trans = strdup($1);
            if (andl_context->current_trans == NULL) {
                warn("out of memory");
                YYABORT;
            }

            transition_t transition;
            transition.name = strdup($1);
            transition.num_arcs = 0;
            transition.arcs_buf_size = 16;
            transition.arcs = malloc(transition.arcs_buf_size * sizeof(arc_t));

            if (andl_context->num_transitions >= andl_context->transition_buf_size - 1) {
                andl_context->transition_buf_size *= 2;
                andl_context->transitions = realloc(andl_context->transitions, andl_context->transition_buf_size * sizeof(transition_t));
            }

            //printf("transition %s, num trans: %d, buf size: %d\n", transition.name, andl_context->num_transitions, andl_context->transition_buf_size);

            andl_context->transitions[andl_context->num_transitions] = transition;
            andl_context->num_transitions++;

            free($1);
        } arcs transition_function SEMICOLON
    |   IDENT error SEMICOLON {
            warn("Something went wrong with transition %s on line %d", $1, @1.first_line);
            andl_context->error = 1;
        }
    ;

conditions
    :   /* empty */
    ;

arcs
    :   arc
    |   arcs AMP arc
    |   /* empty */
    ;

/* parse a single arc */
arc
    :   LBRAC IDENT op const_function RBRAC {
            if ($4 != 1) {
                warn(
                    "Not a 1-safe net."
                    " Petri net should always produce, or consume 1 token,"
                    " Instead, was: %d.",
                    $4);
                andl_context->error = 1;
            }
            /* Here you can do something with
             * andl_context->current_trans */
            if ($3 == ARC_IN) {
                andl_context->num_in_arcs++;
            } else { // $3 == ARC_OUT
                andl_context->num_out_arcs++;
            }

            // construct arc struct
            arc_t arc;
            arc.dir = $3;

            // find place
            int found = 0;
            for (int i = 0; i < andl_context->num_places && !found; i++) {
                char *name = andl_context->places[i].name;

                if(strcmp($2, name) == 0) {
                    arc.place = andl_context->places + i;
                    found = 1;
                }
            }

            if (!found) {
                warn("Unknown place identifier \"%s\" on line %d", $2, @1.first_line);
                andl_context->error = 1;
            }

            // add arc to transition
            int cur = andl_context->num_transitions;
            transition_t *cur_trans = andl_context->transitions + andl_context->num_transitions - 1;

            if (cur_trans->num_arcs >= cur_trans->arcs_buf_size - 1) {
                cur_trans->arcs_buf_size *= 2;
                cur_trans->arcs = realloc(cur_trans->arcs, cur_trans->arcs_buf_size * sizeof(transition_t));
            }


            //printf("arc, transition: %s, num arcs: %d, buf size: %d\n", cur_trans->name, cur_trans->num_arcs, cur_trans->arcs_buf_size);

            //printf("%s (#%d)\n", (andl_context->transitions + 0)->name, andl_context->num_transitions);

            cur_trans->arcs[cur_trans->num_arcs] = arc;
            cur_trans->num_arcs++;

           free($2);
        }
    |   LBRAC error RBRAC {
            warn("Missing identifier on line %d", @1.first_line);
            andl_context->error = 1;
        }
    |   LBRAC IDENT error RBRAC {
            warn("Something went wrong with arc %s on line %d", $2, @1.first_line);
            andl_context->error = 1;
        }
    ;

op
    :   PLUS
    |   MIN
    |   ASSIGN {
            warn("Constant assign arcs not yet supported.");
            andl_context->error = 1;
        }
    ;

const_function
    :   NUMBER {
            if ($1 != 1) {
                warn("This is not a 1-safe Petri net!");
                andl_context->error = 1;
            }
        }
    ;

transition_function
    :   /* empty */
    ;

%%
