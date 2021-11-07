#include "Atom.h"

Atom::Atom(string name) { name_ = name; }
Atom::~Atom() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING ATOM " << name_ << endl;
#endif
}

string Atom::getName() const { return name_; }

string Atom::toString() const { return name_; }
FormulaType Atom::getType() const { return FAtom; }

shared_ptr<Formula> Atom::negatedNormalForm() { return shared_from_this(); }

shared_ptr<Formula> Atom::negate() { return Not::create(shared_from_this()); }

shared_ptr<Formula> Atom::simplify() { return shared_from_this(); }

shared_ptr<Formula> Atom::modalFlatten() { return shared_from_this(); }

shared_ptr<Formula> Atom::create(string name) {
  return shared_ptr<Formula>(new Atom(name));
}

shared_ptr<Formula> Atom::clone() const { return create(name_); }

bool Atom::operator==(const Formula &other) const {
  if (other.getType() != getType()) {
    return false;
  }
  const Atom *otherAtom = dynamic_cast<const Atom *>(&other);
  return otherAtom->name_ == name_;
}

bool Atom::operator!=(const Formula &other) const {
  return !(operator==(other));
}

size_t Atom::hash() const {
  std::hash<FormulaType> ftype_hash;
  std::hash<string> string_hash;
  size_t totalHash = ftype_hash(getType());
  return totalHash + string_hash(name_);
}

bool Atom::isPrimitive() const { return true; }