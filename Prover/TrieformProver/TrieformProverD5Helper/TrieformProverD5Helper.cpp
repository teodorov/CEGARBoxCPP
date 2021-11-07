#include "TrieformProverD5Helper.h"

Cache TrieformProverD5Helper::persistentCache = Cache("P");

unsigned int TrieformProverD5Helper::assumptionsSize = 0;
GlobalSolutionMemo TrieformProverD5Helper::globalMemo = GlobalSolutionMemo();
unordered_map<string, unsigned int> TrieformProverD5Helper::idMap =
    unordered_map<string, unsigned int>();

shared_ptr<Trieform>
TrieformFactory::makeTrieD5Helper(const shared_ptr<Formula> &formula,
                                  shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie =
      shared_ptr<Trieform>(new TrieformProverD5Helper());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD5Helper(const shared_ptr<Formula> &formula,
                                  const vector<int> &newModality,
                                  shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie =
      shared_ptr<Trieform>(new TrieformProverD5Helper());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieD5Helper(const vector<int> &newModality,
                                  shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie =
      shared_ptr<Trieform>(new TrieformProverD5Helper());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverD5Helper::TrieformProverD5Helper() {}
TrieformProverD5Helper::~TrieformProverD5Helper() {}

shared_ptr<Trieform>
TrieformProverD5Helper::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieD5Helper(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverD5Helper::create(const shared_ptr<Formula> &formula,
                               const vector<int> &newModality) {
  if (newModality.size() > 0 &&
      newModality[newModality.size() - 1] == s5Modality) {

    return TrieformFactory::makeTrieD5Helper(formula, newModality,
                                             shared_from_this());
  }
  return TrieformFactory::makeTrieD5(formula, newModality, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverD5Helper::create(const vector<int> &newModality) {
  if (newModality.size() > 0 &&
      newModality[newModality.size() - 1] == s5Modality) {
    return TrieformFactory::makeTrieD5Helper(newModality, shared_from_this());
  }
  return TrieformFactory::makeTrieD5(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverD5Helper::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverD5Helper::updateSolutionMemo(
    const shared_ptr<Bitset> &assumptions, Solution solution) {
  if (solution.satisfiable) {
    globalMemo.insertSat(assumptions, modality);
  } else {
    globalMemo.insertUnsat(assumptions, solution.conflict, modality);
  }
}

bool TrieformProverD5Helper::isInHistory(vector<shared_ptr<Bitset>> history,
                                         shared_ptr<Bitset> bitset) {
  for (shared_ptr<Bitset> assump : history) {
    if (assump->contains(*bitset)) {
      return true;
    }
  }
  return false;
}

void TrieformProverD5Helper::reflexiveHandleBoxClauses() {
  for (ModalClause modalClause : clauses.getBoxClauses()) {
    if (modalClause.modality == s5Modality) {
      formula_set newOr;
      newOr.insert(Not::create(modalClause.left)->negatedNormalForm());
      newOr.insert(modalClause.right);
      clauses.addClause(Or::create(newOr));
    }
  }
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.first == s5Modality) {
      dynamic_cast<TrieformProverD5Helper *>(modalSubtrie.second.get())
          ->reflexiveHandleBoxClauses();
    }
  }
}

void TrieformProverD5Helper::reflexivepropagateLevels() {
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.first == s5Modality) {

      dynamic_cast<TrieformProverD5Helper *>(modalSubtrie.second.get())
          ->reflexivepropagateLevels();
      overShadow(modalSubtrie.second, modalSubtrie.first);
    }
  }
}

void TrieformProverD5Helper::pruneTrie() {
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.first == s5Modality) {

      if (modalSubtrie.second->hasSubtrie(modalSubtrie.first)) {
        modalSubtrie.second->removeSubtrie(modalSubtrie.first);
      }
      dynamic_cast<TrieformProverD5Helper *>(modalSubtrie.second.get())
          ->pruneTrie();
    }
  }
}

void TrieformProverD5Helper::makePersistence() {
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.first == s5Modality)
      dynamic_cast<TrieformProverD5Helper *>(modalSubtrie.second.get())
          ->makePersistence();
  }

  modal_clause_set persistentBoxes;
  for (ModalClause boxClause : clauses.getBoxClauses()) {
    // For a=>[]b in our box clauses replace with.
    // a=>Pb
    // Pb=>[]Pb
    // [](Pb=>b)

    // Make persistence (Pb=>[]Pb). Don't need to add Pb=>Pb
    if (boxClause.modality == s5Modality) {

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
    } else {
      persistentBoxes.insert(boxClause);
    }
  }
  clauses.setBoxClauses(persistentBoxes);

  for (ModalClause persistentBox : persistentBoxes) {
    if (persistentBox.modality == s5Modality) {
      if (hasSubtrie(persistentBox.modality)) {
        subtrieMap[persistentBox.modality]->clauses.addBoxClause(persistentBox);
      }
    }
  }
}

void TrieformProverD5Helper::propagateSymmetricBoxes() {
  for (auto modalitySubtrie : subtrieMap) {
    if (modalitySubtrie.first == s5Modality) {

      dynamic_cast<TrieformProverD5Helper *>(modalitySubtrie.second.get())
          ->propagateSymmetricBoxes();
    }
  }
  for (auto modalitySubtrie : subtrieMap) {
    if (modalitySubtrie.first == s5Modality) {
      for (const ModalClause &boxClause :
           modalitySubtrie.second->getClauses().getBoxClauses()) {
        if (boxClause.modality == s5Modality) {
          if (modalitySubtrie.first == boxClause.modality) {
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
  }
}

void TrieformProverD5Helper::propagateEuclideaness() {
  for (const ModalClause &clause : clauses.getDiamondClauses()) {
    if (hasSubtrie(clause.modality)) {
      formula_set newOr;
      newOr.insert(clause.left->negate());
      newOr.insert(
          Box::create(clause.modality, 1,
                      Diamond::create(clause.modality, 1, clause.right)));

      propagateClauses(Or::create(newOr));
    }
  }
}

void TrieformProverD5Helper::preprocess() {
  // Apply reflexivity first
  reflexiveHandleBoxClauses();
  reflexivepropagateLevels();
  pruneTrie();
  makePersistence();
  propagateSymmetricBoxes();
  propagateEuclideaness();
  for (auto modalSubtrie : subtrieMap) {
    if (modalSubtrie.first != s5Modality) {
      modalSubtrie.second->preprocess();
    }
  }
  for (const int &mod : futureModalities) {
    clauses.addDiamondClause({mod, True::create(), True::create()});
  }
  pruneTrie();
}

void TrieformProverD5Helper::prepareSAT(name_set extra) {
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

Solution TrieformProverD5Helper::prove(vector<shared_ptr<Bitset>> history,
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
        childSolution = dynamic_cast<TrieformProverD5Helper *>(
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

Solution
TrieformProverD5Helper::prove(literal_set assumptions = literal_set()) {
  return prove(vector<shared_ptr<Bitset>>(), assumptions);
}

void TrieformProverD5Helper::setModality(int mod) { s5Modality = mod; }