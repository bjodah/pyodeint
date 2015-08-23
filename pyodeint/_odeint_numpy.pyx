# -*- coding: utf-8 -*-
# distutils: language = c++
# distutils: extra_compile_args = -std=c++11

from cpython.object cimport PyObject
from libcpp.vector cimport vector

import numpy as np
cimport numpy as cnp

cnp.import_array()  # Numpy C-API initialization

cdef extern from "odeint_numpy.hpp" namespace "odeint_numpy":
    cdef cppclass PyOdeintRosenbrock4:
        int ny
        vector[double] xout
        vector[double] yout

        PyOdeintRosenbrock4(PyObject*, PyObject*, int)
        size_t run(PyObject*, double, double, double, double, double)


cdef class OdeintRosenbrock4:

    cdef PyOdeintRosenbrock4 *thisptr

    def __cinit__(self, object f, object j, int ny):
        self.thisptr = new PyOdeintRosenbrock4(<PyObject *>f, <PyObject *>j, ny)

    def __dealloc__(self):
        del self.thisptr

    def run(self, cnp.ndarray[cnp.float64_t, ndim=1] y0, double x0, double xend,
            double dx0, double atol, double rtol):
        if y0.size < self.thisptr.ny:
            raise ValueError("y0 too short")
        return self.thisptr.run(<PyObject*>y0, x0, xend, dx0, atol, rtol)

    def get_xout(self, size_t nsteps):
        cdef cnp.ndarray[cnp.float64_t, ndim=1] xout = np.empty(nsteps, dtype=np.float64)
        cdef int i
        for i in range(nsteps):
            xout[i] = self.thisptr.xout[i]
        return xout

    def get_yout(self, size_t nsteps):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] yout = np.empty((nsteps, self.thisptr.ny),
                                                                dtype=np.float64)
        cdef int i
        cdef int ny = self.thisptr.ny
        for i in range(nsteps):
            for j in range(ny):
                yout[i, j] = self.thisptr.yout[i*ny + j]
        return yout


def integrate_adaptive(f, j, int ny, double atol, double rtol, y0, x0, xend, dx0):
    cdef size_t nsteps
    integr = OdeintRosenbrock4(f, j, ny)
    nsteps = integr.run(np.asarray(y0), x0, xend, dx0, atol, rtol)
    return integr.get_xout(nsteps), integr.get_yout(nsteps)
