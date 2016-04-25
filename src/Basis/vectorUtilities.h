//
// Created by erik on 19/04/16.
//

#ifndef CPP_SPLINES_VECTORUTILITIES_H
#define CPP_SPLINES_VECTORUTILITIES_H

#include <vector>

namespace spline{

    std::vector<double> unionKnots(const std::vector<double> kn1,  const std::vector<double> kn2, int degree, int degree1, int degree2) ;
    std::vector<double> addKnotsUnique(const std::vector<double> knots,  const std::vector<double> newKnots) ;
    std::vector<double> addKnotsNotUnique(const std::vector<double> knots,  const std::vector<double> newKnots) ;

}  // namespace spline

#endif //CPP_SPLINES_VECTORUTILITIES_H