#ifndef BASIS_H_
#define BASIS_H_

#include <iostream>
#include <string>
#include <vector>
#include <any_tensor.hpp>

#include "../SharedObject/SharedObject.h"
#include "../SharedObject/SharedObjectNode.h"

#include "Basis.h"
#include "../Function/Argument.h"

namespace spline {
    class TensorBasis;
    class BSplineBasis;
    class MonomialBasis;

#ifndef SWIG

    class TensorBasisNode : public SharedObjectNode {
    public:
        TensorBasisNode (const std::vector< Basis >& allBasis);
        int getDimension () const;

        std::vector<int> getShape () const;

        void setArguments (const std::vector< Argument >& argument);
        std::vector< Argument > getArguments() const;

        Argument getSubArgument( int index ) const;
        int indexArgument(Argument a) const;

        bool hasArguments() const;

        virtual std::string getRepresentation() const ;

        std::vector< Basis > getSubBasis() const;
        TensorBasis getSubBasis( int index ) const;
        Basis getSubBasis( Argument a) const;

        void addBasis(TensorBasis basis);
        void addBasis(Basis basis);

        TensorBasis operator+(const TensorBasis& rhs) const;
        TensorBasis operator*(const TensorBasis& rhs) const;

        AnyTensor operator()(const std::vector< AnyScalar >& x) const;

        int totalNumberSubBasis() const;

    // protected:
        std::vector< Basis > allSubBasis;
        std::vector< Argument > allArguments;
    };

#endif // SWIG

    class TensorBasis : public SharedObject{
    public:

#ifndef SWIG

        TensorBasisNode* get() const ;
        TensorBasisNode* operator->() const ;

#endif // SWIG
        int getDimension () const;

        std::vector<int> getShape () const;
        TensorBasis ();
        TensorBasis (const Basis & allSubBasis);
        TensorBasis (const std::vector< Basis >& allSubBasis);
        TensorBasis (const std::vector< TensorBasis >& allBasis);

        void setArguments (const std::vector< spline::Argument >& argument);
        std::vector< spline::Argument > getArguments() const;

        spline::Argument getSubArgument( int index ) const;
        int indexArgument(Argument a) const;

        bool hasArguments() const;

        virtual std::string getRepresentation() const ;

        std::vector< Basis > getSubBasis() const;
        TensorBasis getSubBasis( int index ) const;
        Basis getSubBasis( Argument a) const;


        void addBasis(TensorBasis basis);
        void addBasis(Basis basis);

        TensorBasis operator+(const TensorBasis& rhs) const;
        TensorBasis operator*(const TensorBasis& rhs) const;

        AnyTensor operator()(const std::vector< AnyScalar >& x) const;

        int totalNumberSubBasis() const;
    };
}
#endif  // BASIS_H_