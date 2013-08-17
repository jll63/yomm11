// -*- compile-command: "make dl_main dl_shared.so && ./dl_main" -*-

// dl.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DL_DEFINED
#define DL_DEFINED

#include <string>

#include <multi_methods.hpp>

#ifdef SHARED
#include <multi_methods/extern_macros.hpp>
#endif

struct Animal : yorel::multi_methods::selector {
  MM_CLASS(Animal);
  Animal() {
    MM_INIT();
  }
};

MM_EXTERN_CLASS(Animal);

struct Herbivore : Animal {
  MM_CLASS(Herbivore, Animal);
  Herbivore() {
    MM_INIT();
  }
};

MM_EXTERN_CLASS(Herbivore);

struct Carnivore : Animal {
  MM_CLASS(Carnivore, Animal);
  Carnivore() {
    MM_INIT();
  }
};

MM_EXTERN_CLASS(Carnivore);

struct Cow : Herbivore {
  MM_CLASS(Cow, Herbivore);
  Cow() {
    MM_INIT();
  }
};

MM_EXTERN_CLASS(Cow);

struct Wolf : Carnivore {
  MM_CLASS(Wolf, Carnivore);
  Wolf() {
    MM_INIT();
  }
};

MM_EXTERN_CLASS(Wolf);

MULTI_METHOD(encounter, std::string, const yorel::multi_methods::virtual_<Animal>&, const yorel::multi_methods::virtual_<Animal>&);

#endif
