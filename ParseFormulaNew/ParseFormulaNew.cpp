#include "ParseFormulaNew.h"

ParseFormulaNew::ParseFormulaNew(string filename) : formulaFile(filename) {
  if (!formulaFile.good()) {
    throw runtime_error("Invalid file: " + filename);
  }

  formulaFile.read(buffer1, bufferSize);
  formulaFile.read(buffer2, bufferSize);
}

ParseFormulaNew::~ParseFormulaNew() {
  formulaFile.close();
  delete[] buffer1;
  delete[] buffer2;
}

bool ParseFormulaNew::compare(string comparison) {

  for (size_t i = 0; i < comparison.size(); i++) {
    if (getChar() != comparison.at(i)) {
      index -= i;

      return false;
    }
    index++;
  }

  return true;
}

char ParseFormulaNew::getChar() {

  if (index < 0) {

    return *(buffer1 + bufferSize + index);
  } else if (index < bufferSize) {

    return *(buffer2 + index);
  }
  char *temp = buffer1;
  buffer1 = buffer2;
  buffer2 = temp;
  formulaFile.read(buffer2, bufferSize);
  index -= bufferSize;

  return getChar();
}

shared_ptr<Formula> ParseFormulaNew::parseRest() {

  while (getChar() == ' ')
    ++index;

  if (getChar() == '(') {
    ++index;
    shared_ptr<Formula> inside = parseIff();

    while (getChar() == ' ')
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
  } else if (compare("$true")) {

    return True::create();
  } else if (compare("$false")) {

    return False::create();
  } else if (isalnum(getChar()) || getChar() == '_') {
    string name = "";

    while (isalnum(getChar()) || getChar() == '_') {
      name += getChar();
      ++index;
    }

    return Atom::create(name);
  }

  while (getChar() != '\n') {
    index++;
  }
  assert(false);
}

