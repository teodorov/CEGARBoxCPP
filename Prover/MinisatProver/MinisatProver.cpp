#include "MinisatProver.h"

MinisatProver::MinisatProver() {
  // solver->random_var_freq = 0;
  // solver->rnd_init_act = true;
  // solver->ccmin_mode = 1;
  // solver->clause_decay = 0;
  // solver->var_decay = 0.5;
  // solver->luby_restart = false;
  // solver->
  solver->eliminate(true);
}
MinisatProver::~MinisatProver() {}

modal_names_map MinisatProver::prepareSAT(FormulaTriple clauses,
                                          name_set extra) {
  prepareExtras(extra);
  
  modal_names_map newExtra;
  prepareModalClauses(clauses.getBoxClauses(), newExtra, boxLits, boxFromRight);
  prepareModalClauses(clauses.getDiamondClauses(), newExtra, diamondLits,
                      diamondFromRight);
                      
  prepareFalse();
  prepareClauses(clauses.getClauses());
  return newExtra;
}

void MinisatProver::prepareFalse() {
  solver->addClause(~Minisat::mkLit(createOrGetVariable("$false")));
}

void MinisatProver::prepareExtras(unordered_set<string> extra) {
  for (string name : extra) {
    createOrGetVariable(name);
  }
}

void MinisatProver::prepareClauses(clause_set clauses) {
  for (Clause clause : clauses) {
    if (clause.formula->getType() == FOr) {
      Minisat::vec<Minisat::Lit> literals;
      for (shared_ptr<Formula> subformula :
           dynamic_cast<Or *>(clause.formula.get())->getSubformulas()) {
        literals.push(makeLiteral(subformula));
      }
      solver->addClause(literals);
    } else {
      solver->addClause(makeLiteral(clause.formula));
    }
  }
}

void MinisatProver::prepareModalClauses(modal_clause_set modal_clauses,
                                        modal_names_map &newExtra,
                                        modal_lit_implication &modalLits,
                                        modal_lit_implication &modalFromRight) {
  for (ModalClause clause : modal_clauses) {
    if (clause.left->getType() == FAtom) {
      createOrGetVariable(getPrimitiveName(clause.left),
                          Minisat::lbool((uint8_t)0));
    } else if (clause.left->getType() == FNot) {
      createOrGetVariable(getPrimitiveName(clause.left),
                          Minisat::lbool((uint8_t)1));
    }
    newExtra[clause.modality].insert(getPrimitiveName(clause.right));
    createModalImplication(clause.modality, toLiteral(clause.left),
                           toLiteral(clause.right), modalLits, modalFromRight);
  }
}

Minisat::Var MinisatProver::createOrGetVariable(string name,
                                                Minisat::lbool polarity) {
  if (variableMap.find(name) == variableMap.end()) {
    variableMap[name] = solver->newVar(polarity);
    nameMap[variableMap[name]] = name;
  }
  return variableMap[name];
}

Minisat::Lit MinisatProver::makeLiteral(shared_ptr<Formula> formula) {
  if (formula->getType() == FAtom) {
    string name = dynamic_cast<Atom *>(formula.get())->getName();
    return Minisat::mkLit(createOrGetVariable(name));
  } else if (formula->getType() == FNot) {
    string name = dynamic_cast<Atom *>(
                      dynamic_cast<Not *>(formula.get())->getSubformula().get())
                      ->getName();
    return ~Minisat::mkLit(createOrGetVariable(name));
  }
  throw invalid_argument("Expected Atom or Not but got " + formula->toString());
}

shared_ptr<Minisat::vec<Minisat::Lit>>
MinisatProver::convertAssumptions(literal_set assumptions) {
  shared_ptr<Minisat::vec<Minisat::Lit>> literals =
      shared_ptr<Minisat::vec<Minisat::Lit>>(new Minisat::vec<Minisat::Lit>());
  for (Literal assumption : assumptions) {
    Minisat::Var variable = variableMap[assumption.getName()];
    literals->push(assumption.getPolarity() ? Minisat::mkLit(variable)
                                            : ~Minisat::mkLit(variable));
  }
  return literals;
}

bool MinisatProver::modelSatisfiesAssump(Literal assumption) {
  if (variableMap.find(assumption.getName()) == variableMap.end()) {
    return false;
  }
  int lbool =
      Minisat::toInt(solver->modelValue(variableMap[assumption.getName()]));
  if (lbool == 2) {
    throw runtime_error("Model value for " + assumption.getName() +
                        " is undefined");
  }
  return (lbool == 0 && assumption.getPolarity()) ||
         (lbool == 1 && !assumption.getPolarity());
}

literal_set MinisatProver::convertConflictToAssumps(
    Minisat::vec<Minisat::Lit> &conflictLits) {
  literal_set conflict;
  for (int i = 0; i < conflictLits.size(); i++) {
    conflict.insert(Literal(nameMap[Minisat::var(conflictLits[i])],
                            Minisat::sign(conflictLits[i])));
  }
  return conflict;
}

Solution MinisatProver::solve(const literal_set &assumptions) {
  Solution solution;
  shared_ptr<Minisat::vec<Minisat::Lit>> vecAssumps =
      convertAssumptions(assumptions);
  solution.satisfiable = solver->solve(*vecAssumps);
  if (!solution.satisfiable) {
    solution.conflict = convertConflictToAssumps(solver->conflict);
  }
  return solution;
}

void MinisatProver::addClause(literal_set clause) {
  solver->addClause(*convertAssumptions(clause));
}

void MinisatProver::printModel() {
  for (auto varName : nameMap) {
    cout << varName.second << " is "
         << Minisat::toInt(solver->modelValue(varName.first)) << endl;
  }
}

int MinisatProver::getLiteralId(Literal literal) {
  return variableMap[literal.getName()];
}