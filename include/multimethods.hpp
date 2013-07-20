// -*- compile-command: "cd ../tests && make" -*-

#ifndef MULTIMETHODS_INCLUDED
#define MULTIMETHODS_INCLUDED

// multimethod.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include <boost/type_traits/is_virtual_base_of.hpp>

//#define MM_ENABLE_TRACE

#ifdef MM_ENABLE_TRACE
#define MM_TRACE(e) e
#include <iterator>
#else
#define MM_TRACE(e)
#endif

#pragma GCC system_header

namespace multimethods {

  void initialize();

  struct mm_class;
  struct multimethod_base;

  struct mm_class {
    struct mmref {
      multimethod_base* method;
      int arg;
    };
    
    mm_class(const std::type_info& t);
    ~mm_class();

    const std::string name() const;
    void set_bases(std::vector<mm_class*>&& bases);
    int add_multimethod(multimethod_base* pm, int arg);
    void reserve_slot() { mmt.reserve(mmt.size() + 1); }
    void insert_slot(int i);
    void for_each_spec(std::function<void(mm_class*)> pf);
    void for_each_conforming(std::function<void(mm_class*)> pf);
    bool conforms_to(const mm_class& other) const;
    bool specializes(const mm_class& other) const;
    int assign_slots(std::unordered_set<mm_class*>& seen, int slot);
    bool is_marked(int current) const { return mark == current; }
    void set_mark(int current) { mark = current; }
    
    const std::type_info& ti;
    std::vector<mm_class*> bases;
    std::vector<mm_class*> specs;
    mm_class* root;
    std::vector<int> mmt;
    std::vector<mmref> rooted_here; // multimethods rooted here for one or more args.
    bool abstract;
    int mark{0};

    static std::unordered_map<mm_class*, bool> roots;
  };

  inline const std::string mm_class::name() const {
    return ti.name();
  }

