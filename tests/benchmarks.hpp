// benchmarks.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <multimethods.hpp>

struct fast : multimethods::selector {

  MM_CLASS(fast);

  static fast* make();
  
  fast() {
    MM_INIT();
  }

  virtual void do_nothing();
  virtual double do_something(double x, double a, double b, double c);
  virtual void dd1_do_nothing(fast* pf);
  virtual void dd2_do_nothing(fast* pf);
};

MULTIMETHOD(do_something, double(multimethods::virtual_<fast>&, double x, double a, double b, double c));
