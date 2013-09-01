// -*- compile-command: "cd ../../.. && make && make test" -*-

#ifndef MULTI_METHODS_NO_MACROS_INCLUDED
#define MULTI_METHODS_NO_MACROS_INCLUDED

// multi_method/no_macros.hpp
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

//#define YOREL_MM_ENABLE_TRACE
#ifdef YOREL_MM_ENABLE_TRACE
#define YOREL_MM_TRACE(e) e
#include <iterator>
#else
#define YOREL_MM_TRACE(e)
#endif

// Copied from Boost.
#ifdef _MSVC_VER
#pragma warning( push )
#pragma warning( disable : 4584 4250)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#pragma GCC system_header
#endif

namespace yorel {
  namespace multi_methods {

    // Forward declarations.
    // All names in namespace are listed below.
    void initialize();
    struct selector;
    template<class Class> struct virtual_;
    struct mm_class;
    template<class B, class D> struct cast_using_static_cast;
    template<class B, class D> struct cast_using_dynamic_cast;
    template<class B, class D> struct cast;
    struct method_base;
    struct multi_method_base;
    template<template<typename Sig> class Method, typename Sig> struct multi_method;
    class undefined;
    class ambiguous;

    // these are for debugging
    std::ostream& operator <<(std::ostream& os, const mm_class* pc);
    std::ostream& operator <<(std::ostream& os, const std::vector<mm_class*>& classes);
    std::ostream& operator <<(std::ostream& os, method_base* method);
    std::ostream& operator <<(std::ostream& os, const std::vector<method_base*>& methods);

    std::ostream& operator <<(std::ostream& os, const multi_method_base* pc);

    // End of forward declarations.

    namespace detail {
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
            return p[i / bpw] & (1 << (i % bpw));
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
          return p[i / bpw] & (1 << (i % bpw));
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
    }
    
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
  
    struct mm_class {
      struct mmref {
        multi_method_base* method;
        int arg;
      };

      union offset {
        int index;
        void (**ptr)();
      };
    
      mm_class(const std::type_info& t);
      ~mm_class();

      const std::string name() const;
      void initialize(const std::vector<mm_class*>& bases);
      void add_multi_method(multi_method_base* pm, int arg);
      void remove_multi_method(multi_method_base* pm);
      void for_each_spec(std::function<void(mm_class*)> pf);
      void for_each_conforming(std::function<void(mm_class*)> pf);
      void for_each_conforming(std::unordered_set<const mm_class*>& visited, std::function<void(mm_class*)> pf);
      bool conforms_to(const mm_class& other) const;
      bool specializes(const mm_class& other) const;
      bool is_root() const;
    
      const std::type_info& ti;
      std::vector<mm_class*> bases;
      std::vector<mm_class*> specs;
      detail::bitvec mask;
      int index;
      mm_class* root;
      std::vector<mm_class::offset> mmt;
      std::vector<mmref> rooted_here; // multi_methods rooted here for one or more args.
      bool abstract;
    
      static std::unique_ptr<std::unordered_set<mm_class*>> to_initialize;
      static void add_to_initialize(mm_class* pc);
      static void remove_from_initialize(mm_class* pc);
  
      template<class Class>
      struct of {
        static mm_class& the() {
          static mm_class instance(typeid(Class));
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

    inline const std::string mm_class::name() const {
      return ti.name();
    }

    inline bool mm_class::is_root() const {
      return this == root;
    }

    struct method_base {
      virtual ~method_base();
    
      int index; // inside multi_method
      std::vector<mm_class*> args;
      bool specializes(method_base* other) const;
      static method_base undefined;
      static method_base ambiguous;
    };

    struct selector {
      selector() : _yomm11_ptbl(0) { }
      std::vector<mm_class::offset>* _yomm11_ptbl;
      virtual ~selector() { }
      template<class THIS>
      void _init_yomm11_ptr(THIS*);
      std::vector<mm_class::offset>* _get_yomm11_ptbl() const { return _yomm11_ptbl; }
    };

    template<class THIS>
    inline void selector::_init_yomm11_ptr(THIS*) {
      _yomm11_ptbl = &mm_class::of<THIS>::the().mmt;
    }

    template<class Class>
    struct virtual_ {
      using type = Class;
    };

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
    
    struct multi_method_base {
      explicit multi_method_base(const std::vector<mm_class*>& v);
      virtual ~multi_method_base();

      using void_function_pointer = void (*)();
    
      void resolve();
      virtual void_function_pointer* allocate_dispatch_table(int size) = 0;
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

#include <yorel/multi_methods/detail.hpp>
    
    template<class B, class D>
    struct cast : detail::cast_best<B, D, detail::is_virtual_base_of<B, D>::value> {
    };

    template<class B>
    struct cast<B, B> {
      static B& value(B& obj) { return obj; }
      static const B& value(const B& obj) { return obj; }
    };
    
    template<class Class, class... Bases>
    mm_class::initializer<Class, mm_class::base_list<Bases...>>::initializer() {
      static_assert(
        detail::check_bases<Class, mm_class::base_list<Bases...>>::value,
        "Error in MM_CLASS(): not a base in base list");
      mm_class& pc = mm_class::of<Class>::the();
      pc.abstract = std::is_abstract<Class>::value;
      pc.initialize(detail::mm_class_vector_of<Bases...>::get());

      if (!std::is_base_of<selector, Class>::value) {
        if (!detail::get_mm_table<false>::class_of) {
          detail::get_mm_table<false>::class_of = new detail::get_mm_table<false>::class_of_type;
        }
        (*detail::get_mm_table<false>::class_of)[std::type_index(typeid(Class))] = &pc.mmt;
      }
    }

    template<class Class, class... Bases>
    mm_class::initializer<Class, mm_class::base_list<Bases...>> mm_class::initializer<Class, mm_class::base_list<Bases...>>::the;
  
    template<template<typename Sig> class Method, typename R, typename... P>
    struct multi_method<Method, R(P...)> {
    
      R operator ()(typename detail::remove_virtual<P>::type... args) const;

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

      using implementation = detail::multi_method_implementation<R, P...>;
      static implementation& the();
      static std::unique_ptr<implementation> impl;

      template<class Spec>
      static bool specialize() {
        the().template add_spec<Spec>();
        return true;
      }

      template<class Spec>
      struct register_spec {
        register_spec() {
          specialize<Spec>();
        }
        static register_spec the;
      };

      template<class Spec>
      struct specialization {
        static method_pointer_type next;
        // this doesn't work on clang, must do it in BEGIN_SPECIALIZATION
        // virtual void* _yomm11_install() { return &register_spec<Spec>::the; }
      };
    };

    template<template<typename Sig> class Method, typename R, typename... P>
    template<class Spec>
    typename multi_method<Method, R(P...)>::method_pointer_type
    multi_method<Method, R(P...)>::specialization<Spec>::next;

    template<template<typename Sig> class Method, typename R, typename... P>
    template<class Spec>
    multi_method<Method, R(P...)>::register_spec<Spec>
    multi_method<Method, R(P...)>::register_spec<Spec>::the;

    template<template<typename Sig> class Method, typename R, typename... P>
    std::unique_ptr<typename multi_method<Method, R(P...)>::implementation> multi_method<Method, R(P...)>::impl;

    template<template<typename Sig> class Method, typename R, typename... P>
    template<typename Tag>
    typename multi_method<Method, R(P...)>::method_pointer_type multi_method<Method, R(P...)>::next_ptr<Tag>::next;

    template<template<typename Sig> class Method, typename R, typename... P>
    typename multi_method<Method, R(P...)>::implementation& multi_method<Method, R(P...)>::the() {
      if (!impl) {
        impl.reset(new implementation);
      }
    
      return *impl;
    }
  
    template<template<typename Sig> class Method, typename R, typename... P>
    inline R multi_method<Method, R(P...)>::operator ()(typename detail::remove_virtual<P>::type... args) const {
      YOREL_MM_TRACE((std::cout << "() mm table = " << impl->dispatch_table << std::flush));
      return reinterpret_cast<method_pointer_type>(*detail::linear<0, P...>::value(impl->slots.begin(), impl->steps.begin(), &args...))(args...);
    }
  }
}

#endif
