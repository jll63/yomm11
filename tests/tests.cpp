// -*- compile-command: "make runtests" -*-

// tests.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "multimethods.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util/join.hpp"

using namespace std;
using namespace multimethods;
using boost::dynamic_bitset;

#define test(exp, res) _test(__FILE__, __LINE__, #exp, exp, #res, res)
#define testx(exp, res) _test(__FILE__, __LINE__, #exp, exp, 0, res)

namespace {
  int success, failure;
}

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

#define DO void CONCAT(fun, __LINE__)(); int CONCAT(var, __LINE__) = (CONCAT(fun, __LINE__)(), 1); void CONCAT(fun, __LINE__)()

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

  DO {
    cout << "\nClass registration" << endl;
    test(mm_class_of<Cow>::the().bases.size(), 1);
    test(mm_class_of<Cow>::the().bases[0] == &mm_class_of<Herbivore>::the(), true);
    test(mm_class_of<Animal>::the().specs.size(), 2);
    test(mm_class_of<Animal>::the().specs[0] == &mm_class_of<Herbivore>::the(), true);
    test(mm_class_of<Animal>::the().specs[1] == &mm_class_of<Carnivore>::the(), true);

    test(mm_class_of<Animal>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Herbivore>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Carnivore>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Cow>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Wolf>::the().root, mm_class_of<Animal>::the().root);

    
    test(mm_class_of<Animal>::the().abstract, true);
    //test(mm_class_of<Herbivore>::the().abstract, true);
    test(mm_class_of<Carnivore>::the().abstract, true);
    test(mm_class_of<Cow>::the().abstract, false);
    test(mm_class_of<Wolf>::the().abstract, false);
    test(mm_class_of<Tiger>::the().abstract, false);

    cout << "\nClass queries" << endl;
    test(mm_class_of<Wolf>::the().conforms_to(mm_class_of<Wolf>::the()), true);
    test(mm_class_of<Wolf>::the().conforms_to(mm_class_of<Animal>::the()), true);
    test(mm_class_of<Wolf>::the().specializes(mm_class_of<Animal>::the()), true);
    test(mm_class_of<Wolf>::the().specializes(mm_class_of<Wolf>::the()), false);
    test(mm_class_of<Carnivore>::the().specializes(mm_class_of<Wolf>::the()), false);
    test(mm_class_of<Wolf>::the().specializes(mm_class_of<Carnivore>::the()), true);
  }
}

namespace slot_allocation_tests {

  DO {
    cout << "\n--- Slot allocation." << endl;
  }

  struct X : selector {
    MM_CLASS(X);
    X() { MM_INIT(); }
  };

  MULTIMETHOD(m_x, int(const virtual_<X>&));

  struct A : X {
    MM_CLASS(A, X);
    A() { MM_INIT(); }
  };

  MULTIMETHOD(m_a, int(const virtual_<A>&));

  struct B : virtual A {
    MM_CLASS(B, A);
    B() { MM_INIT(); }
  };

  MULTIMETHOD(m_b, int(const virtual_<B>&));

  struct C : virtual A {
    MM_CLASS(C, A);
    C() { MM_INIT(); }
  };

  MULTIMETHOD(m_c, int(const virtual_<C>&));

  struct D : virtual A {
    MM_CLASS(D, A);
    D() { MM_INIT(); }
  };

  MULTIMETHOD(m_d, int(const virtual_<D>&));

  struct BC : B, C {
    MM_CLASS(BC, B, C);
    BC() { MM_INIT(); }
  };

  MULTIMETHOD(m_bc, int(const virtual_<BC>&));

  struct CD : C, D {
    MM_CLASS(CD, C, D);
    CD() { MM_INIT(); }
  };

  MULTIMETHOD(m_cd, int(const virtual_<CD>&));

  struct Y : virtual X {
    MM_CLASS(Y, X);
    Y() { MM_INIT(); }
  };

  MULTIMETHOD(m_y, int(const virtual_<Y>&));

