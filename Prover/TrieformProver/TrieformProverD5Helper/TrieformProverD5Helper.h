#ifndef TRIEFORM_PROVER_D5_HELPER_H
#define TRIEFORM_PROVER_D5_HELPER_H

#include "../../../Bitset/Bitset.h"
#include "../../../Clausifier/Trieform/Trieform.h"
#include "../../../Clausifier/TrieformFactory/TrieformFactory.h"
#include "../../GlobalSolutionMemo/GlobalSolutionMemo.h"
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class TrieformProverD5Helper : public Trieform {
private:
  int s5Modality = 0;

protected:
  static Cache persistentCache;

  static unsigned int assumptionsSize;
  static GlobalSolutionMemo globalMemo;
  static unordered_map<string, unsigned int> idMap;

  shared_ptr<Bitset> convertAssumptionsToBitset(literal_set literals);
  void updateSolutionMemo(const shared_ptr<Bitset> &assumptions,
                          Solution solution);
  bool isInHistory(vector<shared_ptr<Bitset>> history,
                   shared_ptr<Bitset> bitset);

  void reflexiveHandleBoxClauses();
  void reflexivepropagateLevels();
  void pruneTrie();
  void makePersistence();
  void propagateSymmetricBoxes();
  void propagateEuclideaness();

public:
  TrieformProverD5Helper();
  ~TrieformProverD5Helper();

  Solution prove(vector<shared_ptr<Bitset>> history, literal_set assumptions);
  virtual Solution prove(literal_set assumptions);
  virtual void preprocess();
  virtual void prepareSAT(name_set extra = name_set());

  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula);
  virtual shared_ptr<Trieform> create(const shared_ptr<Formula> &formula,
                                      const vector<int> &newModality);
  virtual shared_ptr<Trieform> create(const vector<int> &newModality);

  void setModality(int mod);
};

#endif