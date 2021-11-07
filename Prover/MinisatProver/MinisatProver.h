#ifndef MINISAT_PROVER_H
#define MINISAT_PROVER_H

#include "../../Clausifier/FormulaTriple/FormulaTriple.h"
#include "../../Formula/Atom/Atom.h"
#include "../../Formula/FEnum/FEnum.h"
#include "../../Formula/Not/Not.h"
#include "../../Formula/Or/Or.h"
#include "../Prover/Prover.h"
#include <exception>
#include <memory>
// #include <minisat/core/Solver.h>
#include <minisat/simp/SimpSolver.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class MinisatProver : public Prover {
private:
  shared_ptr<Minisat::SimpSolver> solver =
      shared_ptr<Minisat::SimpSolver>(new Minisat::SimpSolver());

  unordered_map<string, Minisat::Var> variableMap;
  unordered_map<Minisat::Var, string> nameMap;

  void prepareFalse();
  void prepareExtras(unordered_set<string> extra);
  void prepareClauses(clause_set clauses);
  void prepareModalClauses(modal_clause_set modal_clauses,
                           modal_names_map &newExtra,
                           modal_lit_implication &modalLits,
                           modal_lit_implication &modalFromRight);

  Minisat::Var
  createOrGetVariable(string name,
                      Minisat::lbool polarity = Minisat::lbool((uint8_t)2));
  Minisat::Lit makeLiteral(shared_ptr<Formula> formula);

  shared_ptr<Minisat::vec<Minisat::Lit>>
  convertAssumptions(literal_set assumptions);
  literal_set
  convertConflictToAssumps(Minisat::vec<Minisat::Lit> &conflictLits);

  bool modelSatisfiesAssump(Literal assumption);

  virtual int getLiteralId(Literal literal);

public:
  MinisatProver();
  ~MinisatProver();

  modal_names_map prepareSAT(FormulaTriple clauses,
                             name_set extra = name_set());
  Solution solve(const literal_set &assumptions = literal_set());
  void addClause(literal_set clause);

  void printModel();
};

#endif