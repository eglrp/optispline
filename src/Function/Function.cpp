#include <vector>
#include "Function.h"
#include "../Basis/utils/EvaluationGrid.h"
#include "../Basis/MonomialBasis.h"
#include "../common.h"

namespace spline {
    Function::Function(const TensorBasis& basis, const Coefficient& coeff) {
        init(basis, coeff);
    }

    Function::Function(const Basis& basis, const Coefficient& coeff) {
        init(TensorBasis(basis), coeff);
    }

    Function::Function(const AnyTensor& c) {
        Basis basis = MonomialBasis(0);
        std::vector< int > new_dims = c.dims();
        new_dims.insert(new_dims.begin(), 1);
        init(basis, c.shape(new_dims));
    }

    void Function::init(const TensorBasis& basis, const Coefficient& coeff) {
        std::vector< int > dim_basis = basis.dimension();
        std::vector< int > dim_coef = coeff.data().dims();

        int total_size_coef = dim_coef.size();
        spline_assert_message(dim_basis.size() <= total_size_coef,
          "Dimensions of basis " << dim_basis << " and coefficient " <<
          dim_coef << " can not be connected.");
        spline_assert_message(dim_basis.size() + 2 >= total_size_coef,
          "Dimensions of basis " << dim_basis << " and coefficient " <<
          dim_coef << " can not be connected.");
        for (int i = 0; i < dim_basis.size(); i++) {
            spline_assert_message(dim_basis[i] == dim_coef[i],
                "Mismatch of dimention " + std::to_string(i) + " between basis and coefficient: "
                << "Got basis " << dim_basis << " and coefficient" << dim_coef <<".");
        }

        basis_ = basis;
        coeff_ = coeff.add_trival_dimension(2 + dim_basis.size() - dim_coef.size());
    }

    Function Function::Constant(const TensorBasis& basis, const AnyScalar& a,
          const std::vector< int >& size) {
        AnyTensor value = AnyTensor::repeat(AnyTensor(a), size);
        return Function::Constant(basis, value);
    }

    Function Function::Constant(const TensorBasis& basis, const AnyTensor& t) {
        Coefficient coeff = Coefficient(basis.const_coeff_tensor(t));
        return Function(basis, coeff);
    }

    Function Function::Constant(const Basis& basis, const AnyScalar& a,
          const std::vector< int >& size) {
        AnyTensor value = AnyTensor::repeat(AnyTensor(a), size);
        return Function::Constant(basis, value);
    }

    Function Function::Constant(const Basis& basis, const AnyTensor& t) {
        Coefficient coeff = Coefficient(basis.const_coeff_tensor(t));
        return Function(basis, coeff);
    }

    AnyTensor Function::operator()(const std::vector< AnyScalar >& x) const {
        return basis_(x).inner(coeff().data());
    }

    MX Function::operator<=(const MX& x) const {
      return coeff().data().as_MT().data()<=x;
    }
    MX Function::operator>=(const MX& x) const {
      return coeff().data().as_MT().data()>=x;
    }

    Basis Function::basis() const {
      spline_assert_message(basis_.n_basis()==1,
          ".basis() syntax only works for a 1-D TensorBasis.");
      return basis_.bases()[0];
    }
    Basis Function::basis(const Index& index) const {
      return basis_.basis(index);
    }

