// -*- compile-command: "cd ../tests && make" -*-

// multimethod.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <multimethods.hpp>
#include <unordered_set>
#include <functional>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using boost::dynamic_bitset;
using boost::adaptors::reverse;

namespace multimethods {

  mm_class::mm_class(const type_info& t) : ti(t), abstract(false) {
    //cout << "mm_class() for " << ti.name() << " at " << this << endl;
  }

  mm_class::~mm_class() {
    for (mm_class* base : bases) {
      base->specs.erase(
        remove_if(base->specs.begin(), base->specs.end(), [=](mm_class* pc) { return pc == this; }),
        base->specs.end());
    }
    
    to_initialize().insert(root);
  }
  
  void mm_class::for_each_spec(function<void(mm_class*)> pf) {
    for_each(specs.begin(), specs.end(),
             [=](mm_class* p) { p->for_each_conforming(pf); });
  }

  void mm_class::for_each_conforming(unordered_set<const mm_class*>& visited, function<void(mm_class*)> pf) {
    if (visited.find(this) == visited.end()) {
      pf(this);
      visited.insert(this);
      for_each(
        specs.begin(), specs.end(),
        [=](mm_class* p) { p->for_each_conforming(pf); });
    }
  }

  void mm_class::for_each_conforming(function<void(mm_class*)> pf) {
    pf(this);
    for_each(specs.begin(), specs.end(),
             [=](mm_class* p) { p->for_each_conforming(pf); });
  }

  bool mm_class::conforms_to(const mm_class& other) const {
    return index >= other.index && other.mask[index];
  }

  bool mm_class::specializes(const mm_class& other) const {
    return index > other.index && other.mask[index];
  }
  
  void mm_class::initialize(vector<mm_class*>&& b) {
    MM_TRACE(cout << "initialize class_of<" << ti.name() << ">\n");

    if (root) {
      throw runtime_error("multimethods: class redefinition");
    }

    bases.swap(b);

    if (bases.empty()) {
      root = this;
    } else {
      if (any_of(bases.begin(), bases.end(), [&](const mm_class* base) {
            return base->root != bases.front()->root;
          })) {
        throw runtime_error("hierarchy must have a single root");
      }
      root = bases[0]->root;
    }
      
    for (mm_class* pb : bases) {
      pb->specs.push_back(this);
    }

    to_initialize().insert(root);

    root->for_each_conforming([](mm_class* pc) {
        for (auto& mr : pc->rooted_here) {
          mr.method->invalidate();
        }
      });
  }

  unordered_set<mm_class*>& mm_class::to_initialize() {
    static unordered_set<mm_class*> set;
    return set;
  }
  
  method_base method_base::undefined;
  method_base method_base::ambiguous;
  
  hierarchy_initializer::hierarchy_initializer(mm_class& root) : root(root) {
  }

  void hierarchy_initializer::topological_sort_visit(class_set& once, mm_class* pc) {
    if (once.find(pc) == once.end()) {
      once.insert(pc);

      for (mm_class* base : pc->bases) {
        topological_sort_visit(once, base);
      }

      nodes.push_back({ pc });
    }
  }

  void hierarchy_initializer::initialize(mm_class& root) {
    hierarchy_initializer init(root);
    init.execute();
  }
  
  void hierarchy_initializer::execute() {
    MM_TRACE(cout << "assigning slots for hierarchy rooted in " << &root << endl);
    collect_classes();
    make_masks();
    assign_slots();
  }

  void hierarchy_initializer::collect_classes() {
    class_set once;
    root.for_each_conforming([&](mm_class* pc) {
        topological_sort_visit(once, pc);
      });
  }

  void hierarchy_initializer::make_masks() {
    int mark = 0;
    const int nb = nodes.size();
    
    for (auto pc : nodes) {
      pc->mask.resize(nb);
      pc->index = mark++;
      pc->mask[pc->index] = true;
      MM_TRACE(cout << pc->ti.name() << " = " << pc << endl);
    }
    
    for (auto pc : reverse(nodes)) {
      for (mm_class* spec : pc->specs)  {
        for (int bit = spec->index; bit < nb; bit++) {
          pc->mask[bit] |= nodes[spec->index]->mask[bit];
        }
      }
    }
  }

