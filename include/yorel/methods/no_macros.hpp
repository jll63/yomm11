// -*- compile-command: "cd ../../.. && make && make test" -*-

#ifndef YOMM11_NO_MACROS_INCLUDED
#define YOMM11_NO_MACROS_INCLUDED

// method/no_macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Copied from Boost.
#ifdef _MSVC_VER
#pragma warning( push )
#pragma warning( disable: 4250 )
#pragma warning( disable: 4284 )
#pragma warning( disable: 4996 )
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#pragma GCC system_header
#endif

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

//#define YOMM11_ENABLE_TRACE

#ifdef YOMM11_ENABLE_TRACE
#define YOMM11_TRACE(e) e
#define YOMM11_COMMA_TRACE(e) , e
#include <iterator>
#else
#define YOMM11_TRACE(e)
#define YOMM11_COMMA_TRACE(e)
#include <iterator>
#endif

// Copied from Boost.
#ifdef _MSVC_VER
#pragma warning( push )
#pragma warning( disable : 4584 4250)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#pragma GCC system_header
#endif

namespace yorel {
namespace methods {

// Forward declarations.
// All names in namespace are listed below.
void initialize();
struct selector;
template<class Class> struct virtual_;
class undefined;
class ambiguous;

#ifdef YOMM11_ENABLE_TRACE
std::ostream& operator <<(std::ostream& os, const yomm11_class* pc);
std::ostream& operator <<(std::ostream& os, const std::vector<yomm11_class*>& classes);
std::ostream& operator <<(std::ostream& os, specialization_base* method);
std::ostream& operator <<(std::ostream& os, const std::vector<specialization_base*>& methods);
std::ostream& operator <<(std::ostream& os, const method_base* pc);
#endif

// End of forward declarations.

class undefined : public std::runtime_error {
 public:
  undefined();

 protected:
  undefined(const std::string& message);
};

class ambiguous : public undefined {
 public:
  ambiguous();
};

template<class B, class D> struct cast_using_static_cast;
template<class B, class D> struct cast_using_dynamic_cast;
template<class B, class D> struct cast;

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

namespace detail {

struct yomm11_class;
struct specialization_base;
struct method_base;
template<template<typename Sig> class Method, typename Sig> struct method;

//using bitvec = boost::dynamic_bitset<>;

class bitvec {
  using word = unsigned long;
 public:
  class ref {
    word* p;
    int i;
   public:
    ref(word* p, int i) : p(p), i(i) {
    }
    operator bool() const {
      return (p[i / bpw] & (1 << (i % bpw))) != 0;
    }
    ref& operator =(bool val) {
      if (val) {
        p[i / bpw] |= 1 << (i % bpw);
      } else {
        p[i / bpw] &= ~(1 << (i % bpw));
      }
      return *this;
    }
    ref& operator |=(bool val) {
      if (val) {
        p[i / bpw] |= 1 << (i % bpw);
      }
      return *this;
    }
  };

  bitvec() : p(nullptr), n(0) { }

  bitvec(int n) : n(n), p(nullptr) {
    p = new word[wsize(n)];
    std::fill(wbegin(), wend(), 0);
  }

  bitvec(int n, word init) : bitvec(n) {
    *p = init;
  }

  bitvec(const bitvec& other) : bitvec(other.n) {
    std::copy(other.p, other.p + wsize(other.n), p);
  }

  ~bitvec() { delete [] p; }

  bitvec& operator =(const bitvec& other) {
    size_t ws = wsize(other.n);
    word* np = new word[ws];
    std::copy(other.wbegin(), other.wend(), np);
    delete [] p;
    p = np;
    n = other.size();
    return *this;
  }

  int size() const { return n; }

  void resize(int size) {
    size_t new_ws = wsize(size);
    word* new_p = new word[new_ws];
    size_t old_ws = wsize(n);
    auto end = std::copy(p, p + std::min(old_ws, new_ws), new_p);
    std::fill(end, new_p + new_ws, 0);
    if (size < n) {
      if (size_t rem = size % bpw) {
        end[-1] &= (1 << rem) - 1;
      }
    }
    delete [] p;
    p = new_p;
    n = size;
  }

