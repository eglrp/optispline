#ifndef TENSOR_HPP_INCLUDE
#define TENSOR_HPP_INCLUDE

#include <iostream>
#include <fstream>
#include <ctime>
#include <assert.h>
#include <casadi/casadi.hpp>
#include "tensor_exception.hpp"
#include "slice.hpp"
#include "../src/common.h"
#include "../src/Function/NumericIndex.h"
#include "../SharedObject/PrintableObject.h"

template <class T>
std::vector<T> reorder(const std::vector<T>& data, const std::vector<int>& order) {
  std::vector<T> ret(data.size());
  for (int k=0;k<order.size();k++) {
    ret[k] = data[order[k]];
  }
  return ret;
}

template <class T>
std::vector<T> mrange(T stop) {
  return casadi::range(-1, -stop-1, -1);
}

template <class T>
std::vector<T> mrange(T start, T stop) {
  return casadi::range(-start-1, -stop-1, -1);
}

std::vector<int> invert_order(const std::vector<int>& order);

template <class T>
class Tensor : public spline::PrintableObject< Tensor<T> > {
  public:

  template<class S>
  Tensor(const Tensor<S>& a) : data_(vec(T(a.data()))), dims_(a.dims()) {

  }

  static Tensor<T> concat(const std::vector< Tensor<T> >& v, const spline::NumericIndex& axis) {
    tensor_assert(v.size()>0);

    // Establish reference dimensions
    auto dims = v[0].dims();
    int n_dims = v[0].n_dims();

    tensor_assert(axis<n_dims);

    // Choose an ordering to bring the axis dimension to the end
    std::vector<int> order;
    for (int i=0;i<axis;++i) order.push_back(i);
    for (int i=axis+1;i<n_dims;++i) order.push_back(i);
    order.push_back(axis);

    // Dimension checks
    for (auto& t : v) {
      tensor_assert(n_dims==t.n_dims());

      tensor_assert(std::equal(dims.begin(), dims.begin()+axis, t.dims().begin()) &&
            std::equal(dims.begin()+axis+1, dims.end(), t.dims().begin()+axis+1));

    }

    /*
    We will bring each vector into a 'canonical concat' form

        trailing_dim x axis_dim
    */

    // The dimensions
    int trailing_dim = 1;
    for (int i=0;i<dims.size();++i) {
      if (axis!=i) trailing_dim*= dims[i];
    }

    std::vector<T> data;
    for (auto& t : v) {
      int axis_dim = t.dims(axis);
      Tensor<T> e = t.reorder_dims(order);
      data.push_back(reshape(e.data(), trailing_dim, axis_dim));
    }

    T res = horzcat(data);

    std::vector<int> new_dims = dims;
    new_dims[axis] = res.size2();

    std::vector<int> permuted_dims = reorder(new_dims, order);

    return Tensor(res, permuted_dims).reorder_dims(invert_order(order));

  }

  static Tensor<T> repeat(const Tensor<T>&e, const std::vector<int>& factors) {
    tensor_assert(factors.size()>= e.n_dims());

    // Eventual return dimensions
    std::vector<int> dims = factors;
    for (int i=0;i<e.n_dims();++i) {
      dims[i]*= e.dims()[i];
    }

    if (spline::product(factors)==1) {
      return Tensor<T>(e.data(), dims);
    }

    if (spline::product(std::vector<int>(factors.begin(), factors.begin()+e.n_dims()))==1) {
      return Tensor<T>(repmat(e.data(),spline::product(factors),1), dims);
    }

    // e : {n m p q}   factors : {r1 r2 r3 r4 | r5}
    Tensor<T> ones(casadi::DM::ones(spline::product(factors)), factors);

    // r : {n m p q r1 r2 r3 r4 | r5}
    Tensor<T> r = e.outer_product(ones);

    std::vector<int> order_interleave = casadi::range(e.n_dims()+factors.size());

    for (int i=0;i<e.n_dims();++i) {
      order_interleave[i] = 2*i;
      order_interleave[i+e.n_dims()] = 2*i+1;
    }
    // order_leave { 0 2 4 6 1 3 5 7 | 8 }

    r = r.reorder_dims(order_interleave);


    return r.shape(dims);
  }

