#ifndef TRIEFORM_PROVER_DB
#define TRIEFORM_PROVER_DB

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../TrieformProverKB/TrieformProverKB.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverDB : public TrieformProverKB {
protected:
  void ensureSeriality();
  void propagateSymmetry();
  void propagateSymmetricBoxes();

public:
  TrieformProverDB();
  ~TrieformProverDB();

  void preprocess();

  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                              const vector<int> &newModality);
  shared_ptr<Trieform> create(const vector<int> &newModality);
};

#endif