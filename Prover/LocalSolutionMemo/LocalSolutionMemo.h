#ifndef LOCAL_SOLUTION_MEMO_H
#define LOCAL_SOLUTION_MEMO_H

#include "../../Bitset/Bitset.h"
#include "../Literal/Literal.h"
#include <memory>
#include <vector>

using namespace std;

struct LocalSolutionMemoResult {
  bool inSatMemo;
  Solution result;
};

class LocalSolutionMemo {
private:
  struct UnsatHolder {
    shared_ptr<Bitset> activatedLiterals;
    literal_set unsatCore;
  };

  vector<shared_ptr<Bitset>> satSols;
  vector<UnsatHolder> unsatSols;

public:
  LocalSolutionMemo();
  ~LocalSolutionMemo();

  LocalSolutionMemoResult
  getFromMemo(const shared_ptr<Bitset> &assumptions) const;

  void insertSat(const shared_ptr<Bitset> &assumptions);
  void insertUnsat(const shared_ptr<Bitset> &assumptions,
                   const literal_set &unsatCore);
};

#endif