  inline std::ostream& operator <<(std::ostream& os, const mm_class* pc) {
    if (pc) {
      os << pc->ti.name();
    } else {
      os << "(null)";
    }
    return os;
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
  
  template<bool Native>
  struct get_mm_table;

  template<>
  struct get_mm_table<true> {
    template<class C>
    static const std::vector<int>& value(const C* obj) {
      return *obj->__mm_ptbl;
    }
  };

  template<>
  struct get_mm_table<false> {
    using class_of_type = std::unordered_map<std::type_index, const std::vector<int>*>;
    static class_of_type* class_of;
    template<class C>
    static const std::vector<int>& value(const C* obj) {
      MM_TRACE(std::cout << "foreign mm_class_of<" << typeid(*obj).name() << "> = " << (*class_of)[std::type_index(typeid(*obj))] << std::endl);
      return *(*class_of)[std::type_index(typeid(*obj))];
    }
  };

  template<class Class>
  struct mm_class_of<const Class> : mm_class_of<Class> { };

  struct selector {
    selector() : __mm_ptbl(0) { }
    std::vector<int>* __mm_ptbl;
    virtual ~selector() { }
    template<class THIS>
    void init_mmptr(THIS*) {
      __mm_ptbl = &mm_class_of<THIS>::the().mmt;
    }
  };

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

  template<class Class, class BaseList>
  struct mm_class_initializer;
  
  template<class Class, class... Bases>
  struct mm_class_initializer<Class, type_list<Bases...>> {
    mm_class_initializer() {
      mm_class& pc = mm_class_of<Class>::the();
      pc.abstract = std::is_abstract<Class>::value;
      pc.set_bases({ &mm_class_of<Bases>::the()... });

      if (!std::is_base_of<selector, Class>::value) {
        if (!get_mm_table<false>::class_of) {
          get_mm_table<false>::class_of = new get_mm_table<false>::class_of_type;
        }
        (*get_mm_table<false>::class_of)[std::type_index(typeid(Class))] = &pc.mmt;
      }
    }
    static mm_class_initializer the;
  };

  template<class Class, class... Bases>
  mm_class_initializer<Class, type_list<Bases...>> mm_class_initializer<Class, type_list<Bases...>>::the;

  struct method_base {
    std::vector<mm_class*> args;
    bool specializes(const method_base* other) const;
    static method_base undefined;
    static method_base ambiguous;
  };

  std::ostream& operator <<(std::ostream& os, const method_base* method);
  std::ostream& operator <<(std::ostream& os, const std::vector<method_base*>& methods);

  struct multimethod_base {
    explicit multimethod_base(const std::vector<mm_class*>& v);
    void invalidate();
    void shift(int pos);
    void assign_slot(int arg, int slot);
    
    using emit_func = std::function<void(method_base*, int i)>;
    using emit_next_func = std::function<void(method_base*, method_base*)>;
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

  // template<bool is_base_of, class B, class D>
  // struct is_virtual_base_of_;
    
  // template<class B, class D>
  // struct is_virtual_base_of_<false, B, D> {
  //   static const bool value = false;
  // };
  
  // template<class B, class D>
  // struct is_virtual_base_of_<true, B, D> {
  //   struct X : D, private virtual B {
  //     virtual void * __init_mm_class() { }
  //   };
  //   struct Y : D { };
  //   static const bool value = sizeof(X) == sizeof(Y);
  // };

  // template<class B, class D>
  // struct is_virtual_base_of {
  //   static const bool value = is_virtual_base_of_<std::is_base_of<B, D>::value, B, D>::value;
  // };

  // template<class C>
  // struct is_virtual_base_of<C, C> {
  //   static const bool value = false;
  // };

  using boost::is_virtual_base_of;
  
  template<class D, class B>
  inline typename std::enable_if<
    !std::is_same<D, B>::value &&
    is_virtual_base_of<typename std::remove_const<B>::type, typename std::remove_const<D>::type>::value, D&>::type
  cast(B& b) {
    MM_TRACE(std::cout << "using dynamic_cast\n");
    return dynamic_cast<D&>(b);
  }

  template<class D, class B>
  inline typename std::enable_if<
    !std::is_same<B, D>::value &&
    is_virtual_base_of<typename std::remove_const<B>::type, typename std::remove_const<D>::type>::value, const D&>::type
  cast(const B& b) {
    MM_TRACE(std::cout << "using dynamic_cast\n");
    return dynamic_cast<const D&>(b);
  }

  template<class D, class B>
  inline typename std::enable_if<
    !std::is_same<D, B>::value &&
  !is_virtual_base_of<typename std::remove_const<B>::type, typename std::remove_const<D>::type>::value, D&>::type
  cast(B& b) {
    MM_TRACE(std::cout << "using static_cast\n");
    return static_cast<D&>(b);
  }

  template<class D, class B>
  inline typename std::enable_if<
    !std::is_same<D, B>::value &&
  !is_virtual_base_of<typename std::remove_const<B>::type, typename std::remove_const<D>::type>::value, const D&>::type
  cast(const B& b) {
    MM_TRACE(std::cout << "using static_cast\n");
    return static_cast<const D&>(b);
  }

  template<class D, class B>
  inline typename std::enable_if<std::is_same<D, B>::value, D&>::type
  cast(B& b) {
    return b;
  }

  template<class D, class B>
  inline typename std::enable_if<std::is_same<D, B>::value, const D&>::type
  cast(const B& b) {
    return b;
  }
  
  template<class M, typename... A, typename OR, typename... P, typename BR>
  struct wrapper<M, OR(A...), BR(P...)> {
    using type = wrapper;
    static BR body(P... args) {
      return M::body(cast<typename std::remove_reference<A>::type>(args)...);
    }
  };

  template<class M, typename... P, typename R>
  struct wrapper<M, R(P...), R(P...)> {
    using type = M;
  };

  template<class Class>
  struct virtual_ {
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
  struct extract_virtuals< virtuals<Head...>, virtual_<Next>&, Rest... > {
    using type = typename extract_virtuals<
      virtuals<Head..., typename virtual_<Next>::type>,
      Rest...
      >::type;
  };

  template<class... Head, typename Next, typename... Rest>
  struct extract_virtuals< virtuals<Head...>, const virtual_<Next>&, Rest... > {
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
  struct extract_method_virtuals_<virtuals<V...>, R1(virtual_<P1>&, PN...), R2(A1&, AN...)> {
    using type = typename extract_method_virtuals_<
      virtuals<V..., A1>,
      R1(PN...), R2(AN...)
      >::type;
  };

  template<class... V, class P1, class A1, class... PN, class... AN, class R1, class R2>
  struct extract_method_virtuals_<virtuals<V...>, R1(const virtual_<P1>&, PN...), R2(const A1&, AN...)> {
    using type = typename extract_method_virtuals_<
      virtuals<V..., A1>,
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
  struct remove_virtual<virtual_<C>&> {
    using type = C&;
  };

  template<class C>
  struct remove_virtual<const virtual_<C>&> {
    using type = const C&;
  };

  template<class M>
  struct method : method_base {
    M pm;
    M* pn; // next

    method(M pm, std::vector<mm_class*> type_tuple, M* pn) : pm(pm), pn(pn) {
      args = type_tuple;
    }
  };

  template<class... Class> struct mm_class_vector_of_;

  template<class First, class... Rest>
  struct mm_class_vector_of_<First, Rest...> {
    static void get(std::vector<mm_class*>& classes) {
      classes.push_back(&mm_class_of<First>::the());
      mm_class_vector_of_<Rest...>::get(classes);
    }
  };

  template<>
  struct mm_class_vector_of_<> {
    static void get(std::vector<mm_class*>& classes) {
    }
  };

  template<class... Class>
  struct mm_class_vector_of {
    static std::vector<mm_class*> get() {
      std::vector<mm_class*> classes;
      mm_class_vector_of_<Class...>::get(classes);
      return classes;
    }
  };

  template<class... Class>
  struct mm_class_vector_of<virtuals<Class...>> {
    static std::vector<mm_class*> get() {
      return mm_class_vector_of<Class...>::get();
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
    struct linear<virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static int value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      int offset,
      A1 arg, A... args) {
      offset = offset + get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter++] * *step_iter++;
      return linear<P...>::value(slot_iter, step_iter, offset, args...);
    }
  };
    
  template<typename P1, typename... P>
    struct linear<const virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static int value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      int offset,
      A1 arg, A... args) {
      offset = offset + get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter++] * *step_iter++;
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
    void resolve(multimethod_base::emit_func emit, multimethod_base::emit_next_func emit_next);
    void do_resolve(int dim, const std::vector<method_base*>& viable);
    int order(const method_base* a, const method_base* b);
    void assign_next();
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
    multimethod_base::emit_next_func emit_next;
    int emit_at;
  };

  struct hierarchy_initializer {
    hierarchy_initializer(mm_class& root);

    void collect_classes();
    void topological_sort_visit(mm_class* pc);
    void make_masks();
    void assign_slots();

    struct node {
      mm_class* pc;
      boost::dynamic_bitset<> mask;
    };

    mm_class& root;
    std::vector<node> nodes;
    int mark{0};
    
  };

  template<template<typename Sig> class Method, typename Sig>
  struct multimethod_impl;
  
  template<template<typename Sig> class Method, typename R, typename... P>
  struct multimethod_impl<Method, R(P...)> {
    
    template<class M>
    static method_base* add();

    static void init_base();
    
    R operator ()(typename remove_virtual<P>::type... args) const;

    static void prepare();
    static void allocate_dispatch_table(int size, multimethod_base::emit_func& emit, multimethod_base::emit_next_func& emit_next);

    using return_type = R;
    using mptr = return_type (*)(typename remove_virtual<P>::type...);
    using method_entry = method<mptr>;
    using signature = R(typename remove_virtual<P>::type...);
    using virtuals = typename extract_virtuals<P...>::type;

    static multimethod_base* base;
    static mptr* dispatch_table;

    template<typename Tag>
    struct next_ptr {
      static mptr next;
    };
  };

  template<class Method, class Spec>
  struct register_spec {
    register_spec() {
      Method::template add<Spec>();
    }
    static register_spec the;
  };

  template<class Method, class Spec>
  register_spec<Method, Spec> register_spec<Method, Spec>::the;

  template<template<typename Sig> class Method, typename R, typename... P>
  typename multimethod_impl<Method, R(P...)>::mptr* multimethod_impl<Method, R(P...)>::dispatch_table;

  template<template<typename Sig> class Method, typename R, typename... P>
  multimethod_base* multimethod_impl<Method, R(P...)>::base;

  template<template<typename Sig> class Method, typename R, typename... P>
  template<typename Tag>
  typename multimethod_impl<Method, R(P...)>::mptr multimethod_impl<Method, R(P...)>::next_ptr<Tag>::next;

  template<template<typename Sig> class Method, typename R, typename... P>
  void multimethod_impl<Method, R(P...)>::init_base() {
    if (!base) {
      using namespace std;
      MM_TRACE(cout << "init state for " << virtuals() << "\n");
      base = new multimethod_base(mm_class_vector_of<typename multimethod_impl<Method, R(P...)>::virtuals>::get());
    }
  }
  
  template<template<typename Sig> class Method, typename R, typename... P>
  inline R multimethod_impl<Method, R(P...)>::operator ()(typename remove_virtual<P>::type... args) const {
    if (!base->ready) {
      prepare();
    }
    return dispatch_table[linear<P...>::value(base->slots.begin(), base->steps.begin(), 0, &args...)](args...);
  }
  
  template<template<typename Sig> class Method, typename R, typename... P>
  void multimethod_impl<Method, R(P...)>::prepare() {
    resolver r(*base);
    multimethod_base::emit_func emit;
    multimethod_base::emit_next_func emit_next;
    allocate_dispatch_table(r.dispatch_table_size, emit, emit_next);
    r.resolve(emit, emit_next);
    base->ready = true;
  }
  
  template<template<typename Sig> class Method, typename R, typename... P>
  
  void multimethod_impl<Method, R(P...)>::allocate_dispatch_table(
    int size,
    multimethod_base::emit_func& emit,
    multimethod_base::emit_next_func& emit_next) {
    using namespace std;
    delete [] dispatch_table;
    dispatch_table = new mptr[size];
    emit = [=](method_base* method, int i) {
      dispatch_table[i] =
        method == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : method == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<method_entry*>(method)->pm;
      MM_TRACE(cout << "installed at " << dispatch_table << " + " << i << endl);
    };
    emit_next = [](method_base* method, method_base* next) {
      *static_cast<method_entry*>(method)->pn =
        next == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : next == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<method_entry*>(next)->pm;
    };
  }
  
  template<template<typename Sig> class Method, typename R, typename... P>
  template<class M>
  method_base* multimethod_impl<Method, R(P...)>::add() {
    using method_signature = decltype(M::body);
    using target = typename wrapper<M, method_signature, signature>::type;
    using method_virtuals = typename extract_method_virtuals<R(P...), method_signature>::type;
    using namespace std;
    init_base();
    MM_TRACE(cout << "add " << method_virtuals() << " to " << virtuals() << endl);

    method_base* method = new method_entry(target::body, mm_class_vector_of<method_virtuals>::get(), &M::next);
    
    base->methods.push_back(method);
    base->invalidate();

    return method;
  }

  template<typename Tag>
  struct multimethod;

  template<class MM, class M> struct register_method;

#define CONCAT(X, Y) CONCAT1(X, Y)
#define CONCAT1(X, Y) X ## Y
#define INIT_ID(ID) CONCAT(__add_method_, CONCAT(ID, __LINE__))

#define MM_CLASS(CLASS, BASES...)                  \
  using mm_base_list_type = ::multimethods::type_list<BASES>;           \
  using mm_this_type = CLASS;                                           \
  
  // static_assert(std::is_same<mm_this_type, std::remove_reference<decltype(*this)>::type>::value, "Error in MM_CLASS(): declared class is not correct"); \
  // static_assert(::multimethods::check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  // virtual void* __init_mm_class() {                                     \
  //   return &::multimethods::mm_class_initializer<mm_this_type, mm_base_list_type>::the; }
  
#define MM_FOREIGN_CLASS(CLASS, BASES...)                               \
  static_assert(::multimethods::check_bases<CLASS, ::multimethods::type_list<BASES>>::value, "Error in MM_FOREIGN_CLASS(): not a base in base list"); \
  namespace { ::multimethods::mm_class_initializer<CLASS, ::multimethods::type_list<BASES>> INIT_ID(CLASS); }

#define MM_INIT() \
  static_assert(std::is_same<mm_this_type, std::remove_reference<decltype(*this)>::type>::value, "Error in MM_CLASS(): declared class is not correct"); \
  static_assert(::multimethods::check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  &::multimethods::mm_class_initializer<mm_this_type, mm_base_list_type>::the; \
  init_mmptr(this)

#define MULTIMETHOD(ID, SIG)                                            \
  template<typename Sig> struct ID ## _method;                          \
  const ::multimethods::multimethod_impl<ID ## _method, SIG> ID;
  
#define REGISTER_METHOD_ID(MM, M) __register_ ## MM ## _ ## M
#define REGISTER_METHOD(MM, M)                                  \
  static auto REGISTER_METHOD_ID(MM, M) = MM.add<M>();

#define BEGIN_METHOD(ID, RESULT, ARGS...)                               \
  template<>                                                            \
  struct ID ## _method<RESULT(ARGS)> : decltype(ID)::next_ptr<RESULT(ARGS)> { \
  static RESULT body(ARGS) {                                            \
  &::multimethods::register_spec<decltype(ID), ID ## _method>::the;

#define END_METHOD } };

#define STATIC_CALL_METHOD(ID, SIG) ID ## _method<SIG>::body
    
}
#endif
