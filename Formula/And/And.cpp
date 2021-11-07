#include "And.h"

And::And(const formula_set &andSet) {
  andSet_ = andSet;
  for (shared_ptr<Formula> formula : andSet) {
    And *andFormula = dynamic_cast<And *>(formula.get());
    if (andFormula) {
      const formula_set *subformulas = andFormula->getSubformulasReference();
      andSet_.insert(subformulas->begin(), subformulas->end());
    } else {
      andSet_.insert(formula);
    }
  }
}

And::~And() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING AND" << endl;
#endif
}

formula_set And::getSubformulas() const { return andSet_; }
const formula_set *And::getSubformulasReference() const { return &andSet_; };

void And::addSubformula(const shared_ptr<Formula> &subformula) {
  andSet_.insert(subformula);
}

string And::toString() const {
  string rep = "(";
  for (shared_ptr<Formula> formula : andSet_) {
    rep += formula->toString() + " & ";
  }
  rep = rep.substr(0, rep.size() - 3) + ")";
  return rep;
}

FormulaType And::getType() const { return FAnd; }

shared_ptr<Formula> And::negatedNormalForm() {
  formula_set newAndSet(andSet_.size());
  for (shared_ptr<Formula> formula : andSet_) {
    newAndSet.insert(formula->negatedNormalForm());
  }
  andSet_ = newAndSet;
  return shared_from_this();
}

shared_ptr<Formula> And::negate() {
  formula_set newOrSet;
  for (shared_ptr<Formula> formula : andSet_) {
    newOrSet.insert(formula->negate());
  }
  return Or::create(newOrSet);
}

shared_ptr<Formula> And::simplify() {
  formula_set newAndSet;
  for (shared_ptr<Formula> formula : andSet_) {
    newAndSet.insert(formula->simplify());
  }
  andSet_ = newAndSet;

  shared_ptr<Formula> falseFormula = False::create();
  if (andSet_.count(falseFormula)) {
    return falseFormula;
  }

  shared_ptr<Formula> trueFormula = True::create();
  andSet_.erase(trueFormula);

  if (andSet_.size() > 1) {
    return shared_from_this();
  } else if (andSet_.size() == 1) {
    return *andSet_.begin();
  } else {
    return trueFormula;
  }
}

shared_ptr<Formula> And::modalFlatten() {
  unordered_map<int, formula_set> boxModalMap;
  unordered_map<int, int> boxMinPowers;
  formula_set nonBoxSet;

  for (shared_ptr<Formula> formula : andSet_) {
    shared_ptr<Formula> reduced = formula; //->modalFlatten();
    if (reduced->getType() == FBox) {
      Box *b = dynamic_cast<Box *>(reduced.get());

      boxModalMap[b->getModality()].insert(reduced);

      if (boxMinPowers.count(b->getModality()) == 0 ||
          b->getPower() < boxMinPowers[b->getModality()]) {
        boxMinPowers[b->getModality()] = b->getPower();
      }
    } else {
      nonBoxSet.insert(formula);
    }
  }

  for (auto const &x : boxModalMap) {
    int modality = x.first;
    int minPower = boxMinPowers[modality];
    formula_set boxForm;
    for (shared_ptr<Formula> formula : x.second) {
      Box *b = dynamic_cast<Box *>(formula.get());

      if (b->getPower() == minPower) {
        shared_ptr<Formula> bSubformula = b->getSubformula();
        if (bSubformula->getType() == FAnd) {
          And *a = dynamic_cast<And *>(bSubformula.get());
          formula_set andFormulas = a->getSubformulas();
          boxForm.insert(andFormulas.begin(), andFormulas.end());
        } else {
          boxForm.insert(b->getSubformula());
        }
      } else {
        boxForm.insert(Box::create(modality, b->getPower() - minPower,
                                   b->getSubformula()));
      }
    }
    if (boxForm.size() > 1) {
      nonBoxSet.insert(Box::create(modality, minPower,
                                   And::create(boxForm)->modalFlatten()));
    } else {
      nonBoxSet.insert(Box::create(modality, minPower, *boxForm.begin()));
    }
  }

  if (nonBoxSet.size() == 1) {
    return *nonBoxSet.begin();
  }
  andSet_ = nonBoxSet;
  return shared_from_this();
}

shared_ptr<Formula> And::create(formula_set andSet) {
  shared_ptr<Formula> falseFormula = False::create();
  if (andSet.count(falseFormula)) {
    return falseFormula;
  }

  shared_ptr<Formula> trueFormula = True::create();
  andSet.erase(trueFormula);

  if (andSet.size() > 1) {
    return shared_ptr<Formula>(new And(andSet));
  } else if (andSet.size() == 1) {
    return *andSet.begin();
  } else {
    return trueFormula;
  }
}

bool And::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const And *otherAnd = dynamic_cast<const And *>(&other);
  if (otherAnd->andSet_.size() != andSet_.size()) {
    return false;
  }
  for (shared_ptr<Formula> formula : andSet_) {
    if (otherAnd->andSet_.count(formula) == 0) {
      return false;
    }
  }
  return true;
}

shared_ptr<Formula> And::clone() const {
  formula_set newAndSet_;
  for (shared_ptr<Formula> formula : andSet_) {
    newAndSet_.insert(formula->clone());
  }
  return create(newAndSet_);
}

bool And::operator!=(const Formula &other) const {
  return !(operator==(other));
}

size_t And::hash() const {
  std::hash<FormulaType> ftype_hash;
  size_t totalHash = ftype_hash(getType());
  for (shared_ptr<Formula> formula : andSet_) {
    totalHash += formula->hash();
  }
  return totalHash;
}

int And::getLength() const { return andSet_.size(); }