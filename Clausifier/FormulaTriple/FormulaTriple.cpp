#include "FormulaTriple.h"

FormulaTriple::FormulaTriple() {}

FormulaTriple::~FormulaTriple() {}

void FormulaTriple::setClauses(clause_set newClauses) { clauses = newClauses; }
void FormulaTriple::setBoxClauses(modal_clause_set newBoxClauses) {
  boxClauses = newBoxClauses;
}
void FormulaTriple::setDiamondClauses(modal_clause_set newDiamondClauses) {
  diamondClauses = newDiamondClauses;
}

void FormulaTriple::eraseClauses() { clauses.clear(); }
void FormulaTriple::eraseBoxClauses() { boxClauses.clear(); }
void FormulaTriple::eraseDiamondClauses() { diamondClauses.clear(); }

const clause_set &FormulaTriple::getClauses() const { return clauses; }
const modal_clause_set &FormulaTriple::getBoxClauses() const {
  return boxClauses;
}
const modal_clause_set &FormulaTriple::getDiamondClauses() const {
  return diamondClauses;
}

void FormulaTriple::addClause(shared_ptr<Formula> formula) {
  clauses.insert({formula});
}

void FormulaTriple::addBoxClause(int modality, shared_ptr<Formula> left,
                                 shared_ptr<Formula> right) {
  boxClauses.insert({modality, left, right});
}

void FormulaTriple::addDiamondClause(int modality, shared_ptr<Formula> left,
                                     shared_ptr<Formula> right) {
  diamondClauses.insert({modality, left, right});
}

void FormulaTriple::addClause(Clause clause) { clauses.insert(clause); }

void FormulaTriple::addBoxClause(ModalClause boxClause) {
  boxClauses.insert(boxClause);
}

void FormulaTriple::addDiamondClause(ModalClause diamondClause) {
  diamondClauses.insert(diamondClause);
}

void FormulaTriple::extendBoxClauses(const modal_clause_set &otherBoxClauses) {
  boxClauses.insert(otherBoxClauses.begin(), otherBoxClauses.end());
}

void FormulaTriple::extendClauses(const FormulaTriple &otherClauses) {
  clauses.insert(otherClauses.clauses.begin(), otherClauses.clauses.end());
  boxClauses.insert(otherClauses.boxClauses.begin(),
                    otherClauses.boxClauses.end());
  diamondClauses.insert(otherClauses.diamondClauses.begin(),
                        otherClauses.diamondClauses.end());
}

void FormulaTriple::removeTrueAndFalse() {
  clause_set newClauses;
  for (Clause clause : clauses) {
    if (clause.formula->getType() == FTrue) {
      clause.formula = Not::create(Atom::create("$false"));
    } else if (clause.formula->getType() == FFalse) {
      clause.formula = Atom::create("$false");
    }
    newClauses.insert(clause);
  }
  clauses = newClauses;

  modal_clause_set newBoxClauses;
  for (ModalClause clause : boxClauses) {
    if (clause.left->getType() == FTrue) {
      clause.left = Not::create(Atom::create("$false"));
    } else if (clause.left->getType() == FFalse) {
      clause.left = Atom::create("$false");
    }
    if (clause.right->getType() == FTrue) {
      clause.right = Not::create(Atom::create("$false"));
    } else if (clause.right->getType() == FFalse) {
      clause.right = Atom::create("$false");
    }
    newBoxClauses.insert(clause);
  }
  boxClauses = newBoxClauses;

  modal_clause_set newDiamondClauses;
  for (ModalClause clause : diamondClauses) {
    if (clause.left->getType() == FTrue) {
      clause.left = Not::create(Atom::create("$false"));
    } else if (clause.left->getType() == FFalse) {
      clause.left = Atom::create("$false");
    }
    if (clause.right->getType() == FTrue) {
      clause.right = Not::create(Atom::create("$false"));
    } else if (clause.right->getType() == FFalse) {
      clause.right = Atom::create("$false");
    }
    newDiamondClauses.insert(clause);
  }
  diamondClauses = newDiamondClauses;
}

vector<string> FormulaTriple::toStringComponents() const {
  vector<string> components;
  for (Clause clause : clauses) {
    components.push_back(clause.formula->toString());
  }

  for (ModalClause boxClause : boxClauses) {
    components.push_back(boxClause.left->toString() + "->[" +
                         to_string(boxClause.modality) + "]" +
                         boxClause.right->toString());
  }

  for (ModalClause diamondClause : diamondClauses) {
    components.push_back(diamondClause.left->toString() + "-><" +
                         to_string(diamondClause.modality) + ">" +
                         diamondClause.right->toString());
  }
  return components;
}