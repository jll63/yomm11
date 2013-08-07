// -*- compile-command: "make dl_main dl_shared.so && ./dl_main" -*-

#ifndef DL_DEFINED
#define DL_DEFINED

#include <string>

#ifdef SHARED
#define IF_SHARED(x) x
#define IF_NOT_SHARED(x)
#else
#define IF_SHARED(x)
#define IF_NOT_SHARED(x) x
#endif

struct Animal : multi_methods::selector {
  MM_CLASS(Animal);
  Animal() {
    MM_INIT();
  }
};

struct Herbivore : Animal {
  MM_CLASS(Herbivore, Animal);
  Herbivore() {
    MM_INIT();
  }
};

struct Carnivore : Animal {
  MM_CLASS(Carnivore, Animal);
  Carnivore() {
    MM_INIT();
  }
};

struct Cow : Herbivore {
  MM_CLASS(Cow, Herbivore);
  Cow() {
    MM_INIT();
  }
};

struct Wolf : Carnivore {
  MM_CLASS(Wolf, Carnivore);
  Wolf() {
    MM_INIT();
  }
};

MULTI_METHOD(encounter, std::string, const multi_methods::virtual_<Animal>&, const multi_methods::virtual_<Animal>&);

#endif
