// adjust.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

struct X : selector {
  MM_CLASS(X);

  X() {
    MM_INIT();
  }
};

struct A : VIRTUAL X {
  MM_CLASS(A, X);
  int pad;
  int val;

  A() {
    MM_INIT();
    pad = -1;
  }
};

struct Pad {
  char x[200];
};

struct B : Pad, VIRTUAL X {
  MM_CLASS(B, X);
  int val;

  B() {
    MM_INIT();
  }
};

MULTI_METHOD(foo, int, const virtual_<X>&, const virtual_<X>&);

BEGIN_SPECIALIZATION(foo, int, const A& x, const A& y) {
  return x.val + y.val;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(foo, int, const A& x, const B& y) {
  return x.val - y.val;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(foo, int, const B& x, const B& y) {
  return x.val * y.val;
} END_SPECIALIZATION;
