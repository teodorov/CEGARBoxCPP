#ifndef FALSE_H
#define FALSE_H

#include "../../Defines/Defines.h"
#include "../FEnum/FEnum.h"
#include "../Formula/Formula.h"
#include "../True/True.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

class False : public Formula, public enable_shared_from_this<False> {
public:
  False();
  ~False();

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