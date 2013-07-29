// -*- compile-command: "make next && ./next" -*-

// next.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example taken Dylan's documentation, see http://opendylan.org/documentation/intro-dylan/multiple-dispatch.html

#include <iostream>
#include <multimethods.hpp>

using namespace std;
using multimethods::selector;
using multimethods::virtual_;

// Vehicle is the root of a class hierarchy that includes support for fast dispatch.
// That means:
struct Vehicle : selector { // inherit from class selector
  MM_CLASS(Vehicle); // register class
  Vehicle() {
    MM_INIT(); // initialize pointer to multi-method table
  }
};

struct Car : Vehicle {
  MM_CLASS(Car, Vehicle); // register class and its base
  Car() {
    MM_INIT();
  }
};

struct Truck : Vehicle {
  MM_CLASS(Truck, Vehicle);
  Truck() {
    MM_INIT();
  }
};

struct Inspector : multimethods::selector {
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

// declare a multi-method with two virtual arguments
// this can go in a header file
MULTIMETHOD(inspect, void(virtual_<Vehicle>&, virtual_<Inspector>&));

// define a specialization
// this must go in an implementation file
BEGIN_METHOD(inspect, void, Vehicle& v, Inspector& i) {
  cout << "Inspect vehicle.\n";
} END_METHOD;

BEGIN_METHOD(inspect, void, Car& v, Inspector& i) {
  next(v, i); // next calls the next most specialized method
  cout << "Inspect seat belts.\n";
} END_METHOD;

BEGIN_METHOD(inspect, void, Car& v, StateInspector& i) {
  next(v, i);
  cout << "Check insurance.\n";
} END_METHOD;

int main() {
  multimethods::initialize(); // IMPORTANT! - allocates slots and compute dispatch tables
  Car car;
  StateInspector inspector;
  // call method:
  inspect(car, inspector); // Inspect vehicle. Inspect seat belts. Check insurance.
  return 0;
}
