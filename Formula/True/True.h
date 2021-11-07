#ifndef TRUE_H
#define TRUE_H

#include "../../Defines/Defines.h"
#include "../FEnum/FEnum.h"
#include "../False/False.h"
#include "../Formula/Formula.h"
#include <functional>
#include <iostream>
#include <string>

using namespace std;

class True : public Formula, public enable_shared_from_this<True> {
public:
  True();
  ~True();

  string toString() const;
  FormulaType getType() const;

  shared_ptr<Formula> negatedNormalForm();
  shared_ptr<Formula> negate();
  shared_ptr<Formula> simplify();
  shared_ptr<Formula> modalFlatten();

  shared_ptr<Formula> clone() const;

  bool isPrimitive() const;

  static shared_ptr<Formula> create();

  bool operator==(const Formula &other) const;
  bool operator!=(const Formula &other) const;

  size_t hash() const;
};

#endif