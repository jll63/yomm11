// -*- compile-command: "g++ -std=c++11 -I../include -o next next.cpp ../src/methods.cpp && ./next" -*-

// next.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example taken Dylan's documentation, see http://opendylan.org/documentation/intro-dylan/multiple-dispatch.html

#include <iostream>

using namespace std;

//[ prologue
#include <yorel/methods.hpp>

using yorel::methods::virtual_;
using yorel::methods::selector;
//]


//[ vehicle
struct Vehicle : selector {
  YOMM11_CLASS(Vehicle);
  Vehicle() {
    YOMM11_INIT();
  }
};
//]

//[ car
struct Car : Vehicle {
  YOMM11_CLASS(Car, Vehicle);
  Car() {
    YOMM11_INIT();
  }
};
//]
struct Truck : Vehicle {
  YOMM11_CLASS(Truck, Vehicle);
  Truck() {
    YOMM11_INIT();
  }
};

struct Inspector : selector {
  YOMM11_CLASS(Inspector);
  Inspector() {
    YOMM11_INIT();
  }
};

struct StateInspector : Inspector {
  YOMM11_CLASS(StateInspector, Inspector);
  StateInspector() {
    YOMM11_INIT();
  }
};

//[ inspect
YOMM11_METHOD(inspect, void, virtual_<Vehicle>&, virtual_<Inspector>&);
//]

//[ specialization
YOMM11_SPECIALIZATION(inspect, void, Vehicle& v, Inspector& i) {
  cout << "Inspect vehicle.\n";
} YOMM11_END_SPECIALIZATION;
//]

//[ next
YOMM11_SPECIALIZATION(inspect, void, Car& v, Inspector& i) {
  next(v, i);
  cout << "Inspect seat belts.\n";
} YOMM11_END_SPECIALIZATION;

YOMM11_SPECIALIZATION(inspect, void, Car& v, StateInspector& i) {
  next(v, i);
  cout << "Check road tax.\n";
} YOMM11_END_SPECIALIZATION;
//]

//[ call
int main() {
  yorel::methods::initialize(); // IMPORTANT! - allocates slots and compute dispatch tables

  Vehicle&& vehicle1 = Car();
  Inspector&& inspector1 = StateInspector();
  Vehicle&& vehicle2 = Truck();
  Inspector&& inspector2 = Inspector();

  cout << "First inspection:\n";
  inspect(vehicle1, inspector1);

  cout << "\nSecond inspection:\n";
  inspect(vehicle2, inspector2);

  return 0;
}
//]
