#include <algorithm>    // std::set_union, std::sort
#include <vector>       // std::vector
#include "vectorUtilities.h"

namespace spline{

    const std::vector<double> unionKnots(const std::vector<double> kn1_, const std::vector<double> kn2_, int degree, int degree1, int degree2) {
        std::vector<double> kn1 = kn1_;
        std::vector<double> kn2 = kn2_;

        // assume knots are sorted
        // std::sort (kn1.begin(),kn1.end());
        // std::sort (kn2.begin(),kn2.end());

        std::vector<double> uniqueKnots(kn1.size() + kn2.size());
        std::vector<double>::iterator it;

        it=std::set_union (kn1.begin(),kn1.end(), kn2.begin(),kn2.end(), uniqueKnots.begin());
        uniqueKnots.resize(it-uniqueKnots.begin());
        uniqueKnots.erase( unique( uniqueKnots.begin(), uniqueKnots.end() ), uniqueKnots.end() );

        int count1, count2, count;
        std::vector<double> knots (0);
        std::vector<double> multiple (0);

        for (auto const& k: uniqueKnots) {
            count1 = std::count (kn1.begin(), kn1.end(), k);
            count2 = std::count (kn2.begin(), kn2.end(), k);

            count =  std::max(count1 + degree - degree1, count2 + degree - degree2);

            multiple.resize(count);
            std::fill(multiple.begin(), multiple.end(), k);

            knots.insert(knots.end(), multiple.begin(), multiple.end());
        }
        return knots;
    }

    const std::vector<double> addKnotsUnique(const std::vector<double> knots_,  const std::vector<double> newKnots_) {

        std::vector<double> knots = knots_;
        std::vector<double> newKnots = newKnots_;

        // assume knots are sorted
        // std::sort(knots.begin(),knots.end());
        // std::sort (newKnots.begin(),newKnots.end());

        std::vector<double> uniqueKnots(knots.size() + newKnots.size());
        std::vector<double>::iterator it;

        it=std::set_union (knots.begin(),knots.end(), newKnots.begin(),newKnots.end(), uniqueKnots.begin());
        uniqueKnots.resize(it-uniqueKnots.begin());
        uniqueKnots.erase( unique( uniqueKnots.begin(), uniqueKnots.end() ), uniqueKnots.end() );

        int count;
        std::vector<double> returnKnots (0);
        std::vector<double> multiple (0);

        for (auto const& k: uniqueKnots) {
            count = std::count (knots.begin(), knots.end(), k) + std::count (newKnots.begin(), newKnots.end(), k);
            multiple.resize(count);
            std::fill(multiple.begin(), multiple.end(), k);

            returnKnots.insert(returnKnots.end(), multiple.begin(), multiple.end());
        }
        return returnKnots;
    }

    const std::vector<double> addKnotsNotUnique(const std::vector<double> knots_,  const std::vector<double> newKnots_){
        std::vector<double> knots = knots_;
        std::vector<double> newKnots = newKnots_;

        // assume knots are sorted
        // std::sort(knots.begin(),knots.end());
        // std::sort(newKnots.begin(),newKnots.end());

        std::vector<double> returnKnots(knots.size() + newKnots.size());
        std::vector<double>::iterator it;

        it=std::set_union (knots.begin(),knots.end(), newKnots.begin(),newKnots.end(), returnKnots.begin());
        returnKnots.resize(it-returnKnots.begin());

        return returnKnots;
    }

    const std::vector<double> increaseMultiplicityKnots(const std::vector<double> knots_,  int increase){
        if (increase <= 0) { // noting happens
            return knots_;
        } else {
            int newSize = 0; 
            std::vector<double> returnKnots(knots_.size() * (increase+1));

            for (int i = 0; i < knots_.size() - 1; ++i) {
                if (knots_[i] == knots_[i+1]) {
                    returnKnots[newSize] = knots_[i];
                    newSize++;
                } else {
                    for (int j = 0; j < increase + 1; ++j) {
                        returnKnots[newSize] = knots_[i];
                        newSize++;
                    } 
                } 
            }

            for (int j = 0; j < increase + 1; ++j) {
                returnKnots[newSize] = knots_[knots_.size()-1];
                newSize++;
            } 
            returnKnots.resize(newSize);
            return returnKnots;
        }
    }
}  // namespace