  DO {
    m_x.the();
    m_a.the();
    m_b.the();
    m_c.the();
    m_bc.the();
    m_d.the();
    m_cd.the();
    m_y.the();

    hierarchy_initializer init(mm_class_of<X>::the());

    init.collect_classes();
    test( init.nodes.size(), 8);
    test( init.nodes.size(), 8);
    test( init.nodes[0].pc, &mm_class_of<X>::the() );
    test( init.nodes[1].pc, &mm_class_of<A>::the() );
    test( init.nodes[2].pc, &mm_class_of<B>::the() );
    test( init.nodes[3].pc, &mm_class_of<C>::the() );
    test( init.nodes[4].pc, &mm_class_of<BC>::the() );
    test( init.nodes[5].pc, &mm_class_of<D>::the() );
    test( init.nodes[6].pc, &mm_class_of<CD>::the() );
    test( init.nodes[7].pc, &mm_class_of<Y>::the() );

    init.make_masks();
    testx( init.nodes[0].mask, dynamic_bitset<>(8, 0b11111111) ); // X
    testx( init.nodes[1].mask, dynamic_bitset<>(8, 0b01111110) ); // A
    testx( init.nodes[2].mask, dynamic_bitset<>(8, 0b00010100) ); // B
    testx( init.nodes[3].mask, dynamic_bitset<>(8, 0b01011000) ); // C
    testx( init.nodes[4].mask, dynamic_bitset<>(8, 0b00010000) ); // BC
    testx( init.nodes[5].mask, dynamic_bitset<>(8, 0b01100000) ); // D
    testx( init.nodes[6].mask, dynamic_bitset<>(8, 0b01000000) ); // CD
    testx( init.nodes[7].mask, dynamic_bitset<>(8, 0b10000000) ); // Y

    init.assign_slots();
    test(m_x.the().slots[0], 0);
    test(m_a.the().slots[0], 1);
    test(m_b.the().slots[0], 2);
    test(m_c.the().slots[0], 3);
    test(m_bc.the().slots[0], 4);
    test(m_d.the().slots[0], 2);
    test(m_cd.the().slots[0], 4);
    test(m_y.the().slots[0], 1);

    test(mm_class_of<X>::the().mmt.size(), 1);
    test(mm_class_of<A>::the().mmt.size(), 2);
    test(mm_class_of<B>::the().mmt.size(), 3);
    test(mm_class_of<C>::the().mmt.size(), 4);
    test(mm_class_of<BC>::the().mmt.size(), 5);
    test(mm_class_of<D>::the().mmt.size(), 3);
    test(mm_class_of<CD>::the().mmt.size(), 5);
    test(mm_class_of<Y>::the().mmt.size(), 2);
  }
}

namespace single_inheritance {

  DO {
    cout << "\n--- Single inheritance." << endl;
  }

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

  static_assert(!is_virtual_base_of<Animal, Animal>::value, "problem with virtual base detection");
  static_assert(!is_virtual_base_of<Animal, Herbivore>::value, "problem with virtual base detection");
  static_assert(!is_virtual_base_of<Animal, Wolf>::value, "problem with virtual base detection");
  
  static_assert(
    is_same<
    extract_method_virtuals<
    void(int, virtual_<Animal>&, char, const virtual_<Animal>&),
    void(int, Cow&, char, const Wolf&)
    >::type,
    virtuals<Cow, Wolf> >::value, "extraction of virtual method arguments");

  MULTIMETHOD(encounter, string(virtual_<Animal>&, virtual_<Animal>&));
  
  BEGIN_METHOD(encounter, string, Animal&, Animal&) {
    return "ignore";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Carnivore&, Animal&) {
    return "hunt";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Carnivore&, Carnivore&) {
    return "fight";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Wolf&, Wolf&) {
    return "wag tail";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Herbivore&, Carnivore&) {
    return "run";
  } END_METHOD;

  enum action { display_error, print_cow, draw_cow, print_wolf, draw_wolf, print_tiger, draw_tiger, print_herbivore, display_cow, print_animal };

  MULTIMETHOD(display, action(virtual_<Animal>&, virtual_<Interface>&));

  BEGIN_METHOD(display, action, Cow& a, Terminal& b) {
    return print_cow;
  } END_METHOD;

  BEGIN_METHOD(display, action, Wolf& a, Terminal& b) {
    return print_wolf;
  } END_METHOD;

  BEGIN_METHOD(display, action, Tiger& a, Terminal& b) {
    return print_tiger;
  } END_METHOD;

  BEGIN_METHOD(display, action, Cow& a, Window& b) {
    return draw_cow;
  } END_METHOD;

  BEGIN_METHOD(display, action, Wolf& a, Window& b) {
    return draw_wolf;
  } END_METHOD;

  BEGIN_METHOD(display, action, Tiger& a, Window& b) {
    return draw_tiger;
  } END_METHOD;

// following two are ambiguous, e.g. for (Cow, Terminal)

  BEGIN_METHOD(display, action, Herbivore& a, Interface& b) {
    return display_error;
  } END_METHOD;

  BEGIN_METHOD(display, action, Animal& a, Terminal& b) {
    return display_error;
  } END_METHOD;

  DO {
    {
      hierarchy_initializer hinit(mm_class_of<Animal>::the());
      hinit.execute();
    }

    test(encounter.the().vargs.size(), 2);
    test(encounter.the().slots.size(), 2);
    test(encounter.the().slots[0], 0);
    test(encounter.the().slots[1], 1);

    test(mm_class_of<Animal>::the().mmt.size(), 3);
    test(mm_class_of<Herbivore>::the().mmt.size(), 3);
    test(mm_class_of<Herbivore>::the().mmt.size(), 3);
    test(mm_class_of<Cow>::the().mmt.size(), 3);
    test(mm_class_of<Wolf>::the().mmt.size(), 3);
    test(mm_class_of<Tiger>::the().mmt.size(), 3);

    {
      hierarchy_initializer hinit(mm_class_of<Interface>::the());
      hinit.execute();
    }
  }

  DO {
    cout << "\n*** method registration" << endl;

    int i = 0;
    auto aa = encounter.the().methods[i++];
    auto ca = encounter.the().methods[i++];
    auto cc = encounter.the().methods[i++];
    auto ww = encounter.the().methods[i++];
    auto hc = encounter.the().methods[i++];

    test(encounter.the().methods.size(), 5);

    test(encounter.the().methods[4]->args.size(), 2);
    test(encounter.the().methods[4]->args[0], &mm_class_of<Herbivore>::the());
    test(encounter.the().methods[4]->args[0]->name(), mm_class_of<Herbivore>::the().name());
    test(encounter.the().methods[4]->args[1]->name(), mm_class_of<Carnivore>::the().name());

    cout << "\nResolver\n";
    resolver renc(encounter.the());

    test(renc.classes.size(), 2);
    test(renc.classes[0].size(), 6);
    test(renc.classes[0][0]->ti.name(), mm_class_of<Animal>::the().ti.name());
    test(renc.classes[0][1]->ti.name(), mm_class_of<Herbivore>::the().ti.name());
    test(renc.classes[0][2]->ti.name(), mm_class_of<Cow>::the().ti.name());
    test(renc.classes[0][3]->ti.name(), mm_class_of<Carnivore>::the().ti.name());
    test(renc.classes[0][4]->ti.name(), mm_class_of<Wolf>::the().ti.name());
    test(renc.classes[0][5]->ti.name(), mm_class_of<Tiger>::the().ti.name());

    test(encounter.the().steps.size(), 2);
    test(encounter.the().steps[0], 1);
    test(encounter.the().steps[1], 6);
    test(renc.linear(vector<int>{ 0, 1 }), 6);
    test(renc.linear(vector<int>{ 2, 2 }), 14);

    cout << "methods : " << renc.methods << endl;
    test(renc.order(ww, ww), 0);
    test(renc.order(ww, hc), 0);
    test(renc.order(cc, ww), 1);
    test(renc.order(hc, ca), 0);
    test(renc.order(hc, aa), -1);

    cout << "\ndisplay resolver\n";
    resolver rdisp(display.the());

    test(rdisp.classes.size(), 2);
    test(rdisp.classes[0].size(), 6);
    test(rdisp.classes[1].size(), 3);

    test(rdisp.classes[0][0]->ti.name(), mm_class_of<Animal>::the().ti.name());
    test(rdisp.classes[0][1]->ti.name(), mm_class_of<Herbivore>::the().ti.name());
    test(rdisp.classes[0][2]->ti.name(), mm_class_of<Cow>::the().ti.name());
    test(rdisp.classes[0][3]->ti.name(), mm_class_of<Carnivore>::the().ti.name());
    test(rdisp.classes[0][4]->ti.name(), mm_class_of<Wolf>::the().ti.name());
    test(rdisp.classes[0][5]->ti.name(), mm_class_of<Tiger>::the().ti.name());
    test(rdisp.classes[1][0]->ti.name(), mm_class_of<Interface>::the().ti.name());
    test(rdisp.classes[1][1]->ti.name(), mm_class_of<Terminal>::the().ti.name());
    test(rdisp.classes[1][2]->ti.name(), mm_class_of<Window>::the().ti.name());

    rdisp.make_steps();

    test(display.the().steps.size(), 2);
    test(display.the().steps[0], 1);
    test(display.the().steps[1], 6);
    test(rdisp.linear(vector<int>{ 0, 1 }), 6);
    test(rdisp.linear(vector<int>{ 2, 2 }), 14);

    int m = 0;
    using display_method_entry = decltype(display)::method_entry;

    auto animal_interface = new method_base;
    animal_interface->args.push_back(&mm_class_of<Animal>::the());
    animal_interface->args.push_back(&mm_class_of<Interface>::the());
      
    auto cow_terminal = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto wolf_terminal = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto tiger_terminal = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto cow_window = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto wolf_window = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto tiger_window = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto herbivore_interface = static_cast<display_method_entry*>(display.the().methods[m++]);
    auto animal_terminal = static_cast<display_method_entry*>(display.the().methods[m++]);

    test(renc.order(cow_window, animal_interface), -1);
    test(renc.order(cow_terminal, animal_interface), -1);
    test(renc.order(wolf_window, animal_interface), -1);
    test(renc.order(wolf_terminal, animal_interface), -1);
    test(renc.order(tiger_window, animal_interface), -1);
    test(renc.order(tiger_terminal, animal_interface), -1);
    test(renc.order(herbivore_interface, animal_interface), -1);
    test(renc.order(animal_terminal, animal_interface), -1);
    test(renc.order(animal_terminal, herbivore_interface), 0);

    {
      cout << "\nfind_best\n";
      cout << herbivore_interface << endl;
      cout << animal_terminal << endl;

      using methods = vector<method_base*>;
      
      methods viable;

      test(rdisp.find_best(methods { }), &method_base::undefined);
      test(rdisp.find_best(methods { animal_interface }), animal_interface);
      test(rdisp.find_best(methods { animal_interface, cow_window }), cow_window);
      test(rdisp.find_best(methods { cow_window, animal_interface }), cow_window);
      test(rdisp.find_best(methods { herbivore_interface, animal_terminal }), &method_base::ambiguous);
//      }

      cout << "\ndisplay.resolve()\n";
      test(rdisp.dispatch_table_size, 18);
      
      display.the().allocate_dispatch_table(rdisp.dispatch_table_size);
      rdisp.resolve();

      {
        auto mptr = display.the().dispatch_table;
        //              Interface   Terminal   Window  
        // Animal       0           at         0
        // Herbivore    hi          ?          hi 
        // Cow          hi          ct         cw
        // Carnivore    0           at         0
        // Wolf         0           wt         ww
        // Tiger        0           tt         tw

        // 1st column: Interface
        // Animal
        test(mptr[0], throw_undefined<action(Animal&, Interface&)>::body);
        // Herbivore
        test(mptr[1], herbivore_interface->pm);
        // Cow
        test(mptr[2], herbivore_interface->pm);
        // Carnivore
        test(mptr[3], throw_undefined<action(Animal&, Interface&)>::body);
        // Wolf
        test(mptr[4], throw_undefined<action(Animal&, Interface&)>::body);
        // Tiger
        test(mptr[5], throw_undefined<action(Animal&, Interface&)>::body);

        // 2nd column: Terminal
        // Animal
        test(mptr[6 + 0], animal_terminal->pm);
        // Herbivore
        test(mptr[6 + 1], throw_ambiguous<action(Animal&, Interface&)>::body);
        // Cow
        test(mptr[6 + 2], cow_terminal->pm);
        // Carnivore
        test(mptr[6 + 3], animal_terminal->pm);
        // Wolf
        test(mptr[6 + 4], wolf_terminal->pm);
        // Tiger
        test(mptr[6 + 5], tiger_terminal->pm);

        // 3rd column: Window
        // Animal
        test(mptr[12 + 0], throw_undefined<action(Animal&, Interface&)>::body);
        // Herbivore
        test(mptr[12 + 1], herbivore_interface->pm);
        // Cow
        test(mptr[12 + 2], cow_window->pm);
        // Carnivore
        test(mptr[12 + 3], throw_undefined<action(Animal&, Interface&)>::body);
        // Wolf
        test(mptr[12 + 4], wolf_window->pm);
        // Tiger
        test(mptr[12 + 5], tiger_window->pm);
      }
    }

    {
      Cow c;
      Wolf w;
      Tiger t;
      Terminal term;
      Window win;
      Interface interf;
      Herbivore herb;

      test(c.__mm_ptbl->size(), 3); // three because 1 for display and two for encounter
      test((*c.__mm_ptbl)[0], 2); // encounter(1): 0 => Animal, 1 => Herbivore, 2 => Cow
      test((*c.__mm_ptbl)[1], 2); // encounter(2): 0 => Animal, 1 => Herbivore, 2 => Cow
      test((*c.__mm_ptbl)[2], 2); // display(1): 0 => Animal, 1 => Herbivore, 2 => Cow

      test(w.__mm_ptbl->size(), 3); // four because 1 for hunt, 2 for encounter and 1 for display
      test((*w.__mm_ptbl)[0], 4); // encounter(1): 3 => Carnivore, 4 => Wolf
      test((*w.__mm_ptbl)[1], 4); // encounter(2)
      test((*w.__mm_ptbl)[2], 4); // display(1)

      test(t.__mm_ptbl->size(), 3); // three because 1 for display and two for encounter
      test((*t.__mm_ptbl)[0], 5); // encounter(1): 5 => Tiger
      test((*t.__mm_ptbl)[1], 5); // encounter(2)
      test((*t.__mm_ptbl)[2], 5); // display(1)

      test(term.__mm_ptbl->size(), 1); // display(2)
      test((*term.__mm_ptbl)[0], 1); // 0 => Interface, 1 => Terminal

      test(win.__mm_ptbl->size(), 1); // display(2)
      test((*win.__mm_ptbl)[0], 2); // 2 => Window

      test((linear<virtual_<Animal>&, virtual_<Interface>&>::value(display.the().slots.begin(), display.the().steps.begin(), 0, &c, &term)), 8);

      test(display(c, term), print_cow);
      test(display(w, win), draw_wolf);
      test(throws<undefined>([&]() { display(w, interf); }), true);
      test(throws<ambiguous>([&]() { display(herb, term); }), true);

      encounter.the().resolve();
      test(encounter(c, w), "run");
      test(encounter(c, c), "ignore");

      // static call
      test(STATIC_CALL_METHOD(encounter, string(Animal&, Animal&))(c, w), "ignore");
      
      // next
      test(encounter_method<string(Wolf&, Wolf&)>::next(w, w), "fight");
      test(encounter_method<string(Carnivore&, Carnivore&)>::next(w, w), "hunt");
      test(encounter_method<string(Carnivore&, Animal&)>::next(w, w), "ignore");
    }
  }
}

namespace init_tests {

#include "animals.hpp"

  MULTIMETHOD(encounter, string(const virtual_<Animal>&, const virtual_<Animal>&));

  BEGIN_METHOD(encounter, string, const Animal&, const Animal&) {
    return "ignore";
  } END_METHOD;

  DO {
    multimethods::initialize();
    test(encounter(Cow(), Wolf()), "ignore");
    test(encounter(Wolf(), Cow()), "ignore");
  }

  BEGIN_METHOD(encounter, string, const Herbivore&, const Carnivore&) {
    return "run";
  } END_METHOD;

  DO {
    multimethods::initialize();
    test(encounter(Cow(), Wolf()), "run");
    test(encounter(Wolf(), Cow()), "ignore");
  }

  BEGIN_METHOD(encounter, string, const Carnivore&, const Herbivore&) {
    return "hunt";
  } END_METHOD;

  DO {
    multimethods::initialize();
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
    multimethods::initialize();
    test(encounter(Horse(), Wolf()), "run");
    test(encounter(Wolf(), Horse()), "hunt");
  }
}

namespace mi {
  #include "mi.hpp"

  MULTIMETHOD(encounter, string(virtual_<Animal>&, virtual_<Animal>&));
  
  BEGIN_METHOD(encounter, string, Animal&, Animal&) {
    return "ignore";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Stallion&, Mare&) {
    return "court";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Predator&, Herbivore&) {
    return "hunt";
  } END_METHOD;

  DO {
    Stallion stallion;
    Mare mare;
    Wolf wolf;

    static_assert(is_virtual_base_of<Animal, Stallion>::value, "problem with virtual base detection");

    multimethods::initialize();
    
    test( encounter(stallion, mare), "court" );
    test( encounter(mare, mare), "ignore" );
    test( encounter(wolf, mare), "hunt" );
  }
}

int main() {
    cout << "\n" << success << " tests succeeded, " << failure << " failed.\n";
  return 0;
}
