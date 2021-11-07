#ifndef PROVER_H
#define PROVER_H

#include "../../Clausifier/FormulaTriple/FormulaTriple.h"
#include "../Literal/Literal.h"
#include <chrono>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct SolverMemoKeyHasher {
  hash<string> stringHasher;
  size_t operator()(const literal_set &assumptions) const {
    int hash = assumptions.size();
    for (const Literal &literal : assumptions) {
      hash += literal.hash();
    }
    return hash;
  }
};

struct SolverMemoKeyEqual {
  size_t operator()(literal_set const &set1, literal_set const &set2) const {
    if (set1.size() != set2.size())
      return false;
    for (const Literal &literal : set1) {
      if (set2.find(literal) == set1.end())
        return false;
    }
    return true;
  }
};

struct DiamondFail {
  Literal literal;
  unsigned int lastFail;
};
struct DiamondFailCompare {
  bool operator()(DiamondFail const &left, DiamondFail const &right) {
    return left.lastFail < right.lastFail;
  }
};

typedef priority_queue<DiamondFail, vector<DiamondFail>, DiamondFailCompare>
    diamond_queue;

class Prover {
private:
  unsigned int failCount = 0;
  unordered_map<Literal, unsigned int, LiteralHash, LiteralEqual> lastFail;

  unsigned int assumptionsSize;

  modal_literal_map triggeredBoxes;
  modal_literal_map triggeredDiamonds;

  void calculateTriggeredModalClauses(modal_lit_implication &modal_lits,
                                      modal_literal_map &triggered);
  modal_literal_map getTriggeredModalClauses(modal_lit_implication &modalLits);
  literal_set getNotModalTriggerers(Literal right, int modality,
                                    modal_lit_implication &modalFromRight);

protected:
  modal_lit_implication boxLits;
  modal_lit_implication diamondLits;
  modal_lit_implication boxFromRight;
  modal_lit_implication diamondFromRight;

  void createModalImplication(int modality, Literal left, Literal right,
                              modal_lit_implication &modalLits,
                              modal_lit_implication &modalFromRight);

  Literal toLiteral(shared_ptr<Formula> formula);

  virtual void prepareFalse() = 0;
  virtual void prepareExtras(name_set extra) = 0;
  virtual void prepareClauses(clause_set clauses) = 0;
  virtual void prepareModalClauses(modal_clause_set modal_clauses,
                                   modal_names_map &newExtra,
                                   modal_lit_implication &modalLits,
                                   modal_lit_implication &modalFromRight) = 0;

  virtual int getLiteralId(Literal literal) = 0;

public:
  Prover();
  ~Prover();

  string getPrimitiveName(shared_ptr<Formula> formula);

  void calculateTriggeredBoxClauses();
  void calculateTriggeredDiamondsClauses();

  modal_literal_map getTriggeredBoxClauses();
  modal_literal_map getTriggeredDiamondClauses();

  literal_set getNotDiamondLeft(int modality, Literal diamond);
  literal_set getNotAllDiamondLeft(int modality);
  vector<literal_set> getNotProblemBoxClauses(int modality,
                                              literal_set conflicts);

  literal_set getNotBoxTriggerers(Literal right, int modality);
  literal_set getNotDiamondTriggerers(Literal right, int modality);

  diamond_queue getPrioritisedTriggeredDiamonds(
      int modality); // NOTE ENSURE THIS AVOIDS BOXES

  void calculateTriggeredModalClauses();

  void updateLastFail(Literal diamondRight);

  virtual void printModel() = 0;

  virtual modal_names_map prepareSAT(FormulaTriple clauses,
                                     name_set extra = name_set()) = 0;
  virtual Solution solve(const literal_set &assumptions = literal_set()) = 0;
  virtual void addClause(literal_set clause) = 0;

  virtual bool modelSatisfiesAssump(Literal assumption) = 0;
  bool modelSatisfiesAssumps(literal_set assumptions);
};

#endif