#include "TrieformProverB.h"

unsigned int TrieformProverB::assumptionsSize = 0;
GlobalSolutionMemo TrieformProverB::globalMemo = GlobalSolutionMemo();
unordered_map<string, unsigned int> TrieformProverB::idMap =
    unordered_map<string, unsigned int>();

shared_ptr<Trieform>
TrieformFactory::makeTrieB(const shared_ptr<Formula> &formula,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverB());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieB(const shared_ptr<Formula> &formula,
                           const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverB());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieB(const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverB());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverB::TrieformProverB() {}
TrieformProverB::~TrieformProverB() {}

shared_ptr<Trieform>
TrieformProverB::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieB(formula, shared_from_this());
}
shared_ptr<Trieform> TrieformProverB::create(const shared_ptr<Formula> &formula,
                                             const vector<int> &newModality) {
  return TrieformFactory::makeTrieB(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverB::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieB(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverB::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverB::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                         Solution solution) {
  if (solution.satisfiable) {
    globalMemo.insertSat(assumptions, modality);
  } else {
    globalMemo.insertUnsat(assumptions, solution.conflict, modality);
  }
}

void TrieformProverB::handleReflexiveBoxClauses() {
  for (ModalClause modalClause : clauses.getBoxClauses()) {
    formula_set newOr;
    newOr.insert(Not::create(modalClause.left)->negatedNormalForm());
    newOr.insert(modalClause.right);
    clauses.addClause(Or::create(newOr));
  }
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverB *>(modalSubtrie.second.get())
        ->handleReflexiveBoxClauses();
  }
}

void TrieformProverB::propagateLevels() {
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverB *>(modalSubtrie.second.get())
        ->propagateLevels();
    overShadow(modalSubtrie.second, modalSubtrie.first);
  }
}

void TrieformProverB::propagateSymmetricBoxes() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverB *>(modalitySubtrie.second.get())
        ->propagateSymmetricBoxes();
  }
  for (auto modalitySubtrie : subtrieMap) {
    for (const ModalClause &boxClause :
         modalitySubtrie.second->getClauses().getBoxClauses()) {

      if (modalitySubtrie.first == boxClause.modality) {
        // a->[]b on next level means ~b=>[]~a and ~b=>~a here
        clauses.addBoxClause(boxClause.modality, boxClause.right->negate(),
                             boxClause.left->negate());
        formula_set reflexiveSet = formula_set();
        reflexiveSet.insert(boxClause.right);
        reflexiveSet.insert(boxClause.left->negate());
        clauses.addClause(Or::create(reflexiveSet));
      }
    }
  }
}

void TrieformProverB::preprocess() {
  handleReflexiveBoxClauses();
  propagateLevels();
  propagateSymmetricBoxes();
}

void TrieformProverB::prepareSAT(name_set extra) {
  for (string name : extra) {
    if (idMap.find(name) == idMap.end()) {
      idMap[name] = assumptionsSize++;
    }
  }
  modal_names_map modalExtras = prover->prepareSAT(clauses, extra);
  for (auto modalSubtrie : subtrieMap) {
    modalSubtrie.second->prepareSAT(modalExtras[modalSubtrie.first]);
  }
}

Solution TrieformProverB::prove(literal_set assumptions) {
  // Check solution memo
  shared_ptr<Bitset> assumptionsBitset =
      convertAssumptionsToBitset(assumptions);
  GlobalSolutionMemoResult memoResult =
      globalMemo.getFromMemo(assumptionsBitset, modality);

  if (memoResult.inSatMemo) {
    return memoResult.result;
  }

  // Solve locally
  Solution solution = prover->solve(assumptions);

  if (!solution.satisfiable) {
    updateSolutionMemo(assumptionsBitset, solution);
    return solution;
  }

  prover->calculateTriggeredDiamondsClauses();
  modal_literal_map triggeredDiamonds = prover->getTriggeredDiamondClauses();

  // If there are no fired diamonds, it is satisfiable
  if (triggeredDiamonds.size() == 0) {
    updateSolutionMemo(assumptionsBitset, solution);
    return solution;
  }

  prover->calculateTriggeredBoxClauses();
  modal_literal_map triggeredBoxes = prover->getTriggeredBoxClauses();

  for (auto modalitySubtrie : subtrieMap) {
    // Handle each modality
    if (triggeredDiamonds[modalitySubtrie.first].size() == 0) {
      // If there are no triggered diamonds of a certain modality we can skip it
      continue;
    }

    // Note in the cases where the diamonds are a subset of the boxes it is
    // already satisfied by reflexivity

    diamond_queue diamondPriority =
        prover->getPrioritisedTriggeredDiamonds(modalitySubtrie.first);

    while (!diamondPriority.empty()) {
      // Create a world for each diamond
      Literal diamond = diamondPriority.top().literal;
      diamondPriority.pop();

      // If the current world satisfies the diamond, or the parent is of the
      // same modality and satisfies the diamond there is no need to create a
      // new world
      if (prover->modelSatisfiesAssump(diamond) ||
          (modality.size() > 0 &&
           modality[modality.size() - 1] == modalitySubtrie.first &&
           parentSatisfiesAssump(diamond))) {
        continue;
      }

      literal_set childAssumptions =
          literal_set(triggeredBoxes[modalitySubtrie.first]);
      childAssumptions.insert(diamond);

      // Run the solver for the next level
      Solution childSolution = modalitySubtrie.second->prove(childAssumptions);

      // If it is satisfiable create the next world
      if (childSolution.satisfiable) {
        continue;
      }

      // Otherwise there must have been a conflict
      vector<literal_set> badImplications = prover->getNotProblemBoxClauses(
          modalitySubtrie.first, childSolution.conflict);

      if (childSolution.conflict.find(diamond) !=
          childSolution.conflict.end()) {
        // The diamond clause, either on its own or together with box clauses,
        // caused a conflict. We must add diamond implies OR NOT problem
        // box clauses.
        prover->updateLastFail(diamond);
        badImplications.push_back(
            prover->getNotDiamondLeft(modalitySubtrie.first, diamond));

        for (literal_set learnClause : generateClauses(badImplications)) {
          prover->addClause(learnClause);
        }

        // Find new result
        return prove(assumptions);
      } else {
        // Only the box clauses caused a conflict, so we must add each diamond
        // clause implies OR NOT problem box lefts
        badImplications.push_back(
            prover->getNotAllDiamondLeft(modalitySubtrie.first));
        // Add ~leftDiamond=>\/~leftProbemBox
        for (literal_set learnClause : generateClauses(badImplications)) {
          prover->addClause(learnClause);
        }
        // Find new result
        return prove(assumptions);
      }
    }
  }
  // If we reached here the solution is satisfiable under all modalities
  updateSolutionMemo(assumptionsBitset, solution);
  return solution;
}