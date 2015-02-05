// dl_shared.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define SHARED
#include "dl.hpp"

#include <yorel/multi_methods/macros.hpp>

#include <iostream>

using namespace std;
using yorel::multi_methods::virtual_;

struct Tiger : Carnivore {
  MM_CLASS(Tiger, Carnivore);
  Tiger() {
    MM_INIT();
  }
};

extern "C" Tiger* make_tiger() {
  return new Tiger;
}

BEGIN_SPECIALIZATION(encounter, string, const Carnivore&, const Herbivore&) {
  return "hunt";
} END_SPECIALIZATION;
