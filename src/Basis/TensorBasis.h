#ifndef SRC_BASIS_TENSORBASIS_H_
#define SRC_BASIS_TENSORBASIS_H_

#include <iostream>
#include <string>
#include <vector>
#include <any_tensor.hpp>

#include "../SharedObject/SharedObject.h"
#include "../SharedObject/SharedObjectNode.h"

#include "Basis.h"
#include "../Function/Argument.h"
#include "../Function/Index.h"
#include "../Function/NumericIndex.h"

namespace spline {

class TensorBasis;
class BSplineBasis;
class MonomialBasis;

#ifndef SWIG

class TensorBasisNode : public SharedObjectNode {
    public:
        TensorBasisNode(const std::vector< Basis >& allSubBasis);
        TensorBasisNode(const std::vector< Basis >& allSubBasis, const std::vector< Argument >& args);

        int n_basis() const;
        std::vector<int> dimension() const;
        int n_inputs() const;

        std::vector< Argument > arguments() const;

        Argument getSubArgument(int index) const;
        int indexArgument(Argument a) const;

        bool hasArguments() const;

        std::string getRepresentation() const;

        std::vector< Basis > getSubBasis() const;
        TensorBasis getSubBasis(int index) const;
        Basis basis() const;
        Basis basis(const Argument& a) const;
        Basis basis(const Index& index) const;
        // Basis getSubBasis( Argument a) const;

        TensorBasis add_basis(TensorBasis basis) const;
        TensorBasis add_basis(Basis basis) const;

        TensorBasis substitute_bases(const std::vector<Index>& arg_ind, const std::vector<Basis>& bases) const;

        TensorBasis operator+(const TensorBasis& rhs) const;
        TensorBasis operator*(const TensorBasis& rhs) const;

        AnyTensor operator()(const std::vector< AnyScalar >& x) const;

        int totalNumberBasisFunctions() const;
        AnyTensor const_coeff_tensor(const AnyTensor& t) const;

        TensorBasis insert_knots(const std::vector<AnyVector> & new_knots, std::vector<AnyTensor> & T, const std::vector<NumericIndex>& arg_ind) const;
        TensorBasis midpoint_refinement(const std::vector<int> & refinement, std::vector<AnyTensor> & T, const std::vector<NumericIndex>& arg_ind) const;

        TensorBasis derivative(const std::vector<int>& orders, const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& T) const;
        TensorBasis antiderivative(const std::vector<int>& orders, const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& T) const;

    // protected:
        std::vector< Basis > allSubBasis;
        std::vector< Argument > allArguments;
};

#endif // SWIG

class TensorBasis : public SharedObject{
public:
#ifndef SWIG

        TensorBasisNode* get() const;
        TensorBasisNode* operator->() const;

#endif // SWIG
        TensorBasis();
        TensorBasis(const Basis& allSubBasis);
        TensorBasis(const Basis& allSubBasis, const Argument& args);
        TensorBasis(const TensorBasis& tensor, const std::vector< Argument >& args);
        TensorBasis(const std::vector< Basis >& allSubBasis);
        TensorBasis(const std::vector< Basis >& allSubBasis, const std::vector< Argument >& args);
        TensorBasis(const std::vector< TensorBasis >& allBasis);

        int n_basis() const;  // Number of bases, building up the TensorBasis
        int n_inputs() const; // Total number of inputs, over all bases
        std::vector<int> dimension() const; // Vector containing number of basis functions in each basis

        std::vector< spline::Argument > arguments() const;

        spline::Argument getSubArgument(int index) const;
        int indexArgument(Argument a) const;

        bool hasArguments() const;

        std::string getRepresentation() const;

        Basis basis() const;
        Basis basis(const Argument& a) const;
        Basis basis(const Index& index) const;
        // Basis operator[](Argument a) const;
        // Basis operator[](int index) const;

        std::vector< Basis > getSubBasis() const;
        // TensorBasis tensor_basis() const;

        TensorBasis add_basis(TensorBasis basis) const;
        TensorBasis add_basis(Basis basis) const;

        TensorBasis substitute_bases(const std::vector<Index>& arg_ind, const std::vector<Basis>& bases) const;

        TensorBasis operator+(const TensorBasis& rhs) const;
        TensorBasis operator*(const TensorBasis& rhs) const;

        AnyTensor operator()(const std::vector< AnyScalar >& x) const;

        int totalNumberBasisFunctions() const;

        AnyTensor const_coeff_tensor(const AnyTensor& t) const;

        TensorBasis insert_knots(const std::vector<AnyVector> & new_knots, std::vector<AnyTensor> & SWIG_OUTPUT(T), const std::vector<Argument>& args) const;
        TensorBasis insert_knots(const std::vector<AnyVector> & new_knots, std::vector<AnyTensor> & SWIG_OUTPUT(T), const std::vector<NumericIndex>& arg_ind) const;

        TensorBasis midpoint_refinement(const std::vector<int> & refinement, std::vector<AnyTensor> & SWIG_OUTPUT(T), const std::vector<Argument>& args) const;
        TensorBasis midpoint_refinement(const std::vector<int> & refinement, std::vector<AnyTensor> & SWIG_OUTPUT(T), const std::vector<NumericIndex>& arg_ind) const;

        TensorBasis derivative(const std::vector<Argument>& directions, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;  // default order = 1
        TensorBasis derivative(const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;  // default order = 1
        TensorBasis derivative(const std::vector<int>& orders, const std::vector<Argument>& directions, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;
        TensorBasis derivative(const std::vector<int>& orders, const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;

        TensorBasis antiderivative(const std::vector<Argument>& directions, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;  // default order = 1
        TensorBasis antiderivative(const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;  // default order = 1
        TensorBasis antiderivative(const std::vector<int>& orders, const std::vector<Argument>& directions, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;
        TensorBasis antiderivative(const std::vector<int>& orders, const std::vector<NumericIndex>& direction_ind, std::vector<AnyTensor>& SWIG_OUTPUT(T)) const;
};

}   // namespace spline

#endif  // SRC_BASIS_TENSORBASIS_H_
