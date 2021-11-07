#include "ParseFormula.h"
#include <string.h>
#include <string>

ParseFormula::ParseFormula(string *str) { 
  ifstream formulaFile(*str);
  
  getline(formulaFile,s); 
  file = &s;
  formulaFile.close();
}

ParseFormula::~ParseFormula() { // Deletion happens in main
  file = nullptr;
}

char ParseFormula::getChar() {
  if (index < file->size()) {
    return file->at(index);
  }
  return '%';
}

shared_ptr<Formula> ParseFormula::parseRest() {
  while (isspace(getChar()))
    ++index;

  if (getChar() == '(') {
    ++index;
    shared_ptr<Formula> inside = parseIff();

    while (isspace(getChar()))
      ++index;

    assert(getChar() == ')');
    ++index;

    return inside;
  } else if (getChar() == '[') {
    ++index;
    string modality = "";

    while (isdigit(getChar())) {
      modality += getChar();
      ++index;
    }

    assert(getChar() == ']');
    ++index;

    shared_ptr<Formula> rest = parseRest();

    if (modality == "") {
      if (rest->getType() == FBox) {
        Box *restBox = dynamic_cast<Box *>(rest.get());
        if (restBox->getModality() == 1) {
          restBox->incrementPower();
          return rest;
        }
      }
      return Box::create(1, 1, rest);
    } else {
      if (rest->getType() == FBox) {
        Box *restBox = dynamic_cast<Box *>(rest.get());
        if (restBox->getModality() == stoi(modality)) {
          restBox->incrementPower();
          return rest;
        }
      }
      return Box::create(stoi(modality), 1, rest);
    }
  } else if (getChar() == '<') {
    ++index;
    string modality = "";

    while (isdigit(getChar())) {
      modality += getChar();
      ++index;
    }

    assert(getChar() == '>');
    ++index;

    shared_ptr<Formula> rest = parseRest();

    if (modality == "") {
      if (rest->getType() == FDiamond) {
        Diamond *restDiamond = dynamic_cast<Diamond *>(rest.get());
        if (restDiamond->getModality() == 1) {
          restDiamond->incrementPower();
          return rest;
        }
      }
      return Diamond::create(1, 1, rest);
    } else {
      if (rest->getType() == FDiamond) {
        Diamond *restDiamond = dynamic_cast<Diamond *>(rest.get());
        if (restDiamond->getModality() == stoi(modality)) {
          restDiamond->incrementPower();
          return rest;
        }
      }
      return Diamond::create(stoi(modality), 1, rest);
    }
  } else if (getChar() == '~') {
    ++index;
    return Not::create(parseRest());
  } else if (file->substr(index, 5) == "$true") {
    index += 5;
    return True::create();
  } else if (file->substr(index, 6) == "$false") {
    index += 6;
    return False::create();
  } else if (isalnum(getChar()) || getChar() == '_') {
    string name = "";

    while (isalnum(getChar()) || getChar() == '_') {
      name += getChar();
      ++index;
    }
    return Atom::create(name);
  }

  while (getChar() != '%') {
    index++;
  }
  assert(false);
}

shared_ptr<Formula> ParseFormula::parseAnd() {
  shared_ptr<Formula> left = parseRest();

  while (isspace(getChar()))
    ++index;

  if (getChar() == '&') {
    ++index;
    shared_ptr<Formula> right = parseAnd();

    if (left->getType() == FAnd && right->getType() == FAnd) {
      And *leftAnd = dynamic_cast<And *>(left.get());
      And *rightAnd = dynamic_cast<And *>(right.get());
      if (rightAnd->getLength() < leftAnd->getLength()) {
        for (auto rightSub : *rightAnd->getSubformulasReference()) {
          leftAnd->addSubformula(rightSub);
        }
        return left;
      } else {
        for (auto leftSub : *leftAnd->getSubformulasReference()) {
          rightAnd->addSubformula(leftSub);
        }
        return right;
      }
    } else if (left->getType() == FAnd) {
      And *leftAnd = dynamic_cast<And *>(left.get());
      leftAnd->addSubformula(right);
      return left;
    } else if (right->getType() == FAnd) {
      And *rightAnd = dynamic_cast<And *>(right.get());
      rightAnd->addSubformula(left);
      return right;
    } else {
      formula_set andSet;

      andSet.insert(left);
      andSet.insert(right);

      return And::create(andSet);
    }
  }
  return left;
}

shared_ptr<Formula> ParseFormula::parseOr() {
  shared_ptr<Formula> left = parseAnd();

  while (isspace(getChar()))
    ++index;

  if (getChar() == '|') {
    ++index;
    shared_ptr<Formula> right = parseOr();

    if (left->getType() == FOr && right->getType() == FOr) {
      Or *leftOr = dynamic_cast<Or *>(left.get());
      Or *rightOr = dynamic_cast<Or *>(right.get());
      if (rightOr->getLength() < leftOr->getLength()) {
        for (auto rightSub : *rightOr->getSubformulasReference()) {
          leftOr->addSubformula(rightSub);
        }
        return left;
      } else {
        for (auto leftSub : *leftOr->getSubformulasReference()) {
          rightOr->addSubformula(leftSub);
        }
        return right;
      }
    } else if (left->getType() == FOr) {
      Or *leftOr = dynamic_cast<Or *>(left.get());
      leftOr->addSubformula(right);
      return left;
    } else if (right->getType() == FOr) {
      Or *rightOr = dynamic_cast<Or *>(right.get());
      rightOr->addSubformula(left);
      return right;
    } else {
      formula_set orSet;

      orSet.insert(left);
      orSet.insert(right);

      return Or::create(orSet);
    }
  }
  return left;
}

shared_ptr<Formula> ParseFormula::parseImp() {
  shared_ptr<Formula> left = parseOr();
  while (isspace(getChar()))
    ++index;

  if (file->substr(index, 2) == "=>") {
    index += 2;
    shared_ptr<Formula> right = parseImp();

    formula_set orSet;

    if (right->getType() == FOr) {
      Or *rightOr = dynamic_cast<Or *>(right.get());
      rightOr->addSubformula(Not::create(left));
      return right;
    }
    orSet.insert(Not::create(left));
    orSet.insert(right);

    return Or::create(orSet);
  }
  return left;
}

shared_ptr<Formula> ParseFormula::parseIff() {
  shared_ptr<Formula> left = parseImp();

  while (isspace(getChar()))
    ++index;

  if (file->substr(index, 3) == "<=>") {
    index += 3;
    shared_ptr<Formula> right = parseIff();

    formula_set orSet;

    formula_set andSet;
    formula_set andNotSet;

    andSet.insert(left);
    andSet.insert(right);

    andNotSet.insert(Not::create(left));
    andNotSet.insert(Not::create(right));

    orSet.insert(And::create(andSet));
    orSet.insert(And::create(andNotSet));

    return Or::create(orSet);
  }
  return left;
}

shared_ptr<Formula> ParseFormula::parseFormula() {
  shared_ptr<Formula> formula = parseIff();

  while (isspace(getChar()))
    ++index;

  // assert(getChar() == '%');

  return formula;
}