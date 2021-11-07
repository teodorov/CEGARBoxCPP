#include "Cache.h"

Cache::Cache(string prepend) { name = "$" + prepend; }

Cache::~Cache() {}

shared_ptr<Formula> Cache::createVariable() {
  return Atom::create(name + to_string(++variablesGenerated));
}

shared_ptr<Formula>
Cache::getVariableOrCreate(const shared_ptr<Formula> &formula,
                           const vector<int> &modality) {
  if (contains(formula, modality)) {
    return getVariableRepresenting(formula, modality);
  }
  return createVariableFor(formula, modality);
}

shared_ptr<Formula> Cache::createVariableFor(const shared_ptr<Formula> &formula,
                                             const vector<int> &modality) {
  shared_ptr<Formula> newVariable = createVariable();
  cache[{modality, formula /*->clone()*/}] = newVariable;
  return newVariable;
}

shared_ptr<Formula>
Cache::getVariableRepresenting(const shared_ptr<Formula> &formula,
                               const vector<int> &modality) {
  return cache[{modality, formula}];
}

bool Cache::contains(const shared_ptr<Formula> &formula,
                     const vector<int> &modality) const {
  return cache.find({modality, formula}) != cache.end();
}

string Cache::toString() const {
  string cacheString = "";
  for (auto value : cache) {
    if (cacheString.size() > 0) {
      cacheString += "\n";
    }
    string levelModality = "";
    int previousSeenModality = INT_MIN;
    int count = 0;
    for (int mod : value.first.modality) {
      if (mod != previousSeenModality) {
        if (count > 0) {
          levelModality +=
              "[" + to_string(previousSeenModality) + "]^" + to_string(count);
        }
        count = 1;
        previousSeenModality = mod;
      } else {
        count++;
      }
    }
    if (count > 0) {
      levelModality +=
          "[" + to_string(previousSeenModality) + "]^" + to_string(count);
    }
    if (levelModality.size() == 0) {
      cacheString +=
          value.first.formula->toString() + " is " + value.second->toString();
    } else {
      cacheString += levelModality + " " + value.first.formula->toString() +
                     " is " + value.second->toString();
    }
  }
  return cacheString;
}

cache_map Cache::getCache() const { return cache; }