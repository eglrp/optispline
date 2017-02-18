#include <vector>
#include "UnivariateBasis.h"

#include "operations/operationsBasis.h"
#include "../common.h"

namespace spline {
    UnivariateBasisNode::UnivariateBasisNode(int degree) : degree_(degree) { }
    UnivariateBasisNode* UnivariateBasis::get() const {
        return static_cast<UnivariateBasisNode*>(SharedObject::get());
    };
    UnivariateBasisNode* UnivariateBasis::operator->() const { return get(); }

    int UnivariateBasis::getLength() const  { return (*this)->getLength(); }

    int  UnivariateBasis::degree() const { return (*this)->degree (); }
    int  UnivariateBasisNode::degree() const { return degree_ ; }

    int UnivariateBasisNode::n_inputs() const { return 1; }

}  // namespace spline
