// -*- compile-command: "make order12 && ./order12" -*-

// order2.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "order.hpp"
#include <iostream>

using namespace std;

using multimethods::virtual_;

struct business : reason {
  MM_CLASS(business, reason);
  business() {
    MM_INIT();
  }
};

struct manager : role {
  MM_CLASS(manager, role);
  manager() {
    MM_INIT();
  }
};

struct cab : expense {
  MM_CLASS(cab, expense);
  cab() {
    MM_INIT();
  }
};

BEGIN_METHOD(approve, bool, const expense&, const role&, const reason&) {
  return false;
} END_METHOD;

BEGIN_METHOD(approve, bool, const cab&, const manager&, const business&) {
  return true;
} END_METHOD;

#define demo(exp) cout << #exp << " -> " << exp << endl

int main() {
  cout << boolalpha;
  multimethods::initialize();
  unique_ptr<role> ceo = make_ceo();
  // ceo does as he pleases
  demo( approve(plane(), *ceo, comfort()) );
  demo( approve(cab(), *ceo, business()) );
  // managers only take cabs for business
  demo( approve(cab(), manager(), business()) );
  demo( approve(cab(), manager(), comfort()) );
  return 0;
}
