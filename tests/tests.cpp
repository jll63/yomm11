// -*- compile-command: "make tests && ./tests" -*-

// tests.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/multi_methods.hpp>
#include <yorel/multi_methods/runtime.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util/join.hpp"

using namespace std;
using namespace yorel::multi_methods;
using namespace yorel::multi_methods::detail;

#define test(exp, res) _test(__FILE__, __LINE__, #exp, exp, #res, res)
#define testx(exp, res) _test(__FILE__, __LINE__, #exp, exp, 0, res)

namespace {
int success, failure;
}

using methods = vector<method_base*>;

template<class T1, class T2>
bool _test(const char* file, int line, const char* test, const T1& got, const char* expected_expr, const T2& expected) {
  string ee;
  if (expected_expr) {
    ee = expected_expr;
  } else {
    ostringstream os;
    os << expected;
    ee = os.str();
  }
  bool ok = got == expected;
  if (ok) {
    cout << setw(3) << test << " returns " << ee << ", ok.\n";
    ++success;
  } else {
    cout << file << ":" << line << ": error: " << test << ": " << got << ", expected " << ee << ".\n";
    ++failure;
  }
  return ok;
}

#define PP_CAT(X, Y) PP_CAT1(X, Y)
#define PP_CAT1(X, Y) X ## Y

#define DO void PP_CAT(fun, __LINE__)(); int PP_CAT(var, __LINE__) = (PP_CAT(fun, __LINE__)(), 1); void PP_CAT(fun, __LINE__)()

#define show(e) #e << " = " << (e)

template<class Ex>
bool throws(function<void()> fun) {
  try {
    fun();
  } catch (Ex) {
    return true;
  } catch (...) {
  }

  return false;
}

DO {
  cout << boolalpha;
}

namespace single_inheritance {

#include "animals.hpp"
}

namespace slot_allocation_tests {

struct X : selector {
  MM_CLASS(X);
  X() { MM_INIT(); }
};

MULTI_METHOD(m_x, int, const virtual_<X>&);

struct A : X {
  MM_CLASS(A, X);
  A() { MM_INIT(); }
};

MULTI_METHOD(m_a, int, const virtual_<A>&);

struct B : virtual A {
  MM_CLASS(B, A);
  B() { MM_INIT(); }
};

MULTI_METHOD(m_b, int, const virtual_<B>&);

struct C : virtual A {
  MM_CLASS(C, A);
  C() { MM_INIT(); }
};

MULTI_METHOD(m_c, int, const virtual_<C>&);

struct D : virtual A {
  MM_CLASS(D, A);
  D() { MM_INIT(); }
};

MULTI_METHOD(m_d, int, const virtual_<D>&);

struct BC : B, C {
  MM_CLASS(BC, B, C);
  BC() { MM_INIT(); }
};

MULTI_METHOD(m_bc, int, const virtual_<BC>&);

struct CD : C, D {
  MM_CLASS(CD, C, D);
  CD() { MM_INIT(); }
};

MULTI_METHOD(m_cd, int, const virtual_<CD>&);

struct Y : virtual X {
  MM_CLASS(Y, X);
  Y() { MM_INIT(); }
};

MULTI_METHOD(m_y, int, const virtual_<Y>&);
}

namespace grouping_resolver_tests {

#include "animals.hpp"

#define MAKE_CLASS(Class, Base)                 \
  struct Class : Base {                         \
    MM_CLASS(Class, Base);                      \
    Class() {                                   \
      MM_INIT();                                \
    }                                           \
  }

MAKE_CLASS(Mobile, Interface);
MAKE_CLASS(MSWindows, Window);
MAKE_CLASS(X, Window);
MAKE_CLASS(Nokia, Mobile);
MAKE_CLASS(Samsung, Mobile);

enum action { whatever, print_herbivore, draw_herbivore, print_carnivore, draw_carnivore, mobile };

MULTI_METHOD(display, action, const virtual_<Animal>&, const virtual_<Interface>&);

// 0
BEGIN_SPECIALIZATION(display, action, const Herbivore& a, const Terminal& b) {
  return print_herbivore;
} END_SPECIALIZATION;

// 1
BEGIN_SPECIALIZATION(display, action, const Herbivore& a, const Window& b) {
  return draw_herbivore;
} END_SPECIALIZATION;

// 2
BEGIN_SPECIALIZATION(display, action, const Carnivore& a, const Terminal& b) {
  return print_carnivore;
} END_SPECIALIZATION;

// 3
BEGIN_SPECIALIZATION(display, action, const Carnivore& a, const Window& b) {
  return draw_carnivore;
} END_SPECIALIZATION;

// 4
BEGIN_SPECIALIZATION(display, action, const Animal& a, const Mobile& b) {
  return mobile;
} END_SPECIALIZATION;

}

#ifndef __clang__

namespace init_tests {

#include "animals.hpp"

MULTI_METHOD(encounter, string, const virtual_<Animal>&, const virtual_<Animal>&);

BEGIN_SPECIALIZATION(encounter, string, const Animal&, const Animal&) {
  return "ignore";
} END_SPECIALIZATION;

DO {
  yorel::multi_methods::initialize();
  test(encounter(Cow(), Wolf()), "ignore");
  test(encounter(Wolf(), Cow()), "ignore");
}

BEGIN_SPECIALIZATION(encounter, string, const Herbivore&, const Carnivore&) {
  return "run";
} END_SPECIALIZATION;

DO {
  yorel::multi_methods::initialize();
  test(encounter(Cow(), Wolf()), "run");
  test(encounter(Wolf(), Cow()), "ignore");
}

BEGIN_SPECIALIZATION(encounter, string, const Carnivore&, const Herbivore&) {
  return "hunt";
} END_SPECIALIZATION;

DO {
  yorel::multi_methods::initialize();
  test(encounter(Cow(), Wolf()), "run");
  test(encounter(Wolf(), Cow()), "hunt");
}

struct Horse : Herbivore {
  MM_CLASS(Horse, Herbivore);
  Horse() {
    MM_INIT();
  }
};

DO {
  yorel::multi_methods::initialize();
  test(encounter(Horse(), Wolf()), "run");
  test(encounter(Wolf(), Horse()), "hunt");
}
}

#endif

namespace single_inheritance {

MULTI_METHOD(encounter, string, virtual_<Animal>&, virtual_<Animal>&);

BEGIN_SPECIALIZATION(encounter, string, Animal&, Animal&) {
  return "ignore";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Carnivore&, Animal&) {
  return "hunt";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Carnivore&, Carnivore&) {
  return "fight";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Wolf&, Wolf&) {
  return "wag tail";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Herbivore&, Carnivore&) {
  return "run";
} END_SPECIALIZATION;

enum action { display_error, print_cow, draw_cow, print_wolf, draw_wolf, print_tiger, draw_tiger, print_herbivore, display_cow, print_animal };

MULTI_METHOD(display, action, virtual_<Animal>&, virtual_<Interface>&);

BEGIN_SPECIALIZATION(display, action, Cow& a, Terminal& b) {
  return print_cow;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Wolf& a, Terminal& b) {
  return print_wolf;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Tiger& a, Terminal& b) {
  return print_tiger;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Cow& a, Window& b) {
  return draw_cow;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Wolf& a, Window& b) {
  return draw_wolf;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Tiger& a, Window& b) {
  return draw_tiger;
} END_SPECIALIZATION;

// following two are ambiguous, e.g. for (Cow, Terminal)

BEGIN_SPECIALIZATION(display, action, Herbivore& a, Interface& b) {
  return display_error;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(display, action, Animal& a, Terminal& b) {
  return display_error;
} END_SPECIALIZATION;

// following un-registered stuff is for unloading tests

struct Donkey : Herbivore { };

template<>
struct encounter_specialization<string(Cow&, Cow&)> : decltype(encounter)::specialization<encounter_specialization<string(Cow&, Cow&)>> {
  static string body(Cow&, Cow&) {
    return "moo!";
  }
};
}

namespace mi {
#include "mi.hpp"

MULTI_METHOD(encounter, string, virtual_<Animal>&, virtual_<Animal>&);

BEGIN_SPECIALIZATION(encounter, string, Animal&, Animal&) {
  return "ignore";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Stallion&, Mare&) {
  return "court";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, Predator&, Herbivore&) {
  return "hunt";
} END_SPECIALIZATION;
}

namespace adjust {
#define VIRTUAL
#include "adjust.hpp"
#undef VIRTUAL
}

namespace adjust_virtual {
#define VIRTUAL virtual
#include "adjust.hpp"
#undef VIRTUAL
}

namespace multi_roots {

struct X : selector {
  MM_CLASS(X);

  int x;

  X() {
    MM_INIT();
  }
};

struct Y : selector {
  MM_CLASS(Y);

  int y;

  Y() {
    MM_INIT();
  }
};

struct XY : X, Y {
  MM_CLASS_MULTI(XY, X, Y);

