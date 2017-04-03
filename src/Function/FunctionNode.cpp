#include <vector>
#include "FunctionNode.h"
#include "ConstantNode.h"
#include "../Basis/utils/EvaluationGrid.h"
#include "../Basis/TensorBasisConstant.h"
#include "../common.h"

namespace spline {
    FunctionNode::FunctionNode(const TensorBasis& basis, const Coefficient& coeff) : FunNode(coeff), basis_(basis) {}

    AnyTensor FunctionNode::operator()(const AnyTensor& arg, const std::vector< int >& args) const{

        AnyTensor x = arg.squeeze();
        if (x.is_vector()) x = x.as_vector();
        spline_assert(x.n_dims()<=2);

        if (x.n_dims()==1) {
          if (x.dims()[0] == n_inputs()) {
            std::vector< AnyScalar > x_ = x.unpack_1();
            return basis_(x_, Argument::from_vector(args)).inner(coeff().data());
          } else {
            x = x.shape({x.numel(), 1});
          }
        }

        spline_assert_message(x.dims()[1] == n_inputs(),
          "Can evaluate list of " + std::to_string(n_inputs()) + " inputs. Got " +
          std::to_string(x.dims()[0])+ " by " + std::to_string(x.dims()[1]));
        std::vector< AnyTensor > tensor = {};

        std::vector< std::vector< AnyScalar > > X_ = x.unpack_2();
        for (int i = 0; i < X_.size(); i++) {
            tensor.push_back(basis_(X_[i], Argument::from_vector(args)));
        }
        AnyTensor packed_tensor = AnyTensor::pack(tensor, 0);
        int shared_dim = packed_tensor.n_dims();
        std::vector<int> a_r = mrange(0, shared_dim);
        std::vector<int> b_r = mrange(1, shared_dim + 2);
        std::vector<int> c_r = { a_r[0] };
        c_r.push_back(b_r[b_r.size() - 2]);
        c_r.push_back(b_r[b_r.size() - 1]);
        return packed_tensor.einstein(coeff().data(), a_r, b_r, c_r).squeeze();
    }

    Function FunctionNode::partial_eval(const AnyTensor& x, int index) const{
        int n_basis = tensor_basis().n_basis();

        std::vector< Basis > partial_basis = tensor_basis().bases();
        partial_basis.erase(partial_basis.begin() + index);
        std::vector< std::string > partial_arguments = tensor_basis().arguments();
        partial_arguments.erase(partial_arguments.begin() + index);

        std::vector< int > a = mrange(n_basis + 2);
        std::vector< int > b = std::vector< int > {-1-index};
        std::vector< int > c = mrange(n_basis + 2);
        c.erase(c.begin() + index);
        AnyTensor partial_coeff = coeff_tensor().einstein(basis(index)(x),a,b,c);

        return Function(TensorBasis( partial_basis, partial_arguments ), partial_coeff);
     }

    std::string FunctionNode::type() const{
        return "Function";
    }

    std::string FunctionNode::to_string() const{
        return "Function, consisting of a " + basis_.to_string() + "and:\n\t" + coeff_.to_string();
    }

