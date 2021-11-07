#ifndef DIAMOND_H
#define DIAMOND_H

#include "../../Defines/Defines.h"
#include "../Box/Box.h"
#include "../FEnum/FEnum.h"
#include "../False/False.h"
#include "../Formula/Formula.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

class Diamond : public Formula, public enable_shared_from_this<Diamond> {
private:
  int modality_, power_;
  shared_ptr<Formula> subformula_;

public:
  Diamond(int modality, int power, shared_ptr<Formula> subformula);
  ~Diamond();

  int getModality() const;
  int getPower() const;
  shared_ptr<Formula> getSubformula() const;

  void incrementPower();

  string toString() const;
  FormulaType getType() const;

  shared_ptr<Formula> negatedNormalForm();
  shared_ptr<Formula> negate();
  shared_ptr<Formula> simplify();
  shared_ptr<Formula> modalFlatten();

  shared_ptr<Formula> clone() const;

  shared_ptr<Formula> constructDiamondReduced() const;

  static shared_ptr<Formula> create(int modality, int power,
                                    shared_ptr<Formula> subformula);
  static shared_ptr<Formula> create(vector<int> modality,
                                    const shared_ptr<Formula> &subformula);

  bool operator==(const Formula &other) const;
  bool operator!=(const Formula &other) const;

  size_t hash() const;
};

#endif