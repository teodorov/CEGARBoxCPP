#ifndef TRIEFORM_PROVER_B
#define TRIEFORM_PROVER_B

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../../GlobalSolutionMemo/GlobalSolutionMemo.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverB : public Trieform {
protected:
  static unsigned int assumptionsSize;
  static GlobalSolutionMemo globalMemo;
  static unordered_map<string, unsigned int> idMap;

  void handleReflexiveBoxClauses();
  void propagateLevels();
  void propagateSymmetricBoxes();

  shared_ptr<Bitset> convertAssumptionsToBitset(literal_set literals);
  void updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                          Solution solution);

public:
  TrieformProverB();
  ~TrieformProverB();

  void preprocess();
  void prepareSAT(name_set extra = name_set());
  Solution prove(literal_set assumptions = literal_set());

  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                              const vector<int> &newModality);
  shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif