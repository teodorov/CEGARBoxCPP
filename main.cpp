#include "Bitset/Bitset.h"
#include "Clausifier/Trieform/Trieform.h"
#include "Clausifier/TrieformFactory/TrieformFactory.h"
#include "Formula/And/And.h"
#include "Formula/Atom/Atom.h"
#include "Formula/Box/Box.h"
#include "Formula/Diamond/Diamond.h"
#include "Formula/FEnum/FEnum.h"
#include "Formula/False/False.h"
#include "Formula/Formula/Formula.h"
#include "Formula/Not/Not.h"
#include "Formula/Or/Or.h"
#include "Formula/True/True.h"
#include "ParseFormula/ParseFormula.h"
#include "ParseFormulaNew/ParseFormulaNew.h"
#include <argp.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <minisat/core/Solver.h>
#include <string>
#include <unordered_set>

using namespace std;

const char *argp_program_version = "CEGARBox 0.1.0";
const char *argp_program_bug_address = "robert.mcarthur@anu.edu.au";
static char doc[] = "An efficient theorem prover for modal logic.";
static char args_doc[] = "";
static struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "File containing input formula."},
    {"reflexive", 't', 0, 0, "Enables reflexivity."},
    {"symmetric", 'b', 0, 0, "Enables symmetry."},
    {"transitive", '4', 0, 0, "Enables transitivity."},
    {"serial", 'd', 0, 0, "Enables seriality."},
    {"euclidean", 'e', 0, 0, "Enables transitivity."},
    {"valid", 'v', 0, 0, "Prove validity"},
    {0, 0, 0, 0, 0, 0}};

