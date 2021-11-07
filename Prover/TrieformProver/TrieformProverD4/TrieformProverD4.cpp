#include "TrieformProverD4.h"

shared_ptr<Trieform>
TrieformFactory::makeTrieD4(const shared_ptr<Formula> &formula,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD4());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD4(const shared_ptr<Formula> &formula,
                            const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD4());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD4(const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverD4());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverD4::TrieformProverD4() {}
TrieformProverD4::~TrieformProverD4() {}

shared_ptr<Trieform>
TrieformProverD4::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieD4(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverD4::create(const shared_ptr<Formula> &formula,
                         const vector<int> &newModality) {
  return TrieformFactory::makeTrieD4(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverD4::create(const vector<int> &newModality) {
  shared_ptr<Trieform> test = shared_from_this();
  return TrieformFactory::makeTrieD4(newModality, shared_from_this());
}

void TrieformProverD4::makeSerial() {
  for (int modality : futureModalities) {
    propagateClauses(Diamond::create(modality, 1, True::create()));
    dynamic_cast<TrieformProverD4 *>(getSubtrie(modality).get())->makeSerial();
  }
}

void TrieformProverD4::preprocess() {
  makeSerial();
  makePersistence();
  propagateLevels();
}