shared_ptr<Formula> ParseFormulaNew::parseAnd() {

  shared_ptr<Formula> left = parseRest();

  while (getChar() == ' ')
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

shared_ptr<Formula> ParseFormulaNew::parseOr() {

  shared_ptr<Formula> left = parseAnd();

  while (getChar() == ' ')
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

shared_ptr<Formula> ParseFormulaNew::parseImp() {

  shared_ptr<Formula> left = parseOr();
  while (getChar() == ' ')
    ++index;

  if (compare("=>")) {
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

shared_ptr<Formula> ParseFormulaNew::parseIff() {

  shared_ptr<Formula> left = parseImp();

  while (getChar() == ' ')
    ++index;

  if (compare("<=>")) {
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

shared_ptr<Formula> ParseFormulaNew::parseFormula() {

  // shared_ptr<Formula> formula = parseIff();
  vector<ParseMode> callStack;
  vector<shared_ptr<Formula>> resultStack;
  vector<string> modalityStack;
  callStack.push_back(ParseIff);
  while (callStack.size() > 0) {
    ParseMode mode = callStack.back();
    callStack.pop_back();
    switch (mode) {
    case ParseRemoveSpace:
      while (getChar() == ' ')
        ++index;
      break;
    case ParseIff:
      callStack.push_back(ParseIffCore);
      callStack.push_back(ParseRemoveSpace);
      callStack.push_back(ParseImp);
      break;
    case ParseIffCore:
      if (compare("<=>")) {
        callStack.push_back(ParseIffEnd);
        callStack.push_back(ParseIff);
      }
      break;
    case ParseIffEnd: {
      shared_ptr<Formula> right = resultStack.back();
      resultStack.pop_back();
      shared_ptr<Formula> left = resultStack.back();
      resultStack.pop_back();

      formula_set orSet;

      formula_set andSet;
      formula_set andNotSet;

      andSet.insert(left);
      andSet.insert(right);

      andNotSet.insert(Not::create(left));
      andNotSet.insert(Not::create(right));

      orSet.insert(And::create(andSet));
      orSet.insert(And::create(andNotSet));

      resultStack.push_back(Or::create(orSet));
    } break;
    case ParseImp:
      callStack.push_back(ParseImpCore);
      callStack.push_back(ParseRemoveSpace);
      callStack.push_back(ParseOr);
      break;
    case ParseImpCore:
      if (compare("=>")) {
        callStack.push_back(ParseImpEnd);
        callStack.push_back(ParseImp);
      }
      break;
    case ParseImpEnd: {
      shared_ptr<Formula> right = resultStack.back();
      resultStack.pop_back();
      shared_ptr<Formula> left = resultStack.back();
      resultStack.pop_back();

      formula_set orSet;

      if (right->getType() == FOr) {
        Or *rightOr = dynamic_cast<Or *>(right.get());
        rightOr->addSubformula(Not::create(left));
        resultStack.push_back(right);
      } else {
        orSet.insert(Not::create(left));
        orSet.insert(right);

        resultStack.push_back(Or::create(orSet));
      }
    } break;
    case ParseOr:
      callStack.push_back(ParseOrCore);
      callStack.push_back(ParseRemoveSpace);
      callStack.push_back(ParseAnd);
      break;
    case ParseOrCore:
      if (getChar() == '|') {
        ++index;
        callStack.push_back(ParseOrEnd);
        callStack.push_back(ParseOr);
      }
      break;
    case ParseOrEnd: {
      shared_ptr<Formula> right = resultStack.back();
      resultStack.pop_back();
      shared_ptr<Formula> left = resultStack.back();
      resultStack.pop_back();

      if (left->getType() == FOr && right->getType() == FOr) {
        Or *leftOr = dynamic_cast<Or *>(left.get());
        Or *rightOr = dynamic_cast<Or *>(right.get());
        if (rightOr->getLength() < leftOr->getLength()) {
          for (auto rightSub : *rightOr->getSubformulasReference()) {
            leftOr->addSubformula(rightSub);
          }
          resultStack.push_back(left);
        } else {
          for (auto leftSub : *leftOr->getSubformulasReference()) {
            rightOr->addSubformula(leftSub);
          }
          resultStack.push_back(right);
        }
      } else if (left->getType() == FOr) {
        Or *leftOr = dynamic_cast<Or *>(left.get());
        leftOr->addSubformula(right);
        resultStack.push_back(left);
      } else if (right->getType() == FOr) {
        Or *rightOr = dynamic_cast<Or *>(right.get());
        rightOr->addSubformula(left);
        resultStack.push_back(right);
      } else {
        formula_set orSet;

        orSet.insert(left);
        orSet.insert(right);

        resultStack.push_back(Or::create(orSet));
      }
    } break;
    case ParseAnd:
      callStack.push_back(ParseAndCore);
      callStack.push_back(ParseRemoveSpace);
      callStack.push_back(ParseRest);
      break;
    case ParseAndCore:
      if (getChar() == '&') {
        ++index;
        callStack.push_back(ParseAndEnd);
        callStack.push_back(ParseAnd);
      }
      break;
    case ParseAndEnd: {
      shared_ptr<Formula> right = resultStack.back();
      resultStack.pop_back();
      shared_ptr<Formula> left = resultStack.back();
      resultStack.pop_back();

      if (left->getType() == FAnd && right->getType() == FAnd) {
        And *leftAnd = dynamic_cast<And *>(left.get());
        And *rightAnd = dynamic_cast<And *>(right.get());
        if (rightAnd->getLength() < leftAnd->getLength()) {
          for (auto rightSub : *rightAnd->getSubformulasReference()) {
            leftAnd->addSubformula(rightSub);
          }
          resultStack.push_back(left);
        } else {
          for (auto leftSub : *leftAnd->getSubformulasReference()) {
            rightAnd->addSubformula(leftSub);
          }
          resultStack.push_back(right);
        }
      } else if (left->getType() == FAnd) {
        And *leftAnd = dynamic_cast<And *>(left.get());
        leftAnd->addSubformula(right);
        resultStack.push_back(left);
      } else if (right->getType() == FAnd) {
        And *rightAnd = dynamic_cast<And *>(right.get());
        rightAnd->addSubformula(left);
        resultStack.push_back(right);
      } else {
        formula_set andSet;

        andSet.insert(left);
        andSet.insert(right);

        resultStack.push_back(And::create(andSet));
      }
    } break;
    case ParseRestBracketEnd:
      while (getChar() == ' ')
        ++index;

      assert(getChar() == ')');
      ++index;
      break;
    case ParseRestBoxEnd: {
      string modality = modalityStack.back();
      modalityStack.pop_back();
      shared_ptr<Formula> rest = resultStack.back();
      resultStack.pop_back();

      if (modality == "") {
        if (rest->getType() == FBox) {
          Box *restBox = dynamic_cast<Box *>(rest.get());
          if (restBox->getModality() == 1) {
            restBox->incrementPower();
            resultStack.push_back(rest);
          }
        } else {
          resultStack.push_back(Box::create(1, 1, rest));
        }
      } else {
        if (rest->getType() == FBox) {
          Box *restBox = dynamic_cast<Box *>(rest.get());
          if (restBox->getModality() == stoi(modality)) {
            restBox->incrementPower();
            resultStack.push_back(rest);
          }
        } else {

          resultStack.push_back(Box::create(stoi(modality), 1, rest));
        }
      }
    } break;
    case ParseRestDiamondEnd: {
      string modality = modalityStack.back();
      modalityStack.pop_back();
      shared_ptr<Formula> rest = resultStack.back();
      resultStack.pop_back();

      if (modality == "") {
        if (rest->getType() == FDiamond) {
          Diamond *restDiamond = dynamic_cast<Diamond *>(rest.get());
          if (restDiamond->getModality() == 1) {
            restDiamond->incrementPower();
            resultStack.push_back(rest);
          }
        } else {
          resultStack.push_back(Diamond::create(1, 1, rest));
        }
      } else {
        if (rest->getType() == FDiamond) {
          Diamond *restDiamond = dynamic_cast<Diamond *>(rest.get());
          if (restDiamond->getModality() == stoi(modality)) {
            restDiamond->incrementPower();
            resultStack.push_back(rest);
          }
        } else {
          resultStack.push_back(Diamond::create(stoi(modality), 1, rest));
        }
      }
    } break;
    case ParseRestNotEnd: {
      shared_ptr<Formula> rest = resultStack.back();
      resultStack.pop_back();

      resultStack.push_back(Not::create(rest));
    } break;
    case ParseRest: {
      while (getChar() == ' ')
        ++index;
      if (getChar() == '(') {
        ++index;
        callStack.push_back(ParseRestBracketEnd);
        callStack.push_back(ParseIff);
      } else if (getChar() == '[') {
        ++index;
        string modality = "";

        while (isdigit(getChar())) {
          modality += getChar();
          ++index;
        }

        assert(getChar() == ']');
        ++index;

        modalityStack.push_back(modality);

        callStack.push_back(ParseRestBoxEnd);
        callStack.push_back(ParseRest);
      } else if (getChar() == '<') {
        ++index;
        string modality = "";

        while (isdigit(getChar())) {
          modality += getChar();
          ++index;
        }

        assert(getChar() == '>');
        ++index;

        modalityStack.push_back(modality);

        callStack.push_back(ParseRestDiamondEnd);
        callStack.push_back(ParseRest);

      } else if (getChar() == '~') {
        ++index;
        callStack.push_back(ParseRestNotEnd);
        callStack.push_back(ParseRest);
      } else if (compare("$true")) {
        resultStack.push_back(True::create());
      } else if (compare("$false")) {
        resultStack.push_back(False::create());
      } else if (isalnum(getChar()) || getChar() == '_') {
        string name = "";

        while (isalnum(getChar()) || getChar() == '_') {
          name += getChar();
          ++index;
        }
        resultStack.push_back(Atom::create(name));
      } else {
        while (getChar() != '\n') {
          index++;
        }
        assert(false);
      }
    } break;
    }
  }

  while (getChar() == ' ')
    ++index;

  assert(getChar() == '\n');
  assert(resultStack.size() == 1);

  return resultStack.back();
}