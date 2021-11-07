#ifndef NOT_H
#define NOT_H

#include "../../Defines/Defines.h"
#include "../FEnum/FEnum.h"
#include "../Formula/Formula.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

class Not : public Formula, public enable_shared_from_this<Not> {
private:
  shared_ptr<Formula> subformula_;

public:
  Not(shared_ptr<Formula> subformula);
  ~Not();

  shared_ptr<Formula> getSubformula() const;

  string toString() const;
  FormulaType getType() const;

  shared_ptr<Formula> negatedNormalForm();
  shared_ptr<Formula> negate();
  shared_ptr<Formula> simplify();
  shared_ptr<Formula> modalFlatten();

  shared_ptr<Formula> clone() const;

  bool isPrimitive() const;

  static shared_ptr<Formula> create(shared_ptr<Formula> subformula);

  bool operator==(const Formula &other) const;
  bool operator!=(const Formula &other) const;

  size_t hash() const;
};

#endif