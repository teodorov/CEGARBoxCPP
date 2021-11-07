#include "GlobalSolutionMemo.h"

GlobalSolutionMemo::GlobalSolutionMemo() {}
GlobalSolutionMemo::~GlobalSolutionMemo() {}

GlobalSolutionMemoResult
GlobalSolutionMemo::getFromMemo(const shared_ptr<Bitset> &assumptions,
                                vector<int> modality) {
  while (true) {
    for (shared_ptr<Bitset> satisfiable : satSols[vector<int>(modality)]) {
      if (satisfiable->contains(*assumptions)) {
        return {true, {true, literal_set()}};
      }
    }
    if (modality.size() == 0)
      break;
    modality.pop_back();
  }
  return {false, {false, literal_set()}};
}

void GlobalSolutionMemo::insertSat(const shared_ptr<Bitset> &assumptions,
                                   vector<int> modality) {
  for (int i = satSols[modality].size() - 1; i >= 0; i--) {
    if (assumptions->contains(*satSols[modality][i])) {
      satSols[modality].erase(satSols[modality].begin() + i);
    }
  }
  satSols[modality].push_back(assumptions);
}

void GlobalSolutionMemo::insertUnsat(const shared_ptr<Bitset> &assumptions,
                                     const literal_set &unsatCore,
                                     vector<int> modality) {
  for (int i = unsatSols[modality].size() - 1; i >= 0; i--) {
    if (unsatSols[modality][i].activatedLiterals->contains(*assumptions)) {
      unsatSols[modality].erase(unsatSols[modality].begin() + i);
    }
  }
  unsatSols[modality].push_back({assumptions, unsatCore});
}