#include "TrieformProverDB.h"

shared_ptr<Trieform>
TrieformFactory::makeTrieDB(const shared_ptr<Formula> &formula,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverDB());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieDB(const shared_ptr<Formula> &formula,
                            const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverDB());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieDB(const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverDB());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverDB::TrieformProverDB() {}
TrieformProverDB::~TrieformProverDB() {}

shared_ptr<Trieform>
TrieformProverDB::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieDB(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverDB::create(const shared_ptr<Formula> &formula,
                         const vector<int> &newModality) {
  return TrieformFactory::makeTrieDB(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverDB::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieDB(newModality, shared_from_this());
}

void TrieformProverDB::propagateSymmetry() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverDB *>(modalitySubtrie.second.get())
        ->propagateSymmetry();
  }
  for (auto modalitySubtrie : subtrieMap) {
    if (modalitySubtrie.second->hasSubtrie(modalitySubtrie.first)) {
      shared_ptr<Trieform> future =
          modalitySubtrie.second->getSubtrie(modalitySubtrie.first);
      overShadow(future);
    }
  }
}

void TrieformProverDB::propagateSymmetricBoxes() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverDB *>(modalitySubtrie.second.get())
        ->propagateSymmetricBoxes();
  }
  for (auto modalitySubtrie : subtrieMap) {
    for (const ModalClause &boxClause :
         modalitySubtrie.second->getClauses().getBoxClauses()) {

      if (modalitySubtrie.first == boxClause.modality) {
        clauses.addBoxClause(boxClause.modality, boxClause.right->negate(),
                             boxClause.left->negate());
      }
    }
  }
}

void TrieformProverDB::ensureSeriality() {
  for (auto modalitySubtrie : subtrieMap) {
    propagateClauses(Diamond::create(modalitySubtrie.first, 1, True::create()));
    dynamic_cast<TrieformProverDB *>(modalitySubtrie.second.get())
        ->ensureSeriality();
  }
}

void TrieformProverDB::preprocess() {
  ensureSeriality();
  propagateSymmetry();
  propagateSymmetricBoxes();
}