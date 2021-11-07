#include "TrieformProverS5.h"

Cache TrieformProverS5::persistentCache = Cache("P");

unsigned int TrieformProverS5::assumptionsSize = 0;
GlobalSolutionMemo TrieformProverS5::globalMemo = GlobalSolutionMemo();
unordered_map<string, unsigned int> TrieformProverS5::idMap =
    unordered_map<string, unsigned int>();

shared_ptr<Trieform>
TrieformFactory::makeTrieS5(const shared_ptr<Formula> &formula,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverS5());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieS5(const shared_ptr<Formula> &formula,
                            const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverS5());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieS5(const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverS5());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverS5::TrieformProverS5() {}
TrieformProverS5::~TrieformProverS5() {}

shared_ptr<Trieform>
TrieformProverS5::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieS5(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverS5::create(const shared_ptr<Formula> &formula,
                         const vector<int> &newModality) {
  return TrieformFactory::makeTrieS5(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverS5::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieS5(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverS5::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverS5::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                          Solution solution) {
  if (solution.satisfiable) {
    globalMemo.insertSat(assumptions, modality);
  } else {
    globalMemo.insertUnsat(assumptions, solution.conflict, modality);
  }
}

bool TrieformProverS5::isInHistory(vector<shared_ptr<Bitset>> history,
                                   shared_ptr<Bitset> bitset) {
  for (shared_ptr<Bitset> assump : history) {
    if (assump->contains(*bitset)) {
      return true;
    }
  }
  return false;
}

void TrieformProverS5::reflexiveHandleBoxClauses() {
  for (ModalClause modalClause : clauses.getBoxClauses()) {
    formula_set newOr;
    newOr.insert(Not::create(modalClause.left)->negatedNormalForm());
    newOr.insert(modalClause.right);
    clauses.addClause(Or::create(newOr));
  }
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverS5 *>(modalSubtrie.second.get())
        ->reflexiveHandleBoxClauses();
  }
}

void TrieformProverS5::reflexivepropagateLevels() {
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverS5 *>(modalSubtrie.second.get())
        ->reflexivepropagateLevels();
    overShadow(modalSubtrie.second, modalSubtrie.first);
  }
}

void TrieformProverS5::pruneTrie() {
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.second->hasSubtrie(modalSubtrie.first)) {
      modalSubtrie.second->removeSubtrie(modalSubtrie.first);
    }
    dynamic_cast<TrieformProverS5 *>(modalSubtrie.second.get())->pruneTrie();
  }
}

void TrieformProverS5::makePersistence() {
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverS5 *>(modalSubtrie.second.get())
        ->makePersistence();
  }

  modal_clause_set persistentBoxes;
  for (ModalClause boxClause : clauses.getBoxClauses()) {
    // For a=>[]b in our box clauses replace with.
    // a=>Pb
    // Pb=>[]Pb
    // [](Pb=>b)

    // Make persistence (Pb=>[]Pb). Don't need to add Pb=>Pb
    shared_ptr<Formula> persistent =
        persistentCache.getVariableOrCreate(boxClause.right);
    persistentBoxes.insert({boxClause.modality, persistent, persistent});

    // Add a=>Pb
    formula_set leftSet;
    leftSet.insert(Not::create(boxClause.left)->negatedNormalForm());
    leftSet.insert(persistent);
    clauses.addClause(Or::create(leftSet));

    // Add b=>Pb and [](b=>Pb) where appropriate
    formula_set rightSet;
    rightSet.insert(Not::create(persistent)->negatedNormalForm());
    rightSet.insert(boxClause.right);
    shared_ptr<Formula> rightOr = Or::create(rightSet);
    propagateClauses(rightOr);
    if (hasSubtrie(boxClause.modality)) {
      subtrieMap[boxClause.modality]->propagateClauses(rightOr);
    }
  }
  clauses.setBoxClauses(persistentBoxes);

  for (ModalClause persistentBox : persistentBoxes) {
    if (hasSubtrie(persistentBox.modality)) {
      subtrieMap[persistentBox.modality]->clauses.addBoxClause(persistentBox);
    }
  }
}

void TrieformProverS5::propagateSymmetricBoxes() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverS5 *>(modalitySubtrie.second.get())
        ->propagateSymmetricBoxes();
  }
  for (auto modalitySubtrie : subtrieMap) {
    for (const ModalClause &boxClause :
         modalitySubtrie.second->getClauses().getBoxClauses()) {

      if (modalitySubtrie.first == boxClause.modality) {
        clauses.addBoxClause(boxClause.modality, boxClause.right->negate(),
                             boxClause.left->negate());
      }
    }
  }
}