struct arguments_struct {
  string filename = "file.p";
  SolverConstraints settings;
  bool valid = false;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  arguments_struct *arguments = static_cast<arguments_struct *>(state->input);
  switch (key) {
  case 'f': {
    arguments->filename = arg;
  } break;
  case 't':
    arguments->settings.reflexive = true;
    break;
  case 'b':
    arguments->settings.symmetric = true;
    break;
  case '4':
    arguments->settings.transitive = true;
    break;
  case 'd':
    arguments->settings.serial = true;
    break;
  case 'e':
    arguments->settings.euclidean = true;
    break;
  case 'v':
    arguments->valid = true;
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void solve(arguments_struct &args);

int main(int argc, char *argv[]) {
  arguments_struct arguments;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  solve(arguments);
}

void solve(arguments_struct &args) {
#if DEBUG_TIME
  auto start = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Begin" << endl;
#endif

#if DEBUG_TIME
  auto read = chrono::steady_clock::now();
#endif

  shared_ptr<Formula> formula = ParseFormula(&args.filename).parseFormula();
  // string other = "a.p";
  // shared_ptr<Formula> correct = ParseFormula(&other).parseFormula();
  // cout << "Wrong" << formula->toString() << endl;
  // cout << "Right" << correct->toString() << endl;
  // cout << (*formula == *correct) << endl;

  if (args.valid) {
    formula = Not::create(formula);
  }

#if DEBUG_TIME
  auto parse = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Parsed: " << formula->toString() << endl;
#endif

  formula = formula->negatedNormalForm();
  // correct = correct->negatedNormalForm();

  // cout << (*formula == *correct) << endl;

#if DEBUG_TIME
  auto nnf = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Negated normal form: " << formula->toString() << endl;
#endif

  formula = formula->simplify();
  // correct = correct->simplify();

  // cout << (*formula == *correct) << endl;

#if DEBUG_TIME
  auto simplify = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Simplified: " << formula->toString() << endl;
#endif

  formula = formula->modalFlatten();
  // correct = correct->modalFlatten();

  // cout << (*formula == *correct) << endl;

#if DEBUG_TIME
  auto flatten = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Flattenned: " << formula->toString() << endl;
#endif

  shared_ptr<Trieform> trie = TrieformFactory::makeTrie(formula, args.settings);
  // shared_ptr<Trieform> otherTrie =
  //     TrieformFactory::makeTrie(correct, args.settings);
#if DEBUG_PROGRESS
  cout << "Constructed trie" << endl;
#endif
#if DEBUG_INITIAL_TRIE
  cout << "Initial trie:" << endl << trie->toString() << endl;
#endif
#if DEBUG_NORMAL_CACHE
  cout << "Normal cache:" << endl << trie->getCache().toString() << endl;
#endif
#if DEBUG_TIME
  auto construct = chrono::steady_clock::now();
#endif
  // cout << "Initial trie:" << endl << trie->toString() << endl;
  // cout << "Correct trie:" << endl << otherTrie->toString() << endl;

  trie->reduceClauses();
  // otherTrie->reduceClauses();

#if DEBUG_TIME
  auto reduce = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Reduced trie" << endl;
#endif
#if DEBUG_REDUCED_TRIE
  cout << "Reduced trie:" << endl << trie->toString() << endl;
#endif
#if DEBUG_REDUCED_CACHE
  cout << "Reduced cache:" << endl << trie->getCache().toString() << endl;
#endif

  trie->preprocess();
  // cout << "Processed trie:" << endl << trie->toString() << endl;
  // otherTrie->preprocess();

#if DEBUG_PROGRESS
  cout << "Preprocessed trie" << endl;
#endif
#if DEBUG_PROCESSED_TRIE
  cout << "Processed trie:" << endl << trie->toString() << endl;
#endif

  trie->removeTrueAndFalse();
  // otherTrie->removeTrueAndFalse();

  trie->prepareSAT();
  // otherTrie->prepareSAT();

#if DEBUG_TIME
  auto prepare = chrono::steady_clock::now();
#endif
#if DEBUG_PROGRESS
  cout << "Prepared SAT" << endl;
#endif

  bool satisfiable = trie->isSatisfiable();
  if (args.valid) {
    cout << (satisfiable ? "Invalid" : "Valid") << endl;
  } else {
    cout << (satisfiable ? "Satisfiable" : "Unsatisfiable") << endl;
  }
  // satisfiable = otherTrie->isSatisfiable();
  // if (args.valid) {
  //   cout << (satisfiable ? "Invalid" : "Valid") << endl;
  // } else {
  //   cout << (satisfiable ? "Satisfiable" : "Unsatisfiable") << endl;
  // }

#if DEBUG_TIME
  auto solve = chrono::steady_clock::now();
#endif
#if DEBUG_PROGESS
  cout << "Solved" << endl;
#endif

#if DEBUG_TIME
  auto readTime = read - start;
  auto parseTime = parse - start;
  auto nnfTime = nnf - start;
  auto simplifyTime = simplify - start;
  auto flattenTime = flatten - start;
  auto constructTime = construct - start;
  auto reduceTime = reduce - start;
  auto prepareTime = prepare - start;
  auto solveTime = solve - start;
  cout << "READ TIME: " << chrono::duration<double, milli>(readTime).count()
       << " ms" << endl;
  cout << "PARSE TIME: " << chrono::duration<double, milli>(parseTime).count()
       << " ms" << endl;
  cout << "NNF TIME: " << chrono::duration<double, milli>(nnfTime).count()
       << " ms" << endl;
  cout << "SIMPLIFY TIME: "
       << chrono::duration<double, milli>(simplifyTime).count() << " ms"
       << endl;
  cout << "FLATTEN TIME: "
       << chrono::duration<double, milli>(flattenTime).count() << " ms" << endl;
  cout << "CONSTRUCT TIME: "
       << chrono::duration<double, milli>(constructTime).count() << " ms"
       << endl;
  cout << "REDUCE TIME: " << chrono::duration<double, milli>(reduceTime).count()
       << " ms" << endl;
  cout << "PREPARE TIME: "
       << chrono::duration<double, milli>(prepareTime).count() << " ms" << endl;
  cout << "SOLVE TIME: " << chrono::duration<double, milli>(solveTime).count()
       << " ms" << endl;
#endif
}