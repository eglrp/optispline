#ifndef CPP_SPLINE_SUBBASISDUMMY
#define CPP_SPLINE_SUBBASISDUMMY

#include <casadi/casadi.hpp>
#include <string>

#include "Basis.h"

namespace spline {

    class EmptyBasisNode;

    class EmptyBasis : public Basis {
    public:
#ifndef SWIG
        EmptyBasisNode* get() const ;
        EmptyBasisNode* operator->() const ;
#endif // SWIG

        EmptyBasis() ;

    };
} // namespace spline

#endif //CPP_SPLINE_SUBBASISDUMMY
