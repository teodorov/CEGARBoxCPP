#ifndef LITERAL_H
#define LITERAL_H

#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class Literal {
private:
  string name;
  bool polarity;

public:
  Literal(string name_, bool polarity_);
  ~Literal();

  string getName() const;
  bool getPolarity() const;

  bool operator==(const Literal &other) const;
  bool operator!=(const Literal &other) const;

  Literal operator~() const;

  size_t hash() const;
};

struct LiteralHash {
  std::size_t operator()(Literal const &l) const { return l.hash(); }
};
struct LiteralEqual {
  size_t operator()(Literal const &a, Literal const &b) const { return a == b; }
};

typedef unordered_set<Literal, LiteralHash, LiteralEqual> literal_set;
typedef unordered_map<int, literal_set> modal_literal_map;
typedef unordered_map<Literal, literal_set, LiteralHash, LiteralEqual>
    lit_implication;
typedef unordered_map<int, lit_implication> modal_lit_implication;

bool isSubsetOf(literal_set set1, literal_set set2);
literal_set setDifference(literal_set set1, literal_set set2);

struct Solution {
  bool satisfiable;
  literal_set conflict;
};

#endif