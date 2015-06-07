// -*- compile-command: "cd ../../.. && make && make test" -*-

// extern_macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef MM_CLASS

#define MM_CLASS(CLASS, BASES...)                                       \
  using _yomm11_base_list = ::yorel::multi_methods::mm_class::base_list<BASES>; \
  YOREL_MM_TRACE(friend const char* _yomm11_name_(CLASS*) { return #CLASS; })     \
  virtual void _yomm11_init_class_()

#undef MM_EXTERN_CLASS

#define MM_EXTERN_CLASS(CLASS)                                      \
  extern template class ::yorel::multi_methods::mm_class::of<CLASS>

#undef MULTI_METHOD

#define MULTI_METHOD(ID, RETURN_TYPE, ARGS...)                          \
  template<typename Sig> struct ID ## _specialization;                  \
  constexpr ::yorel::multi_methods::multi_method<ID ## _specialization, RETURN_TYPE(ARGS)> ID{}; \
  YOREL_MM_TRACE(inline const char* _yomm11_name_(::yorel::multi_methods::multi_method<ID ## _specialization, RETURN_TYPE(__VA_ARGS__)>*) { return #ID; })     \
  extern template class ::yorel::multi_methods::detail::multi_method_implementation<RETURN_TYPE, ARGS>
