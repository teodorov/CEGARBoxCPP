#include "Prover.h"

Prover::Prover() {}
Prover::~Prover() {}

void Prover::createModalImplication(int modality, Literal left, Literal right,
                                    modal_lit_implication &modalLits,
                                    modal_lit_implication &modalFromRight) {
  modalLits[modality][left].insert(right);
  modalFromRight[modality][right].insert(left);
}

string Prover::getPrimitiveName(shared_ptr<Formula> formula) {
  if (formula->getType() == FAtom) {
    return dynamic_cast<Atom *>(formula.get())->getName();
  } else if (formula->getType() == FNot) {
    return dynamic_cast<Atom *>(
               dynamic_cast<Not *>(formula.get())->getSubformula().get())
        ->getName();
  }
  throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

Literal Prover::toLiteral(shared_ptr<Formula> formula) {
  if (formula->getType() == FAtom) {
    return Literal(dynamic_cast<Atom *>(formula.get())->getName(), true);
  } else if (formula->getType() == FNot) {
    return Literal(
        dynamic_cast<Atom *>(
            dynamic_cast<Not *>(formula.get())->getSubformula().get())
            ->getName(),
        false);
  }
  throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

void Prover::calculateTriggeredModalClauses(modal_lit_implication &modalLits,
                                            modal_literal_map &triggered) {
  triggered.clear();
  for (auto modalityLitImplication : modalLits) {
    for (auto literalImplication : modalityLitImplication.second) {
      if (modelSatisfiesAssump(literalImplication.first)) {
        triggered[modalityLitImplication.first].insert(
            literalImplication.second.begin(), literalImplication.second.end());
      }
    }
  }
}

bool Prover::modelSatisfiesAssumps(literal_set assumptions) {
  for (Literal assump : assumptions) {
    if (!modelSatisfiesAssump(assump)) {
      return false;
    }
  }
  return true;
}
modal_literal_map
Prover::getTriggeredModalClauses(modal_lit_implication &modalLits) {
  modal_literal_map triggered;
  for (auto modalityLitImplication : modalLits) {
    for (auto literalImplication : modalityLitImplication.second) {
      if (modelSatisfiesAssump(literalImplication.first)) {
        triggered[modalityLitImplication.first].insert(
            literalImplication.second.begin(), literalImplication.second.end());
      }
    }
  }
  return triggered;
}

void Prover::calculateTriggeredBoxClauses() {
  calculateTriggeredModalClauses(boxLits, triggeredBoxes);
}

void Prover::calculateTriggeredDiamondsClauses() {
  calculateTriggeredModalClauses(diamondLits, triggeredDiamonds);
}

modal_literal_map Prover::getTriggeredBoxClauses() { return triggeredBoxes; }
modal_literal_map Prover::getTriggeredDiamondClauses() {
  return triggeredDiamonds;
}
literal_set Prover::getNotAllDiamondLeft(int modality) {
  literal_set notAllDiamondLeft;
  for (auto literalImplication : diamondLits[modality]) {
    notAllDiamondLeft.insert(~literalImplication.first);
  }
  return notAllDiamondLeft;
}

vector<literal_set> Prover::getNotProblemBoxClauses(int modality,
                                                    literal_set conflicts) {
  vector<literal_set> notProblemBoxClauses;
  for (Literal conflict : conflicts) {
    literal_set problemLeft;
    for (auto literalImplication : boxLits[modality]) {
      if (modelSatisfiesAssump(literalImplication.first) &&
          literalImplication.second.find(conflict) !=
              literalImplication.second.end()) {
        problemLeft.insert(~literalImplication.first);
      }
    }
    if (problemLeft.size() > 0) {
      notProblemBoxClauses.push_back(problemLeft);
    }
  }
  return notProblemBoxClauses;
};

literal_set Prover::getNotDiamondLeft(int modality, Literal diamond) {
  literal_set notDiamondLeft;
  for (auto literalImplication : diamondLits[modality]) {
    if (modelSatisfiesAssump(literalImplication.first) &&
        literalImplication.second.find(diamond) !=
            literalImplication.second.end()) {
      notDiamondLeft.insert(~literalImplication.first);
    }
  }
  return notDiamondLeft;
}

void Prover::updateLastFail(Literal clause) { lastFail[clause] = ++failCount; }

diamond_queue Prover::getPrioritisedTriggeredDiamonds(int modality) {
  // Note MUST avoid box clauses
  diamond_queue prioritisedTriggeredDiamonds;
  literal_set triggeredBoxes = getTriggeredBoxClauses()[modality];
  literal_set triggeredDiamonds = getTriggeredDiamondClauses()[modality];
  for (Literal diamond : triggeredDiamonds) {
    if (triggeredBoxes.find(diamond) == triggeredBoxes.end()) {
      prioritisedTriggeredDiamonds.push({diamond, lastFail[diamond]});
    }
  }
  return prioritisedTriggeredDiamonds;
}