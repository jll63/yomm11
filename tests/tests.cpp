// -*- compile-command: "make runtests" -*-

#include "multimethods.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util/join.hpp"

using namespace std;
using namespace multimethods;

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
    cout << file << ":" << line << ": error: " << got << ", expected " << ee << ".\n";
    ++failure;
  }
  return ok;
}

#define DO void CONCAT(fun, __LINE__)(); int CONCAT(var, __LINE__) = (CONCAT(fun, __LINE__)(), 1); void CONCAT(fun, __LINE__)()


#include "animals.hpp"

DO {
  cout << "\nClass registration" << endl;
  test(mm_class_of<Cow>::the().bases.size(), 1);
  test(mm_class_of<Cow>::the().bases[0] == &mm_class_of<Herbivore>::the(), true);
  test(mm_class_of<Animal>::the().specs.size(), 2);
  test(mm_class_of<Animal>::the().specs[0] == &mm_class_of<Herbivore>::the(), true);
  test(mm_class_of<Animal>::the().specs[1] == &mm_class_of<Carnivore>::the(), true);

  test(mm_class_of<Animal>::the().abstract, true);
  //test(mm_class_of<Herbivore>::the().abstract, true);
  test(mm_class_of<Carnivore>::the().abstract, true);
  test(mm_class_of<Cow>::the().abstract, false);
  test(mm_class_of<Wolf>::the().abstract, false);
  test(mm_class_of<Tiger>::the().abstract, false);

  cout << "\nClass queries" << endl;
  test(mm_class_of<Wolf>::the().conforms_to(mm_class_of<Wolf>::the()), true);
  test(mm_class_of<Wolf>::the().conforms_to(mm_class_of<Animal>::the()), true);
  test(mm_class_of<Wolf>::the().dominates(mm_class_of<Animal>::the()), true);
  test(mm_class_of<Wolf>::the().dominates(mm_class_of<Wolf>::the()), false);
  test(mm_class_of<Carnivore>::the().dominates(mm_class_of<Wolf>::the()), false);
  test(mm_class_of<Wolf>::the().dominates(mm_class_of<Carnivore>::the()), true);
}

  DO {
    cout << "\n*** Multimethod registration" << endl;
    cout << "Hunt...\n";
  }

MULTIMETHOD(hunt, void(virtual_<Carnivore&>));

DO {
  test(hunt.slots.size(), 1);
  test(hunt.slots[0], 0);
  test(mm_class_of<Carnivore>::the().mmt.size(), 1);
  test(mm_class_of<Wolf>::the().mmt.size(), 1);
  test(mm_class_of<Tiger>::the().mmt.size(), 1);

  {
    // Put a signature in existing mmt entries.
    int i = 1;
    mm_class_of<Carnivore>::the().mmt[0] = i++;
    mm_class_of<Wolf>::the().mmt[0] = i++;
    mm_class_of<Tiger>::the().mmt[0] = i++;
  }
    
  cout << "Encounter...\n";
}

MULTIMETHOD(encounter, string(virtual_<Animal&>, virtual_<Animal&>));

DO {
  test(encounter.vargs.size(), 2);
  test(encounter.slots.size(), 2);
  test(encounter.slots[0], 0);
  test(encounter.slots[1], 1);

  test(mm_class_of<Animal>::the().mmt.size(), 2);
  test(mm_class_of<Herbivore>::the().mmt.size(), 2);
  test(mm_class_of<Herbivore>::the().mmt.size(), 2);
  test(mm_class_of<Cow>::the().mmt.size(), 2);
  test(mm_class_of<Wolf>::the().mmt.size(), 3);
  test(mm_class_of<Tiger>::the().mmt.size(), 3);

  cout << "Check that hunt is intact\n";
  // Test that previously existing mmt entries are ok at their now index
  // multimethod<void, virtual_<Carnivore&>> hunt's entries in Carnivore's
  // mm tables were shifted by 2 (one for each Animal argument in encounter).
  test(hunt.slots[0], 2);
  int i = 1;
  testx(mm_class_of<Carnivore>::the().mmt[2], i++);
  testx(mm_class_of<Wolf>::the().mmt[2], i++);
  testx(mm_class_of<Tiger>::the().mmt[2], i++);
}

