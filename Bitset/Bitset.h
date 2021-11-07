#ifndef BITSET_H
#define BITSET_H

#include <climits>
#include <iostream>

using namespace std;

class Bitset {
private:
  unsigned int size = 0;
  unsigned int bitsPerBucket = 0; // CHAR_BIT * sizeof(int);
  unsigned int buckets = 0;       //(size + bitsPerBucket - 1) / bitsPerBucket;
  unsigned int *bitArray;

public:
  Bitset(const unsigned int bits);
  ~Bitset();

  void set(const unsigned int bit);
  void reset(const unsigned int bit);

  bool contains(const Bitset &other) const;
  bool isContainedBy(const Bitset &other) const;
  unsigned int operator[](const unsigned int &bucket) const;

  void print() const;
};

#endif