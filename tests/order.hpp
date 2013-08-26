// order.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/multi_methods.hpp>
#include <memory>

struct role : yorel::multi_methods::selector {
  MM_CLASS(role);
  role() {
    MM_INIT();
  }
};

struct expense : yorel::multi_methods::selector {
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

struct reason : yorel::multi_methods::selector {
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

MULTI_METHOD(approve, bool, const yorel::multi_methods::virtual_<expense>&, const yorel::multi_methods::virtual_<role>&, const yorel::multi_methods::virtual_<reason>&);

extern std::unique_ptr<role> make_ceo();