  void hierarchy_initializer::assign_slots() {
    vector<dynamic_bitset<>> slots;

    for (auto pc : nodes) {
      int max_slots = 0;
      
      for (auto& mm : pc->rooted_here) {
        auto available_slot = find_if(
          slots.begin(), slots.end(),
          [=](const dynamic_bitset<>& mask) {
            return (mask & pc->mask).none();
          });

        if (available_slot == slots.end()) {
          slots.push_back(dynamic_bitset<>(nodes.size()));
          available_slot = slots.end() - 1;
        }

        *available_slot |= pc->mask;

        int slot = available_slot - slots.begin();
        max_slots = max(max_slots, slot + 1);

        MM_TRACE(cout << "slot " << slot << " -> " << mm.method->vargs << "(" << mm.arg << ")" << endl);
        mm.method->assign_slot(mm.arg, slot);
      }

      int max_inherited_slots = pc->bases.empty() ? 0
        : (*max_element(
             pc->bases.begin(), pc->bases.end(),
             [](const mm_class* b1, const mm_class* b2) { return b1->mmt.size() < b2->mmt.size(); }))->mmt.size();
      
      MM_TRACE(cout << pc << ":max inherited slots: " << max_inherited_slots << ", max direct slots: " << max_slots << endl);

      pc->mmt.resize(max(max_inherited_slots, max_slots));
    }
  }

  void initialize() {
    while (mm_class::to_initialize().size()) {
      auto pc = *mm_class::to_initialize().begin();
      hierarchy_initializer::initialize(*pc);
      mm_class::to_initialize().erase(pc);
    }
    
    while (multimethod_base::to_initialize().size()) {
      auto pm = *multimethod_base::to_initialize().begin();
      pm->resolve();
      multimethod_base::to_initialize().erase(pm);
    }
  }

  method_base::~method_base() {
  }

  bool method_base::specializes(method_base* other) const {

    if (this == other) {
      return false;
    }
    
    bool result = false;
    
    for (int dim = 0; dim < args.size(); dim++) {
      if (args[dim] != other->args[dim]) {
        if (args[dim]->specializes(*other->args[dim])) {
          result = true;
        } else {
          return false;
        }
      }
    }

    return result;
  }

  void mm_class::add_multimethod(multimethod_base* pm, int arg) {
    rooted_here.push_back(mmref { pm, arg });
    to_initialize().insert(this);
  }

  void mm_class::remove_multimethod(multimethod_base* pm) {
    rooted_here.erase(
      remove_if(rooted_here.begin(), rooted_here.end(), [=](mmref& ref) { return ref.method == pm;  }),
      rooted_here.end());
    to_initialize().insert(this);
  }
  
  get_mm_table<false>::class_of_type* get_mm_table<false>::class_of;
  
  ostream& operator <<(ostream& os, const vector<mm_class*>& classes) {
    using namespace std;
    const char* sep = "(";

    for (mm_class* pc : classes) {
      os << sep;
      sep = ", ";
      os << pc;
    }

    return os << ")";
  }
  
  multimethod_base::multimethod_base(const vector<mm_class*>& v) : vargs(v) {
    int i = 0;
    for_each(vargs.begin(), vargs.end(),
             [&](mm_class* pc) {
               MM_TRACE(cout << "add mm rooted in " << pc->ti.name() << " argument " << i << "\n");
               pc->add_multimethod(this, i++);
             });
    slots.resize(v.size());
  }
  
  multimethod_base::~multimethod_base() {
    for (method_base* method : reverse(methods)) {
      delete method;
      method = 0;
    }

    for (mm_class* arg : vargs) {
      arg->remove_multimethod(this);
    }

    to_initialize().erase(this);
  }
  
  void multimethod_base::assign_slot(int arg, int slot) {
    slots[arg] = slot;
    invalidate();
  }

  void multimethod_base::invalidate() {
    MM_TRACE(cout << "add " << this << " to init list" << endl);
    to_initialize().insert(this);
  }

  unordered_set<multimethod_base*>& multimethod_base::to_initialize() {
    static unordered_set<multimethod_base*> set;
    return set;
  }

  grouping_resolver::grouping_resolver(multimethod_base& mm) : mm(mm), dims(mm.vargs.size()) {
  }
  
  void grouping_resolver::resolve() {
    make_groups();
    make_table();
    assign_next();
  }

  void grouping_resolver::make_groups() {
    groups.resize(dims);

    int dim = 0;
    mm.steps.resize(dims);
    int step = 1;
    
    for (auto& dim_groups : groups) {
      MM_TRACE(cout << "make_groups dim = " << dim << endl);

      unordered_set<const mm_class*> once;
      mm.steps[dim] = step;
      
      mm.vargs[dim]->for_each_conforming(once, [&](mm_class* pc) {
          group g;
          find_applicable(dim, pc, g.methods);
          g.classes.push_back(pc);
          make_mask(g.methods, g.mask);
          MM_TRACE(cout << pc << " has " << g.methods << endl);
          auto lower = lower_bound(
            dim_groups.begin(), dim_groups.end(), g,
            []( const group& g1, const group& g2) { return g1.mask < g2.mask; });
          
          if (lower == dim_groups.end() || g.mask < lower->mask) {
            MM_TRACE(cout << "create new group" << endl);
            dim_groups.insert(lower, g);
          } else {
            MM_TRACE(cout << "add " << pc << " to existing group " << lower->methods << endl);
            lower->classes.push_back(pc);
          }
        });
      
      step *= dim_groups.size();

      MM_TRACE(cout << "assign slots" << endl);
      
      int offset = 0;
      
      for (auto& group : dim_groups) {
        for (auto pc : group.classes) {
          pc->mmt[mm.slots[dim]] = offset;
        }
        ++offset;
      }
        
      ++dim;
    }

    mm.allocate_dispatch_table(step);
  }