  static Tensor<T> pack(const std::vector< Tensor<T> >& v, const spline::NumericIndex& axis) {
    tensor_assert(v.size()>0);
    auto dims = v[0].dims();

    std::vector<T> data;

    for (auto& t : v) {
      tensor_assert(dims==t.dims());
      data.push_back(vec(t.data()).T());
    }

    std::vector<int> new_dims = dims;
    new_dims.insert(new_dims.begin(), v.size());
    Tensor<T> ret = Tensor(vertcat(data), new_dims);

    if (axis!=0) tensor_assert_message(false, "Not implemented");
    return ret;

  }

  Tensor(const T& data, const std::vector<int>& dims) : data_(vec(densify(data))), dims_(dims) {
    tensor_assert_message(data.numel()==spline::product(dims), "Data of length " << data.numel()
      << " and dims " << dims << " are incompatible.");
  }

  Tensor(const T& data) : data_(vec(densify(data))), dims_({data.size1(), data.size2()}) {
  }

  Tensor(const Tensor& t) : data_(vec(t.data())), dims_(t.dims()) {
  }

  Tensor(double a) : data_({a}), dims_({}) {
  }

  Tensor() : data_({0}), dims_({}) {
  }

  ~Tensor() { }

#ifndef SWIG
  Tensor& operator=(const Tensor& t) {
    dims_ = t.dims();
    data_ = t.data();
    return *this;
  }
#endif

  bool is_scalar() const { return squeeze().n_dims()==0; }
  T as_scalar() const { tensor_assert(is_scalar()); return data_.nz(0); }

  bool is_vector() const { return squeeze().n_dims()<=1; }

  Tensor as_vector() const {
    Tensor ret = squeeze();
    tensor_assert_message(ret.n_dims()<=1, "Vector can have only one dimension.");
    return ret.shape({numel()});
  }

  T data() const { return data_; }
  T matrix() const {
    tensor_assert(n_dims()<=2);
    if (n_dims()==0) {
      return reshape(data_, std::pair<int, int>{1, 1});
    } else if (n_dims()==1) {
      return reshape(data_, std::pair<int, int>{dims_[0], 1});
    } else if (n_dims()==2) {
      return reshape(data_, std::pair<int, int>{dims_[0], dims_[1]});
    }
    return 0;
  }
  Tensor squeeze() const {
    std::vector<int> dims;
    for (int i=0;i<n_dims();++i) {
      if (dims_[i]!=1) dims.push_back(dims_[i]);
    }
    return Tensor(data_, dims);
  }
  Tensor shape(const std::vector<int>& dims) const {
    return Tensor(data_, dims);
  }
  int numel() const { return data_.numel(); }

  static std::pair<int, int> normalize_dim(const std::vector<int> & dims);

  static std::vector<int> binary_dims(const Tensor& a, const Tensor& b) {
    if (a.is_scalar()) return b.dims();
    if (b.is_scalar()) return a.dims();
    tensor_assert_message(a.dims()==b.dims(), "Mismatch dimentions, got " << a.dims() << " and " << b.dims() );
    return b.dims();
  }

#ifndef SWIG
  static std::vector<int> sub2ind(const std::vector<int>& dims, int sub) {
    std::vector<int> ret(dims.size());
    for (int i=0;i<dims.size();i++) {
      ret[i] = sub % dims[i];
      sub/= dims[i];
    }
    return ret;
  }
  static int ind2sub(const std::vector<int>& dims, const std::vector<int>& ind) {
    tensor_assert(dims.size()==ind.size());
    int ret=0;
    int cumprod = 1;
    for (int i=0;i<dims.size();i++) {
      ret+= ind[i]*cumprod;
      cumprod*= dims[i];
    }
    return ret;
  }
#endif //SWIG

  Tensor get_slice(const AnySlice& i) {
    int n = dims()[n_dims()-2];
    int m = dims()[n_dims()-1];
    tensor_assert(n==1 || m==1);
    if (m==1) {
      return get_slice(i, 0);
    } else {
      return get_slice(0, i);
    }
  }

