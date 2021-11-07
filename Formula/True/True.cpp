#include "True.h"

True::True() {}
True::~True() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING TRUE" << endl;
#endif
}

string True::toString() const { return "$true"; }

FormulaType True::getType() const { return FTrue; }

shared_ptr<Formula> True::negatedNormalForm() { return shared_from_this(); }

shared_ptr<Formula> True::negate() { return False::create(); }

shared_ptr<Formula> True::simplify() { return shared_from_this(); }

shared_ptr<Formula> True::modalFlatten() { return shared_from_this(); }

shared_ptr<Formula> True::create() { return shared_ptr<Formula>(new True()); }

shared_ptr<Formula> True::clone() const { return create(); }

bool True::operator==(const Formula &other) const {
  return other.getType() == getType();
}

bool True::operator!=(const Formula &other) const { return !(operator==(other)); }

size_t True::hash() const {
  std::hash<FormulaType> ftype_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash;
}

bool True::isPrimitive() const { return true; }