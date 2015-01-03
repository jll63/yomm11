// benchmarks.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/multi_methods.hpp>

namespace intrusive {

struct object : yorel::multi_methods::selector {

  MM_CLASS(object);

  static object* make();

  object() {
    MM_INIT();
  }

  virtual void do_nothing();
  virtual double do_something(double x, double a, double b, double c);
  virtual void dd1_do_nothing(object* pf);
  virtual void dd2_do_nothing(object* pf);
};

}

namespace vbase {

struct object : yorel::multi_methods::selector {

  MM_CLASS(object);

  static object* make();

  object() {
    MM_INIT();
  }

  virtual void do_nothing() = 0;
  virtual double do_something(double x, double a, double b, double c) = 0;
  virtual void dd1_do_nothing(object* pf) = 0;
  virtual void dd2_do_nothing(object* pf) = 0;
};

struct derived : virtual object {

  MM_CLASS(derived, object);

  derived() {
    MM_INIT();
  }

  virtual void do_nothing();
  virtual double do_something(double x, double a, double b, double c);
  virtual void dd1_do_nothing(object* pf);
  virtual void dd2_do_nothing(object* pf);
};

}
