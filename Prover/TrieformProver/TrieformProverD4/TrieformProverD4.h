#ifndef TRIEFORM_PROVER_D4
#define TRIEFORM_PROVER_D4

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../../LocalSolutionMemo/LocalSolutionMemo.h"
#include "../TrieformProverK4/TrieformProverK4.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverD4 : public TrieformProverK4 {
private:
  void makeSerial();

public:
  TrieformProverD4();
  ~TrieformProverD4();

  virtual void preprocess();

  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                                      const vector<int> &newModality);
  virtual shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif