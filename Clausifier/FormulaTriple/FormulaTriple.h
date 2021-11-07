#ifndef FORMULA_TRIPLE_H
#define FORMULA_TRIPLE_H

#include "../../Formula/Atom/Atom.h"
#include "../../Formula/Formula/Formula.h"
#include "../../Formula/Not/Not.h"
#include <memory>
#include <string>
#include <vector>

struct Clause {
  shared_ptr<Formula> formula;
};

struct ClauseDeref {
  struct Hash {
    std::size_t operator()(Clause const &p) const {
      return (p.formula)->hash();
    }
  };
  struct Compare {
    size_t operator()(Clause const &a, Clause const &b) const {
      return *a.formula == *b.formula;
    }
  };
};

typedef unordered_set<Clause, ClauseDeref::Hash, ClauseDeref::Compare>
    clause_set;

struct ModalClause {
  int modality;
  shared_ptr<Formula> left;
  shared_ptr<Formula> right;
};

struct ModalClauseDeref {
  struct Hash {
    std::size_t operator()(ModalClause const &p) const {
      return (p.left)->hash() + (p.right)->hash() + p.modality;
    }
  };
  struct Compare {
    size_t operator()(ModalClause const &a, ModalClause const &b) const {
      return a.modality == b.modality && *a.left == *b.left &&
             *a.right == *b.right;
    }
  };
};

typedef unordered_set<ModalClause, ModalClauseDeref::Hash,
                      ModalClauseDeref::Compare>
    modal_clause_set;

class FormulaTriple {
private:
  clause_set clauses;
  modal_clause_set boxClauses;
  modal_clause_set diamondClauses;

public:
  FormulaTriple();
  ~FormulaTriple();

  const clause_set &getClauses() const;
  const modal_clause_set &getBoxClauses() const;
  const modal_clause_set &getDiamondClauses() const;

  void setClauses(clause_set newClauses);
  void setBoxClauses(modal_clause_set newBoxClauses);
  void setDiamondClauses(modal_clause_set newDiamondClauses);

  void eraseClauses();
  void eraseBoxClauses();
  void eraseDiamondClauses();

  void addClause(shared_ptr<Formula> formula);
  void addBoxClause(int modality, shared_ptr<Formula> left,
                    shared_ptr<Formula> right);
  void addDiamondClause(int modality, shared_ptr<Formula> left,
                        shared_ptr<Formula> right);
  void addClause(Clause Clause);
  void addBoxClause(ModalClause boxClause);
  void addDiamondClause(ModalClause diamondClause);

  void extendBoxClauses(const modal_clause_set &otherBoxClauses);
  void extendClauses(const FormulaTriple &otherClauses);

  void removeTrueAndFalse();

  vector<string> toStringComponents() const;
};

#endif