#include "EmptyBasis.h"
#include "MonomialBasis.h"
#include "BSplineBasis.h"

namespace spline {

    EmptyBasisNode* EmptyBasis::get() const {
      return static_cast<EmptyBasisNode*>(SharedObject::get());
    };

    EmptyBasisNode* EmptyBasis::operator->() const { return get(); }

    EmptyBasis::EmptyBasis() {
        assign_node(new EmptyBasisNode());
    }
    std::string EmptyBasisNode::getRepresentation() const {
        return "EmptyBasis object";
    }

    Basis EmptyBasisNode::operator+ (const Basis& other) const  {
        return other + shared_from_this< EmptyBasis >();
    }

    Basis EmptyBasisNode::operator+ (const MonomialBasis& other) const  {
        return other.get()->shared_from_this< MonomialBasis >();
    }

    Basis EmptyBasisNode::operator+ (const BSplineBasis& other) const  {
        return other.get()->shared_from_this< BSplineBasis >();
    }

    Basis EmptyBasisNode::operator* (const Basis& other) const  {
        return other * shared_from_this< EmptyBasis >();
    }

    Basis EmptyBasisNode::operator* (const MonomialBasis& other) const  {
        return other.get()->shared_from_this< MonomialBasis >();
    }

    Basis EmptyBasisNode::operator* (const BSplineBasis& other) const  {
        return other.get()->shared_from_this< BSplineBasis >();
    }

    Basis EmptyBasisNode::derivative(int order, AnyTensor& T) const {
        spline_assert(false) ;
        return shared_from_this< Basis >();
    }

}  // namespace spline