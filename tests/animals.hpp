
struct Animal : multimethods::root {
  virtual void foo() = 0;
  MM_CLASS(Animal);
  Animal() {
    MM_INIT();
  }
};

struct Herbivore : Animal {
  virtual void foo() { }
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
  virtual void foo() { }
  MM_CLASS(Cow, Herbivore);
  Cow() {
    MM_INIT();
  }
};

struct Wolf : Carnivore {
  virtual void foo() { }
  MM_CLASS(Wolf, Carnivore);
  Wolf() {
    MM_INIT();
  }
};

struct Tiger : Carnivore {
  virtual void foo() { }
  MM_CLASS(Tiger, Carnivore);
  Tiger() {
    MM_INIT();
  }
};

struct Interface : root {
  MM_CLASS(Interface);
  Interface() {
    MM_INIT();
  }
};

struct Terminal : Interface {
  MM_CLASS(Terminal, Interface);
  Terminal() {
    MM_INIT();
  }
};

struct Window : Interface {
  MM_CLASS(Window, Interface);
  Window() {
    MM_INIT();
  }
};