    Function Function::generic_operation(const Function& f,
        const BasisComposition & bc, const TensorComposition & tc) const  {

      TensorBasis sumBasis = bc(tensor_basis(), f.tensor_basis());
      EvaluationGrid evaluationGrid = EvaluationGrid(sumBasis);
      std::vector< AnyTensor > basisEvaluated;
      std::vector< AnyTensor > thisFunctionEvaluated;
      std::vector< AnyTensor > otherFunctionEvaluated;
      std::vector< AnyTensor > sumFunctionEvaluated;

      basisEvaluated = evaluationGrid.eval();
      thisFunctionEvaluated = evaluationGrid.eval(*this);
      otherFunctionEvaluated = evaluationGrid.eval(f);

      for (int i = 0; i < basisEvaluated.size(); i++) {
          AnyTensor lhs = thisFunctionEvaluated[i];
          AnyTensor rhs = otherFunctionEvaluated[i];
          if (lhs.dims() == std::vector< int > {1, 1}) {
              lhs = lhs.shape({});
          }
          if (rhs.dims() == std::vector< int > {1, 1}) {
              rhs = rhs.shape({});
          }
          sumFunctionEvaluated.push_back(tc(lhs, rhs));
      }

      AnyTensor A = AnyTensor::pack(basisEvaluated, 0);
      AnyTensor B = AnyTensor::pack(sumFunctionEvaluated, 0);

      int numberEval = basisEvaluated.size();
      int numberBasis = sumBasis.totalNumberBasisFunctions();
      std::vector< int > elemShape = sumFunctionEvaluated[0].dims();
      int numberCoef = (elemShape.size() == 0)? 1: elemShape[0]*elemShape[1];

      std::vector< int > shapeA = {numberEval, numberBasis};
      std::vector< int > shapeB = {numberBasis, numberCoef};
      A = A.shape(shapeA);
      B = B.shape(shapeB);
      AnyTensor C = A.solve(B);

      std::vector< int > shapeCoef = elemShape;
      std::vector< int > shapeBasis = sumBasis.dimension();
      shapeBasis.insert(shapeBasis.end(), shapeCoef.begin(), shapeCoef.end());

      C = C.shape(shapeBasis);
      return Function(sumBasis, C);
    }

    Function Function::operator+(const Function& f) const {
      return generic_operation(f,
          [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs + rhs; },
          [](const AnyTensor& lhs, const AnyTensor& rhs) { return lhs + rhs; });
    }

    Function Function::operator+(const AnyTensor& t) const {
        if (t.is_scalar() && t.dims()!=shape())
          return operator+(AnyTensor::repeat(t.as_scalar(), shape()));
        return operator+(Function::Constant(this->tensor_basis(), t));
    }

    Function Function::operator*(const Function& f) const {
      return generic_operation(f,
          [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs * rhs; },
          [](const AnyTensor& lhs, const AnyTensor& rhs) { return lhs * rhs; });
    }

    void Function::homogenize_args(Function& f, AnyTensor& t) {
      if (t.is_scalar() && t.dims()!=f.shape()) t = AnyTensor::repeat(t.as_scalar(), f.shape());
      if (f.is_scalar() && !t.is_scalar()) {
        AnyTensor ones = AnyTensor::repeat(AnyScalar(1), t.dims());
        Coefficient C = f.coeff();
        f =  Function(f.tensor_basis(),
                C.data().shape(C.dimension()).outer_product(ones)).operator*(t);
      }
    }

    Function Function::operator*(const AnyTensor& rhs) const {
        Function f = *this;
        AnyTensor t = rhs;
        homogenize_args(f, t);

        std::vector< int > tdims = f.coeff_tensor().dims();
        tdims.pop_back();
        tdims.pop_back();

        AnyTensor transf = AnyTensor::repeat(AnyScalar(1), tdims);
        transf = transf.outer_product(t);
        AnyTensor data = f.coeff_tensor()*transf;

        return Function(f.tensor_basis(), Coefficient(data));
    }

    Function Function::mtimes(const Function& f) const {
      return generic_operation(f,
          [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs * rhs; },
          [](const AnyTensor& lhs, const AnyTensor& rhs) { return lhs.mtimes(rhs);});
    }

    Function Function::mtimes(const AnyTensor& t) const {
        if (t.is_scalar()) return operator*(t);
        spline_assert(t.n_dims() == 2);
        Coefficient c = coeff();
        int dir = n_inputs() + 1; //0 based, 2nd matrix dimension
        return Function(tensor_basis(), c.transform(t.reorder_dims({1, 0}), dir));
    }

