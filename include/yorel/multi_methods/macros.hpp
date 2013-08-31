// -*- compile-command: "cd ../../.. && make && make test" -*-

// macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef MM_CLASS
#define MM_CLASS(CLASS, BASES...)                                       \
  virtual void _mm_init_class_() { &::yorel::multi_methods::mm_class::initializer<CLASS, ::yorel::multi_methods::mm_class::base_list<BASES>>::the; }

#undef MM_EXTERN_CLASS
#define MM_EXTERN_CLASS(CLASS)
  
#define MM_FOREIGN_CLASS(CLASS, BASES...)                               \
  static_assert(::yorel::multi_methods::detail::check_bases<CLASS, ::yorel::multi_methods::mm_class::base_list<BASES>>::value, "error in MM_FOREIGN_CLASS(): not a base in base list"); \
  static_assert(std::is_polymorphic<CLASS>::value, "error: class must be polymorphic"); \
  namespace { ::yorel::multi_methods::mm_class::initializer<CLASS, ::yorel::multi_methods::mm_class::base_list<BASES>> _mm_add_class_ ## CLASS; }

#define MM_INIT() \
  this->_init_mmptr(this)

#undef MM_CLASS_MULTI
#define MM_CLASS_MULTI(CLASS, BASE, BASES...)                     \
  MM_CLASS(CLASS, BASE, BASES);                                         \
  std::vector<mm_class::offset>* _get_mm_ptbl() const { return BASE::_mm_ptbl; }

#define MM_INIT_MULTI(BASE)                                       \
  this->BASE::_init_mmptr(this)

#undef MULTI_METHOD
#define MULTI_METHOD(ID, RETURN_TYPE, ARGS...)                           \
  template<typename Sig> struct ID ## _specialization;                          \
  constexpr ::yorel::multi_methods::multi_method<ID ## _specialization, RETURN_TYPE(ARGS)> ID{};
  
#define BEGIN_SPECIALIZATION(ID, RESULT, ARGS...)                       \
  template<>                                                            \
  struct ID ## _specialization<RESULT(ARGS)> : decltype(ID)::specialization<ID ## _specialization<RESULT(ARGS)>> { \
  virtual void* _mm_install() { return &decltype(ID)::register_spec<ID ## _specialization>::the; } \
  static RESULT body(ARGS) {

#define END_SPECIALIZATION } };

#define SPECIALIZATION(ID, RESULT, ARGS...) ID ## _specialization<RESULT(ARGS)>::body
