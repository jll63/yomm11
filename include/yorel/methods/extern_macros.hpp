// extern_macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef YOMM11_CLASS

#define YOMM11_CLASS(CLASS, BASES...)                                   \
  using _yomm11_base_list = ::yorel::methods::detail::type_list<BASES>; \
  const char* _yomm11_class_name_(CLASS*) { return #CLASS; }            \
  virtual void _yomm11_init_class_()

#undef YOMM11_EXTERN_CLASS

#define YOMM11_EXTERN_CLASS(CLASS)                                  \
  const char* _yomm11_class_name_(CLASS*) { return #CLASS; }        \
  extern template class ::yorel::methods::detail::yomm11_class::of<CLASS>

#undef YOMM11_METHOD

#define YOMM11_METHOD(ID, RETURN_TYPE, ARGS...)                          \
  template<typename Sig> struct ID ## _specialization;                  \
  constexpr ::yorel::methods::detail::method<ID ## _specialization, RETURN_TYPE(ARGS)> ID{}; \
  extern template class ::yorel::methods::detail::method_implementation<RETURN_TYPE, ARGS>
