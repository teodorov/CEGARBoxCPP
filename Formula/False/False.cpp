#include "False.h"

False::False() {}
False::~False() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING FALSE" << endl;
#endif
}

string False::toString() const { return "$false"; }

FormulaType False::getType() const { return FFalse; }

shared_ptr<Formula> False::negatedNormalForm() { return shared_from_this(); }

shared_ptr<Formula> False::negate() { return True::create(); }

shared_ptr<Formula> False::simplify() { return shared_from_this(); }

shared_ptr<Formula> False::modalFlatten() { return shared_from_this(); }

shared_ptr<Formula> False::create() { return shared_ptr<Formula>(new False()); }

shared_ptr<Formula> False::clone() const { return create(); }

bool False::operator==(const Formula &other) const {
  return other.getType() == getType();
}

bool False::operator!=(const Formula &other) const {
  return !(operator==(other));
}

size_t False::hash() const {
  std::hash<FormulaType> ftype_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash;
}

bool False::isPrimitive() const { return true; }