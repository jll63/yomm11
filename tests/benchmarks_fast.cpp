// benchmarks_fast.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "benchmarks.hpp"

namespace intrusive {

  object* object::make() {
    return new object;
  }

  double object::do_something(double x, double a, double b, double c) {
    return log(a * x * x + b * x + c);
  }

  void object::do_nothing() {
  }

  void object::dd1_do_nothing(object* pf) {
    pf->dd2_do_nothing(pf);
  }

  void object::dd2_do_nothing(object* pf) {
  }

}

namespace vbase {

  object* object::make() {
    return new derived;
  }

  double derived::do_something(double x, double a, double b, double c) {
    return log(a * x * x + b * x + c);
  }

  void derived::do_nothing() {
  }

  void derived::dd1_do_nothing(object* pf) {
    pf->dd2_do_nothing(pf);
  }

  void derived::dd2_do_nothing(object* pf) {
  }

}
