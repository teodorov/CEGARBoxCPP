#include "TrieformProverK.h"

shared_ptr<Trieform>
TrieformFactory::makeTrieK(const shared_ptr<Formula> &formula,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieK(const shared_ptr<Formula> &formula,
                           const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieK(const vector<int> &newModality,
                           shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverK());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverK::TrieformProverK() {}
TrieformProverK::~TrieformProverK() {}

shared_ptr<Trieform>
TrieformProverK::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieK(formula, shared_from_this());
}
shared_ptr<Trieform> TrieformProverK::create(const shared_ptr<Formula> &formula,
                                             const vector<int> &newModality) {
  return TrieformFactory::makeTrieK(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverK::create(const vector<int> &newModality) {
  shared_ptr<Trieform> test = shared_from_this();
  return TrieformFactory::makeTrieK(newModality, shared_from_this());
}

shared_ptr<Bitset>
TrieformProverK::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverK::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                         Solution solution) {
  if (solution.satisfiable) {
    localMemo.insertSat(assumptions);
  } else {
    localMemo.insertUnsat(assumptions, solution.conflict);
  }
}

void TrieformProverK::preprocess() {}

void TrieformProverK::prepareSAT(name_set extra) {
  for (string name : extra) {
    idMap[name] = assumptionsSize++;
  }
  modal_names_map modalExtras = prover->prepareSAT(clauses, extra);
  for (auto modalSubtrie : subtrieMap) {
    modalSubtrie.second->prepareSAT(modalExtras[modalSubtrie.first]);
  }
}

Solution TrieformProverK::prove(literal_set assumptions) {
  // Check solution memo
  shared_ptr<Bitset> assumptionsBitset =
      convertAssumptionsToBitset(assumptions);
  LocalSolutionMemoResult memoResult = localMemo.getFromMemo(assumptionsBitset);

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
    if (isSubsetOf(triggeredDiamonds[modalitySubtrie.first],
                   triggeredBoxes[modalitySubtrie.first])) {
      // The fired diamonds are a subset of the boxes - we thus can create one
      // world.
      Solution childSolution =
          modalitySubtrie.second->prove(triggeredBoxes[modalitySubtrie.first]);
      // If the one world solution is satisfiable, then we're all good
      if (childSolution.satisfiable) {
        continue;
      }
      // Otherwise, as the diamonds are a subset of the boxes, the left
      // implications of the problem box clauses cannot be true with any diamond
      // clause of this modality
      vector<literal_set> badImplications = prover->getNotProblemBoxClauses(
          modalitySubtrie.first, childSolution.conflict);
      badImplications.push_back(
          prover->getNotAllDiamondLeft(modalitySubtrie.first));
      // Add ~leftDiamond=>\/~leftProbemBox
      for (literal_set learnClause : generateClauses(badImplications)) {
        prover->addClause(learnClause);
      }
      // Find new result
      return prove(assumptions);
    } else {
      // The fired diamonds are not a subset of the fired boxes, we need to
      // create one world for each diamond clause
      diamond_queue diamondPriority =
          prover->getPrioritisedTriggeredDiamonds(modalitySubtrie.first);

      while (!diamondPriority.empty()) {
        // Create a world for each diamond
        Literal diamond = diamondPriority.top().literal;
        diamondPriority.pop();

        literal_set childAssumptions =
            literal_set(triggeredBoxes[modalitySubtrie.first]);
        childAssumptions.insert(diamond);

        // Run the solver for the next level
        Solution childSolution =
            modalitySubtrie.second->prove(childAssumptions);

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
  }
  // If we reached here the solution is satisfiable under all modalities
  updateSolutionMemo(assumptionsBitset, solution);
  return solution;
}
