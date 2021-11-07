#ifndef TRIEFORM_PROVER_D
#define TRIEFORM_PROVER_D

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../TrieformProverK/TrieformProverK.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverD : public TrieformProverK {
protected:
  void ensureSeriality();

public:
  TrieformProverD();
  ~TrieformProverD();

  void preprocess();

  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                              const vector<int> &newModality);
  shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif