#ifndef PARSE_FORMULA_H
#define PARSE_FORMULA_H

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
#include <memory>
#include <stdio.h>
#include <string>
#include <fstream>

using namespace std;

class ParseFormula {
private:
  shared_ptr<Formula> parseIff();
  shared_ptr<Formula> parseImp();
  shared_ptr<Formula> parseOr();
  shared_ptr<Formula> parseAnd();
  shared_ptr<Formula> parseRest();

  char getChar();

  bool compare(string compareString);
  string s; 
  string *file;
  size_t index = 0;

public:
  ParseFormula(string *str);
  ~ParseFormula();

  shared_ptr<Formula> parseFormula();
};

#endif