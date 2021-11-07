#ifndef TRIEFORM_PROVER_KB
#define TRIEFORM_PROVER_KB

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../TrieformProverK/TrieformProverK.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverKB : public Trieform {
protected:
  unsigned int assumptionsSize = 0;
  LocalSolutionMemo localMemo;
  unordered_map<string, unsigned int> idMap;

  shared_ptr<Bitset> convertAssumptionsToBitset(literal_set literals);
  void updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                          Solution solution);

  void propagateSymmetry();
  void propagateSymmetricBoxes();

public:
  TrieformProverKB();
  ~TrieformProverKB();

  virtual void preprocess();
  virtual void prepareSAT(name_set extra = name_set());

  virtual Solution prove(literal_set assumptions = literal_set());

  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                              const vector<int> &newModality);
  shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif