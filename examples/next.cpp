// -*- compile-command: "g++ -g -std=c++11 -I../include ../src/multimethods.cpp next.cpp -o next && ./next" -*-

// Example taken Dylan's documentation, see http://opendylan.org/documentation/intro-dylan/multiple-dispatch.html

#include <iostream>
#include <multimethods.hpp>

using namespace std;
using namespace multimethods;

struct Vehicle : root {
  MM_CLASS(Vehicle);
  Vehicle() {
    MM_INIT();
  }
};

struct Car : Vehicle {
  MM_CLASS(Car, Vehicle);
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

struct Inspector : root {
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

MULTIMETHOD(inspect, void(virtual_<Vehicle>&, virtual_<Inspector>&));

BEGIN_METHOD(inspect, void, Vehicle& v, Inspector& i) {
  cout << "Inspect vehicle.\n";
} END_METHOD(inspect)

BEGIN_METHOD(inspect, void, Car& v, Inspector& i) {
  next(v, i);
  cout << "Inspect seat belts.\n";
} END_METHOD(inspect)

BEGIN_METHOD(inspect, void, Car& v, StateInspector& i) {
  next(v, i);
  cout << "Check insurance.\n";
} END_METHOD(inspect)

int main() {
  Car car;
  StateInspector inspector;
  inspect(car, inspector);
  return 0;
}
