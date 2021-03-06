#!/usr/bin/env python

import os

# Boolean flag to indicate if we run in valgrind
# To speed up valgrind, you may choose to put
# expensive computations within 'if not valgrind'
valgrind = int(os.environ.get("VALGRIND",'0'))

import meco_binaries;meco_binaries(cpp_splines='fill_in_the_branch_you_want')

import unittest
import numpy as np
import casadi as C

from casadi import *
from splines import *
import splines

class BasisTestCase(unittest.TestCase):


    @staticmethod
    def polyval(p,x):
      return np.polyval(list(reversed(p)),x)

    def assertEqualArray(self, a, b,tol=0):
        a = np.array(a)
        if(len(a.shape) == 2):
            if(a.shape[1] == 1):
                a = a[:,0]
        self.assertTrue(C.DM(np.array(a).ravel()).is_regular())
        self.assertTrue(C.DM(np.array(b).ravel()).is_regular())
        self.assertTrue(np.linalg.norm(np.array(a)-np.array(b))<=tol,msg="%e > %e" % (np.linalg.norm(np.array(a)-np.array(b)),tol))

    def assertEqualT(self, a, b,tol=1e-9):
        try:
          dima = a.dims()
        except:
          dima = np.array(a).shape
        try:
          dimb = b.dims()
        except:
          dimb = np.array(b).shape
        self.assertEqualArray(dima, dimb)
        self.assertEqualTensor(a,b,tol=tol)

    def assertEqualTensor(self, a, b, tol=1e-9):
        try:
            a = C.vec(C.DM(a))
        except:
          try:
            a = a.data()
          except:
            a = a.ravel('F')
        try:
            b = C.vec(C.DM(b))
        except:
          try:
            b = b.data()
          except:
            b = b.ravel('F')
        self.assertTrue(C.DM(a).is_regular())
        self.assertTrue(C.DM(b).is_regular())
        self.assertTrue(float(C.norm_inf(a-b)) < tol)

    def assertNotEqualTensor(self, a, b):
        try:
          a = a.data()
        except:
          pass
        self.assertFalse(list(a.full())==b)

