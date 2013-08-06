// -*- compile-command: "make three && ./three" -*-

// three.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "multi_methods.hpp"

#include <iostream>

using namespace std;

using multi_methods::virtual_;

struct role : multi_methods::selector {
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

struct expense : multi_methods::selector {
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

struct reason : multi_methods::selector {
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

MULTI_METHOD(approve, bool, const virtual_<expense>&, const virtual_<role>&, const virtual_<reason>&);

BEGIN_SPECIALIZATION(approve, bool, const expense&, const role&, const reason&) {
  return false;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(approve, bool, const expense&, const ceo&, const reason&) {
  return true;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(approve, bool, const cab&, const manager&, const business&) {
  return true;
} END_SPECIALIZATION;

#define demo(exp) cout << #exp << " -> " << exp << endl

int main() {
  multi_methods::initialize(); // IMPORTANT!
  cout << boolalpha;

  // ceo does as he pleases
  demo( approve(plane(), ceo(), comfort()) ); // true
  demo( approve(cab(), ceo(), business()) ); // true
  // managers only take cabs for business
  demo( approve(cab(), manager(), business()) ); // true
  demo( approve(cab(), manager(), comfort()) ); // false

  return 0;
}