  XY() {
    MM_INIT();
  }
};

MULTI_METHOD(mx, int, const virtual_<X>&);

BEGIN_SPECIALIZATION(mx, int, const X& x) {
  return x.x;
} END_SPECIALIZATION;

MULTI_METHOD(my, int, const virtual_<Y>&);

BEGIN_SPECIALIZATION(my, int, const Y& y) {
  return y.y;
} END_SPECIALIZATION;

MULTI_METHOD(mxy, int, const virtual_<XY>&);

BEGIN_SPECIALIZATION(mxy, int, const XY& xy) {
  return xy.x + xy.y;
} END_SPECIALIZATION;

}

namespace multi_roots_foreign {

struct X {
  virtual ~X() { }
  int x;
};

MM_FOREIGN_CLASS(X);

struct Y {
  virtual ~Y() { }
  int y;
};

MM_FOREIGN_CLASS(Y);

struct XY : X, Y {
};

MM_FOREIGN_CLASS(XY, X, Y);

MULTI_METHOD(mx, int, const virtual_<X>&);

BEGIN_SPECIALIZATION(mx, int, const X& x) {
  return x.x;
} END_SPECIALIZATION;

MULTI_METHOD(my, int, const virtual_<Y>&);

BEGIN_SPECIALIZATION(my, int, const Y& y) {
  return y.y;
} END_SPECIALIZATION;

MULTI_METHOD(mxy, int, const virtual_<XY>&);

BEGIN_SPECIALIZATION(mxy, int, const XY& xy) {
  return xy.x + xy.y;
} END_SPECIALIZATION;

}

namespace repeated {

struct X : selector {
  MM_CLASS(X);

  int x;

  X() {
    MM_INIT();
  }
};

struct A : X {
  MM_CLASS(A, X);

  int a;

  A() {
    MM_INIT();
  }
};

struct B : X {
  MM_CLASS(B, X);

  int b;

  B() {
    MM_INIT();
  }
};

struct AB : A, B {
  MM_CLASS_MULTI(AB, A, B);

