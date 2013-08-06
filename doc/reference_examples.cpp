// -*- compile-command: "g++ -std=c++11 -I../include -I$BOOST_ROOT reference_examples.cpp ../src/multi_methods.cpp -o examples && ./examples" -*-

#include <iostream>
#include <multi_methods.hpp>

using namespace std;

struct role : multi_methods::selector {
  MM_CLASS(role);
  role() {
    MM_INIT();
  }
};

struct manager : role {
  MM_CLASS(manager, role);
  manager() {
    MM_INIT();
  }
};

struct ceo : role {
  MM_CLASS(ceo, role);
  ceo() {
    MM_INIT();
  }
};

struct expense : multi_methods::selector {
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

struct reason : multi_methods::selector {
  MM_CLASS(reason);
  reason() {
    MM_INIT();
  }
};

struct business : reason {
  MM_CLASS(business, reason);
  business() {
    MM_INIT();
  }
};

struct comfort : reason {
  MM_CLASS(comfort, reason);
  comfort() {
    MM_INIT();
  }
};

using namespace multi_methods;

//[ ref_multi_method

template<typename Signature> struct approve_specialization;

constexpr multi_method<
  approve_specialization,
  bool(const virtual_<expense>&, const virtual_<role>&, const virtual_<reason>&)
> approve;
//]

//[ ref_spec

struct approve_ceo_all {
  static bool (*next)(const expense&, const role&, const reason&);
  static bool body(const plane& what, const ceo& who, const reason& why) {
    return true;
  }
};
//]

bool (*approve_ceo_all::next)(const expense&, const role&, const reason&);

int main() {
//[ ref_spec_call
approve.specialize<approve_ceo_all>();
//]
  initialize();
  cout << approve(plane(), ceo(), comfort()) << endl;
}
