// -*- compile-command: "cd ../../.. && make && make test" -*-

#ifndef MULTI_METHODS_RUNTIME_INCLUDED
#define MULTI_METHODS_RUNTIME_INCLUDED

// multi_method/runtime.hpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

namespace yorel {
  namespace multi_methods {
    namespace detail {

      struct hierarchy_initializer {
        hierarchy_initializer(mm_class& root);

        void collect_classes();
        void make_masks();
        void assign_slots();
        void execute();

        static void initialize(mm_class& root);

        void topological_sort_visit(std::unordered_set<const mm_class*>& once, mm_class* pc);

        mm_class& root;
        std::vector<mm_class*> nodes;
      };

      struct grouping_resolver {
        grouping_resolver(multi_method_base& mm);

        void resolve();
        void resolve(int dim, const bitvec& candidates);
        void find_applicable(int dim, const mm_class* pc, std::vector<method_base*>& best);
        method_base* find_best(const bitvec& candidates);
        method_base* find_best(const std::vector<method_base*>& methods);
        void make_mask(const std::vector<method_base*>& best, bitvec& mask);
        void make_groups();
        void make_table();
        void assign_next();

        struct group {
          bitvec mask;
          std::vector<method_base*> methods;
          std::vector<mm_class*> classes;
        };

        multi_method_base& mm;
        const int dims;
        std::vector<std::vector<group>> groups;
        multi_method_base::void_function_pointer* dispatch_table;
        int emit_at;
      };
    }
  }
}
#endif
