#ifndef CACHE_H
#define CACHE_H

#include "../../Formula/Atom/Atom.h"
#include "../../Formula/Formula/Formula.h"
#include <climits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct CacheKey {
  vector<int> modality;
  shared_ptr<Formula> formula;
};

struct CacheKeyHasher {
  int operator()(const CacheKey &CK) const {
    int hash = CK.modality.size();
    for (auto &i : CK.modality) {
      hash ^= i + 0x9e779b9 + (hash << 6) + (hash >> 2);
    }
    return hash + CK.formula->hash();
  }
};

struct CacheKeyEqual {
  size_t operator()(CacheKey const &ck1, CacheKey const &ck2) const {
    return *ck1.formula.get() == *ck2.formula.get() &&
           ck1.modality == ck2.modality;
  }
};

typedef unordered_map<CacheKey, shared_ptr<Formula>, CacheKeyHasher,
                      CacheKeyEqual>
    cache_map;

class Cache {
private:
  cache_map cache;
  string name;
  int variablesGenerated = 0;

  shared_ptr<Formula> createVariable();

public:
  Cache(string prepend);
  ~Cache();

  shared_ptr<Formula>
  getVariableOrCreate(const shared_ptr<Formula> &formula,
                      const vector<int> &modality = vector<int>());

  shared_ptr<Formula>
  createVariableFor(const shared_ptr<Formula> &formula,
                    const vector<int> &modality = vector<int>());
  shared_ptr<Formula>
  getVariableRepresenting(const shared_ptr<Formula> &formula,
                          const vector<int> &modality = vector<int>());
  bool contains(const shared_ptr<Formula> &formula,
                const vector<int> &modality = vector<int>()) const;

  string toString() const;

  cache_map getCache() const;
};

#endif