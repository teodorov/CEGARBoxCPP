#include "Formula.h"

Formula::Formula() {}
Formula::~Formula() {
#if DEBUG_DESTRUCT
  cout << "DESTRUCTING PARENT" << endl;
#endif
}
bool Formula::isPrimitive() const { return false; }