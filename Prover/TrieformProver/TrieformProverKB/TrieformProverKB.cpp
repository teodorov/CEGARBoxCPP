#include "TrieformProverKB.h"

shared_ptr<Trieform>
TrieformFactory::makeTrieKB(const shared_ptr<Formula> &formula,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKB());
  trie->initialise(formula, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieKB(const shared_ptr<Formula> &formula,
                            const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKB());
  trie->initialise(formula, newModality, trieParent);
  return trie;
}
shared_ptr<Trieform>
TrieformFactory::makeTrieKB(const vector<int> &newModality,
                            shared_ptr<Trieform> trieParent) {
  shared_ptr<Trieform> trie = shared_ptr<Trieform>(new TrieformProverKB());
  trie->initialise(newModality, trieParent);
  return trie;
}

TrieformProverKB::TrieformProverKB() {}
TrieformProverKB::~TrieformProverKB() {}

shared_ptr<Trieform>
TrieformProverKB::create(const shared_ptr<Formula> &formula) {
  return TrieformFactory::makeTrieKB(formula, shared_from_this());
}
shared_ptr<Trieform>
TrieformProverKB::create(const shared_ptr<Formula> &formula,
                         const vector<int> &newModality) {
  return TrieformFactory::makeTrieKB(formula, newModality, shared_from_this());
}
shared_ptr<Trieform> TrieformProverKB::create(const vector<int> &newModality) {
  return TrieformFactory::makeTrieKB(newModality, shared_from_this());
}

void TrieformProverKB::updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                                          Solution solution) {
  if (solution.satisfiable) {
    localMemo.insertSat(assumptions);
  } else {
    localMemo.insertUnsat(assumptions, solution.conflict);
  }
}

shared_ptr<Bitset>
TrieformProverKB::convertAssumptionsToBitset(literal_set literals) {
  shared_ptr<Bitset> bitset =
      shared_ptr<Bitset>(new Bitset(2 * assumptionsSize));
  for (Literal literal : literals) {
    bitset->set(2 * idMap[literal.getName()] + literal.getPolarity());
  }
  return bitset;
}

void TrieformProverKB::propagateSymmetry() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverKB *>(modalitySubtrie.second.get())
        ->propagateSymmetry();
  }
  for (auto modalitySubtrie : subtrieMap) {
    if (modalitySubtrie.second->hasSubtrie(modalitySubtrie.first)) {
      shared_ptr<Trieform> future =
          modalitySubtrie.second->getSubtrie(modalitySubtrie.first);
      if (modality.size() > 0 &&
          modality[modality.size() - 1] == modalitySubtrie.first) {
        overShadow(future);
      } else {
        conditionalOverShadow(
            future, Box::create(modalitySubtrie.first, 1, False::create()));
      }
    }
  }
}

void TrieformProverKB::propagateSymmetricBoxes() {
  for (auto modalitySubtrie : subtrieMap) {
    dynamic_cast<TrieformProverKB *>(modalitySubtrie.second.get())
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

void TrieformProverKB::preprocess() {
  propagateSymmetry();
  propagateSymmetricBoxes();
}

void TrieformProverKB::prepareSAT(name_set extra) {
  // Shortcut only do this for levels 0 and 1 as symmetry guarantees every
  // possible assumption is here... I think... yesn't? Reducing could mess
  // things up. Either preprocess twice, (after and before reducing). Two other
  // options -> reduce before is one. The final one is smart reducing - maybe
  // inefficient but perhaps most optimal. CURRENT IMPLEMENTATION USES LOCAL
  // CACHING ONLY

  // if (modality.size() == 1 || modality.size() == 2) {
  for (string name : extra) {
    if (idMap.find(name) == idMap.end()) {
      idMap[name] = assumptionsSize++;
    }
  }
  // }

  // Suppose [1][1](a=><1>b) -> then a=><2>b has already been put into level 0
  // (actually <1>T=>(a=><1>b)) so b appears in level 1 which is what we want.

  // Suppose [1][2](a=><>b). a=><>b hasn't been put in level 0 but we want b to
  // appear in level 1.

  // Similarly then in level Suppose [](a=><>b). We want b to appear in the
  // space of level 0. That hasn't been done though.

  // Solution is to iterate through the diamond clauses of the next level and
  // add into our assumptions
  for (auto modalSubtrie : subtrieMap) {
    for (ModalClause clause :
         modalSubtrie.second->getClauses().getDiamondClauses()) {
      if (clause.modality == modalSubtrie.first) {
        // Only relevant when the modalities are equal
        extra.insert(prover->getPrimitiveName(clause.right));
      }
    }
  }
  modal_names_map modalExtras = prover->prepareSAT(clauses, extra);
  for (auto modalSubtrie : subtrieMap) {
    modalSubtrie.second->prepareSAT(modalExtras[modalSubtrie.first]);
  }
}

Solution TrieformProverKB::prove(literal_set assumptions) {
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

    // Note in the cases where the diamonds are a subset of the boxes it is
    // already satisfied by the backwards link. Say here a => []b. The previous
    // world already has ~b=>[]~a. Therefore this must already be satisfied.

    diamond_queue diamondPriority =
        prover->getPrioritisedTriggeredDiamonds(modalitySubtrie.first);

    while (!diamondPriority.empty()) {
      // Create a world for each diamond
      Literal diamond = diamondPriority.top().literal;
      diamondPriority.pop();

      // If the parent is of the same modality and satisfies the diamond there
      // is no need to create a new world
      if (modality.size() > 0 &&
          modality[modality.size() - 1] == modalitySubtrie.first &&
          parentSatisfiesAssump(diamond)) {
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