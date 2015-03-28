// -*- compile-command: "cd .. && make && make test" -*-

// multi_method.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <yorel/multi_methods.hpp>
#include <yorel/multi_methods/runtime.hpp>
#include <unordered_set>
#include <iterator>
#include <functional>
#include <cassert>

using namespace std;

namespace yorel {
namespace multi_methods {

namespace detail {
ostream& operator <<(ostream& os, const bitvec& v) {
  for (int i = 0; i < v.size(); i++) {
    os << (v[i] ? 1 : 0);
  }
  return os;
}
}

using namespace detail;

undefined::undefined() :
    runtime_error("multi-method call is undefined for these arguments") {
}

undefined::undefined(const std::string& message) : runtime_error(message) {
}

ambiguous::ambiguous() :
    undefined("multi-method call is ambiguous for these arguments") {
}

using class_set = std::unordered_set<const mm_class*>;

mm_class::mm_class(const type_info& t) : ti(t), abstract(false), index(-1), root(nullptr) {
  //cout << "mm_class() for " << ti.name() << " at " << this << endl;
}

mm_class::~mm_class() {
  for (mm_class* base : bases) {
    base->specs.erase(
        remove_if(base->specs.begin(), base->specs.end(), [=](mm_class* pc) {
            return pc == this;
          }),
        base->specs.end());
  }

  add_to_initialize(root);
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

void mm_class::initialize(const vector<mm_class*>& b) {
  YOREL_MM_TRACE(cout << "initialize class_of<" << ti.name() << ">\n");

  if (root) {
    throw runtime_error("multi_methods: class redefinition");
  }

  bases = b;

  if (bases.empty()) {
    root = this;
  } else {
    // if (any_of(bases.begin(), bases.end(), [&](const mm_class* base) {
    //       return base->root != bases.front()->root;
    //     })) {
    //   throw runtime_error("hierarchy must have a single root");
    // }
    root = bases[0]->root;
  }

  for (mm_class* pb : bases) {
    pb->specs.push_back(this);
  }

  add_to_initialize(root);

  root->for_each_conforming([](mm_class* pc) {
      for (auto& mr : pc->rooted_here) {
        mr.method->invalidate();
      }
    });
}

unordered_set<mm_class*>* mm_class::to_initialize;
void mm_class::add_to_initialize(mm_class* pc) {
  YOREL_MM_TRACE(cout << "add to initialize: " << pc << endl);

  if (!to_initialize) {
    to_initialize = new unordered_set<mm_class*>; 
  }

  to_initialize->insert(pc);
}

void mm_class::remove_from_initialize(mm_class* pc) {
  YOREL_MM_TRACE(cout << "remove from initialize: " << pc->ti.name() << endl);

  if (to_initialize) {
    to_initialize->erase(pc);

    if (to_initialize->empty()) {
      delete to_initialize;
      to_initialize = nullptr;
    }
  }
}

method_base method_base::undefined;
method_base method_base::ambiguous;

hierarchy_initializer::hierarchy_initializer(mm_class& root) : root(root) {
}

void hierarchy_initializer::initialize(mm_class& root) {
  hierarchy_initializer init(root);
  init.execute();
}

void hierarchy_initializer::topological_sort_visit(std::unordered_set<const mm_class*>& once, mm_class* pc) {
  if (once.find(pc) == once.end()) {
    once.insert(pc);

    for (mm_class* base : pc->bases) {
      topological_sort_visit(once, base);
    }

    nodes.push_back(pc);
  }
}

void hierarchy_initializer::execute() {
  YOREL_MM_TRACE(cout << "assigning slots for hierarchy rooted in " << &root << endl);
  collect_classes();
  make_masks();
  assign_slots();

  for (auto pc : nodes) {
    if (pc->is_root()) {
      mm_class::remove_from_initialize(pc);
    }
  }
}

void hierarchy_initializer::collect_classes() {
  std::unordered_set<const mm_class*> once;
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
    YOREL_MM_TRACE(cout << pc->ti.name() << " = " << pc << endl);
  }

  for (auto pc_iter = nodes.rbegin(); pc_iter != nodes.rend(); pc_iter++) {
    for (mm_class* spec : (*pc_iter)->specs)  {
      for (int bit = spec->index; bit < nb; bit++) {
        (*pc_iter)->mask[bit] |= nodes[spec->index]->mask[bit];
      }
    }
  }
}

void hierarchy_initializer::assign_slots() {
  vector<bitvec> slots;

  for (auto pc : nodes) {
    int max_slots = 0;

    for (auto& mm : pc->rooted_here) {
      auto available_slot = find_if(
          slots.begin(), slots.end(),
          [=](const bitvec& mask) {
            return (mask & pc->mask).none();
          });

      if (available_slot == slots.end()) {
        slots.push_back(bitvec(nodes.size()));
        available_slot = slots.end() - 1;
      }

      *available_slot |= pc->mask;

      int slot = available_slot - slots.begin();
      max_slots = max(max_slots, slot + 1);

      YOREL_MM_TRACE(cout << "slot " << slot << " -> " << mm.method->vargs << "(" << mm.arg << ")" << endl);
      mm.method->assign_slot(mm.arg, slot);
    }

    int max_inherited_slots = pc->bases.empty() ? 0
                              : (*max_element(
                                    pc->bases.begin(), pc->bases.end(),
                                    [](const mm_class* b1, const mm_class* b2) { return b1->mmt.size() < b2->mmt.size(); }))->mmt.size();

    YOREL_MM_TRACE(cout << pc << ":max inherited slots: " << max_inherited_slots << ", max direct slots: " << max_slots << endl);

    pc->mmt.resize(max(max_inherited_slots, max_slots));
  }
}

void initialize() {
  while (mm_class::to_initialize) {
    auto pc = *mm_class::to_initialize->begin();
    if (pc->is_root()) {
      hierarchy_initializer::initialize(*pc);
    } else {
      mm_class::remove_from_initialize(pc);
    }
  }

  while (multi_method_base::to_initialize) {
    auto pm = *multi_method_base::to_initialize->begin();
    pm->resolve();
    multi_method_base::remove_from_initialize(pm);
  }
}

method_base::~method_base() {
}

bool method_base::specializes(method_base* other) const {

  if (this == other) {
    return false;
  }

  bool result = false;

  for (size_t dim = 0; dim < args.size(); dim++) {
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

void mm_class::add_multi_method(multi_method_base* pm, int arg) {
  rooted_here.push_back(mmref { pm, arg });
  add_to_initialize(this);
}

void mm_class::remove_multi_method(multi_method_base* pm) {
  rooted_here.erase(
      remove_if(rooted_here.begin(), rooted_here.end(), [=](mmref& ref) { return ref.method == pm;  }),
      rooted_here.end());
  add_to_initialize(root);
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

multi_method_base::multi_method_base(const vector<mm_class*>& v) : vargs(v) {
  int i = 0;
  for (auto pc : vargs) {
	YOREL_MM_TRACE(cout << "add mm rooted in " << pc->ti.name() << " argument " << i << "\n");
	pc->add_multi_method(this, i++);
  }
  slots.resize(v.size());
}

multi_method_base::~multi_method_base() {
  for (auto method_iter = methods.rbegin(); method_iter != methods.rend(); method_iter++) {
    delete *method_iter;
    *method_iter = 0;
  }

  for (mm_class* arg : vargs) {
    arg->remove_multi_method(this);
  }

  remove_from_initialize(this);
}

void multi_method_base::assign_slot(int arg, int slot) {
  slots[arg] = slot;
  invalidate();
}

void multi_method_base::invalidate() {
  YOREL_MM_TRACE(cout << "add " << this << " to init list" << endl);
  add_to_initialize(this);
}

unordered_set<multi_method_base*>* multi_method_base::to_initialize;

void multi_method_base::add_to_initialize(multi_method_base* pm) {
  if (!to_initialize) {
    to_initialize = new unordered_set<multi_method_base*>;
  }

  to_initialize->insert(pm);
}

void multi_method_base::remove_from_initialize(multi_method_base* pm) {
  if (to_initialize) {
    to_initialize->erase(pm);

    if (to_initialize->empty()) {
      delete to_initialize;
      to_initialize = nullptr;
    }
  }
}

void multi_method_base::resolve() {
  grouping_resolver r(*this);
  r.resolve();
}

grouping_resolver::grouping_resolver(multi_method_base& mm) : mm(mm), dims(mm.vargs.size()) {
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
    YOREL_MM_TRACE(cout << "make_groups dim = " << dim << endl);

    unordered_set<const mm_class*> once;
    mm.steps[dim] = step;

    mm.vargs[dim]->for_each_conforming(once, [&](mm_class* pc) {
        group g;
        find_applicable(dim, pc, g.methods);
        g.classes.push_back(pc);
        make_mask(g.methods, g.mask);
        YOREL_MM_TRACE(cout << pc << " has " << g.methods << endl);
        auto lower = lower_bound(
            dim_groups.begin(), dim_groups.end(), g,
            []( const group& g1, const group& g2) { return g1.mask < g2.mask; });

        if (lower == dim_groups.end() || g.mask < lower->mask) {
          YOREL_MM_TRACE(cout << "create new group" << endl);
          dim_groups.insert(lower, g);
        } else {
          YOREL_MM_TRACE(cout << "add " << pc << " to existing group " << lower->methods << endl);
          lower->classes.push_back(pc);
        }
      });

    step *= dim_groups.size();

    YOREL_MM_TRACE(cout << "assign slots" << endl);

    int offset = 0;

    for (auto& group : dim_groups) {
      for (auto pc : group.classes) {
        YOREL_MM_TRACE(cout << pc << ": " << offset << endl);
        pc->mmt[mm.slots[dim]].index = offset;
      }
      ++offset;
    }

    ++dim;
  }

  dispatch_table = mm.allocate_dispatch_table(step);
}

void grouping_resolver::make_table() {
  YOREL_MM_TRACE(cout << "make_table" << endl);

  emit_at = 0;
  resolve(dims - 1, ~bitvec(mm.methods.size()));

  const int first_slot = mm.slots[0];

  YOREL_MM_TRACE(cout << "resolve 1st dimension" << endl);
  bitvec once;

  for (auto& group : groups[0]) {
    for (auto pc : group.classes) {
      if (once.size() <= pc->index) {
        once.resize(pc->index + 1);
      }

      if (!once[pc->index]) {
        once[pc->index] = true;
        YOREL_MM_TRACE(cout << pc << ": " << pc->mmt[first_slot].index << " -> " << dispatch_table + pc->mmt[first_slot].index << endl);
        pc->mmt[first_slot].ptr = dispatch_table + pc->mmt[first_slot].index;
      }
    }
  }
}

void grouping_resolver::resolve(int dim, const bitvec& candidates) {
  using namespace std;
  YOREL_MM_TRACE(cout << "\nresolve dim = " << dim << endl);

  for (auto& group : groups[dim]) {
    if (dim == 0) {
      method_base* best = find_best(candidates & group.mask);
      YOREL_MM_TRACE(cout << "install " << best << " at offset " << emit_at << endl);
      mm.emit(best, emit_at++);
    } else {
      resolve(dim - 1, candidates & group.mask);
    }
  }

  YOREL_MM_TRACE(cout << "exiting dim " << dim << endl);
}

method_base* grouping_resolver::find_best(const vector<method_base*>& candidates) {
  using namespace std;

  vector<method_base*> best;

  for (auto method : candidates) {
    auto best_iter = best.begin();

    while (best_iter != best.end()) {
      if (method->specializes(*best_iter)) {
        YOREL_MM_TRACE(cout << method << " specializes " << *best_iter << ", removed\n");
        best_iter = best.erase(best_iter);
      } else if ((*best_iter)->specializes(method)) {
        YOREL_MM_TRACE(cout << *best_iter << " specializes " << method << ", removed\n");
        best_iter = best.end();
        method = 0;
      } else {
        best_iter++;
      }
    }

    if (method) {
      YOREL_MM_TRACE(cout << method << " kept\n");
      best.push_back(method);
    }
  }

  return best.size() == 0 ? &method_base::undefined
      : best.size() == 1 ? best.front()
      : &method_base::ambiguous;
}

method_base* grouping_resolver::find_best(const bitvec& mask) {
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
    YOREL_MM_TRACE(cout << "calculating next for " << pm << ", candidates:\n");
    YOREL_MM_TRACE(copy(candidates.begin(), candidates.end(), ostream_iterator<method_base*>(cout, "\n")));
    auto best = find_best(candidates);
    YOREL_MM_TRACE(cout << "next is: " << best << endl);
    mm.emit_next(pm, best);
  }
}

void grouping_resolver::make_mask(const vector<method_base*>& methods, bitvec& mask) {
  mask.resize(mm.methods.size());

  for (auto pm : methods) {
    mask[pm->index] = true;
  }
}

std::ostream& operator <<(std::ostream& os, const mm_class* pc) {
  if (pc) {
    if (pc->index != -1) {
      os << "class_" << pc->index;
    } else{
      os << pc->ti.name();
    }
  } else {
    os << "(null)";
  }
  return os;
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

ostream& operator <<(ostream& os, const multi_method_base* mm) {
  return os << "mm" << mm->vargs;
}

}
}
