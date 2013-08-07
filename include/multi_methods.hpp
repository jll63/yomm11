// -*- compile-command: "cd ../tests && make" -*-

#ifndef MULTI_METHODS_INCLUDED
#define MULTI_METHODS_INCLUDED

// multi_method.hpp
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
#include <boost/preprocessor/cat.hpp>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

//#define MM_ENABLE_TRACE

#ifdef MM_ENABLE_TRACE
#define MM_TRACE(e) e
#include <iterator>
#else
#define MM_TRACE(e)
#endif

# endif

namespace multi_methods {

  namespace detail {
    union offset {
      int index;
      void (**ptr)();
    };
  }

  void initialize();

  struct selector {
    selector() : _mm_ptbl(0) { }
    std::vector<detail::offset>* _mm_ptbl;
    virtual ~selector() { }
    template<class THIS>
    void _init_mmptr(THIS*);
    std::vector<detail::offset>* _get_mm_ptbl() const { return _mm_ptbl; }
  };

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  struct mm_class;
  struct multi_method_base;

  using class_set = std::unordered_set<const mm_class*>;

  namespace detail {
    using void_function_pointer = void (*)();
  }
  
  struct mm_class {
    struct mmref {
      multi_method_base* method;
      int arg;
    };
    
    mm_class(const std::type_info& t);
    ~mm_class();

    const std::string name() const;
    void initialize(const std::vector<mm_class*>& bases);
    void add_multi_method(multi_method_base* pm, int arg);
    void remove_multi_method(multi_method_base* pm);
    void for_each_spec(std::function<void(mm_class*)> pf);
    void for_each_conforming(std::function<void(mm_class*)> pf);
    void for_each_conforming(class_set& visited, std::function<void(mm_class*)> pf);
    bool conforms_to(const mm_class& other) const;
    bool specializes(const mm_class& other) const;
    
    const std::type_info& ti;
    std::vector<mm_class*> bases;
    std::vector<mm_class*> specs;
    boost::dynamic_bitset<> mask;
    int index;
    mm_class* root;
    std::vector<detail::offset> mmt;
    std::vector<mmref> rooted_here; // multi_methods rooted here for one or more args.
    bool abstract;
    
    static std::unique_ptr<std::unordered_set<mm_class*>> to_initialize;
    static void add_to_initialize(mm_class* pc);
    static void remove_from_initialize(mm_class* pc);
  };

  inline const std::string mm_class::name() const {
    return ti.name();
  }

  std::ostream& operator <<(std::ostream& os, const mm_class* pc);
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

  template<class THIS>
  inline void selector::_init_mmptr(THIS*) {
    _mm_ptbl = &mm_class_of<THIS>::the().mmt;
  }

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
  
  template<bool Native>
  struct get_mm_table;

  template<>
  struct get_mm_table<true> {
    template<class C>
    static const std::vector<detail::offset>& value(const C* obj) {
      return *obj->_get_mm_ptbl();
    }
  };

  template<>
  struct get_mm_table<false> {
    using class_of_type = std::unordered_map<std::type_index, const std::vector<detail::offset>*>;
    static class_of_type* class_of;
    template<class C>
    static const std::vector<detail::offset>& value(const C* obj) {
      MM_TRACE(std::cout << "foreign mm_class_of<" << typeid(*obj).name() << "> = " << (*class_of)[std::type_index(typeid(*obj))] << std::endl);
      return *(*class_of)[std::type_index(typeid(*obj))];
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

  template<class Class, class BaseList>
  struct mm_class_initializer;
  
  template<class Class, class... Bases>
  struct mm_class_initializer<Class, type_list<Bases...>> {
    mm_class_initializer() {
      mm_class& pc = mm_class_of<Class>::the();
      pc.abstract = std::is_abstract<Class>::value;
      pc.initialize(mm_class_vector_of<Bases...>::get());

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
    virtual ~method_base();
    
    int index; // inside multi_method
    std::vector<mm_class*> args;
    bool specializes(method_base* other) const;
    static method_base undefined;
    static method_base ambiguous;
  };

  std::ostream& operator <<(std::ostream& os, method_base* method);
  std::ostream& operator <<(std::ostream& os, const std::vector<method_base*>& methods);

  struct multi_method_base {
    explicit multi_method_base(const std::vector<mm_class*>& v);
    virtual ~multi_method_base();
    
    virtual void resolve() = 0;
    virtual detail::void_function_pointer* allocate_dispatch_table(int size) = 0;
    virtual void emit(method_base*, int i) = 0;
    virtual void emit_next(method_base*, method_base*) = 0;
    void invalidate();
    void assign_slot(int arg, int slot);
    
    std::vector<mm_class*> vargs;
    std::vector<int> slots;
    std::vector<method_base*> methods;
    std::vector<int> steps;
    
    static std::unique_ptr<std::unordered_set<multi_method_base*>> to_initialize;
    static void add_to_initialize(multi_method_base* pm);
    static void remove_from_initialize(multi_method_base* pm);
  };

  std::ostream& operator <<(std::ostream& os, const multi_method_base* pc);

  template<class M, typename Override, class Base>
  struct wrapper;

  using boost::is_virtual_base_of;

  template<class B, class D>
  struct cast_using_static_cast {
    static D& value(B& obj) { return static_cast<D&>(obj); }
    static const D& value(const B& obj) { return static_cast<const D&>(obj); }
  };

  template<class B, class D>
  struct cast_using_dynamic_cast {
    static D& value(B& obj) { return dynamic_cast<D&>(obj); }
    static const D& value(const B& obj) { return dynamic_cast<const D&>(obj); }
  };

  template<class B, class D, bool is_virtual>
  struct cast_best;

  template<class B, class D>
  struct cast_best<B, D, false> : cast_using_static_cast<B, D> {
  };

  template<class B, class D>
  struct cast_best<B, D, true> : cast_using_dynamic_cast<B, D> {
  };

  template<class B, class D>
  struct cast : cast_best<B, D, boost::is_virtual_base_of<B, D>::value> {
  };

  template<class B>
  struct cast<B, B> {
    static B& value(B& obj) { return obj; }
    static const B& value(const B& obj) { return obj; }
  };
  
  template<class M, typename... A, typename OR, typename... P, typename BR>
  struct wrapper<M, OR(A...), BR(P...)> {
    using type = wrapper;
    static BR body(P... args) {
      return M::body(
        cast<
          typename std::remove_const<
            typename std::remove_reference<P>::type
          >::type,
          typename std::remove_const<
            typename std::remove_reference<A>::type
          >::type
        >::value(args)...);
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

  struct grouping_resolver {
    grouping_resolver(multi_method_base& mm);

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

    multi_method_base& mm;
    const int dims;
    std::vector<std::vector<group>> groups;
    detail::void_function_pointer* dispatch_table;
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
    struct multi_method_implementation : multi_method_base {
      using return_type = R;
      using mptr = return_type (*)(typename remove_virtual<P>::type...);
      using method_entry = method<mptr>;
      using signature = R(typename remove_virtual<P>::type...);
      using virtuals = typename extract_virtuals<P...>::type;
      
      multi_method_implementation() :
        multi_method_base(mm_class_vector_of<virtuals>::get()),
        dispatch_table(nullptr) {
      }

      template<class M> method_base* add_spec();

      virtual void resolve();
      virtual detail::void_function_pointer* allocate_dispatch_table(int size);
      virtual void emit(method_base*, int i);
      virtual void emit_next(method_base*, method_base*);

      mptr* dispatch_table;
    };
    
    template<typename R, typename... P>
    template<class M>
    method_base* multi_method_implementation<R, P...>::add_spec() {
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
    void multi_method_implementation<R, P...>::resolve() {
      grouping_resolver r(*this);
      r.resolve();
    }
  
    template<typename R, typename... P>
    detail::void_function_pointer* multi_method_implementation<R, P...>::allocate_dispatch_table(int size) {
      using namespace std;
      delete [] dispatch_table;
      dispatch_table = new mptr[size];
      return reinterpret_cast<void_function_pointer*>(dispatch_table);
    }
  
    template<typename R, typename... P>
    void multi_method_implementation<R, P...>::emit(method_base* method, int i) {
      dispatch_table[i] =
        method == &method_base::ambiguous ? throw_ambiguous<signature>::body
        : method == &method_base::undefined ? throw_undefined<signature>::body
        : static_cast<const method_entry*>(method)->pm;
      using namespace std;
      MM_TRACE(cout << "installed at " << dispatch_table << " + " << i << endl);
    }
  
    template<typename R, typename... P>
    void multi_method_implementation<R, P...>::emit_next(method_base* method, method_base* next) {
      *static_cast<const method_entry*>(method)->pn =
        (next == &method_base::ambiguous || next == &method_base::undefined) ? nullptr
        : static_cast<const method_entry*>(next)->pm;
    }
  }

  template<template<typename Sig> class Method, typename Sig>
  struct multi_method;
  
  template<template<typename Sig> class Method, typename R, typename... P>
  struct multi_method<Method, R(P...)> {
    
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

    mptr next_ptr_type() const;

    using implementation = detail::multi_method_implementation<R, P...>;
    static implementation& the();
    static std::unique_ptr<implementation> impl;

    template<class Spec>
    static bool specialize() {
      the().template add_spec<Spec>();
      return true;
    }
  };

  template<typename Multimethod, typename Sig>
  struct specializer {
    static typename Multimethod::mptr next;
  };

  template<typename Multimethod, typename Sig>
  typename Multimethod::mptr specializer<Multimethod, Sig>::next;

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
  std::unique_ptr<typename multi_method<Method, R(P...)>::implementation> multi_method<Method, R(P...)>::impl;

  template<template<typename Sig> class Method, typename R, typename... P>
  template<typename Tag>
  typename multi_method<Method, R(P...)>::mptr multi_method<Method, R(P...)>::next_ptr<Tag>::next;

  template<template<typename Sig> class Method, typename R, typename... P>
  typename multi_method<Method, R(P...)>::implementation& multi_method<Method, R(P...)>::the() {
    if (!impl) {
      impl.reset(new implementation);
    }
    
    return *impl;
  }
    
  template<int Dim, typename... P>
  struct linear;


  template<typename P1, typename... P>
  struct linear<0, P1, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1, A... args) {
      return linear<0, P...>::value(slot_iter, step_iter, args...);
    }
  };
    
  template<typename P1, typename... P>
  struct linear<0, virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1 arg, A... args) {
      return linear<1, P...>::value(
        slot_iter + 1, step_iter + 1,
        get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].ptr, args...);
    }
  };
    
  template<typename P1, typename... P>
  struct linear<0, const virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1 arg, A... args) {
      return linear<1, P...>::value(
        slot_iter + 1, step_iter + 1,
        get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].ptr, args...);
    }
  };


  
  template<int Dim, typename P1, typename... P>
  struct linear<Dim, P1, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      detail::void_function_pointer* ptr,
      A1, A... args) {
      return linear<Dim, P...>::value(slot_iter, step_iter, ptr, args...);
    }
  };
    
  template<int Dim, typename P1, typename... P>
  struct linear<Dim, virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      detail::void_function_pointer* ptr,
      A1 arg, A... args) {
      MM_TRACE(std::cout << " -> " << ptr);
      return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
    }
  };
    
  template<int Dim, typename P1, typename... P>
  struct linear<Dim, const virtual_<P1>&, P...> {
    template<typename A1, typename... A>
    static detail::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      detail::void_function_pointer* ptr,
      A1 arg, A... args) {
      MM_TRACE(std::cout << " -> " << ptr);
      return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
    }
  };
    
  template<int Dim>
    struct linear<Dim> {
    static detail::void_function_pointer* value(std::vector<int>::const_iterator slot_iter,
              std::vector<int>::const_iterator step_iter,
              detail::void_function_pointer* ptr) {
      MM_TRACE(std::cout << " -> " << ptr << std::endl);
      return ptr;
    }
  };
  
  template<template<typename Sig> class Method, typename R, typename... P>
  inline R multi_method<Method, R(P...)>::operator ()(typename remove_virtual<P>::type... args) const {
    MM_TRACE((std::cout << "() mm table = " << impl->dispatch_table << std::flush));
    return reinterpret_cast<mptr>(*linear<0, P...>::value(impl->slots.begin(), impl->steps.begin(), &args...))(args...);
  }

  template<class MM, class M> struct register_method;

#endif

#define MM_CLASS(CLASS, BASES...)                  \
  using mm_base_list_type = ::multi_methods::type_list<BASES>;           \
  using mm_this_type = CLASS;                                           \
  virtual void _mm_init_class_() { &::multi_methods::mm_class_initializer<mm_this_type, mm_base_list_type>::the; }
  
#define MM_FOREIGN_CLASS(CLASS, BASES...)                               \
  static_assert(::multi_methods::check_bases<CLASS, ::multi_methods::type_list<BASES>>::value, "error in MM_FOREIGN_CLASS(): not a base in base list"); \
  static_assert(std::is_polymorphic<CLASS>::value, "error: class must be polymorphic"); \
  namespace { ::multi_methods::mm_class_initializer<CLASS, ::multi_methods::type_list<BASES>> BOOST_PP_CAT(_mm_add_class, CLASS); }

#define MM_INIT() \
  static_assert(::multi_methods::check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  this->_init_mmptr(this)

#define MM_CLASS_MULTI(CLASS, BASE, BASES...)                     \
  MM_CLASS(CLASS, BASE, BASES)                                          \
  std::vector<detail::offset>* _get_mm_ptbl() const { return BASE::_mm_ptbl; }

#define MM_INIT_MULTI(BASE)                                       \
  static_assert(::multi_methods::check_bases<mm_this_type, mm_base_list_type>::value, "Error in MM_CLASS(): not a base in base list"); \
  this->BASE::_init_mmptr(this)
  
// normally part of MM_INIT(), disabled because g++ doesn't like it in class templates
//  static_assert(std::is_same<mm_this_type, std::remove_reference<decltype(*this)>::type>::value, "Error in MM_CLASS(): declared class is not correct"); \

#define MULTI_METHOD(ID, RETURN_TYPE, ARGS...)                           \
  template<typename Sig> struct ID ## _method;                          \
  constexpr ::multi_methods::multi_method<ID ## _method, RETURN_TYPE(ARGS)> ID{};

#define BEGIN_SPECIALIZATION(ID, RESULT, ARGS...)                               \
  template<>                                                            \
  struct ID ## _method<RESULT(ARGS)> : ::multi_methods::specializer<decltype(ID), RESULT(ARGS)> { \
  static RESULT body(ARGS) {                                            \
  &::multi_methods::register_spec<decltype(ID), ID ## _method>::the;

#define END_SPECIALIZATION } };

#define STATIC_CALL_METHOD(ID, SIG) ID ## _method<SIG>::body
    
}
#endif