BEGIN_METHOD(encounter, string, Animal&, Animal&) {
  return "ignore";
} END_METHOD(encounter);

BEGIN_METHOD(encounter, string, Carnivore&, Animal&) {
  return "hunt";
} END_METHOD(encounter);

BEGIN_METHOD(encounter, string, Carnivore&, Carnivore&) {
  return "fight";
} END_METHOD(encounter);

BEGIN_METHOD(encounter, string, Wolf&, Wolf&) {
  return "wag tail";
} END_METHOD(encounter);

BEGIN_METHOD(encounter, string, Herbivore&, Carnivore&) {
  return "run";
} END_METHOD(encounter);

enum action { display_error, print_cow, draw_cow, print_wolf, draw_wolf, print_tiger, draw_tiger, print_herbivore, display_cow, print_animal };

MULTIMETHOD(display, action(virtual_<Animal&>, virtual_<Interface&>));

BEGIN_METHOD(display, action, Cow& a, Terminal& b) {
  return print_cow;
} END_METHOD(display);

BEGIN_METHOD(display, action, Wolf& a, Terminal& b) {
  return print_wolf;
} END_METHOD(display);

BEGIN_METHOD(display, action, Tiger& a, Terminal& b) {
  return print_tiger;
} END_METHOD(display);

BEGIN_METHOD(display, action, Cow& a, Window& b) {
  return draw_cow;
} END_METHOD(display);

BEGIN_METHOD(display, action, Wolf& a, Window& b) {
  return draw_wolf;
} END_METHOD(display);

BEGIN_METHOD(display, action, Tiger& a, Window& b) {
  return draw_tiger;
} END_METHOD(display);

// following two are ambiguous, e.g. for (Cow, Terminal)

BEGIN_METHOD(display, action, Herbivore& a, Interface& b) {
  return draw_tiger;
} END_METHOD(display);

BEGIN_METHOD(display, action, Animal& a, Terminal& b) {
  return draw_tiger;
} END_METHOD(display);

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

