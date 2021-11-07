#include "Diamond.h"

Diamond::Diamond(int modality, int power, shared_ptr<Formula> subformula) {
  modality_ = modality;
  power_ = power;

  Diamond *diamondFormula = dynamic_cast<Diamond *>(subformula.get());
  if (diamondFormula) {
    if (diamondFormula->getModality() == modality_) {
      power_ += diamondFormula->getPower();
      subformula_ = diamondFormula->getSubformula();
    } else {
      subformula_ = subformula;
    }
  } else {
    subformula_ = subformula;
  }
}

Diamond::~Diamond() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING DIAMOND" << endl;
#endif
}

int Diamond::getModality() const { return modality_; }

int Diamond::getPower() const { return power_; }

shared_ptr<Formula> Diamond::getSubformula() const { return subformula_; }

void Diamond::incrementPower() { power_++; }

string Diamond::toString() const {
  return "<" + to_string(modality_) + ">^" + to_string(power_) + " " +
         subformula_->toString();
}

FormulaType Diamond::getType() const { return FDiamond; }

shared_ptr<Formula> Diamond::negatedNormalForm() {
  subformula_ = subformula_->negatedNormalForm();
  return shared_from_this();
}

shared_ptr<Formula> Diamond::negate() {
  return Box::create(modality_, power_, subformula_->negate());
}

shared_ptr<Formula> Diamond::simplify() {
  subformula_ = subformula_->simplify();

  switch (subformula_->getType()) {
  case FFalse:
    return False::create();
  case FDiamond: {
    Diamond *diamondFormula = dynamic_cast<Diamond *>(subformula_.get());
    if (diamondFormula->getModality() == modality_) {
      power_ += diamondFormula->getPower();
      subformula_ = diamondFormula->getSubformula();
    }
    return shared_from_this();
  }

  default:
    return shared_from_this();
  }
}

shared_ptr<Formula> Diamond::modalFlatten() {
  subformula_ = subformula_->modalFlatten();
  if (subformula_->getType() == FDiamond) {
    Diamond *d = dynamic_cast<Diamond *>(subformula_.get());
    if (d->getModality() == modality_) {
      power_ += d->getPower();
      subformula_ = d->getSubformula();
    }
  }
  return shared_from_this();
}

shared_ptr<Formula> Diamond::create(int modality, int power,
                                    shared_ptr<Formula> subformula) {
  if (power == 0) {
    return subformula;
  }
  return shared_ptr<Formula>(new Diamond(modality, power, subformula));
}

shared_ptr<Formula> Diamond::create(vector<int> modality,
                                    const shared_ptr<Formula> &subformula) {
  if (modality.size() == 0) {
    return subformula;
  }
  shared_ptr<Formula> formula =
      Diamond::create(modality[modality.size() - 1], 1, subformula);
  for (size_t i = modality.size() - 1; i > 0; i--) {
    formula = Diamond::create(modality[i - 1], 1, formula);
  }
  return formula;
}

shared_ptr<Formula> Diamond::constructDiamondReduced() const {
  return Diamond::create(modality_, power_ - 1, subformula_);
}

shared_ptr<Formula> Diamond::clone() const {
  return create(modality_, power_, subformula_->clone());
}

bool Diamond::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const Diamond *otherDiamond = dynamic_cast<const Diamond *>(&other);
  return modality_ == otherDiamond->modality_ &&
         power_ == otherDiamond->power_ &&
         *subformula_ == *(otherDiamond->subformula_);
}

bool Diamond::operator!=(const Formula &other) const {
  return !(operator==(other));
}

size_t Diamond::hash() const {
  std::hash<FormulaType> ftype_hash;
  std::hash<int> int_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash + int_hash(modality_) + int_hash(power_) +
         subformula_->hash();
}