// -*- compile-command: "cd ../tests && make" -*-

#ifndef MULTIMETHODS_INCLUDED
#define  MULTIMETHODS_INCLUDED

#include <typeinfo>
#include <type_traits>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iostream>

//#define MM_ENABLE_TRACE

#ifdef MM_ENABLE_TRACE
#define MM_TRACE(e) e
#include <iterator>
#else
#define MM_TRACE(e)
#endif

namespace multimethods {

  struct mm_class;
  struct multimethod_base;

  struct mm_class {
    mm_class(const std::type_info& t);
    ~mm_class();
    const std::type_info& ti;
    std::vector<mm_class*> bases;
    std::vector<mm_class*> specs;
    std::vector<int> mmt;
    std::vector<multimethod_base*> mms; // multimethods rooted here for one or more args.
    bool abstract;

    const std::string name() const;
    int add_multimethod(multimethod_base* pm);
    void reserve_slot() { mmt.reserve(mmt.size() + 1); }
    void insert_slot(int i);
    void for_each_conforming(std::function<void(mm_class*)> pf);
    bool conforms_to(const mm_class& other) const;
    bool dominates(const mm_class& other) const;
  };

  inline const std::string mm_class::name() const {
    return ti.name();
  }

  inline std::ostream& operator <<(std::ostream& os, mm_class* pc) {
    return os << pc->name();
  }

  std::ostream& operator <<(std::ostream& os, const std::vector<mm_class*>& classes);

  template<typename... T>
  struct type_list;
  
  template<class Class>
  struct mm_class_of {
    static mm_class& the() {
      static mm_class instance(typeid(Class));
      return instance;
    }
  };
  
  template<class Class>
  struct mm_class_of<const Class> : mm_class_of<Class> { };

  template<class... Bases>
  struct bases;

  template<class Class>
  struct bases_of {
    using bases_type = typename Class::bases_type;
  };

  template<class Class, class Bases>
  struct check_bases;

  template<class Class, class Base, class... Bases>
  struct check_bases<Class, type_list<Base, Bases...>> {
    static const bool value = std::is_base_of<Base, Class>::value && check_bases<Class, type_list<Bases...>>::value;
  };

  template<class Class>
  struct check_bases<Class, type_list<>> {
    static const bool value = true;
  };

  template<class Class, class... Bases>
  struct mm_class_initializer {
    mm_class_initializer() {
      mm_class& pc = mm_class_of<Class>::the();
      if (pc.bases.empty()) {
        pc.bases = { &mm_class_of<Bases>::the()... };
        pc.abstract = std::is_abstract<Class>::value;
        MM_TRACE(std::cout << pc.ti.name() << " " << pc.abstract << "\n");
        for (mm_class* pb : pc.bases)
          pb->specs.push_back(&pc);
      } else {
        throw std::runtime_error("multimethods: class redefinition");
      }
    }
    static mm_class_initializer the;
  };

  template<class Class, class... Bases>
  mm_class_initializer<Class, Bases...> mm_class_initializer<Class, Bases...>::the;

  struct root {
    root() : __mm_ptbl(0) { }
    std::vector<int>* __mm_ptbl;
    template<class THIS>
    void init_mmptr(THIS*) {
      __mm_ptbl = &mm_class_of<THIS>::the().mmt;
    }
  };

  struct method_base {
    std::vector<mm_class*> args;
    static method_base undefined;
    static method_base ambiguous;
  };

  std::ostream& operator <<(std::ostream& os, const method_base* method);
  std::ostream& operator <<(std::ostream& os, const std::vector<method_base*>& methods);

  struct multimethod_base {
    explicit multimethod_base(const std::vector<mm_class*>& v);
    void invalidate();
    void shift(int pos);
    
    using emit_func = std::function<void(method_base*, int i)>;
    std::vector<mm_class*> vargs;
    std::vector<int> slots;
    std::vector<method_base*> methods;
    std::vector<int> steps;
    bool ready;
  };

  inline void multimethod_base::invalidate() {
    ready = false;
  }

  template<class M, typename Override, class Base>
  struct wrapper;

  template<class M, typename... A, typename OR, typename... P, typename BR>
  struct wrapper<M, OR(A...), BR(P...)> {
    using type = wrapper;
    static BR body(P... args) {
      return M::body(static_cast<A>(args)...);
    }
  };

  template<class M, typename... P, typename R>
  struct wrapper<M, R(P...), R(P...)> {
    using type = M;
  };

  template<class Class>
  struct virtual_;

  template<class Class>
  struct virtual_<Class&> {
    static_assert(std::is_class<Class>::value, "virtual arguments must be either Class& or Class*");
    using type = Class;
  };

  template<class Class>
  struct virtual_<Class*> {
    static_assert(std::is_class<Class>::value, "virtual arguments must be either Class& or Class*");
    using type = Class;
  };

  template<typename... Class>
  struct virtuals {
  };

#ifdef MM_TRACE

