#include "Not.h"

Not::Not(shared_ptr<Formula> subformula) { subformula_ = subformula; }
Not::~Not() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING NOT" << endl;
#endif
}

shared_ptr<Formula> Not::getSubformula() const { return subformula_; }

string Not::toString() const { return "~" + subformula_->toString(); }

FormulaType Not::getType() const { return FNot; }

shared_ptr<Formula> Not::negatedNormalForm() { return subformula_->negate(); }
shared_ptr<Formula> Not::negate() { return subformula_->negatedNormalForm(); }

shared_ptr<Formula> Not::simplify() {
  subformula_ = subformula_->simplify();
  return shared_from_this();
}

shared_ptr<Formula> Not::modalFlatten() {
  subformula_ = subformula_->modalFlatten();
  return shared_from_this();
}

shared_ptr<Formula> Not::create(shared_ptr<Formula> subformula) {
  return shared_ptr<Formula>(new Not(subformula));
}

shared_ptr<Formula> Not::clone() const { return create(subformula_->clone()); }

bool Not::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const Not *otherNot = dynamic_cast<const Not *>(&other);
  return *subformula_ == *(otherNot->subformula_);
}

bool Not::operator!=(const Formula &other) const { return !(operator==(other)); }

size_t Not::hash() const {
  std::hash<FormulaType> ftype_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash + subformula_->hash();
}

bool Not::isPrimitive() const { return subformula_->isPrimitive(); }