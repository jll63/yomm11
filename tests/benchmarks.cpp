// -*- compile-command: "make benchmarks && ./benchmarks" -*-

// benchmarks.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// chrt -f 99 ./benchmarks
// 10000000 iterations, time in millisecs
// virtual function, do_nothing                      :    75.50
// open method, intrusive, do_nothing                :   100.56
// open method, foreign, do_nothing                  :  1397.75
// virtual function, do_something                    :  1541.11
// open method, intrusive, do_something              :  1608.20
// open method, foreign, do_something                :  2607.76
// virtual function, 2-dispatch, do_nothing          :   250.75
// open method with 2 args, intrusive, do_nothing    :   150.77
// open method with 2 args, foreign, do_nothing      :  2832.26

// results obtained on my computer (ThinkPad x200s):
// processor	: 0 & 1
// vendor_id	: GenuineIntel
// cpu family	: 6
// model	: 23
// model name	: Intel(R) Core(TM)2 Duo CPU     L9400  @ 1.86GHz
// stepping	: 10
// microcode	: 0xa0c
// cpu MHz	: 800.000
// cache size	: 6144 KB
// cpu cores	: 2
// fpu		: yes
// fpu_exception: yes
// cpuid level	: 13
// wp		: yes
// bogomips	: 3724.05
// clflush size	: 64

#include <iostream>
#include <iomanip>
#include <chrono>
#include "benchmarks.hpp"

using namespace std;
using namespace std::chrono;
using multimethods::virtual_;

MULTIMETHOD(do_nothing, void(virtual_<fast>&));

BEGIN_METHOD(do_nothing, void, fast&) {
} END_METHOD;

MULTIMETHOD(do_nothing_2, void(virtual_<fast>&, virtual_<fast>&));

BEGIN_METHOD(do_nothing_2, void, fast&, fast&) {
} END_METHOD;

struct foreign {
  virtual ~foreign() { }
};

MM_FOREIGN_CLASS(foreign);

MULTIMETHOD(do_nothing_f, void(virtual_<foreign>&));

BEGIN_METHOD(do_nothing_f, void, foreign&) {
} END_METHOD;

MULTIMETHOD(do_nothing_2_f, void(virtual_<foreign>&, virtual_<foreign>&));

BEGIN_METHOD(do_nothing_2_f, void, foreign&, foreign&) {
} END_METHOD;

MULTIMETHOD(do_something_f, double(virtual_<foreign>&, double x, double a, double b, double c));

BEGIN_METHOD(do_something_f, double, foreign&, double x, double a, double b, double c) {
  return log(a * x * x + b * x + c);
} END_METHOD;

using time_type = decltype(high_resolution_clock::now());

void post(const string& description, time_type start, time_type end) {
  cout << setw(50) << left << description << ": " << setw(8) << fixed << right << setprecision(2) << duration<double, milli>(end - start).count() << endl;
}

int main() {
  multimethods::initialize();

  const int repeats = 10 * 1000 * 1000;
  fast* pfast = fast::make();
  foreign* pforeign = new foreign;

  cout << repeats << " iterations, time in millisecs\n";

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      pfast->do_nothing();
    
    auto end = high_resolution_clock::now();
    post("virtual function, do_nothing", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing(*pfast);
    
    auto end = high_resolution_clock::now();
    post("open method, intrusive, do_nothing", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing_f(*pforeign);
    
    auto end = high_resolution_clock::now();
    post("open method, foreign, do_nothing", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      pfast->do_something(1, 2, 3, 4);
    
    auto end = high_resolution_clock::now();
    post("virtual function, do_something", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_something(*pfast, 1, 2, 3, 4);
    
    auto end = high_resolution_clock::now();
    post("open method, intrusive, do_something", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_something_f(*pforeign, 1, 2, 3, 4);
    
    auto end = high_resolution_clock::now();
    post("open method, foreign, do_something", start, end);
  }

  // double dispatch

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      pfast->dd1_do_nothing(pfast);
    
    auto end = high_resolution_clock::now();
    post("virtual function, 2-dispatch, do_nothing", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing_2(*pfast, *pfast);
    
    auto end = high_resolution_clock::now();
    post("open method with 2 args, intrusive, do_nothing", start, end);
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing_2_f(*pforeign, *pforeign);
    
    auto end = high_resolution_clock::now();
    post("open method with 2 args, foreign, do_nothing", start, end);
  }
  
  return 0;
}
