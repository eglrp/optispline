#!/usr/bin/env python

from helpers import *

from numpy.polynomial.polynomial import polyval

class Test_Function_Operations(BasisTestCase):

#    def test_polynomial_operations(self):
#
#      p1 = Polynomial([0,0,1],'x')
#      p2 = Polynomial([0,1],'y')
#
#      x = Polynomial([0,1],'x')
#      y = p2
#      # f = x + y
#      # f = y*x
#      # f = x + y*x + y*y
#
#
#      # for p,poly in [
#      #                 (p1+p2, lambda x,y: x**2+y),
#      #                 (p1*p2, lambda x,y: x**2*y),
#      #                 (f, lambda x,y: x + y*x + y*y),
#      #                 (f + p1+p2, lambda x,y: x + y*x + y*y+x**2+y),
#      #                 (f + p1*p2, lambda x,y: x + y*x + y*y+x**2*y)
#      #               ]:
#
#      #   for arg in [ [0,0], [1,0], [0,1], [2,2], [0.7,1.3]]:
#      #     self.assertEqualTensor(p(*arg), poly(*arg))
#
#
#      # p1 = Polynomial([0,0,1],'x')
#      # p2 = Polynomial([0,1])
#
#      # for p,poly in [
#      #                 (p1+p2, lambda x: x**2+x),
#      #                 (p1*p2, lambda x: x**3),
#      #               ]:
#
#      #   for arg in [0,1,2,1.3]:
#      #     self.assertEqualTensor(p(arg), poly(arg))
#        x = Polynomial([0,1],'x')
#        y = p2
#        f = x + y
#        # f = y*x
#        # f = x + y*x + y*y
      
#    def test_const_function(self):
#        knots = [0,0,0.4,1,1]
#        degree = 1
#        basis = Basis.BSplineBasis(knots,degree)
#        
#        c = Function.Constant(basis,1)
#        self.assertEqualT(c(0.5),DTensor(1))
#
#        c2 = Function.Constant(basis,DTensor([2,2],[1,2]))
#        self.assertEqualT(c2(0.5),DTensor([2,2],[1,2]))
#        
#        basis2 = Basis.MonomialBasis(3)
#        c3 = Function.Constant(basis2,1)
#        self.assertEqualT(c3(0.5),DTensor(1))

    def test_bspline_operation(self):
        knots1 = [0,0,0.4,1,1]
        degree = 1
        basis1 = Basis.BSplineBasis(knots1,degree)

        knots2 = [0.,0.,0.5,1,1]
        degree = 1
        basis2 = Basis.BSplineBasis(knots2,degree)

        coeff1 = DTensor(numpy.random.randn(3,1),[3,1,1])
        coeff2 = DTensor(numpy.random.randn(3,1),[3,1,1])
        func1 = Function(basis1,coeff1)
        func2 = Function(basis2,coeff2)

        p1 = Polynomial([0,0,1],'x')
        
        # function evaluations
        x = [0.1,0.35,0.4,0.5,0.8,0.99] #range(-1.,2.1,10) 
        func1_value = []
        func2_value = []
        p1_value = []
        for v in x:
            func1_value.append(func1(v))
            func2_value.append(func2(v))
            p1_value.append(p1(v))

        # function operations
        u1 = -func1
        s1 = func1 + 2.0
        s3 = func1 + func1
        s2 = func1 + func2
        s4 = p1 + 3.3
        d1 = func2 - 5.0
        d2 = func2 - func1
        d3 = p1 - 2.0
        m1 = func1*2.0
        m2 = func1*func2
        m3 = p1*2.0
        p = func1.pow(3)
        pp = p1.pow(4)

        # assertion over grid
        k = 0
        for k in range(0,len(x)):
            v = x[k]
            self.assertAlmostEqual(s1(v),func1_value[k]+2.)
            self.assertAlmostEqual(s2(v),func1_value[k]+func2_value[k])
            self.assertAlmostEqual(u1(v),-func1_value[k])
            self.assertAlmostEqual(d1(v),func2_value[k]-5.)
            self.assertAlmostEqual(d2(v),func2_value[k]-func1_value[k])
            self.assertAlmostEqual(m1(v),func1_value[k]*2.)
            self.assertAlmostEqual(m2(v),func1_value[k]*func2_value[k])
            self.assertAlmostEqual(p(v),func1_value[k]**3)
            self.assertAlmostEqual(s4(v),p1_value[k]+3.3)
            self.assertAlmostEqual(d3(v),p1_value[k]-2.0)
            self.assertAlmostEqual(m3(v),p1_value[k]*2.0)
            self.assertAlmostEqual(pp(v),p1_value[k]**4)

if __name__ == '__main__':
    unittest.main()
