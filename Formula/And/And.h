#ifndef AND_H
#define AND_H

#include "../../Defines/Defines.h"
#include "../Box/Box.h"
#include "../FEnum/FEnum.h"
#include "../False/False.h"
#include "../Formula/Formula.h"
#include "../Or/Or.h"
#include "../True/True.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class And : public Formula, public enable_shared_from_this<And> {
private:
  formula_set andSet_;

public:
  And(const formula_set &andSet);
  ~And();

  formula_set getSubformulas() const;
  const formula_set *getSubformulasReference() const;

  void addSubformula(const shared_ptr<Formula> &subformula);
  int getLength() const;

  string toString() const;
  FormulaType getType() const;

  shared_ptr<Formula> negatedNormalForm();
  shared_ptr<Formula> negate();
  shared_ptr<Formula> simplify();
  shared_ptr<Formula> modalFlatten();

  shared_ptr<Formula> clone() const;

  static shared_ptr<Formula> create(formula_set andList);

  bool operator==(const Formula &other) const;
  bool operator!=(const Formula &other) const;

  size_t hash() const;
};

#endif