#ifndef PARSE_FORMULA_NEW_H
#define PARSE_FORMULA_NEW_H

#include "../Formula/And/And.h"
#include "../Formula/Atom/Atom.h"
#include "../Formula/Box/Box.h"
#include "../Formula/Diamond/Diamond.h"
#include "../Formula/False/False.h"
#include "../Formula/Formula/Formula.h"
#include "../Formula/Not/Not.h"
#include "../Formula/Or/Or.h"
#include "../Formula/True/True.h"
#include <assert.h>
#include <fstream>
#include <memory>
#include <queue>
#include <stdio.h>
#include <string>

using namespace std;

enum ParseMode {
  ParseRemoveSpace,
  ParseIff,
  ParseIffCore,
  ParseIffEnd,
  ParseImp,
  ParseImpCore,
  ParseImpEnd,
  ParseOr,
  ParseOrCore,
  ParseOrEnd,
  ParseAnd,
  ParseAndCore,
  ParseAndEnd,
  ParseRest,
  ParseRestBracketEnd,
  ParseRestBoxEnd,
  ParseRestDiamondEnd,
  ParseRestNotEnd,
};

class ParseFormulaNew {
private:
  shared_ptr<Formula> parseIff();
  shared_ptr<Formula> parseImp();
  shared_ptr<Formula> parseOr();
  shared_ptr<Formula> parseAnd();
  shared_ptr<Formula> parseRest();

  char getChar();

  bool compare(string comparison);

  ifstream formulaFile;
  static constexpr int bufferSize = 1024 * 1024;

  char *buffer1 = new char[bufferSize];
  char *buffer2 = new char[bufferSize];

  int index = -bufferSize;

public:
  ParseFormulaNew(string filename);
  ~ParseFormulaNew();

  shared_ptr<Formula> parseFormula();
};

#endif