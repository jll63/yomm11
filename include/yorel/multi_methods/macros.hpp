// -*- compile-command: "cd ../../.. && make && make test" -*-

// macros.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef MM_CLASS
#define MM_CLASS(CLASS, ...)                                       \
  using _yomm11_base_list = ::yorel::multi_methods::mm_class::base_list<__VA_ARGS__>; \
  YOREL_MM_TRACE(friend const char* _yomm11_name_(CLASS*) { return #CLASS; })     \
  virtual void _yomm11_init_class_() { &::yorel::multi_methods::mm_class::initializer<CLASS, ::yorel::multi_methods::mm_class::base_list<__VA_ARGS__>>::the; }

#undef MM_EXTERN_CLASS
#define MM_EXTERN_CLASS(CLASS)

#define MM_FOREIGN_CLASS(CLASS, ...)                               \
  static_assert(::yorel::multi_methods::detail::check_bases<CLASS, ::yorel::multi_methods::mm_class::base_list<__VA_ARGS__>>::value, "error in MM_FOREIGN_CLASS(): not a base in base list"); \
  static_assert(std::is_polymorphic<CLASS>::value, "error: class must be polymorphic"); \
  YOREL_MM_TRACE(const char* _yomm11_name_(CLASS*) { return #CLASS; })     \
  namespace { ::yorel::multi_methods::mm_class::initializer<CLASS, ::yorel::multi_methods::mm_class::base_list<__VA_ARGS__>> _yomm11_add_class_ ## CLASS; }

#define MM_INIT()                                                       \
  ::yorel::multi_methods::detail::init_ptr<_yomm11_base_list>::init(this)

#undef MM_CLASS_MULTI
#define MM_CLASS_MULTI(CLASS, BASE, ...)                           \
  MM_CLASS(CLASS, BASE, __VA_ARGS__);                                         \
  std::vector<mm_class::offset>* _get_yomm11_ptbl() const { return BASE::_yomm11_ptbl; }

#define MM_INIT_MULTI(BASE)                     \
  this->BASE::_init_yomm11_ptr(this)

#ifdef __cpp_constexpr
#define YOMM_CONSTEXPR constexpr
#else
#define YOMM_CONSTEXPR const
#endif

#undef MULTI_METHOD
#define MULTI_METHOD(ID, RETURN_TYPE, ...)                          \
  template<typename Sig> struct ID ## _specialization;                  \
  YOREL_MM_TRACE(inline const char* _yomm11_name_(::yorel::multi_methods::multi_method<ID ## _specialization, RETURN_TYPE(__VA_ARGS__)>*) { return #ID; })     \
  YOMM_CONSTEXPR ::yorel::multi_methods::multi_method<ID ## _specialization, RETURN_TYPE(__VA_ARGS__)> ID

#define BEGIN_SPECIALIZATION(ID, RESULT, ...)                       \
  template<>                                                            \
  struct ID ## _specialization<RESULT(__VA_ARGS__)> : std::remove_const<decltype(ID)>::type::specialization< ID ## _specialization<RESULT(__VA_ARGS__)> > { \
  virtual void* _yomm11_install() { return &::yorel::multi_methods::register_spec<decltype(ID), ID ## _specialization>::the; } \
  using body_signature = ::yorel::multi_methods::detail::signature<RESULT(__VA_ARGS__)>; \
    static RESULT body(__VA_ARGS__) {

#define END_SPECIALIZATION } };

#define GET_SPECIALIZATION(ID, RESULT, ...) ID ## _specialization<RESULT(__VA_ARGS__)>::body