  template<typename C1, typename C2, typename... CN>
  void write(std::ostream& os, virtuals<C1, C2, CN...>) {
    os << typeid(C1).name() << ", ";
    write(os, virtuals<C2, CN...>());
  }

  template<typename C>
  void write(std::ostream& os, virtuals<C>) {
    os << typeid(C).name();
  }

  inline void write(std::ostream& os, virtuals<>) {
  }

  template<typename... Class>
  std::ostream& operator <<(std::ostream& os, virtuals<Class...> v){
    os << "virtuals<";
    write(os, v);
    return os << ">";
  }

#endif

  template<class... Class>
  struct extract_virtuals {
    using type = typename extract_virtuals<virtuals<>, Class...>::type;
  };

  template<class... Head, typename Next, typename... Rest>
  struct extract_virtuals< virtuals<Head...>, Next, Rest... > {
    using type = typename extract_virtuals< virtuals<Head...>, Rest... >::type;
  };

  template<class... Head, typename Next, typename... Rest>
  struct extract_virtuals< virtuals<Head...>, virtual_<Next>, Rest... > {
    using type = typename extract_virtuals<
      virtuals<Head..., typename virtual_<Next>::type>,
      Rest...
      >::type;
  };

  template<class... Head>
  struct extract_virtuals< virtuals<Head...> > {
    using type = virtuals<Head...>;
  };

  template<class Result, class Multi, class Method>
  struct extract_method_virtuals_;

  template<class Multi, class Method>
  struct extract_method_virtuals {
    using type = typename extract_method_virtuals_<virtuals<>, Multi, Method>::type;
  };

  template<class... V, class P1, class A1, class... PN, class... AN, class R1, class R2>
  struct extract_method_virtuals_<virtuals<V...>, R1(P1, PN...), R2(A1, AN...)> {
    using type = typename extract_method_virtuals_<
      virtuals<V...>, R1(PN...), R2(AN...)>::type;
  };

  template<class... V, class P1, class A1, class... PN, class... AN, class R1, class R2>
  struct extract_method_virtuals_<virtuals<V...>, R1(virtual_<P1>, PN...), R2(A1, AN...)> {
    using type = typename extract_method_virtuals_<
      virtuals<V..., typename virtual_<A1>::type>, // (Cow& or Cow*) -> Cow
      R1(PN...), R2(AN...)
      >::type;
  };

  template<class... V, class R1, class R2>
  struct extract_method_virtuals_<virtuals<V...>, R1(), R2()> {
    using type = virtuals<V...>;
  };

  template<typename T>
  struct remove_virtual {
    using type = T;
  };

  template<class C>
  struct remove_virtual<virtual_<C>> {
    using type = C;
  };

  template<class M>
  struct method : method_base {
    M pm;

    method(M pm, std::vector<mm_class*> type_tuple) : pm(pm) {
      args = type_tuple;
    }
  };

  template<class... Class> struct mm_class_vector_of;

  template<class... Class>
  struct mm_class_vector_of<virtuals<Class...>> {
    static std::vector<mm_class*> get() {
      std::vector<mm_class*> classes;
      mm_class_vector_of<Class...>::get(classes);
      return classes;
    }
  };

  template<class First, class... Rest>
  struct mm_class_vector_of<First, Rest...> {
    static void get(std::vector<mm_class*>& classes) {
      classes.push_back(&mm_class_of<First>::the());
      mm_class_vector_of<Rest...>::get(classes);
    }
  };

  template<>
  struct mm_class_vector_of<> {
    static void get(std::vector<mm_class*>& classes) {
    }
  };

  struct undefined : std::runtime_error {
    undefined() : std::runtime_error("multi-method call is undefined for these arguments") { }
  };

  template<typename Sig>
  struct throw_undefined;

  template<typename R, typename... A>
  struct throw_undefined<R(A...)> {
    static R body(A...);
  };

  template<typename R, typename... A>
  R throw_undefined<R(A...)>::body(A...) {
    throw undefined();
  }

  struct ambiguous : std::runtime_error {
    ambiguous() : std::runtime_error("multi-method call is ambiguous for these arguments") { }
  };

  template<typename Sig>
  struct throw_ambiguous;

  template<typename R, typename... A>
  struct throw_ambiguous<R(A...)> {
    static R body(A...);
  };

  template<typename R, typename... A>
    R throw_ambiguous<R(A...)>::body(A...) {
    throw ambiguous();
  }
    
  template<typename... P>
  struct linear;
    
