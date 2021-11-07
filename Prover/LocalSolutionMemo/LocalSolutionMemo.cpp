#include "LocalSolutionMemo.h"

LocalSolutionMemo::LocalSolutionMemo() {}
LocalSolutionMemo::~LocalSolutionMemo() {}

LocalSolutionMemoResult
LocalSolutionMemo::getFromMemo(const shared_ptr<Bitset> &assumptions) const {
  for (shared_ptr<Bitset> satisfiable : satSols) {
    if (satisfiable->contains(*assumptions)) {
      return {true, {true, literal_set()}};
    }
  }
  for (UnsatHolder unsatisfiable : unsatSols) {
    if (assumptions->contains(*unsatisfiable.activatedLiterals)) {
      return {true, {false, unsatisfiable.unsatCore}};
    }
  }
  return {false, {false, literal_set()}};
}

void LocalSolutionMemo::insertSat(const shared_ptr<Bitset> &assumptions) {
  for (int i = satSols.size() - 1; i >= 0; i--) {
    if (assumptions->contains(*satSols[i])) {
      satSols.erase(satSols.begin() + i);
    }
  }
  satSols.push_back(assumptions);
}

void LocalSolutionMemo::insertUnsat(const shared_ptr<Bitset> &assumptions,
                                    const literal_set &unsatCore) {
  for (int i = unsatSols.size() - 1; i >= 0; i--) {
    if (unsatSols[i].activatedLiterals->contains(*assumptions)) {
      unsatSols.erase(unsatSols.begin() + i);
    }
  }
  unsatSols.push_back({assumptions, unsatCore});
}