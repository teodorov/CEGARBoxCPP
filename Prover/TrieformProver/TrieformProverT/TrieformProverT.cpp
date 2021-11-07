#include "TrieformProverT.h"

unsigned int TrieformProverT::assumptionsSize = 0;
GlobalSolutionMemo TrieformProverT::globalMemo = GlobalSolutionMemo();
unordered_map<string, unsigned int> TrieformProverT::idMap =
    unordered_map<string, unsigned int>();

shared_ptr<Trieform>
TrieformFactory::makeTrieT(const shared_ptr<Formula> &formula,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverT());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieT(const shared_ptr<Formula> &formula,
                           const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverT());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieT(const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverT());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverT::TrieformProverT() {}
TrieformProverT::~TrieformProverT() {}

shared_ptr<Trieform>
TrieformProverT::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieT(formula, shared_from_this());
}
shared_ptr<Trieform> TrieformProverT::create(const shared_ptr<Formula> &formula,
                                             const vector<int> &newModality) {
  return TrieformFactory::makeTrieT(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverT::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieT(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverT::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverT::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                         Solution solution) {
  if (solution.satisfiable) {
    globalMemo.insertSat(assumptions, modality);
  } else {
    globalMemo.insertUnsat(assumptions, solution.conflict, modality);
  }
}

bool TrieformProverT::isInHistory(vector<shared_ptr<Bitset>> history,
                                  shared_ptr<Bitset> bitset) {
  for (shared_ptr<Bitset> assump : history) {
    if (assump->contains(*bitset)) {
      return true;
    }
  }
  return false;
}

void TrieformProverT::handleReflexiveBoxClauses() {
  for (ModalClause modalClause : clauses.getBoxClauses()) {
    formula_set newOr;
    newOr.insert(Not::create(modalClause.left)->negatedNormalForm());
    newOr.insert(modalClause.right);
    clauses.addClause(Or::create(newOr));
  }
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverT *>(modalSubtrie.second.get())
        ->handleReflexiveBoxClauses();
  }
}

void TrieformProverT::propagateLevels() {
  for (auto modalSubtrie : subtrieMap) {
    dynamic_cast<TrieformProverT *>(modalSubtrie.second.get())
        ->propagateLevels();
    overShadow(modalSubtrie.second, modalSubtrie.first);
  }
}

void TrieformProverT::preprocess() {
  // If a->[]b then a->b
  // cout << "DOING BOX" << endl;
  handleReflexiveBoxClauses();
  // Add all successors to current node
  // cout << "DOING LEVELS" << endl;
  propagateLevels();
}

void TrieformProverT::prepareSAT(name_set extra) {
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

Solution TrieformProverT::prove(vector<shared_ptr<Bitset>> history,
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

  for (auto modalitySubtrie : subtrieMap) {
    // Handle each modality
    if (triggeredDiamonds[modalitySubtrie.first].size() == 0) {
      // If there are no triggered diamonds of a certain modality we can skip it
      continue;
    }
    // Note in the cases diamonds are a subset of boxes then we don't need to
    // create any worlds (reflexivity satisfies this)
    diamond_queue diamondPriority =
        prover->getPrioritisedTriggeredDiamonds(modalitySubtrie.first);

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
          literal_set(triggeredBoxes[modalitySubtrie.first]);
      childAssumptions.insert(diamond);

      // Run the solver for the next level
      history.push_back(assumptionsBitset);
      Solution childSolution =
          dynamic_cast<TrieformProverT *>(modalitySubtrie.second.get())
              ->prove(history, childAssumptions);
      history.pop_back();

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
        // caused a conflict. We must add diamond implies OR NOT problem box
        // clauses.
        prover->updateLastFail(diamond);
        badImplications.push_back(
            prover->getNotDiamondLeft(modalitySubtrie.first, diamond));

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
            prover->getNotAllDiamondLeft(modalitySubtrie.first));
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

Solution TrieformProverT::prove(literal_set assumptions = literal_set()) {
  return prove(vector<shared_ptr<Bitset>>(), assumptions);
}