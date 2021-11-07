#include "Literal.h"

Literal::Literal(string name_, bool polarity_) {
  name = name_;
  polarity = polarity_;
}

Literal::~Literal() {}

string Literal::getName() const { return name; }
bool Literal::getPolarity() const { return polarity; }

bool Literal::operator==(const Literal &other) const {
  return name == other.name && polarity == other.polarity;
}

bool Literal::operator!=(const Literal &other) const {
  return !(operator==(other));
}

Literal Literal::operator~() const { return Literal(name, !polarity); }

size_t Literal::hash() const {
  std::hash<string> stringHasher;
  return stringHasher(name) + ((size_t)polarity);
}

bool isSubsetOf(literal_set set1, literal_set set2) {
  if (set1.size() > set2.size()) {
    return false;
  }
  for (Literal literal : set1) {
    if (set2.find(literal) == set2.end()) {
      return false;
    }
  }
  return true;
}

literal_set setDifference(literal_set set1, literal_set set2) {
  literal_set difference;
  for (Literal literal : set1) {
    if (set2.find(literal) == set2.end()) {
      difference.insert(literal);
    }
  }
  return difference;
}