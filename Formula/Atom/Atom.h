#ifndef ATOM_H
#define ATOM_H

#include "../../Defines/Defines.h"
#include "../FEnum/FEnum.h"
#include "../Formula/Formula.h"
#include "../Not/Not.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

class Atom : public Formula, public enable_shared_from_this<Atom> {
private:
  string name_;

public:
  Atom(string name);
  ~Atom();

  string getName() const;
  FormulaType getType() const;

  string toString() const;

  shared_ptr<Formula> negatedNormalForm();
  shared_ptr<Formula> negate();
  shared_ptr<Formula> simplify();
  shared_ptr<Formula> modalFlatten();

  shared_ptr<Formula> clone() const;

  bool isPrimitive() const;

  static shared_ptr<Formula> create(string name);

  bool operator==(const Formula &other) const;
  bool operator!=(const Formula &other) const;

  size_t hash() const;
};

#endif