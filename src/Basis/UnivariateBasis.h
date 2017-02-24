#ifndef CPP_SPLINE_UNIVARIATEBASIS_H
#define CPP_SPLINE_UNIVARIATEBASIS_H

#include <casadi/casadi.hpp>
#include <string>

#include "../common.h"
#include "Basis.h"
#include "../Domain/Interval.h"

namespace spline {

    class MonomialBasis;
    class BSplineBasis;

#ifndef SWIG

    class UnivariateBasisNode : public BasisNode {
    public:
        UnivariateBasisNode(int degree, const Domain& domain);


        virtual AnyTensor const_coeff_tensor(const AnyTensor& t) const = 0;
        virtual AnyTensor operator()(const std::vector< AnyScalar >& x) const = 0;

        int degree() const;
        int n_inputs() const;
        // virtual std::string to_string() const override;
        // virtual std::string type() const override;

        virtual Basis derivative(int order, AnyTensor& T) const = 0;
        virtual Basis antiderivative(int order, AnyTensor& T) const = 0;
        virtual AnyTensor integral(const Domain& domain) const override;
        virtual AnyTensor integral(const Interval& domain) const = 0;
        virtual Basis kick_boundary(const Domain& domain, AnyTensor& T) const override;
        virtual Basis kick_boundary(const Interval& domain, AnyTensor& T) const;
        virtual std::vector< std::vector < AnyScalar > > getEvaluationGrid() const = 0;

    protected:
        int degree_;
    };

#endif // SWIG

    class UnivariateBasis : public Basis {
    public:
#ifndef SWIG
        //   virtual std::vector<double> evaluationGrid (void) const = 0;
        UnivariateBasis() { };
        UnivariateBasisNode* get() const ;
        UnivariateBasisNode* operator->() const ;
#endif // SWIG

        std::string to_string() const;
        std::string type() const;

        int degree () const ;
    };
} // namespace spline

#endif //CPP_SPLINE_UNIVARIATEBASIS_H