    Function Function::rmtimes(const AnyTensor& t) const {
        if (t.is_scalar()) return operator*(t);
        return Function::Constant(this->tensor_basis(), t).mtimes(*this);
    }

    Function Function::pow(int power) const {
        spline_assert(power >= 0);
        Function f = *this;
        Function fpow = *this;

        if (power == 0) {
            fpow = Function::Constant(tensor_basis(), 1, shape());
        } else {
            for (int i = 1; i < power; i++) {
                fpow = fpow*f;
            }
        }

        return fpow;
    }

    Function Function::operator-(const Function& f) const {
        return operator+(-f);
    }

    Function Function::operator-(const AnyTensor& t) const {
        return operator+(-t);
    }

    Function Function::operator-() const {
        return Function(basis_, -coeff_);
    }

    Function Function::transpose() const {
        return Function(tensor_basis(), coeff().transpose());
    }

    Function Function::trace() const {
        std::vector< int > shape_ = shape();
        spline_assert_message(shape_[0] == shape_[1],
            "Trace only defined for square matrices. Dimensions are " << shape_ << ".");

        AnyTensor t = DT(DM::densify(DM::eye(shape_[0])));
        Function fdiag = operator*(t); //keep diagonal entries only

        Coefficient cdiag = fdiag.coeff();
        AnyTensor ones = AnyTensor::repeat(AnyScalar(1), std::vector< int >{1, shape_[0]});
        cdiag = cdiag.transform(ones, cdiag.dimension().size()); //sum over all columns
        cdiag = cdiag.transform(ones, cdiag.dimension().size()+1); //sum over all rows

        return Function(tensor_basis(), cdiag);
    }

    Function Function::vertcat(const std::vector< spline::Function >& f) const {
        return cat(0, f);
    }

    Function Function::horzcat(const std::vector< spline::Function >& f) const {
        return cat(1, f);
    }

    Function Function::blkdiag(const std::vector< spline::Function >& f) const {
        Function b = *this;
        for (int i = 0; i < f.size(); i++) {
            std::vector< int > shape12 = std::vector< int >{b.shape()[0], f[i].shape()[1]};
            std::vector< int > shape21 = std::vector< int >{f[i].shape()[0], b.shape()[1]};

            Function zero12 = Function::Constant(b.tensor_basis(), 0, shape12);
            Function zero21 = Function::Constant(b.tensor_basis(), 0, shape21);

            Function upper = b.horzcat(std::vector< Function >{zero12});
            Function lower = zero21.horzcat(std::vector< Function >{f[i]});

            b = upper.vertcat(std::vector< Function >{lower});
        }

        return b;
    }

    std::string Function::getRepresentation() const {return "Function";};

    int Function::n_inputs() const {
        return basis_.n_inputs();
    }

    std::vector< int > Function::shape() const {
        return coeff_.shape();
    }

    Function Function::insert_knots(const AnyVector & new_knots) const {
      spline_assert_message(tensor_basis().n_basis() == 1,
        "I don't know the direction for knot insertion. Please supply argument.")
      return insert_knots(std::vector<AnyVector>{new_knots}, std::vector<NumericIndex>{0});
    }

    Function Function::insert_knots(const AnyVector & new_knots,
      const NumericIndex & arg_ind) const {
      return insert_knots(std::vector<AnyVector>{new_knots}, std::vector<NumericIndex>{arg_ind});
    }

    Function Function::insert_knots(const AnyVector & new_knots, const Argument & arg) const {
      return insert_knots(std::vector<AnyVector>{new_knots}, std::vector<Argument>{arg});
    }

    Function Function::insert_knots(const std::vector<AnyVector> & new_knots,
      const std::vector<Argument> & arg) const {
      std::vector<NumericIndex> arg_ind(arg.size());
      for (int i=0; i<arg.size(); i++) {
        arg_ind[i] = tensor_basis().indexArgument(arg[i]);
      }
      return insert_knots(new_knots, arg_ind);
    }

