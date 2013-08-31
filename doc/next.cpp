// -*- compile-command: "g++ -std=c++11 -I../include -o next next.cpp ../src/multi_methods.cpp && ./next" -*-

// next.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example taken Dylan's documentation, see http://opendylan.org/documentation/intro-dylan/multiple-dispatch.html

#include <iostream>

using namespace std;

//[ prologue
#include <yorel/multi_methods.hpp>

using yorel::multi_methods::virtual_;
using yorel::multi_methods::selector;
//]


//[ vehicle
struct Vehicle : selector {
  MM_CLASS(Vehicle);
  Vehicle() {
    MM_INIT();
  }
};
//]

//[ car
struct Car : Vehicle {
  MM_CLASS(Car, Vehicle);
  Car() {
    MM_INIT();
  }
};
//]
struct Truck : Vehicle {
  MM_CLASS(Truck, Vehicle);
  Truck() {
    MM_INIT();
  }
};

struct Inspector : selector {
  MM_CLASS(Inspector);
  Inspector() {
    MM_INIT();
  }
};

struct StateInspector : Inspector {
  MM_CLASS(StateInspector, Inspector);
  StateInspector() {
    MM_INIT();
  }
};

//[ inspect
MULTI_METHOD(inspect, void, virtual_<Vehicle>&, virtual_<Inspector>&);
//]

//[ specialization
BEGIN_SPECIALIZATION(inspect, void, Vehicle& v, Inspector& i) {
  cout << "Inspect vehicle.\n";
} END_SPECIALIZATION;
//]

//[ next
BEGIN_SPECIALIZATION(inspect, void, Car& v, Inspector& i) {
  next(v, i);
  cout << "Inspect seat belts.\n";
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(inspect, void, Car& v, StateInspector& i) {
  next(v, i);
  cout << "Check road tax.\n";
} END_SPECIALIZATION;
//]

//[ call
int main() {
  yorel::multi_methods::initialize(); // IMPORTANT! - allocates slots and compute dispatch tables

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
