#include "Or.h"

Or::Or(const formula_set &orSet) {
  for (shared_ptr<Formula> formula : orSet) {
    Or *orFormula = dynamic_cast<Or *>(formula.get());
    if (orFormula) {
      const formula_set *subformulas = orFormula->getSubformulasReference();
      orSet_.insert(subformulas->begin(), subformulas->end());
    } else {
      orSet_.insert(formula);
    }
  }
}
Or::~Or() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING OR" << endl;
#endif
}

formula_set Or::getSubformulas() const { return orSet_; }
const formula_set *Or::getSubformulasReference() const { return &orSet_; }

void Or::addSubformula(const shared_ptr<Formula> &subformula) {
  orSet_.insert(subformula);
}

string Or::toString() const {
  string rep = "(";
  for (shared_ptr<Formula> formula : orSet_) {
    rep += formula->toString() + " | ";
  }
  rep = rep.substr(0, rep.size() - 3) + ")";
  return rep;
}

FormulaType Or::getType() const { return FOr; }

shared_ptr<Formula> Or::negatedNormalForm() {
  formula_set newOrSet;
  for (shared_ptr<Formula> formula : orSet_) {
    newOrSet.insert(formula->negatedNormalForm());
  }
  orSet_ = newOrSet;
  return shared_from_this();
}

shared_ptr<Formula> Or::negate() {
  formula_set newAndSet;
  for (shared_ptr<Formula> formula : orSet_) {
    newAndSet.insert(formula->negate());
  }
  return And::create(newAndSet);
}

shared_ptr<Formula> Or::simplify() {
  formula_set newOrSet;
  for (shared_ptr<Formula> formula : orSet_) {
    newOrSet.insert(formula->simplify());
  }
  orSet_ = newOrSet;

  shared_ptr<Formula> trueFormula = True::create();
  if (orSet_.count(trueFormula)) {
    return trueFormula;
  }

  shared_ptr<Formula> falseFormula = False::create();
  orSet_.erase(falseFormula);

  if (orSet_.size() > 1) {
    return shared_from_this();
  } else if (orSet_.size() == 1) {
    return *orSet_.begin();
  } else {
    return falseFormula;
  }
}

shared_ptr<Formula> Or::modalFlatten() {
  unordered_map<int, formula_set> diamondModalMap;
  unordered_map<int, int> diamondMinPowers;
  formula_set nonDiamondSet;

  for (shared_ptr<Formula> formula : orSet_) {
    shared_ptr<Formula> reduced = formula; //->modalFlatten();

    if (reduced->getType() == FDiamond) {
      Diamond *d = dynamic_cast<Diamond *>(reduced.get());

      diamondModalMap[d->getModality()].insert(reduced);

      if (diamondMinPowers.count(d->getModality()) == 0 ||
          d->getPower() < diamondMinPowers[d->getModality()]) {
        diamondMinPowers[d->getModality()] = d->getPower();
      }
    } else {
      nonDiamondSet.insert(formula);
    }
  }

  for (auto const &x : diamondModalMap) {
    int modality = x.first;
    int minPower = diamondMinPowers[modality];

    formula_set diamondForm;
    for (shared_ptr<Formula> formula : x.second) {
      Diamond *d = dynamic_cast<Diamond *>(formula.get());

      if (d->getPower() == minPower) {
        shared_ptr<Formula> dSubformula = d->getSubformula();
        if (dSubformula->getType() == FOr) {
          Or *o = dynamic_cast<Or *>(dSubformula.get());
          formula_set orFormulas = o->getSubformulas();
          diamondForm.insert(orFormulas.begin(), orFormulas.end());
        } else {
          diamondForm.insert(d->getSubformula());
        }
      } else {
        diamondForm.insert(Diamond::create(modality, d->getPower() - minPower,
                                           d->getSubformula()));
      }
    }
    if (diamondForm.size() > 1) {
      nonDiamondSet.insert(
          Diamond::create(modality, minPower, Or::create(diamondForm)));
    } else {
      nonDiamondSet.insert(
          Diamond::create(modality, minPower, *diamondForm.begin()));
    }
  }

  if (nonDiamondSet.size() == 1) {
    return *nonDiamondSet.begin();
  }
  orSet_ = nonDiamondSet;
  return shared_from_this();
}

shared_ptr<Formula> Or::create(formula_set orSet) {
  shared_ptr<Formula> trueFormula = True::create();
  if (orSet.count(trueFormula)) {
    return trueFormula;
  }

  shared_ptr<Formula> falseFormula = False::create();
  orSet.erase(falseFormula);

  if (orSet.size() > 1) {
    return shared_ptr<Formula>(new Or(orSet));
  } else if (orSet.size() == 1) {
    return *orSet.begin();
  } else {
    return falseFormula;
  }
}

shared_ptr<Formula> Or::clone() const {
  formula_set newOrSet_;
  for (shared_ptr<Formula> formula : orSet_) {
    newOrSet_.insert(formula->clone());
  }
  return create(newOrSet_);
}

bool Or::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const Or *otherOr = dynamic_cast<const Or *>(&other);
  if (otherOr->orSet_.size() != orSet_.size()) {
    return false;
  }
  for (shared_ptr<Formula> formula : orSet_) {
    if (otherOr->orSet_.count(formula) == 0) {
      return false;
    }
  }
  // cout << "THEY ARE EQUAL " << toString() << " AND " << other.toString() << endl;
  return true;
}

bool Or::operator!=(const Formula &other) const { return !(operator==(other)); }

size_t Or::hash() const {
  std::hash<FormulaType> ftype_hash;
  size_t totalHash = ftype_hash(getType());
  for (shared_ptr<Formula> formula : orSet_) {
    totalHash += formula->hash();
  }
  return totalHash;
}

int Or::getLength() const { return orSet_.size(); }