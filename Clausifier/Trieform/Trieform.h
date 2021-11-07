#ifndef TRIEFORM_H
#define TRIEFORM_H

#include "../../Formula/And/And.h"
#include "../../Formula/Atom/Atom.h"
#include "../../Formula/Box/Box.h"
#include "../../Formula/Diamond/Diamond.h"
#include "../../Formula/FEnum/FEnum.h"
#include "../../Formula/Formula/Formula.h"
#include "../../Formula/Or/Or.h"
#include "../../Prover/MinisatProver/MinisatProver.h"
#include "../Cache/Cache.h"
#include "../FormulaTriple/FormulaTriple.h"
#include <climits>
#include <exception>
#include <memory>
#include <minisat/core/Solver.h>
#include <unordered_map>
#include <vector>

using namespace std;

class Trieform;
typedef unordered_map<int, shared_ptr<Trieform>> trie_map;

struct CommonModalKey {
  int modality;
  shared_ptr<Formula> common;
};

struct CommonModalKeyHasher {
  int operator()(const CommonModalKey &CMK) const {
    int hash = CMK.modality;
    return hash + CMK.common->hash();
  }
};

struct CommonModalKeyEqual {
  size_t operator()(CommonModalKey const &cmk1,
                    CommonModalKey const &cmk2) const {
    return cmk1.modality == cmk2.modality &&
           *cmk1.common.get() == *cmk2.common.get();
  }
};

typedef unordered_map<CommonModalKey, formula_set, CommonModalKeyHasher,
                      CommonModalKeyEqual>
    common_modal_map;

class Trieform : public enable_shared_from_this<Trieform> {
private:
  static Cache cache;

  void propagateOr(const shared_ptr<Formula> &formula);
  void orBoxClausify(const shared_ptr<Formula> &box,
                     const shared_ptr<Formula> &primitive);
  void orDiamondClausify(const shared_ptr<Formula> &diamond,
                         const shared_ptr<Formula> &primitive);
  void sequenceClausify(const shared_ptr<Formula> &formula,
                        const shared_ptr<Formula> &left);
  shared_ptr<Formula> nameFor(const shared_ptr<Formula> &formula);

  vector<int> constructNewModality(int subModality);

  formula_set removeAnds(const shared_ptr<Formula> &formula);

  void combineBoxRight();
  void combineDiamondRight();
  void combineBoxLeft();

protected:
  shared_ptr<Trieform> parent;
  void ensureSubtrieExistence(int submodality);

  bool parentSatisfiesAssump(Literal literal);
  bool parentSatisfiesAssumps(literal_set literals);

  vector<literal_set> generateClauses(vector<literal_set> literalCombinations);

  trie_map subtrieMap;
  unordered_set<int> futureModalities;

  shared_ptr<Prover> prover = shared_ptr<Prover>(new MinisatProver());

  vector<int> modality;

public:
  Trieform();
  ~Trieform();

  void propagateClauses(const shared_ptr<Formula> &formula);
  void overShadow(shared_ptr<Trieform> shadowTrie, int skipModality = 0);
  void conditionalOverShadow(shared_ptr<Trieform> shadowTrie,
                             shared_ptr<Formula> condition,
                             vector<int> prefix = vector<int>());
  FormulaTriple clauses;
  unsigned int getLevel();

  void initialise(const shared_ptr<Formula> &formula,
                  shared_ptr<Trieform> parentTrie);
  void initialise(const shared_ptr<Formula> &formula,
                  const vector<int> &newModality,
                  shared_ptr<Trieform> parentTrie);
  void initialise(const vector<int> &newModality,
                  shared_ptr<Trieform> parentTrie);

  trie_map getTrieMap();
  shared_ptr<Trieform> getSubtrie(int subModality);
  shared_ptr<Trieform> getSubtrieOrEmpty(int subModality);
  bool hasSubtrie(int subModality);
  void removeSubtrie(int subModality);

  void reduceClauses();
  void removeTrueAndFalse();

  string toString();

  static Cache &getCache();
  const FormulaTriple &getClauses();
  shared_ptr<Prover> &getProver();
  shared_ptr<Trieform> getParent();

  bool isSatisfiable();

  virtual Solution prove(literal_set assumptions = literal_set()) = 0;
  virtual void preprocess() = 0;
  virtual void prepareSAT(name_set extra = name_set()) = 0;

  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula) = 0;
  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                                      const vector<int> &newModality) = 0;
  virtual shared_ptr<Trieform> create(const vector<int> &newModality) = 0;

  unordered_set<int> getFutureModalities();
};

#endif