
struct Animal : mm {
  Animal() {
    mminit<Animal>();
  }
};

struct Carnivore : virtual Animal {
  Carnivore() {
    mminit<Carnivore, Animal>();
  }
};

struct Herbivore : virtual Animal {
  Herbivore() {
    mminit<Herbivore, Animal>();
  }
};

struct Mammal : virtual Animal {
  Mammal() {
    mminit<Mammal, Animal>();
  }
};

struct Cow : Mammal, Herbivore {
  Cow() {
    mminit<Cow, Mammal, Herbivore>();
  }
};

struct Wolf : Mammal, Carnivore {
  Wolf() {
    mminit<Wolf, Mammal, Carnivore>();
  }
};

#define show(e) #e << " = " << (e)
