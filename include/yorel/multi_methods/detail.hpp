// -*- compile-command: "cd ../../.. && make && make test" -*-

// multi_method/detail.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

namespace detail {

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
  };
  struct internal_struct_Y : public virtual Derived
  {
    internal_struct_Y();
    internal_struct_Y(const internal_struct_Y&);
    internal_struct_Y& operator=(const internal_struct_Y&);
    ~internal_struct_Y()throw();
  };
#else
  struct internal_struct_X : public Derived, virtual Base
  {
    internal_struct_X();
    internal_struct_X(const internal_struct_X&);
    internal_struct_X& operator=(const internal_struct_X&);
    ~internal_struct_X()throw();
  };
  struct internal_struct_Y : public Derived
  {
    internal_struct_Y();
    internal_struct_Y(const internal_struct_Y&);
    internal_struct_Y& operator=(const internal_struct_Y&);
    ~internal_struct_Y()throw();
  };
#endif
  static const int value = sizeof(internal_struct_X) == sizeof(internal_struct_Y);
};

template<class B, class D, bool is_virtual>
struct cast_best;

template<class B, class D>
struct cast_best<B, D, false> : cast_using_static_cast<B, D> {
};

template<class B, class D>
struct cast_best<B, D, true> : cast_using_dynamic_cast<B, D> {
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

template<class... Class> struct mm_class_vector_of_;

template<class First, class... Rest>
struct mm_class_vector_of_<First, Rest...> {
  static void get(std::vector<mm_class*>& classes) {
    classes.push_back(&mm_class::of<First>::the());
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

template<bool Native>
struct get_mm_table;

template<>
struct get_mm_table<true> {
  template<class C>
  static const std::vector<mm_class::offset>& value(const C* obj) {
    return *obj->_get_yomm11_ptbl();
  }
};

template<class... X>
struct init_ptr;

template<class... Bases>
struct init_ptr<mm_class::base_list<Bases...>> : init_ptr<Bases...> { };

template<>
struct init_ptr<mm_class::base_list<>> {
  template<class This> static void init(This* p) {
    p->selector::_yomm11_ptbl = &mm_class::of<This>::the().mmt;
  }
};

template<class Base, class... Bases>
struct init_ptr<Base, Bases...> {
  template<class This> static void init(This* p) {
    p->Base::_yomm11_ptbl = &mm_class::of<This>::the().mmt;
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
  using class_of_type = std::unordered_map<std::type_index, const std::vector<mm_class::offset>*>;
  static class_of_type* class_of;
  template<class C>
  static const std::vector<mm_class::offset>& value(const C* obj) {
    YOREL_MM_TRACE(std::cout << "foreign mm_class::of<" << typeid(*obj).name() << "> = " << (*class_of)[std::type_index(typeid(*obj))] << std::endl);
    return *(*class_of)[std::type_index(typeid(*obj))];
  }
};

template<class Class, class Bases>
struct check_bases;

template<class Class, class Base, class... Bases>
struct check_bases<Class, mm_class::base_list<Base, Bases...>> {
  static const bool value = std::is_base_of<Base, Class>::value && check_bases<Class, mm_class::base_list<Bases...>>::value;
};

template<class Class>
struct check_bases<Class, mm_class::base_list<>> {
  static const bool value = true;
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
struct method_impl : method_base {
  M pm;
  M* pn; // next

  method_impl(int index, M pm, std::vector<mm_class*> type_tuple, M* pn) : pm(pm), pn(pn) {
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
struct multi_method_implementation : multi_method_base {
  using return_type = R;
  using method_pointer_type = return_type (*)(typename remove_virtual<P>::type...);
  using method_entry = method_impl<method_pointer_type>;
  using signature = R(typename remove_virtual<P>::type...);
  using virtuals = typename extract_virtuals<P...>::type;

  multi_method_implementation(YOREL_MM_TRACE(const char* name)) :
      multi_method_base(mm_class_vector_of<virtuals>::get() YOREL_MM_COMMA_TRACE(name)),
      dispatch_table(nullptr) {
  }

  template<class M> method_base* add_spec();

  virtual void_function_pointer* allocate_dispatch_table(int size);
  virtual void emit(method_base*, int i);
  virtual void emit_next(method_base*, method_base*);

  method_pointer_type* dispatch_table;
};

template<typename R, typename... P>
template<class M>
method_base* multi_method_implementation<R, P...>::add_spec() {
  using method_signature = typename M::body_signature::type;
  using target = typename wrapper<M, method_signature, signature>::type;
  using method_virtuals = typename extract_method_virtuals<R(P...), method_signature>::type;

  using namespace std;
  YOREL_MM_TRACE(cout << "add " << method_virtuals() << " to " << virtuals() << endl);

  method_base* method = new method_entry(methods.size(), target::body, mm_class_vector_of<method_virtuals>::get(), &M::next);
  methods.push_back(method);
  invalidate();

  return method;
}

template<typename R, typename... P>
multi_method_base::void_function_pointer* multi_method_implementation<R, P...>::allocate_dispatch_table(int size) {
  using namespace std;
  delete [] dispatch_table;
  dispatch_table = new method_pointer_type[size];
  return reinterpret_cast<void_function_pointer*>(dispatch_table);
}

template<typename R, typename... P>
void multi_method_implementation<R, P...>::emit(method_base* method, int i) {
  dispatch_table[i] =
      method == &method_base::ambiguous ? throw_ambiguous<signature>::body
      : method == &method_base::undefined ? throw_undefined<signature>::body
      : static_cast<const method_entry*>(method)->pm;
  using namespace std;
}

template<typename R, typename... P>
void multi_method_implementation<R, P...>::emit_next(method_base* method, method_base* next) {
  *static_cast<const method_entry*>(method)->pn =
      (next == &method_base::ambiguous || next == &method_base::undefined) ? nullptr
      : static_cast<const method_entry*>(next)->pm;
}

template<int Dim, typename... P>
struct linear;

template<typename P1, typename... P>
struct linear<0, P1, P...> {
  template<typename A1, typename... A>
  static multi_method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      A1, A... args) {
    return linear<0, P...>::value(slot_iter, step_iter, args...);
  }
};

template<typename P1, typename... P>
struct linear<0, virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static multi_method_base::void_function_pointer* value(
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
  static multi_method_base::void_function_pointer* value(
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
  static multi_method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      multi_method_base::void_function_pointer* ptr,
      A1, A... args) {
    return linear<Dim, P...>::value(slot_iter, step_iter, ptr, args...);
  }
};

template<int Dim, typename P1, typename... P>
struct linear<Dim, virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static multi_method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      multi_method_base::void_function_pointer* ptr,
      A1 arg, A... args) {
    YOREL_MM_TRACE(std::cout << " -> " << ptr);
    return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
  }
};

template<int Dim, typename P1, typename... P>
struct linear<Dim, const virtual_<P1>&, P...> {
  template<typename A1, typename... A>
  static multi_method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      multi_method_base::void_function_pointer* ptr,
      A1 arg, A... args) {
    YOREL_MM_TRACE(std::cout << " -> " << ptr);
    return linear<Dim + 1, P...>::value(
        slot_iter + 1, step_iter + 1,
        ptr + detail::get_mm_table<std::is_base_of<selector, P1>::value>::value(arg)[*slot_iter].index * *step_iter,
        args...);
  }
};

template<int Dim>
struct linear<Dim> {
  static multi_method_base::void_function_pointer* value(
      std::vector<int>::const_iterator slot_iter,
      std::vector<int>::const_iterator step_iter,
      multi_method_base::void_function_pointer* ptr) {
    YOREL_MM_TRACE(std::cout << " -> " << ptr << std::endl);
    return ptr;
  }
};

#ifdef YOREL_MM_TRACE

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
}
