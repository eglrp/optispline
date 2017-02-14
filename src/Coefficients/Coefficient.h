#ifndef CPP_SPLINE_COEFFICIENTS_COEFFICIENT_H
#define CPP_SPLINE_COEFFICIENTS_COEFFICIENT_H

#include <string>
#include <vector>
#include <any_tensor.hpp>
#include "../Function/NumericIndex.h"

#include "../SharedObject/SharedObject.h"
#include "../SharedObject/SharedObjectNode.h"

namespace spline {

#ifndef SWIG
    class CoefficientNode : public SharedObjectNode {
    public:

        CoefficientNode(const AnyTensor& t) : data(t){}
        CoefficientNode(const std::vector< double >& v);

        std::vector< int > getShape() const;
        int getNumberCoefficents() const;

        // Coefficient operator+ (Coefficient & other) const;
        CoefficientNode operator-() const ;
        AnyTensor getData() const {return data;}
        std::string getRepresentation() const ;
        void repr() const { userOut() << getRepresentation() << std::endl;}
    public:
        AnyTensor data;
    };
#endif


    class Coefficient : public SharedObject {
    public:

    #ifndef SWIG

        CoefficientNode* get() const ;
        CoefficientNode* operator->() const ;

    #endif // SWIG
        Coefficient();
        Coefficient(const AnyTensor& t);
        Coefficient(const std::vector< double >& v);

        std::vector< int > getShape() const;
        int getNumberCoefficents() const;

        Coefficient operator-() const ;
        AnyTensor getData() const;
        std::string getRepresentation() const;
        /// Transform a direction of the coeffient's data tensor using given transformation matrix
        AnyTensor transform(const AnyTensor& T, const NumericIndex& direction) const;
    };
} // namespace spline

#endif //CPP_SPLINE_COEFFICIENTS_COEFFICIENT_H