int main() {

  cout << "Starting tests...\n\n";
  
  static_assert(
    is_same<
    typename extract_virtuals<virtual_<Animal&>, virtual_<Animal&> >::type,
    virtuals<Animal, Animal>
    >::value, "not ok !!!");

  static_assert(
    is_same<
    typename extract_virtuals<virtual_<Animal&>, int, virtual_<Animal&> >::type,
    virtuals<Animal, Animal>
    >::value, "not ok !!!");
  
  cout << "\n*** method registration" << endl;
  
  {
    static_assert(
      is_same<
      extract_method_virtuals<
      void(int, virtual_<Animal&>, char, virtual_<Animal*>),
      void(int, Cow&, char, Wolf*)
      >::type,
      virtuals<Cow, Wolf> >::value, "extraction of virtual method arguments");

    int i = 0;
    auto aa = encounter.methods[i++];
    auto ca = encounter.methods[i++];
    auto cc = encounter.methods[i++];
    auto ww = encounter.methods[i++];
    auto hc = encounter.methods[i++];

    test(encounter.methods.size(), 5);

    test(encounter.methods[4]->args.size(), 2);
    test(encounter.methods[4]->args[0], &mm_class_of<Herbivore>::the());
    test(encounter.methods[4]->args[0]->name(), mm_class_of<Herbivore>::the().name());
    test(encounter.methods[4]->args[1]->name(), mm_class_of<Carnivore>::the().name());

    cout << "\nResolver\n";
    resolver renc(encounter);

    test(renc.classes.size(), 2);
    test(renc.classes[0].size(), 6);
    test(renc.classes[0][0]->ti.name(), mm_class_of<Animal>::the().ti.name());
    test(renc.classes[0][1]->ti.name(), mm_class_of<Herbivore>::the().ti.name());
    test(renc.classes[0][2]->ti.name(), mm_class_of<Cow>::the().ti.name());
    test(renc.classes[0][3]->ti.name(), mm_class_of<Carnivore>::the().ti.name());
    test(renc.classes[0][4]->ti.name(), mm_class_of<Wolf>::the().ti.name());
    test(renc.classes[0][5]->ti.name(), mm_class_of<Tiger>::the().ti.name());

    test(encounter.steps.size(), 2);
    test(encounter.steps[0], 1);
    test(encounter.steps[1], 6);
    test(renc.linear(vector<int>{ 0, 1 }), 6);
    test(renc.linear(vector<int>{ 2, 2 }), 14);

    cout << "methods : " << renc.methods << endl;
    test(renc.dominates(ww, ww), 0);
    test(renc.dominates(ww, hc), -1); // although impossible because no wolfcow
    test(renc.dominates(cc, ww), 1);
    test(renc.dominates(hc, ca), -1); // impossible too
    test(renc.dominates(hc, aa), -1);

    cout << "\ndisplay resolver\n";
    resolver rdisp(display);

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

    test(display.steps.size(), 2);
    test(display.steps[0], 1);
    test(display.steps[1], 6);
    test(rdisp.linear(vector<int>{ 0, 1 }), 6);
    test(rdisp.linear(vector<int>{ 2, 2 }), 14);

    int m = 0;
    using display_method_entry = decltype(display)::method_entry;

    auto animal_interface = new method_base;
    animal_interface->args.push_back(&mm_class_of<Animal>::the());
    animal_interface->args.push_back(&mm_class_of<Interface>::the());
      
    auto cow_terminal = static_cast<display_method_entry*>(display.methods[m++]);
    auto wolf_terminal = static_cast<display_method_entry*>(display.methods[m++]);
    auto tiger_terminal = static_cast<display_method_entry*>(display.methods[m++]);
    auto cow_window = static_cast<display_method_entry*>(display.methods[m++]);
    auto wolf_window = static_cast<display_method_entry*>(display.methods[m++]);
    auto tiger_window = static_cast<display_method_entry*>(display.methods[m++]);
    auto herbivore_interface = static_cast<display_method_entry*>(display.methods[m++]);
    auto animal_terminal = static_cast<display_method_entry*>(display.methods[m++]);

    test(renc.dominates(cow_window, animal_interface), -1);
    test(renc.dominates(cow_terminal, animal_interface), -1);
    test(renc.dominates(wolf_window, animal_interface), -1);
    test(renc.dominates(wolf_terminal, animal_interface), -1);
    test(renc.dominates(tiger_window, animal_interface), -1);
    test(renc.dominates(tiger_terminal, animal_interface), -1);
    test(renc.dominates(herbivore_interface, animal_interface), -1);
    test(renc.dominates(animal_terminal, animal_interface), -1);
    test(renc.dominates(animal_terminal, herbivore_interface), 0);

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
    }

    cout << "\ndisplay.resolve()\n";
    test(rdisp.dispatch_table_size, 18);
    rdisp.resolve(display.allocate_dispatch_table(rdisp.dispatch_table_size));

    {
      auto mptr = display.dispatch_table;
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

    test(w.__mm_ptbl->size(), 4); // four because 1 for hunt, 2 for encounter and 1 for display
    test((*w.__mm_ptbl)[0], 4); // encounter(1): 3 => Carnivore, 4 => Wolf
    test((*w.__mm_ptbl)[1], 4); // encounter(2)
    test((*w.__mm_ptbl)[2], 4); // display(1)

    test(t.__mm_ptbl->size(), 4); // three because 1 for display and two for encounter
    test((*t.__mm_ptbl)[0], 5); // encounter(1): 5 => Tiger
    test((*t.__mm_ptbl)[1], 5); // encounter(2)
    test((*t.__mm_ptbl)[2], 5); // display(1)

    test(term.__mm_ptbl->size(), 1); // display(2)
    test((*term.__mm_ptbl)[0], 1); // 0 => Interface, 1 => Terminal

    test(win.__mm_ptbl->size(), 1); // display(2)
    test((*win.__mm_ptbl)[0], 2); // 2 => Window

    display.ready = true;

    test(display(c, term), print_cow);
    test(display(w, win), draw_wolf);
    test(throws<undefined>([&]() { display(w, interf); }), true);
    test(throws<ambiguous>([&]() { display(herb, term); }), true);

    test(encounter(c, w), "run");
    test(encounter(c, c), "ignore");
  }
  
  cout << "\n" << success << " tests succeeded, " << failure << " failed.\n";
  
  return 0;
}
