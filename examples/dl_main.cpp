// -*- compile-command: "make -B dl_main dl_shared.so && ./dl_main" -*-

// dl_main.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// compile with the following commands:
// g++ -I$BOOST_ROOT -std=c++11 -I../include -c dl_main.cpp
// g++ -I$BOOST_ROOT -std=c++11 -I../include -c ../src/multi_methods.cpp
// g++ dl_main.o multi_methods.o -o dl_main -ldl -rdynamic
// g++ -I$BOOST_ROOT -std=c++11 -I../include -fPIC -c -o dl_shared.o dl_shared.cpp
// g++ -shared -Wl,-soname,dl_shared.so -o dl_shared.so dl_shared.o

#include <multi_methods.hpp>
#include "dl.hpp"

#include <iostream>
#include <dlfcn.h>

using namespace std;
using multi_methods::virtual_;

BEGIN_SPECIALIZATION(encounter, string, const Animal&, const Animal&) {
  return "ignore";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(encounter, string, const Herbivore&, const Carnivore&) {
  return "run";
} END_SPECIALIZATION;

int main() {
  multi_methods::initialize(); // IMPORTANT!

  cout << "Before loading library\n";
  cout << "encounter(Cow(), Wolf()) -> " << encounter(Cow(), Wolf()) << endl;
  cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

  void* handle = dlopen("./dl_shared.so", RTLD_NOW);

  if (!handle) {
    cout << "dlopen() failed: " << dlerror() << "\n";
    exit(1);
  }

  cout << "After loading library\n";
  multi_methods::initialize(); // IMPORTANT!

  using make_tyget_type = Animal* (*)();
  make_tyget_type make_tiger = reinterpret_cast<make_tyget_type>(dlsym(handle, "make_tiger"));

  if (!make_tiger) {
    cout << "dlsym() failed: " << dlerror() << "\n";
    exit(1);
  }

  cout << "encounter(Cow(), *make_tiger()) -> " << encounter(Cow(), *make_tiger()) << endl;
  cout << "encounter(Wolf(), Cow()) -> " << encounter(Wolf(), Cow()) << endl;

  return 0;
}