    Function Function::insert_knots(const std::vector<AnyVector> & new_knots,
      const std::vector<NumericIndex> & arg_ind) const {
      spline_assert(arg_ind.size() == new_knots.size())
      std::vector<AnyTensor> T;
      TensorBasis tbasis = tensor_basis();
      TensorBasis new_tbasis = tbasis.insert_knots(new_knots, T, arg_ind);
      Coefficient new_coefficient = coeff().transform(T, arg_ind);
      return Function(new_tbasis, new_coefficient);
    }

    Function Function::midpoint_refinement(int refinement) const {
      // apply on all directions
      std::vector<NumericIndex> arg_ind(tensor_basis().n_basis());
      std::vector<int> refs(tensor_basis().n_basis());
      for (int k=0; k<arg_ind.size(); k++) {
        arg_ind[k] = k;
        refs[k] = refinement;
      }
      return midpoint_refinement(refs, arg_ind);
    }

    Function Function::midpoint_refinement(int refinement, const NumericIndex & arg_ind) const {
      return midpoint_refinement(std::vector<int>{refinement}, std::vector<NumericIndex>{arg_ind});
    }

    Function Function::midpoint_refinement(int refinement, const Argument & arg) const {
      return midpoint_refinement(std::vector<int>{refinement}, std::vector<Argument>{arg});
    }

    Function Function::midpoint_refinement(const std::vector<int> & refinement,
      const std::vector<Argument> & arg) const {
      std::vector<NumericIndex> arg_ind(arg.size());
      for (int i=0; i<arg.size(); i++) {
        arg_ind[i] = tensor_basis().indexArgument(arg[i]);
      }
      return midpoint_refinement(refinement, arg_ind);
    }

    Function Function::midpoint_refinement(const std::vector<int> & refinement,
      const std::vector<NumericIndex> & arg_ind) const {
      spline_assert(arg_ind.size() == refinement.size())
      std::vector<AnyTensor> T;
      TensorBasis tbasis = tensor_basis();
      TensorBasis new_tbasis = tbasis.midpoint_refinement(refinement, T, arg_ind);
      Coefficient new_coefficient = coeff().transform(T, arg_ind);
      return Function(new_tbasis, new_coefficient);
    }

    Function Function::derivative() const {
      return derivative(1);
    }

    Function Function::derivative(int order) const {
      spline_assert_message(tensor_basis().n_basis() == 1,
        "I don't know the direction for derivation. Please supply argument.");
      return derivative(std::vector<int>{order}, std::vector<NumericIndex>{0});
    }

    Function Function::derivative(int order, const Argument& direction) const {
      return derivative(std::vector<int>{order}, std::vector<Argument>{direction});
    }

    Function Function::derivative(int order, const NumericIndex& direction) const {
      return derivative(std::vector<int>{order}, std::vector<NumericIndex>{direction});
    }

    Function Function::derivative(const std::vector<int>& orders,
          const std::vector<Argument>& directions) const {
        std::vector<NumericIndex> direction_ind(directions.size());
        for (int i=0; i<directions.size(); i++) {
            direction_ind[i] = tensor_basis().indexArgument(directions[i]);
        }
        return derivative(orders, direction_ind);
    }

