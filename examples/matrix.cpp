#include <yorel/multi_methods.hpp>
#include <iostream>
#include <iomanip>
#include <tuple>

using namespace std;
using yorel::multi_methods::virtual_;
using yorel::multi_methods::selector;

namespace matrix_lib {

class matrix;

using any_matrix = shared_ptr<matrix>;

class matrix : public yorel::multi_methods::selector {
public:
  MM_CLASS(matrix);

  matrix(int rows, int cols) : nr(rows), nc(cols) {
    MM_INIT();
  }

  int rows() const { return nr; }
  int cols() const { return nc; }

  virtual double get(int r, int c) const = 0;
  virtual const vector<double>& elements(vector<double>&) const = 0;

private:
  const int nr, nc;
};

MULTI_METHOD(add, any_matrix, const virtual_<matrix>& m1, const virtual_<matrix>& m2);

any_matrix operator +(const any_matrix& m1, const any_matrix& m2) {
  return add(*m1, *m2);
}

MULTI_METHOD(write, void, ostream& os, const virtual_<matrix>& m);

ostream& operator <<(ostream& os, const any_matrix& m) {
  write(os, *m);
  return os;
}

BEGIN_SPECIALIZATION(write, void, ostream& os, const matrix& m) {
  for (int r = 0; r < m.rows(); r++) {
    const char* sep = "";
    for (int c = 0; c < m.cols(); c++) {
      os << sep << m.get(r, c);
      sep = " ";
    }
    os << "\n";
  }
} END_SPECIALIZATION;

class ordinary : public matrix {
public:
  MM_CLASS(ordinary, matrix);
  ordinary(int rows, int cols);
  template<class Iter>
  ordinary(int rows, int cols, Iter init);
  virtual double get(int r, int c) const { return data[r * cols() + c]; }
  const vector<double>& elements(vector<double>&) const { return data; }
private:
  vector<double> data;
};

ordinary::ordinary(int rows, int cols) : matrix(rows, cols), data(rows * cols) {
  MM_INIT();
}

template<class Iter>
ordinary::ordinary(int rows, int cols, Iter init) : matrix(rows, cols), data(init, init + rows * cols) {
  MM_INIT();
}

BEGIN_SPECIALIZATION(add, any_matrix, const matrix& m1, const matrix& m2) {
  vector<double> result(m1.rows() * m1.cols());
  vector<double> v1, v2;
  const vector<double>& elt1 = m1.elements(v1);
  const vector<double>& elt2 = m2.elements(v2);
  transform(elt1.begin(), elt1.end(), elt2.begin(), result.begin(), plus<double>());
  return make_shared<ordinary>(m1.rows(), m1.cols(), result.begin());
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(write, void, ostream& os, const ordinary& m) {
  os << "ordinary\n";
  next(os, m);
} END_SPECIALIZATION;

class diagonal : public matrix {
public:
  MM_CLASS(diagonal, matrix);
  template<class Iter>
  diagonal(int rows, Iter init);
  virtual double get(int r, int c) const { return r == c ? data[r] : 0; }
  const vector<double>& elements(vector<double>&) const;
  const vector<double>& get_diagonal() const { return data; }
private:
  vector<double> data;
};

template<class Iter>
diagonal::diagonal(int rows, Iter init) : matrix(rows, rows), data(init, init + rows) {
  MM_INIT();
}

const vector<double>& diagonal::elements(vector<double>& v) const {
  const int n = data.size();
  v.resize(n * n);
  int offset{0};
  for (double elt : data) {
    v[offset] = elt;
    offset += n + 1;
  }

  return v;
}

BEGIN_SPECIALIZATION(add, any_matrix, const diagonal& m1, const diagonal& m2) {
  vector<double> result(m1.get_diagonal().size());
  transform(m1.get_diagonal().begin(), m1.get_diagonal().end(),
    m2.get_diagonal().begin(), result.begin(), plus<double>());
  return make_shared<diagonal>(result.size(), result.begin());
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(write, void, ostream& os, const diagonal& m) {
  os << "diagonal\n";
  next(os, m);
} END_SPECIALIZATION;

class tridiagonal : public matrix {
public:
  MM_CLASS(tridiagonal, matrix);
  template<class Iter1, class Iter2, class Iter3>
  tridiagonal(int rows, Iter1 middle, Iter2 upper, Iter3 lower);
  virtual double get(int r, int c) const;
  const vector<double>& elements(vector<double>&) const;
  const vector<double>& middle() const { return d1; }
  const vector<double>& upper() const { return d2; }
  const vector<double>& lower() const { return d3; }
private:
  vector<double> d1, d2, d3;
};

template<class Iter1, class Iter2, class Iter3>
tridiagonal::tridiagonal(int rows, Iter1 middle, Iter2 upper, Iter3 lower) : matrix(rows, rows),
  d1(middle, middle + rows), d2(upper, upper + (rows - 1)), d3(lower, lower + (rows - 1)) {
  MM_INIT();
}

double tridiagonal::get(int r, int c) const {
  return c == r ? middle()[r] : c == r + 1 ? upper()[r] : c == r - 1 ? lower()[c] : 0;
}

const vector<double>& tridiagonal::elements(vector<double>& v) const {
  const int n = middle().size();
  v.resize(n * n);

  for (auto diag : { make_tuple(middle(), 0), make_tuple(upper(), 1), make_tuple(lower(), n) }) {
    int offset{std::get<1>(diag)};
    for (double elt : std::get<0>(diag)) {
      v[offset] = elt;
      offset += n + 1;
    }
  }

  return v;
}

BEGIN_SPECIALIZATION(add, any_matrix, const tridiagonal& m1, const tridiagonal& m2) {
  int n = m1.middle().size();
  vector<double> middle(n), upper(n - 1), lower(n - 1);
  transform(m1.middle().begin(), m1.middle().end(), m2.middle().begin(), middle.begin(), plus<double>());
  transform(m1.upper().begin(), m1.upper().end(), m2.upper().begin(), upper.begin(), plus<double>());
  transform(m1.lower().begin(), m1.lower().end(), m2.lower().begin(), lower.begin(), plus<double>());
  return make_shared<tridiagonal>(middle.size(), middle.begin(), upper.begin(), lower.begin());
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(write, void, ostream& os, const tridiagonal& m) {
  os << "tridiagonal\n";
  next(os, m);
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(add, any_matrix, const tridiagonal& m1, const diagonal& m2) {
  double v[] = { 1 };
  int n = m1.middle().size();
  vector<double> middle(n);
  transform(m1.middle().begin(), m1.middle().end(), m2.get_diagonal().begin(), middle.begin(), plus<double>());
  return make_shared<tridiagonal>(middle.size(), middle.begin(), m1.upper().begin(), m1.lower().begin());
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(add, any_matrix, const diagonal& m1, const tridiagonal& m2) {
  return GET_SPECIALIZATION(add, any_matrix, const tridiagonal&, const diagonal&)(m2, m1);
} END_SPECIALIZATION;

// ambiguity example

class sparse : public matrix {
public:
  MM_CLASS(sparse, matrix);
  sparse(int rows, int cols);
  static any_matrix make(int rows, int cols);
  virtual double get(int r, int c) const { return 0; }
  const vector<double>& elements(vector<double>& data) const { return data; }
private:
  // ...
};

sparse::sparse(int rows, int cols) : matrix(rows, cols) {
  MM_INIT();
}

any_matrix sparse::make(int rows, int cols) {
  return make_shared<sparse>(rows, cols);
}

BEGIN_SPECIALIZATION(add, any_matrix, const sparse& m1, const matrix& m2) {
  return make_shared<ordinary>(m1.rows(), m1.cols());
} END_SPECIALIZATION;

BEGIN_SPECIALIZATION(add, any_matrix, const matrix& m1, const diagonal& m2) {
  return make_shared<ordinary>(m1.rows(), m1.cols());
} END_SPECIALIZATION;
  
 BEGIN_SPECIALIZATION(add, any_matrix, const sparse& m1, const diagonal& m2) {
  return make_shared<ordinary>(m1.rows(), m1.cols());
 } END_SPECIALIZATION;

}

using namespace matrix_lib;

int main() {
  yorel::multi_methods::initialize();

  cout << setw(8) << fixed << right << setprecision(0);

  double content[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  {
    any_matrix m1 = make_shared<ordinary>(3, 3, content);
    any_matrix m2 = make_shared<diagonal>(3, content + 2);
    any_matrix sum = add(*m1, *m2);
    cout << sum << "\n";
  }

  {
    any_matrix m1 = make_shared<diagonal>(3, content);
    any_matrix m2 = make_shared<diagonal>(3, content + 3);
    any_matrix sum = add(*m1, *m2);
    cout << sum << "\n";
  }

  any_matrix m1 = make_shared<diagonal>(4, content);
  any_matrix m2 = make_shared<tridiagonal>(4, content, content + 1, content + 2);
  cout << m1 + m2 << "\n";

  m1 = make_shared<sparse>(3, 3) + make_shared<diagonal>(3, content);
  
  return 0;
}
