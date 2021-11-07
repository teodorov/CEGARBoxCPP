#include "TrieformFactory.h"

shared_ptr<Trieform>
TrieformFactory::makeTrie(const shared_ptr<Formula> &formula,
                          SolverConstraints constraints) {
  if ((constraints.reflexive && constraints.euclidean) ||
      (constraints.symmetric && constraints.euclidean) ||
      (constraints.reflexive && constraints.symmetric &&
       constraints.transitive) ||
      (constraints.serial && constraints.symmetric && constraints.transitive) ||
      (constraints.serial && constraints.symmetric && constraints.euclidean)) {
    return makeTrieS5(formula);
  } else if (constraints.symmetric &&
             (constraints.euclidean || constraints.transitive)) {
    return makeTrieKB5(formula);
  }
  if (constraints.reflexive && constraints.symmetric) {
    return makeTrieB(formula);
  } else if (constraints.serial && constraints.symmetric) {
    return makeTrieDB(formula);
  } else if (constraints.reflexive && constraints.transitive) {
    return makeTrieS4(formula);
  } else if (constraints.serial && constraints.transitive) {
    return makeTrieD4(formula);
  } else if (constraints.serial && constraints.euclidean) {
    return makeTrieD5(formula);
  } else if (constraints.serial && constraints.transitive &&
             constraints.euclidean) {
    return makeTrieD45(formula);
  } else if (constraints.transitive && constraints.euclidean) {
    return makeTrieK45(formula);
  } else if (constraints.reflexive) {
    return makeTrieT(formula);
  } else if (constraints.serial) {
    return makeTrieD(formula);
  } else if (constraints.symmetric) {
    return makeTrieKB(formula);
  } else if (constraints.transitive) {
    return makeTrieK4(formula);
  } else if (constraints.euclidean) {
    return makeTrieK5(formula);
  }
  return makeTrieK(formula);
}
