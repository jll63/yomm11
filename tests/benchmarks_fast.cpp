// benchmarks_fast.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "benchmarks.hpp"

fast* fast::make() {
  return new fast;
}

double fast::do_something(double x, double a, double b, double c) {
  return log(a * x * x + b * x + c);
}

void fast::do_nothing() {
}

void fast::dd1_do_nothing(fast* pf) {
  pf->dd2_do_nothing(pf);
}

void fast::dd2_do_nothing(fast* pf) {
}

BEGIN_METHOD(do_something, double, fast&, double x, double a, double b, double c) {
  return log(a * x * x + b * x + c);
} END_METHOD;