  template<typename P1, typename... P>
  struct linear<P1, P...> {
    template<typename A1, typename... A>
    static int value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      int offset,
      A1, A... args) {
      return linear<P...>::value(slot_iter, step_iter, offset, args...);
    }
  };
    
  template<typename P1, typename... P>
    struct linear<virtual_<P1>, P...> {
    template<typename A1, typename... A>
    static int value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      int offset,
      A1 arg, A... args) {
      offset = offset + (*arg->__mm_ptbl)[*slot_iter++] * *step_iter++;
      return linear<P...>::value(slot_iter, step_iter, offset, args...);
    }
  };
    
  template<>
    struct linear<> {
    static int value(std::vector<int>::const_iterator slot_iter,
              std::vector<int>::const_iterator step_iter,
              int offset) {
      return offset;
    }
  };

  struct resolver {
    resolver(multimethod_base& mm);
    void resolve(multimethod_base::emit_func emit);
    void do_resolve(int dim, const std::vector<method_base*>& viable);
    int dominates(const method_base* a, const method_base* b);
    void assign_class_indices();
    void make_steps();
    int linear(const std::vector<int>& tuple);
    method_base* find_best(const std::vector<method_base*>& methods);

    const int dims;
    int dispatch_table_size;
    multimethod_base& mm;
    std::vector<method_base*> methods;
    std::vector<int> tuple;
    std::vector<std::vector<mm_class*>> classes;
    multimethod_base::emit_func emit;
    int emit_at;
  };

  template<class Tag, typename Sig>
  struct multimethod;
  
  template<class Tag, typename R, typename... P>
  struct multimethod<Tag, R(P...)> : multimethod_base {
    multimethod();
    template<class M> method_base* add();
    R operator ()(typename remove_virtual<P>::type... args);

    void prepare();
    emit_func allocate_dispatch_table(int size);

    using return_type = R;
    using mptr = return_type (*)(typename remove_virtual<P>::type...);
    using method_entry = method<mptr>;
    using signature = R(typename remove_virtual<P>::type...);
    using virtuals = typename extract_virtuals<P...>::type;

    mptr* dispatch_table;
  };

  template<class Tag, typename R, typename... P>
  multimethod<Tag, R(P...)>::multimethod()
    : multimethod_base(mm_class_vector_of<virtuals>::get()),
      dispatch_table(0) {
  }
  
  template<class Tag, typename R, typename... P>
  inline R multimethod<Tag, R(P...)>::operator ()(typename remove_virtual<P>::type... args) {
    if (!ready) {
      prepare();
    }
    return dispatch_table[linear<P...>::value(slots.begin(), steps.begin(), 0, &args...)](args...);
  }
  
  template<class Tag, typename R, typename... P>
  void multimethod<Tag, R(P...)>::prepare() {
    resolver r(*this);
    r.resolve(allocate_dispatch_table(r.dispatch_table_size));
    ready = true;
  }
  
  template<class Tag, typename R, typename... P>
  typename multimethod<Tag, R(P...)>::emit_func
  multimethod<Tag, R(P...)>::allocate_dispatch_table(int size) {
    using namespace std;
    delete [] dispatch_table;
    dispatch_table = new mptr[size];
    return [=](method_base* method, int i) {
      dispatch_table[i] =
        method == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : method == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<method_entry*>(method)->pm;
      MM_TRACE(cout << "installed at " << dispatch_table << " + " << i << endl);
    };
  }
  
  template<class Tag, typename R, typename... P>
  template<class M>
  method_base* multimethod<Tag, R(P...)>::add() {
    using method_signature = decltype(M::body);
    using target = typename wrapper<M, method_signature, signature>::type;
    using method_virtuals = typename extract_method_virtuals<R(P...), method_signature>::type;
    using namespace std;
    MM_TRACE(cout << virtuals() << endl);
    MM_TRACE(cout << method_virtuals() << endl);

    method_base* method = new method_entry(target::body, mm_class_vector_of<method_virtuals>::get());
    methods.push_back(method);
    invalidate();

    return method;
  }

  template<class MM, class M> struct register_method;

#define MM_CLASS(CLASS, BASES...)              \
  using mm_base_list_type = type_list<BASES>;      \
  using mm_this_type = CLASS;                  \
  virtual void* __init_mm_class() {                                     \
    static_assert(std::is_same<mm_this_type, std::remove_reference<decltype(*this)>::type>::value, "Error in MM_CLASS(): declared class is not correct"); \
    static_assert(check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  return &mm_class_initializer<CLASS, BASES>::the; }

#define MM_INIT() \
  init_mmptr(this)

#define MULTIMETHOD(ID, SIG)             \
    struct ID ## _tag;                          \
    multimethod<ID ## _tag, SIG> ID
  
#define REGISTER_METHOD_ID(MM, M) __register_ ## MM ## _ ## M
#define REGISTER_METHOD(MM, M)                                  \
    static auto REGISTER_METHOD_ID(MM, M) = MM.add<M>();

#define CONCAT(X, Y) CONCAT1(X, Y)
#define CONCAT1(X, Y) X ## Y
#define METHOD_NAMESPACE(MM) CONCAT(MM, __LINE__)
  
#define BEGIN_METHOD(MM, RESULT, ARGS...)               \
    namespace { namespace METHOD_NAMESPACE(MM) {        \
    struct method {                                     \
    static RESULT body(ARGS) {

#define END_METHOD(MM)                          \
    } static bool init; };                      \
    bool method::init = MM.add<method>(); } }

  }



#endif
