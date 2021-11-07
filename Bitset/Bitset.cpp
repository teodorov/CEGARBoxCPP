
#include "Bitset.h"

Bitset::Bitset(const unsigned int bits) {
  size = bits;
  bitsPerBucket = CHAR_BIT * sizeof(int);
  buckets = (size + bitsPerBucket - 1) / bitsPerBucket;
  // cout << "Buckets " << buckets << " bits " << bits << endl;
  if (buckets > 0) {
    bitArray = new unsigned int[buckets]{0};
  }
  // print();
}
Bitset::~Bitset() {
  // cout << "DELETING" << endl;
  // cout << "BUCKETS " << buckets << endl;
  if (buckets > 0) {
    delete[] bitArray;
  }
  // cout << "DELETED" << endl;
}

void Bitset::set(const unsigned int bit) {
  // print();
  *(bitArray + bit / bitsPerBucket) |= 1U << (bit % bitsPerBucket);
  // print();
}

void Bitset::reset(const unsigned int bit) {
  *(bitArray + bit / bitsPerBucket) &= ~(1U << (bit % bitsPerBucket));
}

bool Bitset::contains(const Bitset &other) const {
  for (unsigned int i = 0; i < buckets; i++) {
    if (((~*(bitArray + i)) & other[i]) != 0) {
      return false;
    }
  }
  return true;
}

bool Bitset::isContainedBy(const Bitset &other) const {
  for (unsigned int i = 0; i < buckets; i++) {
    if ((*(bitArray + i) & ~other[i]) != 0) {
      return false;
    }
  }
  return true;
}

unsigned int Bitset::operator[](const unsigned int &bucket) const {
  return *(bitArray + bucket);
}

void Bitset::print() const {
  for (int bit = (size % bitsPerBucket) - 1; bit >= 0; bit--) {
    cout << ((*(bitArray + buckets - 1) >> bit) & 1U);
  }
  for (int bucket = buckets - 2; bucket >= 0; bucket--) {
    for (int bit = bitsPerBucket - 1; bit >= 0; bit--) {
      cout << ((*(bitArray + bucket) >> bit) & 1U);
    }
  }

  cout << endl;
}