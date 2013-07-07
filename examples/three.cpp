// -*- compile-command: "g++ -g -std=c++11 -I../include ../src/multimethods.cpp three.cpp -o three && ./three" -*-

#include "multimethods.hpp"

#include <iostream>

using namespace std;
using namespace multimethods;

struct role : root {
  MM_CLASS(role);
  role() {
    MM_INIT();
  }
};

struct manager : role {
  MM_CLASS(manager, role);
  manager() {
    MM_INIT();
  }
};

struct ceo : role {
  MM_CLASS(ceo, role);
  ceo() {
    MM_INIT();
  }
};

struct expense : root {
  MM_CLASS(expense);
  expense() {
    MM_INIT();
  }
};

struct plane : expense {
  MM_CLASS(plane, expense);
  plane() {
    MM_INIT();
  }
};

struct cab : expense {
  MM_CLASS(cab, expense);
  cab() {
    MM_INIT();
  }
};

struct reason : root {
  MM_CLASS(reason);
  reason() {
    MM_INIT();
  }
};

struct business : reason {
  MM_CLASS(business, reason);
  business() {
    MM_INIT();
  }
};

struct comfort : reason {
  MM_CLASS(comfort, reason);
  comfort() {
    MM_INIT();
  }
};

MULTIMETHOD(approve, bool(const virtual_<expense>&, const virtual_<role>&, const virtual_<reason>&));

BEGIN_METHOD(approve, bool, const expense&, const role&, const reason&) {
  return false;
} END_METHOD(approve)

BEGIN_METHOD(approve, bool, const expense&, const ceo&, const reason&) {
  return true;
} END_METHOD(approve)

BEGIN_METHOD(approve, bool, const cab&, const manager&, const business&) {
  return true;
} END_METHOD(approve)

#define demo(exp) cout << #exp << " -> " << exp << endl

int main() {
  cout << boolalpha;
  // ceo does as he pleases
  demo( approve(plane(), ceo(), comfort()) );
  demo( approve(cab(), ceo(), business()) );
  // managers only take cabs for business
  demo( approve(cab(), manager(), business()) );
  demo( approve(cab(), manager(), comfort()) );
  return 0;
}
