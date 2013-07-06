#ifndef JOIN_MANIP_HPP
#define JOIN_MANIP_HPP

#include <ostream>
#include <type_traits>

template<typename Expr, typename... Exprs>
struct join_manip {
  join_manip(const std::string& sep, Expr first, Exprs... others) : sep(sep), first(first), others(sep, others...) { }
  const std::string& sep;
  typename std::conditional<
    std::is_trivial<Expr>::value,
    Expr,
    const Expr&>::type first;
  join_manip<Exprs...> others;
};

template<typename Expr>
struct join_manip<Expr> {
  join_manip(const std::string& sep, Expr only) : sep(sep), only(only) { }
  const std::string& sep;
  typename std::conditional<
    std::is_trivial<Expr>::value,
    Expr,
    const Expr&>::type only;
};

template<typename... Exprs>
join_manip<Exprs...> join(const std::string& sep, Exprs... args) {
  return join_manip<Exprs...>(sep, args...);
}

template<typename Expr, typename... Exprs>
std::ostream& operator <<(std::ostream& os, const join_manip<Expr, Exprs...>& m) {
  return os << m.first << m.sep << m.others;
}

template<typename Expr>
std::ostream& operator <<(std::ostream& os, const join_manip<Expr>& m) {
  return os << m.only;
}

#endif
