// -*- compile-command: "make benchmarks && ./benchmarks" -*-

// benchmarks.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// 50000000 iterations
// virtual function, do_nothing            :   142.91 ms
// open method, intrusive, do_nothing      :   260.79 ms
// open method, foreign, do_nothing        :  3048.16 ms
// virtual function, do_something          :  2929.18 ms
// open method, intrusive, do_something    :  3104.17 ms
// open method, foreign, do_something      :  5651.20 ms

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

struct foreign {
  virtual ~foreign() { }
};

MM_FOREIGN_CLASS(foreign);

MULTIMETHOD(do_nothing_f, void(virtual_<foreign>&));

BEGIN_METHOD(do_nothing_f, void, foreign&) {
} END_METHOD;

MULTIMETHOD(do_something_f, double(virtual_<foreign>&, double x, double a, double b, double c));

BEGIN_METHOD(do_something_f, double, foreign&, double x, double a, double b, double c) {
  return log(a * x * x + b * x + c);
} END_METHOD;

void post(const string& description, double milli) {
  cout << setw(40) << left << description << ": " << setw(8) << fixed << right << setprecision(2) << milli << endl;
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
    
    auto diff = high_resolution_clock::now() - start;
    post("virtual function, do_nothing", duration<double, milli>(diff).count());
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing(*pfast);
    
    auto diff = high_resolution_clock::now() - start;
    post("open method, intrusive, do_nothing", duration<double, milli>(diff).count());
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_nothing_f(*pforeign);
    
    auto diff = high_resolution_clock::now() - start;
    post("open method, foreign, do_nothing", duration<double, milli>(diff).count());
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      pfast->do_something(1, 2, 3, 4);
    
    auto diff = high_resolution_clock::now() - start;
    post("virtual function, do_something", duration<double, milli>(diff).count());
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_something(*pfast, 1, 2, 3, 4);
    
    auto diff = high_resolution_clock::now() - start;
    post("open method, intrusive, do_something", duration<double, milli>(diff).count());
  }

  {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < repeats; i++)
      do_something_f(*pforeign, 1, 2, 3, 4);
    
    auto diff = high_resolution_clock::now() - start;
    post("open method, foreign, do_something", duration<double, milli>(diff).count());
  }
  
  return 0;
}
