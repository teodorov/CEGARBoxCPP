#include "Box.h"

Box::Box(int modality, int power, shared_ptr<Formula> subformula) {
  modality_ = modality;
  power_ = power;

  Box *boxFormula = dynamic_cast<Box *>(subformula.get());
  if (boxFormula) {
    if (boxFormula->getModality() == modality_) {
      power_ += boxFormula->getPower();
      subformula_ = boxFormula->getSubformula();
    } else {
      subformula_ = subformula;
    }
  } else {
    subformula_ = subformula;
  }
}

Box::~Box() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING BOX" << endl;
#endif
}

int Box::getModality() const { return modality_; }

int Box::getPower() const { return power_; }

shared_ptr<Formula> Box::getSubformula() const { return subformula_; }

void Box::incrementPower() { power_++; }

string Box::toString() const {
  return "[" + to_string(modality_) + "]^" + to_string(power_) + " " +
         subformula_->toString();
}

FormulaType Box::getType() const { return FBox; }

shared_ptr<Formula> Box::negatedNormalForm() {
  subformula_ = subformula_->negatedNormalForm();
  return shared_from_this();
}
shared_ptr<Formula> Box::negate() {
  return Diamond::create(modality_, power_, subformula_->negate());
}

shared_ptr<Formula> Box::simplify() {
  subformula_ = subformula_->simplify();

  switch (subformula_->getType()) {
  case FTrue:
    return True::create();
  case FBox: {
    Box *boxFormula = dynamic_cast<Box *>(subformula_.get());
    if (boxFormula->getModality() == modality_) {
      power_ += boxFormula->getPower();
      subformula_ = boxFormula->getSubformula();
    }
    return shared_from_this();
  }

  default:
    return shared_from_this();
  }
}

shared_ptr<Formula> Box::modalFlatten() {
  subformula_ = subformula_->modalFlatten();
  if (subformula_->getType() == FBox) {
    Box *b = dynamic_cast<Box *>(subformula_.get());
    if (b->getModality() == modality_) {
      power_ += b->getPower();
      subformula_ = b->getSubformula();
    }
  }
  return shared_from_this();
}

shared_ptr<Formula> Box::create(int modality, int power,
                                const shared_ptr<Formula> &subformula) {
  if (power == 0) {
    return subformula;
  }
  return shared_ptr<Formula>(new Box(modality, power, subformula));
}

shared_ptr<Formula> Box::create(vector<int> modality,
                                const shared_ptr<Formula> &subformula) {
  if (modality.size() == 0) {
    return subformula;
  }
  shared_ptr<Formula> formula =
      Box::create(modality[modality.size() - 1], 1, subformula);
  for (size_t i = modality.size() - 1; i > 0; i--) {
    formula = Box::create(modality[i - 1], 1, formula);
  }
  return formula;
}

shared_ptr<Formula> Box::constructBoxReduced() const {
  return Box::create(modality_, power_ - 1, subformula_);
}

shared_ptr<Formula> Box::clone() const {
  return create(modality_, power_, subformula_->clone());
}

bool Box::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const Box *otherBox = dynamic_cast<const Box *>(&other);
  return modality_ == otherBox->modality_ && power_ == otherBox->power_ &&
         *subformula_ == *(otherBox->subformula_);
}

bool Box::operator!=(const Formula &other) const {
  return !(operator==(other));
}

size_t Box::hash() const {
  std::hash<FormulaType> ftype_hash;
  std::hash<int> int_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash + int_hash(modality_) + int_hash(power_) +
         subformula_->hash();
}