  AB() {
    MM_INIT();
  }
};
}

namespace yorel {
namespace multi_methods {
template<>
struct cast<repeated::X, repeated::AB> :
      cast_using_dynamic_cast<repeated::X, repeated::AB> { };
}
}

namespace repeated {

MULTI_METHOD(mx, int, const virtual_<X>&);

BEGIN_SPECIALIZATION(mx, int, const X& x) {
  return x.x;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(mx, int, const A& a) {
  return a.x + a.a;
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(mx, int, const AB& ab) {
  return ab.A::x + ab.B::x + ab.a + ab.b;
} END_SPECIALIZATION;

MULTI_METHOD(ma, int, const virtual_<A>&);

BEGIN_SPECIALIZATION(ma, int, const A& a) {
  return a.x + a.a;
} END_SPECIALIZATION;

MULTI_METHOD(mb, int, const virtual_<B>&);

BEGIN_SPECIALIZATION(mb, int, const B& b) {
  return b.x + b.b;
} END_SPECIALIZATION;

MULTI_METHOD(mab, int, const virtual_<AB>&);

BEGIN_SPECIALIZATION(mab, int, const AB& ab) {
  return ab.A::x + ab.B::x + ab.a + ab.b;
} END_SPECIALIZATION;

}

int main() {
  {
    using namespace single_inheritance;
    static_assert(
        is_same<
        typename extract_virtuals<virtual_<Animal>&, const virtual_<Animal>&>::type,
        virtuals<Animal, Animal>
        >::value, "not ok !!!");

    static_assert(
        is_same<
        typename extract_virtuals<virtual_<Animal>&, int, virtual_<Animal>&>::type,
        virtuals<Animal, Animal>
        >::value, "not ok !!!");

    static_assert(
        is_same<
        extract_method_virtuals<
        void(int, virtual_<Animal>&, char, const virtual_<Animal>&),
        void(int, Cow&, char, const Wolf&)
        >::type,
        virtuals<Cow, Wolf> >::value, "extraction of virtual method arguments");

    cout << "\nClass registration" << endl;
    test(mm_class::of<Cow>::the().bases.size(), 1);
    test(mm_class::of<Cow>::the().bases[0] == &mm_class::of<Herbivore>::the(), true);
    test(mm_class::of<Animal>::the().specs.size(), 2);
    test(mm_class::of<Animal>::the().specs[0] == &mm_class::of<Herbivore>::the(), true);
    test(mm_class::of<Animal>::the().specs[1] == &mm_class::of<Carnivore>::the(), true);

    test(mm_class::of<Animal>::the().root, mm_class::of<Animal>::the().root);
    test(mm_class::of<Herbivore>::the().root, mm_class::of<Animal>::the().root);
    test(mm_class::of<Carnivore>::the().root, mm_class::of<Animal>::the().root);
    test(mm_class::of<Cow>::the().root, mm_class::of<Animal>::the().root);
    test(mm_class::of<Wolf>::the().root, mm_class::of<Animal>::the().root);
  }

  {
    cout << "\n--- bitvec." << endl;

    {
      bitvec v;
      test(v.size(), 0);
      test(v.none(), true);
    }

    for (int n : { numeric_limits<unsigned long>::digits - 1,
            numeric_limits<unsigned long>::digits,
            numeric_limits<unsigned long>::digits + 1 }) {
      cout << "n = " << n << endl;

      {
        bitvec v(n);
        test(v.size(), n);
        test(v.none(), true);
        v[0] = 1;
        test(v[0], true);
        v[0] = 0;
        test(v[0], false);
        v[n - 1] = 1;
        test(v[n - 1], true);
        v[n - 1] = 0;
        test(v[n - 1], false);
      }

      {
        bitvec v(n, 0b101);
        test(v[0], true);
        test(v[1], false);
        test(v[2], true);
      }

      {
        bitvec v(n);
        v[0] = 1;
        v[n - 1] = 1;
        v.resize(n - 1);
        test(v[0], true);
        test(v[1], false);
        v.resize(n);
        test(v[n - 1], false);
        v[0] = false;
        test(v.none(), true);
      }

      {
        bitvec v(n);
        v[0] = 1;
        v[n - 1] = 1;
        v.resize(n + 1);
        test(v[0], true);
        test(v[1], false);
        test(v[n - 1], true);
        test(v[n], false);
      }

      {
        bitvec v(n);
        bitvec v2(v);
        v[0] = 1;
        v[n - 1] = 1;
        test(v2.none(), true);
      }

      {
        bitvec v(n);
        v[0] = 1;
        v[n - 1] = 1;
        bitvec v2;
        v2 = v;
        test(v2[0], true);
        test(v2[n - 1], true);
        v[0] = 0;
        v[n - 1] = 0;
        test(v2[0], true);
        test(v2[n - 1], true);
      }

      {
        bitvec v(n);
        test(v.none(), true);
        test((~~v).none(), true);
        v[0] = 1;
        v[n - 1] = 1;
        test(v.none(), false);
        v |= bitvec(n, 0b10);
        test(v[0], true);
        test(v[1], true);
        test(v[n  - 1], true);
      }

      {
        bitvec v1(n), v2(n);
        v1[0] = 1;
        v1[n - 1] = 1;
        v2[0] = 1;
        v2[n - 1] = 1;
        test(v1 == v2, true);
      }

      {
        bitvec v1(n), v2(n), v3(n);
        v1[0] = 1;
        v1[n - 1] = 1;
        v2[0] = 1;
        v2[n - 1] = 1;
        v1[1] = 1;
        v2[2] = 1;
        v3[0] = 1;
        v3[n - 1] = 1;
        test((v1 & v2) == v3, true);
      }

      {
        bitvec v(n);
        v[0] = 1;
        v[n - 1] = 1;
        test((v & bitvec(n)) == bitvec(n), true);
      }

      {
        bitvec v1(n), v2(n), v3(n);
        v1[0] = 1;
        v1[n - 1] = 1;
        v2[1] = 1;
        v2[n - 2] = 1;
        v3[0] = 1;
        v3[n - 1] = 1;
        v3[1] = 1;
        v3[n - 2] = 1;
        v1 |= v2;
        test(v1 == v3, true);
      }

      {
        bitvec v(n);
        v[0] = 1;
        v[n - 1] = 1;
        bitvec v2(~v);
        test(v2[0], false);
        test(v2[1], true);
        test(v2[n - 2], true);
        test(v2[n - 1], false);
      }
    }
    {
      bitvec v(2);
      test(v.none(), true);
      test(v[0], false);
      v[0] = 1;
      test(v.none(), false);
      test(v[0], true);
      test(v[1], false);
      v = ~v;
      test(v[0], false);
      test(v[1], true);
    }

    {
      bitvec v(3, 0b111);
      test(v.size(), 3);
      test(v[0], 1);
      test(v[1], 1);
      test(v[2], 1);
      v.resize(2);
      test(v.size(), 2);
      test(v[0], 1);
      test(v[1], 1);
      v.resize(3);
      test(v[0], 1);
      test(v[1], 1);
      test(v[2], 0);
    }
  }

  {
    cout << "\n--- Slot allocation." << endl;

    using namespace slot_allocation_tests;

    // Init multi_method implementations; this is normally done when the
    // first method is added.
    m_x.the();
    m_a.the();
    m_b.the();
    m_c.the();
    m_bc.the();
    m_d.the();
    m_cd.the();
    m_y.the();

    hierarchy_initializer init(mm_class::of<X>::the());

    init.collect_classes();
    test( init.nodes.size(), 8);
    test( init.nodes.size(), 8);
    test( init.nodes[0], &mm_class::of<X>::the() );
    test( init.nodes[1], &mm_class::of<A>::the() );
    test( init.nodes[2], &mm_class::of<B>::the() );
    test( init.nodes[3], &mm_class::of<C>::the() );
    test( init.nodes[4], &mm_class::of<BC>::the() );
    test( init.nodes[5], &mm_class::of<D>::the() );
    test( init.nodes[6], &mm_class::of<CD>::the() );
    test( init.nodes[7], &mm_class::of<Y>::the() );

    init.make_masks();
    testx( init.nodes[0]->mask, bitvec(8, 0b11111111) ); // X
    testx( init.nodes[1]->mask, bitvec(8, 0b01111110) ); // A
    testx( init.nodes[2]->mask, bitvec(8, 0b00010100) ); // B
    testx( init.nodes[3]->mask, bitvec(8, 0b01011000) ); // C
    testx( init.nodes[4]->mask, bitvec(8, 0b00010000) ); // BC
    testx( init.nodes[5]->mask, bitvec(8, 0b01100000) ); // D
    testx( init.nodes[6]->mask, bitvec(8, 0b01000000) ); // CD
    testx( init.nodes[7]->mask, bitvec(8, 0b10000000) ); // Y

    init.assign_slots();
    test(m_x.the().slots[0], 0);
    test(m_a.the().slots[0], 1);
    test(m_b.the().slots[0], 2);
    test(m_c.the().slots[0], 3);
    test(m_bc.the().slots[0], 4);
    test(m_d.the().slots[0], 2);
    test(m_cd.the().slots[0], 4);
    test(m_y.the().slots[0], 1);

    test(mm_class::of<X>::the().mmt.size(), 1);
    test(mm_class::of<A>::the().mmt.size(), 2);
    test(mm_class::of<B>::the().mmt.size(), 3);
    test(mm_class::of<C>::the().mmt.size(), 4);
    test(mm_class::of<BC>::the().mmt.size(), 5);
    test(mm_class::of<D>::the().mmt.size(), 3);
    test(mm_class::of<CD>::the().mmt.size(), 5);
    test(mm_class::of<Y>::the().mmt.size(), 2);
  }

  {
    cout << "\n--- Slot allocation - multiple roots." << endl;
    using namespace multi_roots;

    test(mm_class::of<X>::the().root, &mm_class::of<X>::the());
    test(mm_class::of<Y>::the().root, &mm_class::of<Y>::the());

    {
      hierarchy_initializer init(mm_class::of<X>::the());
      init.collect_classes();
      test(init.nodes.size(), 3);
      test(init.nodes[0], &init.root);
      test(init.nodes[0], &mm_class::of<X>::the());
      test(init.nodes[1], &mm_class::of<Y>::the());
      test(init.nodes[2], &mm_class::of<XY>::the());
    }

    {
      hierarchy_initializer init(mm_class::of<Y>::the());
      init.collect_classes();
      test(init.nodes.size(), 3);
      test(init.nodes[0], &mm_class::of<Y>::the());
      test(init.nodes[1], &mm_class::of<X>::the());
      test(init.nodes[2], &mm_class::of<XY>::the());
    }

    test(mm_class::of<X>::the().is_root(), true);
    test(mm_class::of<Y>::the().is_root(), true);
    test(mm_class::of<XY>::the().is_root(), false);

    mm_class::add_to_initialize(&mm_class::of<X>::the());
    mm_class::add_to_initialize(&mm_class::of<Y>::the());
    hierarchy_initializer::initialize(mm_class::of<Y>::the());
    test(mm_class::to_initialize->empty() ||
         find(mm_class::to_initialize->begin(),
              mm_class::to_initialize->end(),
              &mm_class::of<X>::the()) == mm_class::to_initialize->end(),
         true);
    test(mm_class::to_initialize->empty() ||
         find(mm_class::to_initialize->begin(),
              mm_class::to_initialize->end(),
              &mm_class::of<Y>::the()) == mm_class::to_initialize->end(),
         true);

    mm_class::add_to_initialize(&mm_class::of<X>::the());
    mm_class::add_to_initialize(&mm_class::of<Y>::the());
    hierarchy_initializer::initialize(mm_class::of<Y>::the());
    test(mm_class::to_initialize->empty() ||
         find(mm_class::to_initialize->begin(),
              mm_class::to_initialize->end(),
              &mm_class::of<X>::the()) == mm_class::to_initialize->end(),
         true);
    test(mm_class::to_initialize->empty() ||
         find(mm_class::to_initialize->begin(),
              mm_class::to_initialize->end(),
              &mm_class::of<Y>::the()) == mm_class::to_initialize->end(),
         true);
  }

  cout << "\n--- Grouping resolver tests\n";

  {
    using namespace grouping_resolver_tests;

    // we want to build:
    //              Interface   Terminal   Window+ Mobile+
    // Animal       0           0          0       mob
    // Herbivore+   0           p_herb     d_herb  mob
    // Carnivore+   0           p_carn     d_carn  mob

    hierarchy_initializer::initialize(mm_class::of<Animal>::the());
    hierarchy_initializer::initialize(mm_class::of<Interface>::the());

    grouping_resolver rdisp(display.the());

    methods animal_applicable;
    rdisp.find_applicable(0, &mm_class::of<Animal>::the(), animal_applicable);
    test( animal_applicable, methods { display.the().methods[4] } );

    methods herbivore_applicable;
    rdisp.find_applicable(0, &mm_class::of<Herbivore>::the(), herbivore_applicable);
    test( herbivore_applicable, (methods { display.the().methods[0], display.the().methods[1], display.the().methods[4] } ));

    methods cow_applicable;
    rdisp.find_applicable(0, &mm_class::of<Cow>::the(), cow_applicable);
    test( cow_applicable, (methods { display.the().methods[0], display.the().methods[1], display.the().methods[4] } ));

    methods carnivore_applicable;
    rdisp.find_applicable(0, &mm_class::of<Carnivore>::the(), carnivore_applicable);
    test( carnivore_applicable, (methods { display.the().methods[2],  display.the().methods[3], display.the().methods[4] }) );

    methods wolf_applicable;
    rdisp.find_applicable(0, &mm_class::of<Wolf>::the(), wolf_applicable);
    test( wolf_applicable, (methods { display.the().methods[2],  display.the().methods[3], display.the().methods[4] }) );

    methods interface_applicable;
    rdisp.find_applicable(1, &mm_class::of<Interface>::the(), interface_applicable);
    test( interface_applicable, methods { } );

    methods terminal_applicable;
    rdisp.find_applicable(1, &mm_class::of<Terminal>::the(), terminal_applicable);
    test( terminal_applicable, (methods { display.the().methods[0], display.the().methods[2] }) );

    methods window_applicable;
    rdisp.find_applicable(1, &mm_class::of<Window>::the(), window_applicable);
    test( window_applicable, (methods { display.the().methods[1], display.the().methods[3] }) );

    methods mobile_applicable;
    rdisp.find_applicable(1, &mm_class::of<Mobile>::the(), mobile_applicable);
    test( mobile_applicable, (methods { display.the().methods[4] }) );

    // Animal = class_0
    // Herbivore = class_1
    // Cow = class_2
    // Carnivore = class_3
    // Wolf = class_4
    // Tiger = class_5

    // Interface = class_0
    // Terminal = class_1
    // Window = class_2
    // MsWindows = class_3
    // X = class_4
    // Mobile = class_5
    // Nokia = class_6
    // Samsung = class_7

    rdisp.make_groups();
    test( rdisp.groups.size(), 2 ) &&
        test( rdisp.groups[0].size(), 3) &&
        test( rdisp.groups[0][0].methods, animal_applicable) &&
        test( rdisp.groups[0][1].methods, herbivore_applicable) &&
        test( rdisp.groups[0][2].methods, carnivore_applicable) &&
        test( rdisp.groups[1].size(), 4) &&
        test( rdisp.groups[1][0].methods, interface_applicable) &&
        test( rdisp.groups[1][1].methods, terminal_applicable) &&
        test( rdisp.groups[1][2].methods, window_applicable) &&
        test( rdisp.groups[1][3].methods, mobile_applicable);

    test(display.the().steps.size(), 2) &&
        test(display.the().steps[0], 1) &&
        test(display.the().steps[1], 3);

    test( (*Animal()._yomm11_ptbl)[0].index, 0 );
    test( (*Herbivore()._yomm11_ptbl)[0].index, 1 );
    test( (*Cow()._yomm11_ptbl)[0].index, 1 );
    test( (*Carnivore()._yomm11_ptbl)[0].index, 2 );
    test( (*Wolf()._yomm11_ptbl)[0].index, 2 );
    test( (*Tiger()._yomm11_ptbl)[0].index, 2 );
    test( (*Interface()._yomm11_ptbl).size(), 1 );
    test( (*Interface()._yomm11_ptbl)[0].index, 0 );
    test( (*Terminal()._yomm11_ptbl)[0].index, 1 );
    test( (*Window()._yomm11_ptbl)[0].index, 2 );
    test( (*MSWindows()._yomm11_ptbl)[0].index, 2 );
    test( (*X()._yomm11_ptbl)[0].index, 2 );
    test( (*Mobile()._yomm11_ptbl)[0].index, 3 );
    test( (*Nokia()._yomm11_ptbl)[0].index, 3 );
    test( (*Samsung()._yomm11_ptbl)[0].index, 3 );

    rdisp.make_table();
    auto table = display.the().dispatch_table;
    auto methods = display.the().methods;
    using method = decltype(display)::method_entry;

    test( table != 0, true);
    // Interface
    test( table[0], throw_undefined<decltype(display)::signature>::body );
    test( table[1], throw_undefined<decltype(display)::signature>::body );
    test( table[2], throw_undefined<decltype(display)::signature>::body );

    // Terminal
    test( table[3], throw_undefined<decltype(display)::signature>::body );
    test( table[4], static_cast<method*>(methods[0])->pm );
    test( table[5], static_cast<method*>(methods[2])->pm );

    // Window
    test( table[6], throw_undefined<decltype(display)::signature>::body );
    test( table[7], static_cast<method*>(methods[1])->pm );
    test( table[8], static_cast<method*>(methods[3])->pm );

    // Mobile
    test( table[9], static_cast<method*>(methods[4])->pm );
    test( table[10], static_cast<method*>(methods[4])->pm );
    test( table[11], static_cast<method*>(methods[4])->pm );

    rdisp.assign_next();
    test( (display_specialization<action(const Carnivore&, const Window&)>::next) == nullptr, true );

    testx( (void*) mm_class::of<Animal>::the().mmt[0].ptr,
           (void*) display.impl->dispatch_table );

    testx( (void*) mm_class::of<Herbivore>::the().mmt[0].ptr,
           (void*) (display.impl->dispatch_table + 1) );

    test( display(Herbivore(), Terminal()), print_herbivore );
    test( display(Cow(), Terminal()), print_herbivore );

    test( display(Carnivore(), Terminal()), print_carnivore );
    test( display(Wolf(), Terminal()), print_carnivore );
    test( display(Tiger(), Terminal()), print_carnivore );

    test( display(Herbivore(), Window()), draw_herbivore );
    test( display(Cow(), Window()), draw_herbivore );
    test( display(Cow(), MSWindows()), draw_herbivore );
    test( display(Cow(), X()), draw_herbivore );

    test( display(Carnivore(), Window()), draw_carnivore );
    test( display(Wolf(), X()), draw_carnivore );
    test( display(Tiger(), MSWindows()), draw_carnivore );

    test( display(Herbivore(), Samsung()), mobile );
    test( display(Cow(), Nokia()), mobile );

    test( display(Carnivore(), Samsung()), mobile );
    test( display(Wolf(), Nokia()), mobile );

    test( decltype(display)::method(Wolf(), Nokia()), mobile );
  }

  cout << "\n--- Single inheritance." << endl;

  {
    using namespace single_inheritance;

    Cow c;
    Wolf w;
    Tiger t;
    Terminal term;
    Window win;
    Interface interf;
    Herbivore herb;

    yorel::multi_methods::initialize();

    test(encounter.impl != nullptr, true);
    test(encounter.impl->dispatch_table != nullptr, true);
    test(encounter(c, w), "run");
    test(encounter(c, c), "ignore");

    // static call
    test(GET_SPECIALIZATION(encounter, string, Animal&, Animal&)(c, w), "ignore");

    // next
    test(encounter_specialization<string(Wolf&, Wolf&)>::next(w, w), "fight");
    test(encounter_specialization<string(Carnivore&, Carnivore&)>::next(w, w), "hunt");
    test(encounter_specialization<string(Carnivore&, Animal&)>::next(w, w), "ignore");
  }

  cout << "\n--- multiple inheritance" << endl;

  {
    using namespace mi;

    Animal animal;
    Herbivore herbivore;
    Stallion stallion;
    Mare mare;
    Wolf wolf;

    static_assert(is_virtual_base_of<Animal, Stallion>::value, "problem with virtual base detection");

    yorel::multi_methods::initialize();

    testx( (void*) mm_class::of<Animal>::the().mmt[0].ptr,
           (void*) encounter.impl->dispatch_table );

    testx( (void*) mm_class::of<Herbivore>::the().mmt[0].ptr,
           (void*) (encounter.impl->dispatch_table) );

    testx( (void*) mm_class::of<Stallion>::the().mmt[0].ptr,
           (void*) (encounter.impl->dispatch_table + 1) );

    test( encounter(animal, animal), "ignore" );
    test( encounter(herbivore, herbivore), "ignore" );
    test( encounter(stallion, mare), "court" );
    test( encounter(mare, mare), "ignore" );
    test( encounter(wolf, mare), "hunt" );
  }

  {
    cout << "\n--- adjustments." << endl;
    using namespace adjust;

    A a;
    a.val = 2;
    B b;
    b.val = 5;
    X& xa = a;
    X& xb = b;

    test( (&cast<A, A>::value(a)), &a);
    test( (&cast<X, B>::value(xb)), &b);
    test( (&cast<X, B>::value(xb)), &b);

    test( (void*) &b != (void*) (X*) &a, true );

    test( foo(a, a), 4 );
    test( foo(a, b), -3 );
    test( foo(b, b), 25 );
  }

  {
    cout << "\n--- adjustments." << endl;
    using namespace adjust_virtual;

    A a;
    a.val = 2;
    B b;
    b.val = 5;

    test( (void*) &a != (void*) (X*) &a, true );
    test( (void*) &b != (void*) (X*) &a, true );

    test( foo(a, a), 4 );
    test( foo(a, b), -3 );
    test( foo(b, b), 25 );
  }

  {
    cout << "\n--- Unloading multi_methods." << endl;
    using namespace single_inheritance;

    encounter.the().add_spec<encounter_specialization<string(Cow&, Cow&)>>();

    test( mm_class::of<Animal>::the().rooted_here.size(), 3 );
    test( mm_class::of<Interface>::the().rooted_here.size(), 1 );
    test( multi_method_base::to_initialize != nullptr, true );
    test( multi_method_base::to_initialize->size(), 1 );

    encounter.impl.reset();
    test( mm_class::of<Animal>::the().rooted_here.size(), 1 );
    test( !multi_method_base::to_initialize, true );

    cout << "\n--- Unloading classes." << endl;
    {
      // fake a class
      mm_class donkey_class(typeid(Donkey));
      donkey_class.initialize(mm_class_vector_of<Herbivore>::get());
      test( mm_class::to_initialize != nullptr, true );
      test( mm_class::to_initialize->size(), 1 );
      yorel::multi_methods::initialize();
      test( !mm_class::to_initialize, true );
    }

    test( mm_class::to_initialize != nullptr, true );
    test( mm_class::to_initialize->size(), 1 );
    yorel::multi_methods::initialize();
    test( !mm_class::to_initialize, true );
  }

  {
    cout << "\n--- Adjustments." << endl;
    using namespace adjust;

    A a;
    a.val = 2;
    B b;
    b.val = 5;

    test( (void*) &b != (void*) (X*) &a, true );

    test( foo(a, a), 4 );
    test( foo(a, b), -3 );
    test( foo(b, b), 25 );
  }

  {
    cout << "\n--- Adjustments - virtual." << endl;
    using namespace adjust_virtual;

    A a;
    a.val = 2;
    B b;
    b.val = 5;

    test( (void*) &a != (void*) (X*) &a, true );
    test( (void*) &b != (void*) (X*) &a, true );

    test( foo(a, a), 4 );
    test( foo(a, b), -3 );
    test( foo(b, b), 25 );
  }

  {
    cout << "\n--- Multiple roots." << endl;
    using namespace multi_roots;

    XY xy;
    xy.x = 1;
    xy.y = 2;

    test( xy.X::_yomm11_ptbl == xy.Y::_yomm11_ptbl, true );

    test( mx(xy), 1 );
    test( my(xy), 2 );
    test( mxy(xy), 3 );
  }

  {
    cout << "\n--- Multiple roots - foreign." << endl;
    using namespace multi_roots_foreign;

    XY xy;
    xy.x = 1;
    xy.y = 2;

    test( mx(xy), 1 );
    test( my(xy), 2 );
    test( mxy(xy), 3 );
  }

  {
    cout << "\n--- Repeated." << endl;
    using namespace repeated;

    AB ab;
    ab.A::x = 2;
    ab.B::x = 3;
    ab.a = 5;
    ab.b = 7;
    A& a = ab;
    B& b = ab;

    test( ab.A::_yomm11_ptbl == ab.B::_yomm11_ptbl, true );

    test( ma(ab), 7 );
    test( mb(ab), 10 );
    test( mab(ab), 17 );
    test( mx(a), 17 );
    test( mx(b), 17 );
  }

  cout << "\n" << success << " tests succeeded, " << failure << " failed.\n";

  return 0;
}
