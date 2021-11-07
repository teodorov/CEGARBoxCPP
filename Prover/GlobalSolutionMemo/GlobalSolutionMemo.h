#ifndef GLOBAL_SOLUTION_MEMO_H
#define GLOBAL_SOLUTION_MEMO_H

#include "../../Bitset/Bitset.h"
#include "../Literal/Literal.h"
#include <memory>
#include <unordered_map>
#include <vector>

// THIS DOESN'T WORK FOR MULTIMODAL LOGIC

using namespace std;

struct GlobalSolutionMemoResult {
  bool inSatMemo;
  Solution result;
};

struct IntVectorHash {
  std::size_t operator()(vector<int> const &v) const {
    std::hash<int> int_hash;
    size_t hash = 0;
    for (const int &n : v) {
      hash = (hash << 2) + int_hash(n);
    }
    return hash;
  }
};

struct IntVectorEqual {
  std::size_t operator()(vector<int> const &v1, vector<int> const &v2) const {
    return v1 == v2;
  }
};

class GlobalSolutionMemo {
private:
  struct UnsatHolder {
    shared_ptr<Bitset> activatedLiterals;
    literal_set unsatCore;
  };

  unordered_map<vector<int>, vector<shared_ptr<Bitset>>, IntVectorHash,
                IntVectorEqual>
      satSols;
  unordered_map<vector<int>, vector<UnsatHolder>, IntVectorHash, IntVectorEqual>
      unsatSols;

public:
  GlobalSolutionMemo();
  ~GlobalSolutionMemo();

  GlobalSolutionMemoResult getFromMemo(const shared_ptr<Bitset> &assumptions,
                                       vector<int> modality);

  void insertSat(const shared_ptr<Bitset> &assumptions, vector<int> modality);
  void insertUnsat(const shared_ptr<Bitset> &assumptions,
                   const literal_set &unsatCore, vector<int> modality);
};

#endif