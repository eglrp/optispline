// Copyright 2016 Erik
//
// Created by erik on 08/04/16.
//

#include <math.h>       /* pow */

#include <casadi/casadi.hpp> // range

#include "MonomialeBasis.h"

#include "plus.h"
#include "times.h"

namespace spline {
    //    Monomialestd::shared_ptr<Basis>::Monomialestd::shared_ptr<Basis> (int degree) : Univariatestd::shared_ptr<Basis>(degree) { }
    //
    //   std::vector<double> Monomialestd::shared_ptr<Basis>::evalstd::shared_ptr<Basis> (double x) const {
    //       std::vector<double> evaluation_basis(this->length());
    //
    //       for (int i = 0; i < this->length (); ++i) {
    //           evaluation_basis[i] = pow (x,i);
    //       }
    //
    //       return evaluation_basis;
    //   }
    //

    //   int Monomialestd::shared_ptr<Basis>::length () const{return this->getDegree() + 1; }
    std::string MonomialeBasisNode::getRepresentation() const {return "MonomialeBasis, number is ";};
    std::string MonomialeBasis::getRepresentation() const { return (*this)->getRepresentation() ;};


    MonomialeBasisNode* MonomialeBasis::get() const { return static_cast<MonomialeBasisNode*>(SharedObject::get()); };
    MonomialeBasisNode* MonomialeBasis::operator->() const { return get(); }

    // MonomialeBasis::MonomialeBasis(int number)  { assign_node(new MonomialeBasisNode(number)); };


    Basis MonomialeBasis::operator+ (const Basis& other)const  {
        // return plusBasis(*this, other);
        return (*this)->operator+(other);
    }

    Basis MonomialeBasisNode::operator+ (const Basis& other)const  {
        return Basis();
    }

    Basis MonomialeBasis::operator+ (const MonomialeBasis& other)const  {
        return (*this)->operator+(other);
    }

    Basis MonomialeBasisNode::operator+ (const MonomialeBasis& other)const  {
        return MonomialeBasis();
    }

    Basis MonomialeBasis::operator+ (const BSplineBasis& other)const  {
        return (*this)->operator+(other);
    }

    Basis MonomialeBasisNode::operator+ (const BSplineBasis& other)const  {
        return Basis();
    }

    //   std::shared_ptr<Basis> Monomialestd::shared_ptr<Basis>::operator* (const Monomialestd::shared_ptr<Basis> &other)const  {
    //       return timesstd::shared_ptr<Basis>(*this, other);
    //   }
    //
    //   std::shared_ptr<Basis> Monomialestd::shared_ptr<Basis>::operator* (const BSplinestd::shared_ptr<Basis> &other)const  {
    //       return timesstd::shared_ptr<Basis>(*this, other);
    //   }
    //
    /// TODO(ErikLambr) check is grid is valid
    //   std::vector<double> Monomialestd::shared_ptr<Basis>::evaluationGrid () const {
    ///        std::vector<double> grid = casadi::range(length());
    //       std::vector<double> grid;
    //       for( int i = 0; i < length(); i++ )
    //           grid.push_back( (double)i );
    //       return grid;
    //   }
} // namespace spline
