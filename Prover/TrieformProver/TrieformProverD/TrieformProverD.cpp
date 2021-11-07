#include "TrieformProverD.h"

shared_ptr<Trieform>
TrieformFactory::makeTrieD(const shared_ptr<Formula> &formula,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD(const shared_ptr<Formula> &formula,
                           const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD(const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverD::TrieformProverD() {}
TrieformProverD::~TrieformProverD() {}

shared_ptr<Trieform>
TrieformProverD::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieD(formula, shared_from_this());
}
shared_ptr<Trieform> TrieformProverD::create(const shared_ptr<Formula> &formula,
                                             const vector<int> &newModality) {
  return TrieformFactory::makeTrieD(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverD::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieD(newModality, shared_from_this());
}

void TrieformProverD::ensureSeriality() {
  for (auto modalitySubtrie : subtrieMap) {
    propagateClauses(Diamond::create(modalitySubtrie.first, 1, True::create()));
    dynamic_cast<TrieformProverD *>(modalitySubtrie.second.get())
        ->ensureSeriality();
  }
}

void TrieformProverD::preprocess() { ensureSeriality(); }