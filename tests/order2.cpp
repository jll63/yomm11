// -*- compile-command: "make order21 && ./order21" -*-

// order2.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "order.hpp"
#include <iostream>

using namespace std;

using multimethods::virtual_;

struct ceo : role {
  MM_CLASS(ceo, role);
  ceo() {
    MM_INIT();
  }
};

BEGIN_METHOD(approve, bool, const expense&, const ceo&, const reason&) {
  return true;
} END_METHOD;

unique_ptr<role> make_ceo() {
  return unique_ptr<role>(new ceo());
}
