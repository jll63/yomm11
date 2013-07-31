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

namespace intrusive {

  MULTIMETHOD(do_nothing, void(virtual_<object>&));

  BEGIN_METHOD(do_nothing, void, object&) {
  } END_METHOD;

  MULTIMETHOD(do_something, double(multimethods::virtual_<object>&, double x, double a, double b, double c));

  BEGIN_METHOD(do_something, double, object&, double x, double a, double b, double c) {
    return log(a * x * x + b * x + c);
  } END_METHOD;

  MULTIMETHOD(do_nothing_2, void(virtual_<object>&, virtual_<object>&));

  BEGIN_METHOD(do_nothing_2, void, object&, object&) {
  } END_METHOD;

}

namespace vbase {

  MULTIMETHOD(do_nothing, void(virtual_<object>&));

  BEGIN_METHOD(do_nothing, void, object&) {
  } END_METHOD;

  MULTIMETHOD(do_something, double(multimethods::virtual_<object>&, double x, double a, double b, double c));

  BEGIN_METHOD(do_something, double, derived&, double x, double a, double b, double c) {
    return log(a * x * x + b * x + c);
  } END_METHOD;

  MULTIMETHOD(do_nothing_2, void(virtual_<object>&, virtual_<object>&));

  BEGIN_METHOD(do_nothing_2, void, object&, object&) {
  } END_METHOD;

}

namespace foreign {
  
  struct object {
    virtual ~object() { }
  };

  MM_FOREIGN_CLASS(object);

  MULTIMETHOD(do_nothing, void(virtual_<object>&));

  BEGIN_METHOD(do_nothing, void, object&) {
  } END_METHOD;

  MULTIMETHOD(do_nothing_2, void(virtual_<object>&, virtual_<object>&));

  BEGIN_METHOD(do_nothing_2, void, object&, object&) {
  } END_METHOD;

  MULTIMETHOD(do_something, double(virtual_<object>&, double x, double a, double b, double c));

  BEGIN_METHOD(do_something, double, object&, double x, double a, double b, double c) {
    return log(a * x * x + b * x + c);
  } END_METHOD;
}

using time_type = decltype(high_resolution_clock::now());

void post(const string& description, time_type start, time_type end) {
  cout << setw(50) << left << description << ": " << setw(8) << fixed << right << setprecision(2) << duration<double, milli>(end - start).count() << endl;
}

struct benchmark {
  benchmark(const string& label) : label(label), start(high_resolution_clock::now()) {
  }
  
  ~benchmark() {
      auto end = high_resolution_clock::now();
      cout << setw(50) << left << label << ": "
           << setw(8) << fixed << right << setprecision(2)
           << duration<double, milli>(end - start).count() << endl;
  }

  const string label;
  decltype(high_resolution_clock::now()) start;
};

int main() {
  multimethods::initialize();

  const int repeats = 10 * 1000 * 1000;

  {
    auto pf = new foreign::object;
    auto pi = intrusive::object::make();

    cout << repeats << " iterations, time in millisecs\n";

    {
      benchmark b("virtual function, do_nothing");
      for (int i = 0; i < repeats; i++)
        pi->do_nothing();
    }
  
    {
      benchmark b("open method, intrusive, do_nothing");
      for (int i = 0; i < repeats; i++)
        intrusive::do_nothing(*pi);
    }

    {
      benchmark b("open method, foreign, do_nothing");
      for (int i = 0; i < repeats; i++)
        foreign::do_nothing(*pf);
    }

    {
      benchmark b("virtual function, do_something");
      for (int i = 0; i < repeats; i++)
        pi->do_something(1, 2, 3, 4);
    }

    {
      benchmark b("open method, intrusive, do_something");
      for (int i = 0; i < repeats; i++)
        intrusive::do_something(*pi, 1, 2, 3, 4);
    }

    {
      benchmark b("open method, foreign, do_something");
      for (int i = 0; i < repeats; i++)
        foreign::do_something(*pf, 1, 2, 3, 4);
    }

    // double dispatch

    {
      benchmark b("virtual function, 2-dispatch, do_nothing");
      for (int i = 0; i < repeats; i++)
        pi->dd1_do_nothing(pi);
    }

    {
      benchmark b("open method with 2 args, intrusive, do_nothing");
      for (int i = 0; i < repeats; i++)
        intrusive::do_nothing_2(*pi, *pi);
    }

    {
      benchmark b("open method with 2 args, foreign, do_nothing");
      for (int i = 0; i < repeats; i++)
        foreign::do_nothing_2(*pf, *pf);
    }
  }
  
  // virtual inheritance
  {
    auto pi = vbase::object::make();
    
    {
      benchmark b("virtual function, vbase, do_nothing");
      for (int i = 0; i < repeats; i++)
        pi->do_nothing();
    }
  
    {
      benchmark b("open method, vbase, do_nothing");
      for (int i = 0; i < repeats; i++)
        vbase::do_nothing(*pi);
    }

    {
      benchmark b("virtual function, vbase, do_something");
      for (int i = 0; i < repeats; i++)
        pi->do_something(1, 2, 3, 4);
    }

    {
      benchmark b("open method, vbase, do_something");
      for (int i = 0; i < repeats; i++)
        vbase::do_something(*pi, 1, 2, 3, 4);
    }

    // double dispatch

    {
      benchmark b("virtual function, 2-dispatch, vbase, do_nothing");
      for (int i = 0; i < repeats; i++)
        pi->dd1_do_nothing(pi);
    }

    {
      benchmark b("open method with 2 args, vbase, do_nothing");
      for (int i = 0; i < repeats; i++)
        vbase::do_nothing_2(*pi, *pi);
    }
  }
  
  return 0;
}