  void grouping_resolver::make_table() {
    emit_at = 0;
    resolve(dims - 1, ~dynamic_bitset<>(mm.methods.size()));
  }
  
  void grouping_resolver::resolve(int dim, const dynamic_bitset<>& candidates) {
    using namespace std;
    MM_TRACE(cout << "\nresolve dim = " << dim << endl);
           
    for (auto& group : groups[dim]) {
      if (dim == 0) {
        method_base* best = find_best(candidates & group.mask);
        MM_TRACE(cout << "install " << best << " at offset " << emit_at << endl); 
        mm.emit(best, emit_at++);
      } else {
        resolve(dim - 1, candidates & group.mask);
      }
    }
    
    MM_TRACE(cout << "exiting dim " << dim << endl);
  }

  method_base* grouping_resolver::find_best(const vector<method_base*>& candidates) {
    using namespace std;

    vector<method_base*> best;

    for (auto method : candidates) {
      auto best_iter = best.begin();

      while (best_iter != best.end()) {
        if (method->specializes(*best_iter)) {
          MM_TRACE(cout << method << " specializes " << *best_iter << ", removed\n");
          best.erase(best_iter);
        } else if ((*best_iter)->specializes(method)) {
          MM_TRACE(cout << *best_iter << " specializes " << method << ", removed\n");
          best_iter = best.end();
          method = 0;
        } else {
          best_iter++;
        }
      }

      if (method) {
        MM_TRACE(cout << method << " kept\n");
        best.push_back(method);
      }
    }
      
    return best.size() == 0 ? &method_base::undefined
      : best.size() == 1 ? best.front()
      : &method_base::ambiguous;
  }

  method_base* grouping_resolver::find_best(const dynamic_bitset<>& mask) {
    vector<method_base*> candidates;
    copy_if(mm.methods.begin(), mm.methods.end(), back_inserter(candidates),
            [&](method_base* method) { return mask[method->index]; });
    return find_best(candidates);
  }
  
  void grouping_resolver::find_applicable(int dim, const mm_class* pc, vector<method_base*>& methods) {
    copy_if(
      mm.methods.begin(), mm.methods.end(),
      back_inserter(methods),
      [=](method_base* pm) { return pc->conforms_to(*pm->args[dim]); });
  }

  void grouping_resolver::assign_next() {
    for (method_base* pm : mm.methods) {
      vector<method_base*> candidates;
      copy_if(
        mm.methods.begin(), mm.methods.end(), back_inserter(candidates),
        [&](method_base* other) {
          return pm != other && pm->specializes(other);
        });
      MM_TRACE(cout << "calculating next for " << pm << ", candidates:\n");
      MM_TRACE(copy(candidates.begin(), candidates.end(), ostream_iterator<method_base*>(cout, "\n")));
      auto best = find_best(candidates);
      MM_TRACE(cout << "next is: " << best << endl);
      mm.emit_next(pm, best);
    }
  }

  void grouping_resolver::make_mask(const vector<method_base*>& methods, dynamic_bitset<>& mask) {
    mask.resize(mm.methods.size());

    for (auto pm : methods) {
      mask.set(pm->index);
    }
  }
    
  ostream& operator <<(ostream& os, method_base* method) {
    using namespace std;

    if (method == &method_base::undefined) {
      return os << "undefined";
    }

    if (method == &method_base::ambiguous) {
      return os << "ambiguous";
    }

    os << "method_";
    os << method->index;
    const char* sep = "(";
  
    for (mm_class* pc : method->args) {
      os << sep;
      sep = ", ";
      os << pc->index;
    }

    return os << ")";
  }
  
  ostream& operator <<(ostream& os, const vector<method_base*>& methods) {
    using namespace std;
    const char* sep = "";
  
    for (method_base* pm : methods) {
      os << sep;
      sep = " ";
      os << pm;
    }

    return os;
  }

  ostream& operator <<(ostream& os, const multimethod_base* mm) {
    return os << "mm" << mm->vargs;
  }

}
