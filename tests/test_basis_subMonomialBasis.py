#!/usr/bin/env python
from helpers import *

import pdb

class Test_Basis_SubMonomial(BasisTestCase):

    def test_getLength(self):
        s = MonomialBasis(3)
        self.assertEqual(s.getLength(), 4)

    def test_getDimension(self):
        s = MonomialBasis(3)
        self.assertEqual(s.getNumberOfSubBasis(), 1)

    def test_getDegree(self):
        s = MonomialBasis(3)
        self.assertEqual(s.getDegree(), 3)

    def test_evaluation1(self):
        s = MonomialBasis(3)
        r = s([2.0])
        self.assertEqual(r.shape[0],s.getLength())

    def test_evaluation2(self):
        s = MonomialBasis(3)
        r = s([2.0])
        self.assertEqualTensor(r,[1,2,4,8])

    def test_evaluation3(self):
        s = MonomialBasis(3)
        r = s([0.0])
        self.assertEqualTensor(r,[1,0,0,0])

    def test_evaluationMX1(self):
        s = MonomialBasis(3)
        x = MX.sym("x")
        r = s([x])
        self.assertTrue(isinstance(r,MX))

    def test_evaluationSX1(self):
        s = MonomialBasis(3)
        x = SX.sym("x")
        r = s([x])
        self.assertTrue(isinstance(r,SX))

if __name__ == '__main__':
    unittest.main()
