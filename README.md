# Software Science Project 1, assignment 2

### Running the model checker

`./run_smc.bash <directory>` e.g. `./run_smc.bash examples`

#### Code layout

- ss.c contains the main function as well as parsing the xml-encoded formulas.
- ctl.h defines the ctl abstract syntax tree.
- ctl.c contains implementations for normalizing ctl formulas.
- smc.h models a kripke structure using BDDs.
- smc.c contains implementations for checking ctl formulas on the model.

## Introduction
This repository contains all files necessary for the Software Science project 1, assignment 2.
This README will first explain how to configure this autotools project
on your machine. Then it will explain how to compile the project.
When successful, you should be able to load a Petri net,
print the size of the Petri net specification, and parse a CTL formula specified in XML.
Do not hesitate to send me an e-mail at j.j.g.meijer [at] utwente [dot] nl,
if you at any step below run into an issue. Furthermore, there is also a
[Travis CI build script](https://github.com/Meijuh/SS/blob/master/.travis.yml).
which may help guide you in your journey described in the next sections 
(Build statusses, with expected output can be found e.g. 
[here](https://travis-ci.org/Meijuh/SS)). Completing exercises 3 (symbolic state space generation for Petri nets) and 4 (CTL model checking for Petri nets) is mandatory, doing exercise 5 will give you bonus points.

## Configuring
To configure this autotools project on your machine, make sure
to meet the following requirements:
 *  Ubuntu 64-bit >= 14.04, (or any other Linux distro with similar
    packages). If you do not have Linux installed, easiest is to download 
    a Virtual box virtual machine from http://www.osboxes.org/ubuntu/.
    Furthermore, any recent enough OSX version also works for this assignment.
 *  This project depends on the following packages:
    * A C compiler, preferably GCC, or Clang:
    * GNU automake: 
    * GNU autoconf:
    * GNU libtool:
    * flex:
    * bison:
    * pkgconf:
    * GNU make:
    * libnuma: http://oss.sgi.com/projects/libnuma/
    * cmake (>= 3.0)
    * gmplib
    * hwloc
    * libxml2

The easiest way to install these packages (on Ubuntu) is by running:
 * `apt-get install build-essential bison flex automake autoconf pkgconf
   libnuma-dev libhwloc-dev libgmp-dev libxml2-dev`.

If you are using Ubuntu 14.04 you need to add an extra repository to
get a recent enough version of `cmake` do:
 * `add-apt-repository ppa:george-edison55/cmake-3.x`
 * `apt-get update`
 * `apt-get install cmake`    

## Building
We first compile and install the dependency Sylvan, next we will compile this
project.

### Sylvan
 * Download the Sylvan 1.2.0 release from 
    https://github.com/utwente-fmt/sylvan/releases: 
    `wget https://github.com/utwente-fmt/sylvan/archive/v1.2.0.tar.gz`
 * Extract: `tar xf v1.2.0.tar.gz`
 * Enter Sylvan directory: `cd sylvan-1.2.0`
 * Create build directory: `mkdir build`
 * Enter build dir: `cd build`
 * Configure Sylvan: `cmake .. -DBUILD_SHARED_LIBS=OFF  
    -DSYLVAN_BUILD_EXAMPLES=OFF -DSYLVAN_BUILD_DOCS=OFF`
 * Compile Sylvan: `make`
 * Install Sylvan: `make install`

### SS lab files
 * Clone this git repository: `git clone https://github.com/Meijuh/SS.git`
 * Enter the git repository: `cd SS`
 * Generate config files: `./ssreconf`
 * Configure lab class: `./configure`
 * Compile the lab files: `make`
 * Parse a simple Petri net in ANDL format: `src/ss examples/model.andl`,
    if successful you should see something similar to:        

        2017-05-01 16:38:45: Successful parse of file 'examples/model.andl' :)         
        2017-05-01 16:38:45: The name of the Petri net is: Philosophers_PT_000005
        2017-05-01 16:38:45: There are 25 transitions
        2017-05-01 16:38:45: There are 25 places
        2017-05-01 16:38:45: There are 45 in arcs
        2017-05-01 16:38:45: There are 35 out arcs

## Exercise 1
Read Sylvan's documentation at: https://trolando.github.io/sylvan/.

## Exercise 2
Familiarize yourself with
[Petri nets](https://en.wikipedia.org/wiki/Petri_net).
In this course, many example Petri nets are provided as
[PNML](http://www.pnml.org/), and 
[ANDL](http://www-dssz.informatik.tu-cottbus.de/track/download.php?id=187).
The latter URL contains a document containing an EBNF grammar for
Petri net specifications, it is only for reference, you do not need to read it 
now. 

## Exercise 3
The first programming exercise is to compute the marking graph symbolically,
this means computing the state space with BDDs. The main idea is to use Sylvan's
[sylvan_satcount](https://github.com/utwente-fmt/sylvan/blob/v1.2.0/src/sylvan_bdd.h#L161)
function, to compute the number of markings, and
[sylvan_relnext](https://github.com/utwente-fmt/sylvan/blob/v1.2.0/src/sylvan_bdd.h#L111)
to compute the next marking of a particular transition.
It is advised to "consult"
[this file](https://github.com/utwente-fmt/sylvan/blob/v1.2.0/examples/mc.c),
which contains code for computing a state space.

The lab files in this repository already provide three important aspects:
 * A lexer for ANDL: https://github.com/Meijuh/SS/blob/master/src/andl-lexer.l
 * A parser for ANDL: https://github.com/Meijuh/SS/blob/master/src/andl-parser.y
 * Some borderplate code for printing the size of the Petri net specification:
   https://github.com/Meijuh/SS/blob/master/src/ss.c.
 * Some borderplate code for parsing CTL formulae in XML format (for the next exercise).

For SS we think it is much easer to extend the ANDL parser, than it is to
create a PNML parser. Feel free to create a PNML parser though.

On Blackboard, under `Course Materials | Project 1: Symbolic Model Checking | Model Checking Petri Nets (Week 2-4)` many
example Petri nets have been made available in an archive. Every directory
in the archive contains a Petri net specification. Important files for
this exercise in each directory are:
 * `1-safe`: if this file exists the Petri net is 1-safe.
 * `model.pnml`: the Petri net specified in PNML, which is not irrelevant if
   you use the ANDL parser.
 * `model.andl`: the Petri net specified in ANDL.

Whenever you have finished writing your symbolic state space generator you
can consult
[raw-results-analysis.csv](https://github.com/Meijuh/SS/blob/master/MCC/raw-result-analysis.csv).
This CSV file, contains all known answers from last year's
[Petri net Model Checking Contest](http://mcc.lip6.fr/), more specifically the 
column:
 * Input: the name of the Petri net. **Note** the Petri nets with `-COL-`,
   are irrelevant for this course.
 * estimated result: contains the known answer for Petri nets.
 * Examination: the category to which the answer belongs.

Relevant for this exercise is the `StateSpace`, and `Examination` column.
For example, if we want to know the size of the marking graph of a Petri net
specification with 5 dining philosophers, we look at:
 * Input = Philosophers-PT-000005,
 * Examination = StateSpace,

This shows *243 945 1 10* in the column *estimated result*, meaning that
 1. the marking graph has 243 vertices,
 1. the marking graph has 945 edges,
 1. the maximum number of tokens that any place can have is 1 (hence
    it is 1-safe),
 1. the maximum sum of tokens never exceeds 10 tokens.

So, if you want to verify if the answer given by `sylvan_satcount` is correct,
you are interested in the first value (243).

The exercise is now as follows.
 1. Think about what data structures you require while parsing the
    ANDL files, e.g. what do you need to map names of places to BDD variables,
    how are you going to store the initial marking, or how are you going to encode
    the transition relations?
 1. Write/download whatever code for these datastructures is necessary, and
    declare those data structures in
    [`andl_context_t`](https://github.com/Meijuh/SS/blob/master/src/andl.h),
    feel free to change any existing code there.
    The `andl_context_t` is a structure that is available while parsing ANDL files.
 1. Modify the [parser](https://github.com/Meijuh/SS/blob/master/src/andl-parser.y)
    to fill the data structures accordingly.
 1. Implement a *Breadth-first* symbolic state space generator, starting
    [here](https://github.com/Meijuh/SS/blob/master/src/ss.c#L91).
 1. Make sure the return value of `sylvan_satcount` corresponds with the known
    answers in `raw-result-analysis.csv`, for as many Petri nets as possible
    (which you downloaded from Blackboard).

## Exercise 4
In this exercise we are going to build a CTL model checker, where atomic
predicates are expressions for the fireability of Petri net transitions.
The Petri net model checking contest specifies these kind of formulas in files
named CTLFireability.[xml|txt]. The exact syntax and semantics is specified
in [MCC2016-FormulaManual.pdf](https://github.com/Meijuh/SS/blob/master/MCC2016-FormulaManual.pdf),
located in the root directory of this git repository. The syntax and semantics 
are very much the same as in the lectures. However, whenever there is a
predicate `is-fireable(t1,t2,)`, this actually means a disjunction; 
`t1`, or `t2` should be fireable.

Example code for parsing temporal logic formulas can be found here:
https://github.com/Meijuh/SS/blob/master/src/ss.c#L107-L230. The current
implementation (specifically function `parse_formula`) is an in-order traversal
over the XML nodes, that simply prints the formula. The `parse_xml` function
implements a bit of bootstrapping for printing the actual formulas, indeed
there are multiple formulas in each CTLFireability.xml file.

You can parse an XML file e.g. as follows:
`src/ss examples/model.andl examples/CTLFireability.xml`.

To get started with the CTL model checker it is probably easiest to change the
in-order traversal to a post-order traversal and to create a simple parse tree.

Use this simple parse tree to implement a CTL model checker such as described
in the lecture slides.

Notes:

1. whenever you change the XML parser, never access fields `next`, `content`,
`children` of type `xmlNode*` directly. Use their respective accessors
`xmlNextElementSibling`, `xmlNodeGetContent`, and `xmlFirstElementChild`.
1. You are at this moment only required to support formulas in the CTLFireability
category. Other formulas such as CTLCardinality have different atomic predicates, you
do not need to support.
1. As mentioned; the CTL formulas are also available in a textual (.txt)
   format, feel free to generate a parser based on an BNF grammar, if that
   is easier.
1. When you need to transform a CTL formula to some normal form, make sure not to edit the parse tree in-place. Instead, construct an entire new tree, and then delete/free the old parse tree.
1. Known answers to the CTL formulae can also be found in 
[raw-results-analysis.csv](https://github.com/Meijuh/SS/blob/master/MCC/raw-result-analysis.csv), select `Examination=CTLFireability`. **Note** the answers `T`(=true), `F`(=false), and `?`(=unknown) are not given in numerical order, instead the order in which the answers are provided is: 0, 1, 10, 11, 12, 13, 14, 15, 2, 3, 4, 5, 6, 7, 8, 9. 
1. The Model Checking Contest uses different semantics for Petri nets that have deadlocks than we do in this assignment. You only need to evaluate CTL formulae for Petri nets that do not have deadlocks. Whether a Petri net has a deadlock can be found if you select `Examination=ReachabilityDeadlock`. Here `T` means the Petri net does have dead locks.
1. In some ANDL files the transition names do not correspond with those in CTLFireability.xml; you do not have to model check those Petri nets. An example of such a Petri net you may skip is `NeoElection-PT-2`, here transition `T-startSec_1` in `CTLFireability.xml` can not be matched in `model.andl`.

## Exercise 5
In this exercise you can choose what kind of feature you want to extend your
model checker with. You can of course implement more of the following.

1. Support for Cardinality predicates (as specified in `MCC2016-FormulaManual.pdf`).
1. LTL model checking with support for LTLFireability.xml.
1. Static variable ordering.
1. Deadlock detection.
1. Advanced reachability algorithms, such as chaining, or saturation.
1. Unsafe Petri nets.
1. Multi-valued Decision Diagrams (e.g. with sylvan-ldd.h)
1. Any other MCC category (as specified in `MCC2016-FormulaManual.pdf`).
1. CTL + fairness
1. CTL + counter examples

## Common Pitfalls and hints
 1. If you have protected pointers to BDDs (with `sylvan_protect`) make sure to
    unprotect (with `sylvan_unprotect`) those before closing the
    variable scope. The pointers will become invalid after the scope is closed!
 1. The default LACE deque size may be too small, even for smaller Petri nets.
    If you get unexpected segfaults, try increasing the deque size, e.g.
    `lace_init(n_workers, 40960000)`. I suggest changing this value anyway.
 1. The easiest way to declare a set of BDD variables is using functions like
    `sylvan_set_empty()`, and `sylvan_set_add()`. Do not forget to protect
    the set.
 1. The easiest way to declare a map for variable renaming use functions like
    `sylvan_map_empty()`, and `sylvan_map_add()`. Do not forget to protect
    the map. Variable renaming is not necessary when you use `sylvan_relnext`.
 1. Whenever you constructed a BDD, you can visualize it with Graphviz, e.g.:

        {
             BDD bdd; // some BDD
             int i = 0;
             char b[256];
             snprintf(b, 256, "/tmp/sylvan/BDD-%d.dot", i);
             FILE *f = fopen(b, "w+");
             sylvan_fprintdot(f, bdd);
             fclose(f);
        }
    This allows you to make sure you constructed the correct BDD. A 
    `.dot` file can be visualized with the program `xdot`, which 
    should be in the Ubuntu repositories.
 1. Whenever you need a hashmap while parsing an ANDL file; I have found that
    [this one](https://github.com/petewarden/c_hashmap) is quite suitable.
    To include it in a binary, e.g. the `ss` binary, add the following line
    to `src/Makefile.am`: `ss_SOURCES   +=      hashmap.h hashmap.c`, and
    add `hashmap.h`, and `hashmap.c` to the `src` directory.