  Tensor get_slice(const AnySlice& i, const AnySlice& j) {
    int n = dims()[n_dims()-2];
    int m = dims()[n_dims()-1];

    std::vector<int> i_e = i.indices(n);
    std::vector<int> j_e = j.indices(m);

    if (n_dims()==2) {
      return T::reshape(data_, n, m)(i_e, j_e);
    }

    casadi::DM R = casadi::DM::zeros(m, j_e.size());
    for (int ii=0;ii<j_e.size();++ii) {
      R(j_e[ii],ii)=1;
    }

    casadi::DM L = casadi::DM::zeros(i_e.size(), n);
    for (int ii=0;ii<i_e.size();++ii) {
      L(ii,i_e[ii])=1;
    }

    return Tensor<T>(L).trailing_rmtimes(trailing_mtimes(Tensor<T>(R)));


  }



  static T get(const T& data, const std::vector<int> dims, const std::vector<int>& ind) {
    return data.nz(ind2sub(dims, ind));
  }

  static void set(T& data, const std::vector<int> dims, const std::vector<int>& ind,
                  const T& rhs) {
    data.nz(ind2sub(dims, ind)) = rhs;
  }

  int n_dims() const {return dims_.size(); }
  const std::vector<int>& dims() const { return dims_; }
  const int dims(int i) const { return dims_[i]; }

  static Tensor sym(const std::string& name, const std::vector<int> & dims) {
    T v = T::sym(name, normalize_dim(dims));
    return Tensor<T>(v, dims);
  }

  Tensor solve(const Tensor& B) const {
    return T::solve(matrix(), B.matrix(), "lapackqr", casadi::Dict());
  }

  Tensor operator+(const Tensor& rhs) const {
    std::vector< int > new_dims = binary_dims(*this, rhs);
    return Tensor(data_+rhs.data_, new_dims);
  }

  Tensor operator-(const Tensor& rhs) const {
    return operator+(-rhs);
  }

  Tensor operator-() const {
    return Tensor(-data_, dims_);
  }

  Tensor operator*(const Tensor& rhs) const {
    std::vector< int > new_dims = binary_dims(*this, rhs);
    return Tensor(data_*rhs.data_, new_dims);
  }

  Tensor operator<=(const Tensor& rhs) const {
    return Tensor(data_<=rhs.data_, dims_);
  }
  Tensor operator>=(const Tensor& rhs) const {
    return Tensor(data_<=rhs.data_, dims_);
  }
  Tensor operator==(const Tensor& rhs) const {
    return Tensor(data_==rhs.data_, dims_);
  }
  /** \brief Make a slice
  *
  *   -1  indicates a slice
  */
  Tensor operator()(const std::vector<int>& ind) const {
    return index(ind);
  }

  Tensor transform(const Tensor& tr, const spline::NumericIndex& axis) const {
    std::vector<int> ind1(n_dims());
    std::vector<int> ind2(n_dims());
    int cnt = -3;
    for (int i=0; i<n_dims(); i++) {
        if (i == axis) {
            ind1[i] = -2;
            ind2[i] = -1;
        } else {
            ind1[i] = cnt;
            ind2[i] = cnt;
            cnt--;
        }
    }
    return einstein(tr, ind1, {-1, -2}, ind2);
  }

  Tensor index(const std::vector<int>& ind) const {
    if (ind.size()==1 && ind[0]==-1) {
      return shape({numel()});
    }
    // Check that input is a permutation of range(n_dims())
    tensor_assert(ind.size()==n_dims());

    std::vector<int> slice_dims;

    std::vector<int> slice_location;

    for (int i=0;i<n_dims();++i) {
      if (ind[i]==-1) {
        slice_dims.push_back(dims(i));
        slice_location.push_back(i);
      } else {
        tensor_assert(ind[i]>=0);
        tensor_assert(ind[i]<dims(i));
      }
    }

    int c=1;
    std::vector<int> a_e;
    std::vector<int> c_e;
    for (int i=0;i<ind.size();++i) {
      if (ind[i]>=0) {
        a_e.push_back(ind[i]);
      } else {
        a_e.push_back(-c);
        c_e.push_back(-c);
        c+=1;
      }
    }

    return einstein(a_e, c_e);
  }

