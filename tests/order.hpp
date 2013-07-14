// order.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <multimethods.hpp>
#include <memory>

struct role : multimethods::selector {
  MM_CLASS(role);
  role() {
    MM_INIT();
  }
};

struct expense : multimethods::selector {
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

struct reason : multimethods::selector {
  MM_CLASS(reason);
  reason() {
    MM_INIT();
  }
};

struct comfort : reason {
  MM_CLASS(comfort, reason);
  comfort() {
    MM_INIT();
  }
};

MULTIMETHOD(approve, bool(const multimethods::virtual_<expense>&, const multimethods::virtual_<role>&, const multimethods::virtual_<reason>&));

extern std::unique_ptr<role> make_ceo();
