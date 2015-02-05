#include <iostream>
#include <yorel/methods.hpp>

using namespace std;

struct role : yorel::methods::selector {
  YOMM11_CLASS(role);
  role() {
    YOMM11_INIT();
  }
};

struct manager : role {
  YOMM11_CLASS(manager, role);
  manager() {
    YOMM11_INIT();
  }
};

struct ceo : role {
  YOMM11_CLASS(ceo, role);
  ceo() {
    YOMM11_INIT();
  }
};

struct expense : yorel::methods::selector {
  YOMM11_CLASS(expense);
  expense() {
    YOMM11_INIT();
  }
};

struct plane : expense {
  YOMM11_CLASS(plane, expense);
  plane() {
    YOMM11_INIT();
  }
};

struct cab : expense {
  YOMM11_CLASS(cab, expense);
  cab() {
    YOMM11_INIT();
  }
};

struct reason : yorel::methods::selector {
  YOMM11_CLASS(reason);
  reason() {
    YOMM11_INIT();
  }
};

struct business : reason {
  YOMM11_CLASS(business, reason);
  business() {
    YOMM11_INIT();
  }
};

struct comfort : reason {
  YOMM11_CLASS(comfort, reason);
  comfort() {
    YOMM11_INIT();
  }
};

using namespace yorel::methods;

//[ ref_method

template<typename Signature> struct approve_specialization;

constexpr method<
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
