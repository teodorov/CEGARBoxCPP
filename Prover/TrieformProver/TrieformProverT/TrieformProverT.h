#ifndef TRIEFORM_PROVER_T
#define TRIEFORM_PROVER_T

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/Trieform/Trieform.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../../GlobalSolutionMemo/GlobalSolutionMemo.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverT : public Trieform {
protected:
  static unsigned int assumptionsSize;
  static GlobalSolutionMemo globalMemo;
  static unordered_map<string, unsigned int> idMap;

  void handleReflexiveBoxClauses();
  void propagateLevels();

  shared_ptr<Bitset> convertAssumptionsToBitset(literal_set literals);
  void updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                          Solution solution);
  bool isInHistory(vector<shared_ptr<Bitset>> history,
                   shared_ptr<Bitset> bitset);

public:
  TrieformProverT();
  ~TrieformProverT();

  Solution prove(vector<shared_ptr<Bitset>> history, literal_set assumptions);
  virtual Solution prove(literal_set assumptions);
  virtual void preprocess();
  virtual void prepareSAT(name_set extra = name_set());

  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                                      const vector<int> &newModality);
  virtual shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif