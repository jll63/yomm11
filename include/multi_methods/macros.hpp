// macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef MM_CLASS
#define MM_CLASS(CLASS, BASES...)                                       \
  virtual void _mm_init_class_() { &::multi_methods::mm_class_initializer<CLASS, ::multi_methods::type_list<BASES>>::the; }

#undef MM_EXTERN_CLASS
#define MM_EXTERN_CLASS(CLASS)
  
#define MM_FOREIGN_CLASS(CLASS, BASES...)                               \
  static_assert(::multi_methods::check_bases<CLASS, ::multi_methods::type_list<BASES>>::value, "error in MM_FOREIGN_CLASS(): not a base in base list"); \
  static_assert(std::is_polymorphic<CLASS>::value, "error: class must be polymorphic"); \
  namespace { ::multi_methods::mm_class_initializer<CLASS, ::multi_methods::type_list<BASES>> BOOST_PP_CAT(_mm_add_class, CLASS); }

#define MM_INIT() \
  this->_init_mmptr(this)

#undef MM_CLASS_MULTI
#define MM_CLASS_MULTI(CLASS, BASE, BASES...)                     \
  MM_CLASS(CLASS, BASE, BASES)                                          \
  std::vector<detail::offset>* _get_mm_ptbl() const { return BASE::_mm_ptbl; }

#define MM_INIT_MULTI(BASE)                                       \
  this->BASE::_init_mmptr(this)

#undef MULTI_METHOD
#define MULTI_METHOD(ID, RETURN_TYPE, ARGS...)                           \
  template<typename Sig> struct ID ## _method;                          \
  constexpr ::multi_methods::multi_method<ID ## _method, RETURN_TYPE(ARGS)> ID{};
  
#define BEGIN_SPECIALIZATION(ID, RESULT, ARGS...)                               \
  template<>                                                            \
  struct ID ## _method<RESULT(ARGS)> : ::multi_methods::specialization<decltype(ID), RESULT(ARGS)> { \
  static RESULT body(ARGS) {                                            \
  &::multi_methods::register_spec<decltype(ID), ID ## _method>::the;

#define END_SPECIALIZATION } };
