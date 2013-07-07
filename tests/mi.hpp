struct Animal : multimethods::root {
  MM_CLASS(Animal);
  Animal() {
    MM_INIT();
  }
};

struct Herbivore : virtual Animal {
  MM_CLASS(Herbivore, Animal);
  Herbivore() {
    MM_INIT();
  }
};

struct Predator : virtual Animal {
  MM_CLASS(Predator, Animal);
  Predator() {
    MM_INIT();
  }
};

struct Male : virtual Animal {
  MM_CLASS(Male, Animal);
  Male() {
    MM_INIT();
  }
};

struct Female : virtual Animal {
  MM_CLASS(Female, Animal);
  Female() {
    MM_INIT();
  }
};

struct Horse : Herbivore {
  MM_CLASS(Horse, Herbivore);
  Horse() {
    MM_INIT();
  }
};

struct Stallion : Male, Horse {
  MM_CLASS(Stallion, Male, Horse);
  Stallion() {
    MM_INIT();
  }
};

struct Mare : Female, Horse {
  MM_CLASS(Mare, Female, Horse);
  Mare() {
    MM_INIT();
  }
};

struct Wolf : Predator {
  MM_CLASS(Wolf, Predator);
  Wolf() {
    MM_INIT();
  }
};