  bool none() const {
    return std::all_of(p, p + wsize(n), [](word w) { return w == 0; });
  }

  bool operator [](int i) const {
    return (p[i / bpw] & (1 << (i % bpw))) != 0;
  }

  ref operator [](int i) { return ref(p, i); }

  friend bitvec operator &(const bitvec& v1, const bitvec& v2) {
    bitvec res(v1.size());
    std::transform(
        v1.wbegin(), v1.wend(), v2.wbegin(), res.wbegin(),
        [](word w1, word w2) { return w1 & w2; });
    return res;
  }

  friend bool operator ==(const bitvec& v1, const bitvec& v2) {
    return std::equal(v1.wbegin(), v1.wend(), v2.wbegin());
  }

  friend bool operator <(const bitvec& v1, const bitvec& v2) {
    return std::lexicographical_compare(
        v1.wbegin(), v1.wend(),
        v2.wbegin(), v2.wend());
  }

  bitvec& operator |=(const bitvec& other) {
    std::transform(
        other.wbegin(), other.wend(), wbegin(), wbegin(),
        [](word w1, word w2) { return w1 | w2; });
    return *this;
  }

  bitvec operator ~() const {
    bitvec res(size());
    auto res_last = std::transform(
        wbegin(), wend(), res.wbegin(),
        [](word w) { return ~w; });
    if (size_t rem = n % bpw) {
      res_last[-1] &= (1 << rem) - 1;
    }
    return res;
  }

  word* wbegin() { return p; }

  const word* wbegin() const { return p; }

  word* wend() { return p + wsize(n); }

  const word* wend() const { return p + wsize(n); }

 private:
  static size_t wsize(int n) { return (n + bpw - 1) / bpw; }
  static const int bpw = std::numeric_limits<word>::digits;
  int n;
  word* p;
};

std::ostream& operator <<(std::ostream& os, const bitvec& v);

struct yomm11_class {
  struct method_param {
    method_base* method;
    int arg;
  };

  union offset {
    int index;
    void (**ptr)();
  };

  yomm11_class(YOMM11_TRACE(const char* name));
  ~yomm11_class();

  void initialize(const std::vector<yomm11_class*>& bases);
  void add_method(method_base* pm, int arg);
  void remove_method(method_base* pm);
  void for_each_spec(std::function<void(yomm11_class*)> pf);
  void for_each_conforming(std::function<void(yomm11_class*)> pf);
  void for_each_conforming(std::unordered_set<const yomm11_class*>& visited, std::function<void(yomm11_class*)> pf);
  bool conforms_to(const yomm11_class& other) const;
  bool specializes(const yomm11_class& other) const;
  bool is_root() const;
  YOMM11_TRACE(const char* name);
  std::vector<yomm11_class*> bases;
  std::vector<yomm11_class*> specs;
  detail::bitvec mask;
  int index;
  yomm11_class* root;
  std::vector<yomm11_class::offset> mmt;
  std::vector<method_param> rooted_here; // methods rooted here for one or more args.
  bool abstract;

  static std::unordered_set<yomm11_class*>* to_initialize;
  static void add_to_initialize(yomm11_class* pc);
  static void remove_from_initialize(yomm11_class* pc);

  template<class Class>
  struct of {
    static yomm11_class* pc;
    static yomm11_class& the() {
      static yomm11_class instance YOMM11_TRACE({_yomm11_name_((Class*) nullptr)});
      pc = &instance;
      return instance;
    }
  };

  template<class Class>
  struct of<const Class> : of<Class> { };

  template<typename... T>
  struct base_list;

  template<class Class, class BaseList>
  struct initializer;

  template<class Class, class... Bases>
  struct initializer<Class, base_list<Bases...>> {
    initializer();
    static initializer the;
  };
};

template<class Class>
yomm11_class* yomm11_class::of<Class>::pc;

inline bool yomm11_class::is_root() const {
  return this == root;
}

struct specialization_base {
  virtual ~specialization_base();

  int index; // inside method
  std::vector<yomm11_class*> args;
  bool specializes(specialization_base* other) const;
  static specialization_base undefined;
  static specialization_base ambiguous;
};

struct method_base {
  explicit method_base(const std::vector<yomm11_class*>& v YOMM11_COMMA_TRACE(const char* name));
  virtual ~method_base();

  using void_function_pointer = void (*)();

  void resolve();
  virtual void_function_pointer* allocate_dispatch_table(int size) = 0;
  virtual void emit(specialization_base*, int i) = 0;
  virtual void emit_next(specialization_base*, specialization_base*) = 0;
  void invalidate();
  void assign_slot(int arg, int slot);

  std::vector<yomm11_class*> vargs;
  std::vector<int> slots;
  std::vector<specialization_base*> methods;
  std::vector<int> steps;
  YOMM11_TRACE(const char* name);

  static std::unordered_set<method_base*>* to_initialize;
  static void add_to_initialize(method_base* pm);
  static void remove_from_initialize(method_base* pm);
};

// Copied from Boost.
template<typename Base, typename Derived>
struct is_virtual_base_of
{
#ifdef __BORLANDC__
  struct internal_struct_X : public virtual Derived, public virtual Base
  {
    internal_struct_X();
    internal_struct_X(const internal_struct_X&);
    internal_struct_X& operator=(const internal_struct_X&);
    ~internal_struct_X()throw();
    virtual void _yomm11_init_class_() { }
  };
  struct internal_struct_Y : public virtual Derived
  {
    internal_struct_Y();
    internal_struct_Y(const internal_struct_Y&);
    internal_struct_Y& operator=(const internal_struct_Y&);
    ~internal_struct_Y()throw();
    virtual void _yomm11_init_class_() { }
  };
#else
  struct internal_struct_X : public Derived, virtual Base
  {
    internal_struct_X();
    internal_struct_X(const internal_struct_X&);
    internal_struct_X& operator=(const internal_struct_X&);
    ~internal_struct_X()throw();
    virtual void _yomm11_init_class_() { }
  };
  struct internal_struct_Y : public Derived
  {
    internal_struct_Y();
    internal_struct_Y(const internal_struct_Y&);
    internal_struct_Y& operator=(const internal_struct_Y&);
    ~internal_struct_Y()throw();
    virtual void _yomm11_init_class_() { }
  };
#endif
  static const int value = sizeof(internal_struct_X) == sizeof(internal_struct_Y);
};

template<typename... Class>
struct virtuals {
};

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

template<class... Class> struct yomm11_class_vector_of_;

template<class First, class... Rest>
struct yomm11_class_vector_of_<First, Rest...> {
  static void get(std::vector<yomm11_class*>& classes) {
    classes.push_back(&yomm11_class::of<First>::the());
    yomm11_class_vector_of_<Rest...>::get(classes);
  }
};

template<>
struct yomm11_class_vector_of_<> {
  static void get(std::vector<yomm11_class*>& classes) {
  }
};

template<class... Class>
struct yomm11_class_vector_of {
  static std::vector<yomm11_class*> get() {
    std::vector<yomm11_class*> classes;
    yomm11_class_vector_of_<Class...>::get(classes);
    return classes;
  }
};

template<class... Class>
struct yomm11_class_vector_of<virtuals<Class...>> {
  static std::vector<yomm11_class*> get() {
    return yomm11_class_vector_of<Class...>::get();
  }
};

template<bool Native>
struct get_mm_table;

template<>
struct get_mm_table<true> {
  template<class C>
  static const std::vector<yomm11_class::offset>& value(const C* obj) {
    return *obj->_get_yomm11_ptbl();
  }
};

template<class... X>
struct init_ptr;

template<class... Bases>
struct init_ptr<yomm11_class::base_list<Bases...>> : init_ptr<Bases...> { };

template<>
struct init_ptr<yomm11_class::base_list<>> {
  template<class This> static void init(This* p) {
    p->selector::_yomm11_ptbl = &yomm11_class::of<This>::pc->mmt;
  }
};

template<class Base, class... Bases>
struct init_ptr<Base, Bases...> {
  template<class This> static void init(This* p) {
    p->Base::_yomm11_ptbl = &yomm11_class::of<This>::pc->mmt;
    init_ptr<Bases...>::init(p);
  }
};

template<>
struct init_ptr<> {
  template<class This> static void init(This* p) {
  }
};

template<typename SIG>
struct signature {
  using type = SIG;
};

template<>
struct get_mm_table<false> {
  using class_of_type = std::unordered_map<std::type_index, const std::vector<yomm11_class::offset>*>;
  static class_of_type* class_of;
  template<class C>
  static const std::vector<yomm11_class::offset>& value(const C* obj) {
    YOMM11_TRACE(std::cout << "foreign yomm11_class::of<" << typeid(*obj).name() << "> = " << (*class_of)[std::type_index(typeid(*obj))] << std::endl);
    return *(*class_of)[std::type_index(typeid(*obj))];
  }
};

template<class Class, class Bases>
struct check_bases;

template<class Class, class Base, class... Bases>
struct check_bases<Class, yomm11_class::base_list<Base, Bases...>> {
  static const bool value = std::is_base_of<Base, Class>::value && check_bases<Class, yomm11_class::base_list<Bases...>>::value;
};

template<class Class>
struct check_bases<Class, yomm11_class::base_list<>> {
  static const bool value = true;
};

template<class Class, class... Bases>
struct has_nonvirtual_bases;

template<class... Classes>
struct has_nonvirtual_bases<yomm11_class::base_list<Classes...>> : has_nonvirtual_bases<Classes...>{};

template<class Class, class Base, class... More>
struct has_nonvirtual_bases<Class, Base, More...> {
  static const int value = !is_virtual_base_of<Base, Class>::value
    || has_nonvirtual_bases<Class, More...>::value;
};

template<class Class>
struct has_nonvirtual_bases<Class> {
  static const int value = false;
};

template<class M, typename Override, class Base>
struct wrapper;

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

template<class M>
struct method_impl : specialization_base {
  M pm;
  M* pn; // next

  method_impl(int index, M pm, std::vector<yomm11_class*> type_tuple, M* pn) : pm(pm), pn(pn) {
    this->index = index;
    this->args = type_tuple;
  }
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

template<typename R, typename... P>
struct method_implementation : method_base {
  using return_type = R;
  using method_pointer_type = return_type (*)(typename remove_virtual<P>::type...);
  using method_entry = method_impl<method_pointer_type>;
  using signature = R(typename remove_virtual<P>::type...);
  using virtuals = typename extract_virtuals<P...>::type;

  method_implementation(YOMM11_TRACE(const char* name)) :
    method_base(yomm11_class_vector_of<virtuals>::get() YOMM11_COMMA_TRACE(name)),
    dispatch_table(nullptr) {
  }

  template<class M> specialization_base* add_spec();

  virtual void_function_pointer* allocate_dispatch_table(int size);
  virtual void emit(specialization_base*, int i);
  virtual void emit_next(specialization_base*, specialization_base*);

  method_pointer_type* dispatch_table;
};

template<typename R, typename... P>
template<class M>
specialization_base* method_implementation<R, P...>::add_spec() {
  using method_signature = typename M::body_signature::type;
  using target = typename wrapper<M, method_signature, signature>::type;
  using method_virtuals = typename extract_method_virtuals<R(P...), method_signature>::type;

  using namespace std;
  YOMM11_TRACE(cout << "add " << method_virtuals() << " to " << virtuals() << endl);

  specialization_base* method = new method_entry(methods.size(), target::body, yomm11_class_vector_of<method_virtuals>::get(), &M::next);
  methods.push_back(method);
  invalidate();

  return method;
}

template<typename R, typename... P>
method_base::void_function_pointer* method_implementation<R, P...>::allocate_dispatch_table(int size) {
  using namespace std;
  delete [] dispatch_table;
  dispatch_table = new method_pointer_type[size];
  return reinterpret_cast<void_function_pointer*>(dispatch_table);
}

template<typename R, typename... P>
void method_implementation<R, P...>::emit(specialization_base* method, int i) {
  dispatch_table[i] =
      method == &specialization_base::ambiguous ? throw_ambiguous<signature>::body
      : method == &specialization_base::undefined ? throw_undefined<signature>::body
      : static_cast<const method_entry*>(method)->pm;
  using namespace std;
  YOMM11_TRACE(cout << "installed at " << dispatch_table << " + " << i << endl);
}

template<typename R, typename... P>
void method_implementation<R, P...>::emit_next(specialization_base* method, specialization_base* next) {
  *static_cast<const method_entry*>(method)->pn =
      (next == &specialization_base::ambiguous || next == &specialization_base::undefined) ? nullptr
      : static_cast<const method_entry*>(next)->pm;
}

template<int Dim, typename... P>
struct linear;

template<typename P1, typename... P>
struct linear<0, P1, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1, A... args) {
    return linear<0, P...>::value(slot_iter, step_iter, args...);
  }
};

template<typename P1, typename... P>
struct linear<0, virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1 arg, A... args) {
    return linear<1, P...>::value(
        slot_iter + 1, step_iter + 1,
        detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].ptr, args...);
  }
};

template<typename P1, typename... P>
struct linear<0, const virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1 arg, A... args) {
    return linear<1, P...>::value(
        slot_iter + 1, step_iter + 1,
        detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].ptr, args...);
  }
};

template<int Dim, typename P1, typename... P>
struct linear<Dim, P1, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      method_base::void_function_pointer* ptr,
      A1, A... args) {
    return linear<Dim, P...>::value(slot_iter, step_iter, ptr, args...);
  }
};

template<int Dim, typename P1, typename... P>
struct linear<Dim, virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      method_base::void_function_pointer* ptr,
      A1 arg, A... args) {
    YOMM11_TRACE(std::cout << " -> " << ptr);
    return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
  }
};

template<int Dim, typename P1, typename... P>
struct linear<Dim, const virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      method_base::void_function_pointer* ptr,
      A1 arg, A... args) {
    YOMM11_TRACE(std::cout << " -> " << ptr);
    return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
  }
};

template<int Dim>
struct linear<Dim> {
  static method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      method_base::void_function_pointer* ptr) {
    YOMM11_TRACE(std::cout << " -> " << ptr << std::endl);
    return ptr;
  }
};

#ifdef YOMM11_TRACE

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

template<class Class, class... Bases>
yomm11_class::initializer<Class, yomm11_class::base_list<Bases...>>::initializer() {
  static_assert(
      detail::check_bases<Class, yomm11_class::base_list<Bases...>>::value,
      "Error in YOMM11_CLASS(): not a base in base list");
  yomm11_class& pc = yomm11_class::of<Class>::the();
  pc.abstract = std::is_abstract<Class>::value;
  pc.initialize(detail::yomm11_class_vector_of<Bases...>::get());

  if (!std::is_base_of<selector, Class>::value) {
    if (!detail::get_mm_table<false>::class_of) {
      detail::get_mm_table<false>::class_of = new detail::get_mm_table<false>::class_of_type;
    }
    (*detail::get_mm_table<false>::class_of)[std::type_index(typeid(Class))] = &pc.mmt;
  }
}

template<class Class, class... Bases>
yomm11_class::initializer<Class, yomm11_class::base_list<Bases...>> yomm11_class::initializer<Class, yomm11_class::base_list<Bases...>>::the;

template<template<typename Sig> class Method, typename R, typename... P>
struct method<Method, R(P...)> {

#ifdef __cpp_constexpr
  constexpr
#endif
  method() {}
  R operator ()(typename detail::remove_virtual<P>::type... args) const;
  static R resolve(typename detail::remove_virtual<P>::type... args);

  using return_type = R;
  using method_pointer_type = return_type (*)(typename detail::remove_virtual<P>::type...);
  using method_entry = detail::method_impl<method_pointer_type>;
  using signature = R(typename detail::remove_virtual<P>::type...);
  using virtuals = typename detail::extract_virtuals<P...>::type;

  template<typename Tag>
  struct next_ptr {
    static method_pointer_type next;
  };

  method_pointer_type next_ptr_type() const;

  using implementation = detail::method_implementation<R, P...>;
  static implementation& the();
  static implementation* impl;

  template<class Spec>
  static bool specialize() {
    the().template add_spec<Spec>();
    return true;
  }

  template<class Spec>
  struct specialization {
    static method_pointer_type next;
    // this doesn't work on clang, must do it in YOMM11_SPECIALIZATION
    // virtual void* _yomm11_install() { return &register_spec<Spec>::the; }
  };
};

template<class Method, class Spec>
struct register_spec {
  register_spec() {
    Method::template specialize<Spec>();
  }
  static register_spec the;
};

template<class Method, class Spec>
register_spec<Method, Spec> register_spec<Method, Spec>::the;

template<template<typename Sig> class Method, typename R, typename... P>
template<class Spec>
typename method<Method, R(P...)>::method_pointer_type
method<Method, R(P...)>::specialization<Spec>::next;

template<template<typename Sig> class Method, typename R, typename... P>
typename method<Method, R(P...)>::implementation* method<Method, R(P...)>::impl;

template<template<typename Sig> class Method, typename R, typename... P>
template<typename Tag>
typename method<Method, R(P...)>::method_pointer_type method<Method, R(P...)>::next_ptr<Tag>::next;

template<template<typename Sig> class Method, typename R, typename... P>
typename method<Method, R(P...)>::implementation& method<Method, R(P...)>::the() {
  if (!impl) {
    impl = new implementation(YOMM11_TRACE(_yomm11_name_((multi_method<Method, R(P...)>*) nullptr)));
  }

  return *impl;
}

template<template<typename Sig> class Method, typename R, typename... P>
inline R method<Method, R(P...)>::operator ()(typename detail::remove_virtual<P>::type... args) const {
  YOMM11_TRACE((std::cout << "() mm table = " << impl->dispatch_table << std::flush));
  return reinterpret_cast<method_pointer_type>(*detail::linear<0, P...>::value(impl->slots.begin(), impl->steps.begin(), &args...))(args...);
}

template<template<typename Sig> class Method, typename R, typename... P>
inline R method<Method, R(P...)>::resolve(typename detail::remove_virtual<P>::type... args) {
  YOMM11_TRACE((std::cout << "() mm table = " << impl->dispatch_table << std::flush));
  return reinterpret_cast<method_pointer_type>(*detail::linear<0, P...>::value(impl->slots.begin(), impl->steps.begin(), &args...))(args...);
}

} // detail

template<class B, class D>
struct cast : cast_best<B, D, detail::is_virtual_base_of<B, D>::value> {
};

template<class B>
struct cast<B, B> {
  static B& value(B& obj) { return obj; }
  static const B& value(const B& obj) { return obj; }
};

struct selector {
  selector() : _yomm11_ptbl(0) { }
  std::vector<detail::yomm11_class::offset>* _yomm11_ptbl;
  virtual ~selector() { }
  template<class THIS>
  void _init_yomm11_ptr(THIS*);
  std::vector<detail::yomm11_class::offset>* _get_yomm11_ptbl() const { return _yomm11_ptbl; }
};

template<class THIS>
inline void selector::_init_yomm11_ptr(THIS*) {
  _yomm11_ptbl = &detail::yomm11_class::of<THIS>::the().mmt;
}

template<class Class>
struct virtual_ {
  using type = Class;
};

} // methods
  namespace multi_methods = methods;
} // yorel

#ifdef _MSVC_VER
#pragma warning( pop )
#endif
#endif