  /** \brief Generalization of transpose
  */
  Tensor reorder_dims(const spline::NumericIndexVector& order) const {
    // Check that input is a permutaion of range(n_dims())
    tensor_assert(order.size()==n_dims());

    std::vector<bool> occured(n_dims(), false);

    for (int i : order) {
      tensor_assert(i>=0);
      tensor_assert(i<n_dims());
      occured[i] = true;
    }

    for (bool occ : occured) {
      tensor_assert(occ);
    }

    std::vector<int> ind(order.size());
    for (int i=0;i<ind.size();++i) {
      ind[i] = -order[i]-1;
    }

    return einstein(mrange(n_dims()), ind);
  }

  Tensor einstein(const std::vector<int>& a_e, const std::vector<int>& c_e) const {
    return einstein(Tensor(1, {}), a_e, {}, c_e);
  }

  /** \brief Compute any contraction of two tensors, using index/einstein notation

    A.einstein(B, a, b, c) -> C

    Given two tensors, A and B, computes a third tensor C such that:

    C_c = A_a * B_b

    With a, b, c representing einstein indices.
    Instead of the classical index labels i,j,k,... we employ -1,-2,-3,...

  */
  Tensor einstein(const Tensor &B, const std::vector<int>& a,
      const std::vector<int>& b, const std::vector<int>& c) const {

    const Tensor& A = *this;

    // Dimension check
    tensor_assert_message(A.n_dims()==a.size(), "Mismatch dimentions, got " << A.n_dims() << " and " << a.size());
    tensor_assert_message(B.n_dims()==b.size(), "Mismatch dimentions, got " << B.n_dims() << " and " << b.size());

    tensor_assert(c.size()<=a.size()+b.size());

    std::map<int, int> dim_map;

    // Check if shared nodes dimensions match up
    for (int i=0;i<a.size();++i) {
      int ai = a[i];
      if (ai>=0) continue;
      auto al = dim_map.find(ai);
      if (al==dim_map.end()) {
        dim_map[ai] = A.dims(i);
      } else {
        tensor_assert(al->second==A.dims(i));
      }
    }

    for (int i=0;i<b.size();++i) {
      int bi = b[i];
      if (bi>=0) continue;
      auto bl = dim_map.find(bi);
      if (bl==dim_map.end()) {
        dim_map[bi] = B.dims(i);
      } else {
        tensor_assert(bl->second==B.dims(i));
      }
    }

    std::vector<int> new_dims;
    for (int i=0;i<c.size();++i) {
      int ci = c[i];
      tensor_assert(ci<0);
      auto cl = dim_map.find(ci);
      tensor_assert(cl!=dim_map.end());
      new_dims.push_back(dim_map[ci]);
    }

    T data = T::zeros(casadi::product(new_dims), 1);

    if (b==c && a.empty()) return Tensor(B.data()*A.data(), new_dims);
    if (a==c && b.empty()) return Tensor(A.data()*B.data(), new_dims);
    data = T::einstein(A.data(), B.data(), data, A.dims(), B.dims(), new_dims, a, b, c);

    return Tensor(data, new_dims);
  }

  /**
    c_ijkm = a_ij*b_km
  */
  Tensor outer_product(const Tensor &b) const {
    return einstein(b, mrange(n_dims()), mrange(n_dims(), n_dims()+b.n_dims()),
                                         mrange(n_dims()+b.n_dims()));
  }

  Tensor mtimes(const Tensor &rhs) const {
    if (rhs.is_scalar() || is_scalar()) return operator*(rhs);
    if (n_dims()==0 && rhs.n_dims()==2) {
        return einstein(rhs, {}, {-1, -2}, {-1, -2});
    }
    if (n_dims()==2 && rhs.n_dims()==0) {
        return einstein(rhs, {-1, -2}, {}, {-1, -2});
    }
    tensor_assert_message(n_dims()==2 && rhs.n_dims()==2, "mtimes dim mismatch: " << dims() << ", " << rhs.dims() << ".");

    return T::mtimes(T::reshape(data_,dims()[0],dims()[1]), T::reshape(rhs.data_, rhs.dims()[0],rhs.dims()[1]));
    //return einstein(rhs, {-1, -2}, {-2, -3}, {-1, -3});
  }

