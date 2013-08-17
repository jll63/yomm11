// -*- compile-command: "make foreign && ./foreign" -*-

// foreign.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <multi_methods.hpp>

#include <iostream>

using namespace std;

using yorel::multi_methods::virtual_;
using yorel::multi_methods::selector;

// following hierarchy does not collaborate with multi-methods

struct role {
  virtual ~role() { }
};

MM_FOREIGN_CLASS(role);

struct manager : role {
};

MM_FOREIGN_CLASS(manager, role);

struct ceo : role {
};

MM_FOREIGN_CLASS(ceo, role);

// this one does

struct expense : selector {
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

// this one does not

struct reason {
  virtual ~reason() { }
};

MM_FOREIGN_CLASS(reason);

struct business : reason {
};

MM_FOREIGN_CLASS(business, reason);

struct comfort : reason {
};

MM_FOREIGN_CLASS(comfort, reason);

// collaborating and foreign classes can appear in same multi-method definition

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
  yorel::multi_methods::initialize(); // IMPORTANT!
  cout << boolalpha;

  // ceo does as he pleases
  demo( approve(plane(), ceo(), comfort()) ); // true
  demo( approve(cab(), ceo(), business()) ); // true
  // managers only take cabs for business
  demo( approve(cab(), manager(), business()) ); // true
  demo( approve(cab(), manager(), comfort()) ); // false

  return 0;
}
