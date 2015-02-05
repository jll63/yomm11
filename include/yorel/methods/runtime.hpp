#ifndef YOMM11_RUNTIME_INCLUDED
#define YOMM11_RUNTIME_INCLUDED

// method/runtime.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

namespace yorel {
namespace methods {
namespace detail {

struct hierarchy_initializer {
  hierarchy_initializer(yomm11_class& root);

  void collect_classes();
  void make_masks();
  void assign_slots();
  void execute();

  static void initialize(yomm11_class& root);

  void topological_sort_visit(std::unordered_set<const yomm11_class*>& once, yomm11_class* pc);

  yomm11_class& root;
  std::vector<yomm11_class*> nodes;
};

struct grouping_resolver {
  grouping_resolver(method_base& mm);

  void resolve();
  void resolve(int dim, const bitvec& candidates);
  void find_applicable(int dim, const yomm11_class* pc, std::vector<specialization_base*>& best);
  specialization_base* find_best(const bitvec& candidates);
  specialization_base* find_best(const std::vector<specialization_base*>& methods);
  void make_mask(const std::vector<specialization_base*>& best, bitvec& mask);
  void make_groups();
  void make_table();
  void assign_next();

  struct group {
    bitvec mask;
    std::vector<specialization_base*> methods;
    std::vector<yomm11_class*> classes;
  };

  method_base& mm;
  const int dims;
  std::vector<std::vector<group>> groups;
  method_base::void_function_pointer* dispatch_table;
  int emit_at;
};
}
}
}
#endif