    Function Function::derivative(const std::vector<int>& orders,
          const std::vector<NumericIndex>& direction_ind) const {
        spline_assert(orders.size() == direction_ind.size())  // each direction should have an order
        std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.derivative(orders, direction_ind, T);
        std::vector<NumericIndex> directions(direction_ind.size());
        Coefficient new_coefficient = coeff().transform(T, direction_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function Function::antiderivative() const {
      return antiderivative(1);
    }

    Function Function::antiderivative(int order) const {
      spline_assert_message(tensor_basis().n_basis() == 1,
        "I don't know the direction for derivation. Please supply argument.");
      return antiderivative(std::vector<int>{order}, std::vector<NumericIndex>{0});
    }

    Function Function::antiderivative(int order, const Argument& direction) const {
      return antiderivative(std::vector<int>{order}, std::vector<Argument>{direction});
    }

    Function Function::antiderivative(int order, const NumericIndex& direction) const {
      return antiderivative(std::vector<int>{order}, std::vector<NumericIndex>{direction});
    }

    Function Function::antiderivative(const std::vector<int>& orders,
        const std::vector<Argument>& directions) const {
      std::vector<NumericIndex> direction_ind(directions.size());
      for (int i=0; i<directions.size(); i++) {
          direction_ind[i] = tensor_basis().indexArgument(directions[i]);
      }
      return antiderivative(orders, direction_ind);
    }

    Function Function::antiderivative(const std::vector<int>& orders,
        const std::vector<NumericIndex>& direction_ind) const {
      spline_assert(orders.size() == direction_ind.size())  // each direction should have an order
      std::vector<AnyTensor> T;
      TensorBasis tbasis = tensor_basis();
      TensorBasis new_tbasis = tbasis.antiderivative(orders, direction_ind, T);
      std::vector<NumericIndex> directions(direction_ind.size());
      Coefficient new_coefficient = coeff().transform(T, direction_ind);
      return Function(new_tbasis, new_coefficient);
    }

    std::vector<Function> Function::jacobian() const {
        std::vector<Function> Jacobian(n_inputs());
        for (int i=0; i<n_inputs(); i++) {
            Jacobian[i] = derivative(1, i);
        }
        return Jacobian;
    }

    Function Function::transform_to(const TensorBasis& basis) const {

      TensorBasis unionBasis = tensor_basis() + basis;
      EvaluationGrid evaluationGrid = EvaluationGrid(unionBasis);
      std::vector< AnyTensor > basisEvaluated;
      std::vector< AnyTensor > thisFunctionEvaluated;

      basisEvaluated = evaluationGrid.eval();
      thisFunctionEvaluated = evaluationGrid.eval(*this);

      AnyTensor A = AnyTensor::pack(basisEvaluated, 0);
      AnyTensor B = AnyTensor::pack(thisFunctionEvaluated, 0);
      int numberEval = basisEvaluated.size();
      int numberBasis = unionBasis.totalNumberBasisFunctions();
      std::vector< int > elemShape = thisFunctionEvaluated[0].dims();
      int numberCoef = elemShape[0]*elemShape[1];

      std::vector< int > shapeA = {numberEval, numberBasis};
      std::vector< int > shapeB = {numberBasis, numberCoef};
      A = A.shape(shapeA);
      B = B.shape(shapeB);
      AnyTensor C = A.solve(B);

      std::vector< int > shapeCoef = elemShape;
      std::vector< int > shapeBasis = unionBasis.dimension();
      shapeBasis.insert(shapeBasis.end(), shapeCoef.begin(), shapeCoef.end());

      C = C.shape(shapeBasis);
      return Function(unionBasis, C);
    }

    Function Function::cat(const NumericIndex& index,
          const std::vector< spline::Function >& functions) const {
        TensorBasis unionBasis = tensor_basis();
        for (auto& f : functions) {
            unionBasis = unionBasis + f.tensor_basis();
        }

        std::vector< Coefficient > coefVec;
        for (auto& f : functions) {
            coefVec.push_back(f.transform_to(unionBasis).coeff());
        }

        Coefficient coef = this->transform_to(unionBasis).coeff();
        return Function(unionBasis, coef.cat(index, coefVec));
    }

    Function Function::slice(const std::vector<NumericIndex>& i,
        const std::vector<NumericIndex>& j) const {
      return Function(tensor_basis(), coeff_tensor().get_slice(NumericIndex::as_int(i), NumericIndex::as_int(j)));
    }

}  // namespace spline
