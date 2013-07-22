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
#include <map>
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

  using class_set = std::unordered_set<const mm_class*>;

  struct mm_class {
    struct mmref {
      multimethod_base* method;
      int arg;
    };
    
    mm_class(const std::type_info& t);
    ~mm_class();

    const std::string name() const;
    void initialize(std::vector<mm_class*>&& bases);
    void add_multimethod(multimethod_base* pm, int arg);
    void for_each_spec(std::function<void(mm_class*)> pf);
    void for_each_conforming(std::function<void(mm_class*)> pf);
    void for_each_conforming(class_set& visited, std::function<void(mm_class*)> pf);
    bool conforms_to(const mm_class& other) const;
    bool specializes(const mm_class& other) const;
    
    const std::type_info& ti;
    std::vector<mm_class*> bases;
    std::vector<mm_class*> specs;
    boost::dynamic_bitset<> mask;
    int index{-1};
    mm_class* root;
    std::vector<int> mmt;
    std::vector<mmref> rooted_here; // multimethods rooted here for one or more args.
    bool abstract;
    
    static std::unordered_set<mm_class*>& to_initialize();
  };

  inline const std::string mm_class::name() const {
    return ti.name();
  }

  inline std::ostream& operator <<(std::ostream& os, const mm_class* pc) {
    if (pc) {
      if (pc->index != -1) {
        os << "class_" << pc->index;
      } else{
        os << pc->ti.name();
      }
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
      pc.initialize({ &mm_class_of<Bases>::the()... });

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
    int index; // inside multimethod
    std::vector<mm_class*> args;
    bool specializes(method_base* other) const;
    static method_base undefined;
    static method_base ambiguous;
  };

  std::ostream& operator <<(std::ostream& os, method_base* method);
  std::ostream& operator <<(std::ostream& os, const std::vector<method_base*>& methods);

  struct multimethod_base {
    explicit multimethod_base(const std::vector<mm_class*>& v);
    virtual void resolve() = 0;
    virtual void allocate_dispatch_table(int size) = 0;
    virtual void emit(method_base*, int i) = 0;
    virtual void emit_next(method_base*, method_base*) = 0;
    void invalidate();
    void assign_slot(int arg, int slot);
    
    std::vector<mm_class*> vargs;
    std::vector<int> slots;
    std::vector<method_base*> methods;
    std::vector<int> steps;
    static std::unordered_set<multimethod_base*>& to_initialize();
  };

  std::ostream& operator <<(std::ostream& os, const multimethod_base* pc);

  template<class M, typename Override, class Base>
  struct wrapper;

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

    method(int index, M pm, std::vector<mm_class*> type_tuple, M* pn) : pm(pm), pn(pn) {
      this->index = index;
      this->args = type_tuple;
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

  struct grouping_resolver {
    grouping_resolver(multimethod_base& mm);

    void resolve();
    void resolve(int dim, const boost::dynamic_bitset<>& candidates);
    void find_applicable(int dim, const mm_class* pc, std::vector<method_base*>& best);
    method_base* find_best(const boost::dynamic_bitset<>& candidates);
    method_base* find_best(const std::vector<method_base*>& methods);
    void make_mask(const std::vector<method_base*>& best, boost::dynamic_bitset<>& mask);
    void make_groups();
    void make_table();
    void assign_next();

    struct group {
      boost::dynamic_bitset<> mask;
      std::vector<method_base*> methods;
      std::vector<mm_class*> classes;
    };

    multimethod_base& mm;
    const int dims;
    std::vector<std::vector<group>> groups;
    int emit_at;
  };

  struct hierarchy_initializer {
    hierarchy_initializer(mm_class& root);

    void collect_classes();
    void make_masks();
    void assign_slots();
    void execute();

    static void initialize(mm_class& root);

    void topological_sort_visit(class_set& once, mm_class* pc);

    mm_class& root;
    std::vector<mm_class*> nodes;
  };

  namespace detail {
  
    template<typename R, typename... P>
    struct multimethod_implementation : multimethod_base {
      using return_type = R;
      using mptr = return_type (*)(typename remove_virtual<P>::type...);
      using method_entry = method<mptr>;
      using signature = R(typename remove_virtual<P>::type...);
      using virtuals = typename extract_virtuals<P...>::type;
      
      multimethod_implementation() :
        multimethod_base(mm_class_vector_of<virtuals>::get()) {
      }

      template<class M> method_base* add_spec();

      virtual void resolve();
      virtual void allocate_dispatch_table(int size);
      virtual void emit(method_base*, int i);
      virtual void emit_next(method_base*, method_base*);

      mptr* dispatch_table{0};
    };
    
    template<typename R, typename... P>
    template<class M>
    method_base* multimethod_implementation<R, P...>::add_spec() {
      using method_signature = decltype(M::body);
      using target = typename wrapper<M, method_signature, signature>::type;
      using method_virtuals = typename extract_method_virtuals<R(P...), method_signature>::type;

      using namespace std;
      MM_TRACE(cout << "add " << method_virtuals() << " to " << virtuals() << endl);

      method_base* method = new method_entry(methods.size(), target::body, mm_class_vector_of<method_virtuals>::get(), &M::next);
      methods.push_back(method);
      invalidate();

      return method;
    }

    template<typename R, typename... P>
    void multimethod_implementation<R, P...>::resolve() {
      grouping_resolver r(*this);
      r.resolve();
    }
  
    template<typename R, typename... P>
    void multimethod_implementation<R, P...>::allocate_dispatch_table(int size) {
      using namespace std;
      delete [] dispatch_table;
      dispatch_table = new mptr[size];
    }
  
    template<typename R, typename... P>
    void multimethod_implementation<R, P...>::emit(method_base* method, int i) {
      dispatch_table[i] =
        method == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : method == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<const method_entry*>(method)->pm;
      using namespace std;
      MM_TRACE(cout << "installed at " << dispatch_table << " + " << i << endl);
    }
  
    template<typename R, typename... P>
    void multimethod_implementation<R, P...>::emit_next(method_base* method, method_base* next) {
      *static_cast<const method_entry*>(method)->pn =
        next == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : next == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<const method_entry*>(next)->pm;
    }
  }

  template<template<typename Sig> class Method, typename Sig>
  struct multimethod;
  
  template<template<typename Sig> class Method, typename R, typename... P>
  struct multimethod<Method, R(P...)> {
    
    R operator ()(typename remove_virtual<P>::type... args) const;

    using return_type = R;
    using mptr = return_type (*)(typename remove_virtual<P>::type...);
    using method_entry = method<mptr>;
    using signature = R(typename remove_virtual<P>::type...);
    using virtuals = typename extract_virtuals<P...>::type;

    template<typename Tag>
    struct next_ptr {
      static mptr next;
    };

    using implementation = detail::multimethod_implementation<R, P...>;
    static implementation& the();
  private:
    static implementation* impl;
  };

  template<class Method, class Spec>
  struct register_spec {
    register_spec() {
      Method::the().template add_spec<Spec>();
    }
    static register_spec the;
  };

  template<class Method, class Spec>
  register_spec<Method, Spec> register_spec<Method, Spec>::the;

  template<template<typename Sig> class Method, typename R, typename... P>
  typename multimethod<Method, R(P...)>::implementation* multimethod<Method, R(P...)>::impl;

  template<template<typename Sig> class Method, typename R, typename... P>
  template<typename Tag>
  typename multimethod<Method, R(P...)>::mptr multimethod<Method, R(P...)>::next_ptr<Tag>::next;

  template<template<typename Sig> class Method, typename R, typename... P>
  typename multimethod<Method, R(P...)>::implementation& multimethod<Method, R(P...)>::the() {
    if (!impl) {
      impl = new implementation;
    }
    
    return *impl;
  }
  
  template<template<typename Sig> class Method, typename R, typename... P>
  inline R multimethod<Method, R(P...)>::operator ()(typename remove_virtual<P>::type... args) const {
    return impl->dispatch_table[linear<P...>::value(impl->slots.begin(), impl->steps.begin(), 0, &args...)](args...);
  }

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
  static_assert(::multimethods::check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  &::multimethods::mm_class_initializer<mm_this_type, mm_base_list_type>::the; \
  this->init_mmptr(this)

// normally part of MM_INIT(), disabled because g++ doesn't like it in class templates
//  static_assert(std::is_same<mm_this_type, std::remove_reference<decltype(*this)>::type>::value, "Error in MM_CLASS(): declared class is not correct"); \

#define MULTIMETHOD(ID, SIG)                                            \
  template<typename Sig> struct ID ## _method;                          \
  constexpr ::multimethods::multimethod<ID ## _method, SIG> ID;
  
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