  Tensor trailing_mtimes(const Tensor &rhs) const {
    tensor_assert(rhs.n_dims()==2 && n_dims()>=2);
    std::vector<int> a_e = mrange(n_dims());
    std::vector<int> b_e = {a_e[a_e.size()-1], -n_dims()-1};
    std::vector<int> c_e = a_e; c_e[c_e.size()-1] = -n_dims()-1;

    return einstein(rhs, a_e, b_e, c_e);
  }

  Tensor trailing_rmtimes(const Tensor &rhs) const {
    tensor_assert(n_dims()==2 && rhs.n_dims()>=2);
    std::vector<int> a_e = mrange(rhs.n_dims());
    std::vector<int> b_e = {-rhs.n_dims()-1,a_e[a_e.size()-2]};
    std::vector<int> c_e = a_e; c_e[c_e.size()-2] = -rhs.n_dims()-1;
    c_e[c_e.size()-1] = -rhs.n_dims();

    return einstein(rhs, b_e, a_e, c_e);
  }

  Tensor inner(const Tensor&b) {
    const Tensor& a = *this;

    //assert(a.dims(0)==b.dims(0));
    int shared_dim = std::min(a.n_dims(), b.n_dims());
    int max_dim    = std::max(a.n_dims(), b.n_dims());
    std::vector<int> common = mrange(shared_dim);

    std::vector<int> c_r = mrange(shared_dim, max_dim);

    std::vector<int> a_r = a.n_dims()>b.n_dims() ? mrange(a.n_dims()) : common;
    std::vector<int> b_r = b.n_dims()>a.n_dims() ? mrange(b.n_dims()) : common;

    return einstein(b, a_r, b_r, c_r);
  }

  /** \brief Perform a matrix product on the first two indices */
  Tensor partial_product(const Tensor & b) {

    const Tensor& a = *this;

    tensor_assert(b.n_dims()>=2);
    tensor_assert(a.n_dims()>=2);

    bool fixed = (a.n_dims()==2);

    if (!fixed) {
      for (int i=2;i<a.n_dims();i++) tensor_assert(a.dims(i)==b.dims(i));
    }

    tensor_assert(b.dims(1)==a.dims(0));

    std::vector<int> a_r;
    if (fixed) {
      a_r = {-1, -b.n_dims()-1};
    } else {
      a_r = mrange(a.n_dims());
      a_r[1] = -b.n_dims()-1;
    }
    std::vector<int> b_r = mrange(b.n_dims());
    b_r[0] = -b.n_dims()-1;
    std::vector<int> c_r = mrange(b.n_dims());

    return einstein(b, a_r, b_r, c_r);
  }

  virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "Tensor(" << data_.type_name() << ", "
        << dims() << "): " << data();
      return ss.str();
  }

  private:
    T data_;
    std::vector<int> dims_;
};

template <>
Tensor<casadi::SX> Tensor<casadi::SX>::solve(const Tensor<casadi::SX>& B) const;

template <class T>
std::pair<int, int> Tensor<T>::normalize_dim(const std::vector<int> & dims) {
    if (dims.size()==0) {
      return {1, 1};
    } else if (dims.size()==2) {
      return {dims[0], dims[1]};
    } else if (dims.size()==1) {
      return {dims[0], 1};
    } else if (dims.size()>2) {
      int prod = 1;
      for (int i=1;i<dims.size();i++) {
        prod*= dims[i];
      }
      return {dims[0], prod};
    } else {
      tensor_assert(false);
    }
    return {0, 0};
}

typedef Tensor<casadi::SX> ST;
typedef Tensor<casadi::DM> DT;
typedef Tensor<casadi::MX> MT;

#endif