    Function FunctionNode::generic_operation(const Function& f,
            const BasisComposition & bc, const TensorComposition & tc) const  {

        TensorBasis sumBasis = bc(tensor_basis(), f.tensor_basis());
        EvaluationGrid evaluationGrid = EvaluationGrid(sumBasis);
        std::vector< AnyTensor > basisEvaluated;
        AnyTensor thisFunctionEvaluated;
        AnyTensor otherFunctionEvaluated;

        basisEvaluated = evaluationGrid.eval();
        thisFunctionEvaluated = evaluationGrid.eval(shared_from_this<Function>());
        otherFunctionEvaluated = evaluationGrid.eval(f);

        AnyTensor A = AnyTensor::pack(basisEvaluated, 0);
        AnyTensor B = tc(thisFunctionEvaluated, otherFunctionEvaluated);

        // [n m p q] + [n m 1 1]   (*)

        int numberEval = basisEvaluated.size();
        int numberBasis = sumBasis.totalNumberBasisFunctions();
        std::vector< int > elemShape = B.dims();
        elemShape = std::vector<int>(elemShape.begin()+1, elemShape.end());
        int numberCoef = (elemShape.size() == 0)? 1: spline::product(elemShape);

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

    Function FunctionNode::operator+(const FunNode& f) const {
        return f + *this;
    }

    Function FunctionNode::operator+(const FunctionNode& f) const {
        return generic_operation(f.shared_from_this<Function>(),
                [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs + rhs; },
                [](const AnyTensor& lhs, const AnyTensor& rhs) { return lhs + rhs; });
    }

    Function FunctionNode::operator+(const ConstantNode& f) const {
        return f + *this;
    }

    Function FunctionNode::operator*(const FunNode& f) const {
        return f * *this;
    }

    Function FunctionNode::operator*(const FunctionNode& f) const {
        return generic_operation(f.shared_from_this<Function>(),
                [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs * rhs; },
                [](const AnyTensor& lhs, const AnyTensor& rhs) {

        int n = lhs.n_dims();
        std::vector<int> rep(n, 1);
        if (lhs.dims()[n-2]==1 && lhs.dims()[n-1]==1) {
          rep[n-2] = rhs.dims()[n-2];
          rep[n-1] = rhs.dims()[n-1];
          return AnyTensor::repeat(lhs, rep) * rhs;
        }
        if (rhs.dims()[n-2]==1 && rhs.dims()[n-1]==1) {
          rep[n-2] = lhs.dims()[n-2];
          rep[n-1] = lhs.dims()[n-1];
          return lhs * AnyTensor::repeat(rhs, rep);
        }
        return lhs * rhs;
      });
    }

    Function FunctionNode::operator*(const ConstantNode& f) const {
        return f * *this;
    }

    Function FunctionNode::mtimes(const FunNode& f) const {
        return f.rmtimes(*this);
    }

    Function FunctionNode::mtimes(const FunctionNode& f) const {
        return generic_operation(f.shared_from_this<Function>(),
                [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs * rhs; },
                [](const AnyTensor& lhs, const AnyTensor& rhs) { return lhs.einstein(rhs, {-4, -1, -2}, {-4, -2, -3}, {-4, -1, -3});});
    }

    Function FunctionNode::mtimes(const ConstantNode& f) const {
        return f.rmtimes(*this);
    }

    Function FunctionNode::rmtimes(const FunctionNode& f) const {
        return generic_operation(f.shared_from_this<Function>(),
                [](const TensorBasis& lhs, const TensorBasis& rhs) { return lhs * rhs; },
                [](const AnyTensor& lhs, const AnyTensor& rhs) { return rhs.einstein(lhs, {-4, -1, -2}, {-4, -2, -3}, {-4, -1, -3});});
    }

    Function FunctionNode::rmtimes(const ConstantNode& f) const {
        return f.mtimes(*this);
    }

    Function FunctionNode::operator-() const {
        return Function(tensor_basis(), -coeff_);
    }

    Function FunctionNode::transpose() const {
        return Function(tensor_basis(), coeff().transpose());
    }

    Function FunctionNode::trace() const {
        std::vector< int > shape_ = shape();
        spline_assert_message(shape_[0] == shape_[1],
                "Trace only defined for square matrices. Dimensions are " << shape_ << ".");

        AnyTensor t = DT(casadi::DM::densify(casadi::DM::eye(shape_[0])));
        Function fdiag = operator*(ConstantNode(t)); //keep diagonal entries only

        Coefficient cdiag = fdiag.coeff();
        AnyTensor ones = AnyTensor::repeat(AnyScalar(1), std::vector< int >{1, shape_[0]});
        cdiag = cdiag.transform(ones, cdiag.dimension().size()); //sum over all columns
        cdiag = cdiag.transform(ones, cdiag.dimension().size()+1); //sum over all rows

        return Function(tensor_basis(), cdiag);
    }

    Function FunctionNode::transform_to(const TensorBasis& basis) const {
        TensorBasis unionBasis = tensor_basis() + basis;
        EvaluationGrid evaluationGrid = EvaluationGrid(unionBasis);
        std::vector< AnyTensor > basisEvaluated;
        AnyTensor thisFunctionEvaluated;

        basisEvaluated = evaluationGrid.eval();
        thisFunctionEvaluated = evaluationGrid.eval(shared_from_this<Function>());

        AnyTensor A = AnyTensor::pack(basisEvaluated, 0);
        AnyTensor B = thisFunctionEvaluated;
        int numberEval = basisEvaluated.size();
        int numberBasis = unionBasis.totalNumberBasisFunctions();
        std::vector< int > elemShape = B.dims();
        elemShape = std::vector<int>(elemShape.begin()+1, elemShape.end());
        int numberCoef = spline::product(elemShape);

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

    Function FunctionNode::project_to(const TensorBasis& b) const {
        Function b2 = b.basis_functions();
        Function f = reshape(std::vector< int >{1,spline::product(shape())});

        Function b22 = b2.mtimes(b2.transpose());
        Function b2f = b2.mtimes(f); //f already is a row vector

        AnyTensor B22 = b22.integral();
        AnyTensor B2f = b2f.integral();

        AnyTensor C = B22.solve(B2f);

        std::vector< int > M = b.dimension();
        std::vector< int > N = shape();
        std::vector< int > shapeC = M;
        shapeC.insert(shapeC.end(), N.begin(), N.end());
        C = C.shape(shapeC);

        return Function(b,C);
    }

    Function FunctionNode::reshape(const std::vector< int >& shape) const {
        return Function(tensor_basis(), coeff().reshape(shape));
    }

    Function FunctionNode::slice(const AnySlice& i, const AnySlice& j) const {
        return Function(tensor_basis(), coeff_tensor().get_slice(i, j));
    }

    int FunctionNode::n_inputs() const {
        return tensor_basis().n_inputs();
    }

    Function FunctionNode::slice(const AnySlice& i) const {
        return Function(tensor_basis(), coeff_tensor().get_slice(i));
    }

    TensorBasis FunctionNode::tensor_basis() const {
        return basis_;
    }

        Basis FunctionNode::basis() const {
        spline_assert_message(tensor_basis().n_basis()==1,
                ".basis() syntax only works for a 1-D TensorBasis.");
        return tensor_basis().bases()[0];
    }

        Basis FunctionNode::basis(const Argument& index) const {
        return tensor_basis().basis(index);
    }

    Function FunctionNode::insert_knots(const std::vector<AnyVector> & new_knots,
            const std::vector<int> & arg_ind) const {
        spline_assert(arg_ind.size() == new_knots.size())
            std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.insert_knots(new_knots, Argument::from_vector(arg_ind), T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::midpoint_refinement(const std::vector<int>& refinement,
            const std::vector< int >& arg_ind) const {
        spline_assert(arg_ind.size() == refinement.size())
            std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.midpoint_refinement(refinement,
          Argument::from_vector(arg_ind), T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::degree_elevation(const std::vector<int>& elevation,
            const std::vector< int >& arg_ind) const {
        spline_assert(arg_ind.size() == elevation.size());
            std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.degree_elevation(elevation,
          Argument::from_vector(arg_ind), T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::kick_boundary(const TensorDomain& boundary) const {
        // apply on all directions
        NumericIndexVector arg_ind(tensor_basis().n_basis());
        for (int k=0; k<arg_ind.size(); k++) {
            arg_ind[k] = k;
        }
        return kick_boundary(boundary, arg_ind);
    }

    Function FunctionNode::kick_boundary(const TensorDomain& boundary, const NumericIndex& arg_ind) const {
        return kick_boundary(boundary, NumericIndexVector{arg_ind});
    }

    Function FunctionNode::kick_boundary(const TensorDomain& boundary,
            const std::vector<std::string>& args) const {
        std::vector<AnyTensor> T;
        TensorBasis new_tbasis = tensor_basis().kick_boundary(boundary, args, T);
        NumericIndexVector arg_ind(args.size());
        for (int i=0; i<args.size(); i++) {
            arg_ind[i] = tensor_basis().indexArgument(args[i]);
        }
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::kick_boundary(const TensorDomain& boundary,
            const NumericIndexVector& arg_ind) const {
        std::vector<AnyTensor> T;
        TensorBasis new_tbasis = tensor_basis().kick_boundary(boundary, arg_ind, T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::derivative(const std::vector<int>& orders,
            const std::vector< int >& arg_ind) const {
        spline_assert(orders.size() == arg_ind.size())  // each direction should have an order
            std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.derivative(orders, Argument::from_vector(arg_ind), T);
        NumericIndexVector args(arg_ind.size());
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }


    Function FunctionNode::antiderivative(const std::vector<int>& orders,
            const std::vector< int >& arg_ind) const {
        spline_assert(orders.size() == arg_ind.size())  // each direction should have an order
            std::vector<AnyTensor> T;
        TensorBasis tbasis = tensor_basis();
        TensorBasis new_tbasis = tbasis.antiderivative(orders, Argument::from_vector(arg_ind), T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    std::vector<Function> FunctionNode::jacobian() const {
        std::vector<Function> Jacobian(n_inputs());
        for (int i=0; i<n_inputs(); i++) {
            Jacobian[i] = derivative({ 1 }, {i});
        }
        return Jacobian;
    }

    AnyTensor FunctionNode::integral() const {
        return integral(tensor_basis().domain());
    }

    AnyTensor FunctionNode::integral(const TensorDomain& domain) const {
        std::vector<AnyTensor> T = tensor_basis().integral(domain);
        NumericIndexVector arg_ind(tensor_basis().n_basis());
        for (int i=0; i<tensor_basis().n_basis(); i++) {
            arg_ind[i] = i;
        }
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        return new_coefficient.data().shape(shape());
    }

    Function FunctionNode::partial_integral(const TensorDomain& domain,
            const std::vector<std::string>& args) const {
        std::vector<AnyTensor> T;
        TensorBasis new_tbasis = tensor_basis().partial_integral(domain, args, T);
        NumericIndexVector arg_ind(args.size());
        for (int i=0; i<args.size(); i++) {
            arg_ind[i] = tensor_basis().indexArgument(args[i]);
        }
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        new_coefficient = new_coefficient.rm_direction(arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

    Function FunctionNode::partial_integral(const TensorDomain& domain,
            const NumericIndexVector& arg_ind) const {
        std::vector<AnyTensor> T;
        TensorBasis new_tbasis = tensor_basis().partial_integral(domain, arg_ind, T);
        Coefficient new_coefficient = coeff().transform(T, arg_ind);
        new_coefficient = new_coefficient.rm_direction(arg_ind);
        return Function(new_tbasis, new_coefficient);
    }

}  // namespace spline
