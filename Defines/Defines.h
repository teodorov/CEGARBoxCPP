#ifndef DEFINES_H
#define DEFINES_H

#include "../Formula/Formula/Formula.h"
#include <functional>
#include <memory>
#include <unordered_set>

using namespace std;

#define DEBUG_DESTRUCT false
#define DEBUG_TIME false
#define DEBUG_PROGRESS false
#define DEBUG_NORMAL_CACHE false
#define DEBUG_REDUCED_CACHE false
#define DEBUG_INITIAL_TRIE false
#define DEBUG_PROCESSED_TRIE false
#define DEBUG_REDUCED_TRIE false

typedef unordered_set<string> name_set;
typedef unordered_map<int, name_set> modal_names_map;

struct SolverConstraints {
  bool reflexive = false;
  bool symmetric = false;
  bool transitive = false;
  bool serial = false;
  bool euclidean = false;
};

#endif