void TrieformProverS5::preprocess() {
  // Apply reflexivity first
  reflexiveHandleBoxClauses();
  reflexivepropagateLevels();
  pruneTrie();
  makePersistence();
  propagateSymmetricBoxes();
}

void TrieformProverS5::prepareSAT(name_set extra) {
  // Shortcut only do this for level 1 as reflexivity guarantees every possible
  // assumption is here. Renaming could stuff this up
  for (string name : extra) {
    if (idMap.find(name) == idMap.end()) {
      idMap[name] = assumptionsSize++;
    }
  }
  for (ModalClause clause : clauses.getDiamondClauses()) {
    extra.insert(prover->getPrimitiveName(clause.right));
  }
  modal_names_map modalExtras = prover->prepareSAT(clauses, extra);
  for (auto modalSubtrie : subtrieMap) {
    modalSubtrie.second->prepareSAT(modalExtras[modalSubtrie.first]);
  }
}

Solution TrieformProverS5::prove(vector<shared_ptr<Bitset>> history,
                                 literal_set assumptions) {
  // Check solution memo
  shared_ptr<Bitset> assumptionsBitset =
      convertAssumptionsToBitset(assumptions);
  GlobalSolutionMemoResult memoResult =
      globalMemo.getFromMemo(assumptionsBitset, modality);

  if (memoResult.inSatMemo) {
    return memoResult.result;
  }
  // If the assumptions are in a higher valuation, connect back so it is
  // satisfiable
  if (isInHistory(history, assumptionsBitset)) {
    return {true, literal_set()};
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

  for (auto modalityDiamonds : triggeredDiamonds) {
    // Handle each modality
    if (modalityDiamonds.second.size() == 0) {
      // If there are no triggered diamonds of a certain modality we can skip it
      continue;
    }
    // Note in the cases diamonds are a subset of boxes then we don't need to
    // create any worlds (reflexivity satisfies this)
    diamond_queue diamondPriority =
        prover->getPrioritisedTriggeredDiamonds(modalityDiamonds.first);
    while (!diamondPriority.empty()) {
      // Create a world for each diamond if necessary
      Literal diamond = diamondPriority.top().literal;
      diamondPriority.pop();

      // If the diamond is already satisfied by reflexivity no need to create
      // a successor.
      if (prover->modelSatisfiesAssump(diamond)) {
        continue;
      }

      literal_set childAssumptions =
          literal_set(triggeredBoxes[modalityDiamonds.first]);
      childAssumptions.insert(diamond);

      // Run the solver for the next level
      history.push_back(assumptionsBitset);
      Solution childSolution;
      if (getLevel() == 1) {
        childSolution = prove(history, childAssumptions);
      } else {
        childSolution = dynamic_cast<TrieformProverS5 *>(
                            subtrieMap[modalityDiamonds.first].get())
                            ->prove(history, childAssumptions);
      }
      history.pop_back();

      // If it is satisfiable create the next world
      if (childSolution.satisfiable) {
        continue;
      }

      // Otherwise there must have been a conflict
      vector<literal_set> badImplications = prover->getNotProblemBoxClauses(
          modalityDiamonds.first, childSolution.conflict);

      if (childSolution.conflict.find(diamond) !=
          childSolution.conflict.end()) {
        // The diamond clause, either on its own or together with box clauses,
        // caused a conflict. We must add diamond implies OR NOT problem box
        // clauses.
        prover->updateLastFail(diamond);
        badImplications.push_back(
            prover->getNotDiamondLeft(modalityDiamonds.first, diamond));

        for (literal_set learnClause : generateClauses(badImplications)) {
          prover->addClause(learnClause);
        }

        // Find new result
        return prove(history, assumptions);
      } else {
        // Should be able to remove this (boxes must be able to satisfied
        // because of reflexivity)
        // Only the box clauses caused a conflict, so
        // we must add each diamond clause implies OR NOT problem box lefts
        badImplications.push_back(
            prover->getNotAllDiamondLeft(modalityDiamonds.first));
        // Add ~leftDiamond=>\/~leftProbemBox
        for (literal_set learnClause : generateClauses(badImplications)) {
          prover->addClause(learnClause);
        }
        // Find new result
        return prove(history, assumptions);
      }
    }
  }
  // If we reached here the solution is satisfiable under all modalities
  updateSolutionMemo(assumptionsBitset, solution);
  return solution;
}

Solution TrieformProverS5::prove(literal_set assumptions = literal_set()) {
  return prove(vector<shared_ptr<Bitset>>(), assumptions);
}