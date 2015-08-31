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
        size_t adaptive(PyObject*, double, double, double, double, double)
        void predefined(PyObject*, PyObject *, PyObject *, double, double, double)

    cdef cppclass PyOdeintDopri5:
        int ny
        vector[double] xout
        vector[double] yout

        PyOdeintDopri5(PyObject*, int)
        size_t adaptive(PyObject*, double, double, double, double, double)
        void predefined(PyObject*, PyObject *, PyObject *, double, double, double)


cdef class IntegrBase:
    pass


cdef class OdeintRosenbrock4(IntegrBase):

    cdef PyOdeintRosenbrock4 *thisptr

    def __cinit__(self, object rhs, object jac, int ny):
        self.thisptr = new PyOdeintRosenbrock4(<PyObject *>rhs, <PyObject *>jac, ny)

    def __dealloc__(self):
        del self.thisptr

    def adaptive(self, cnp.ndarray[cnp.float64_t, ndim=1] y0, double x0, double xend,
            double dx0, double atol, double rtol):
        if y0.size < self.thisptr.ny:
            raise ValueError("y0 too short")
        return self.thisptr.adaptive(<PyObject*>y0, x0, xend, dx0, atol, rtol)

    def predefined(self, cnp.ndarray[cnp.float64_t, ndim=1] y0,
                   cnp.ndarray[cnp.float64_t, ndim=1] xout, double dx0, double atol, double rtol):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] yout = np.empty((xout.size, y0.size))
        yout[0, :] = y0
        self.thisptr.predefined(<PyObject*>y0, <PyObject*>xout, <PyObject*>yout,
                                dx0, atol, rtol)
        return yout

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

cdef class OdeintDopri5(IntegrBase):

    cdef PyOdeintDopri5 *thisptr

    def __cinit__(self, object rhs, object jac, int ny):
        # silently drop jac, maybe not the best design..
        self.thisptr = new PyOdeintDopri5(<PyObject *>rhs, ny)

    def __dealloc__(self):
        del self.thisptr

    def adaptive(self, cnp.ndarray[cnp.float64_t, ndim=1] y0, double x0, double xend,
            double dx0, double atol, double rtol):
        if y0.size < self.thisptr.ny:
            raise ValueError("y0 too short")
        return self.thisptr.adaptive(<PyObject*>y0, x0, xend, dx0, atol, rtol)

    def predefined(self, cnp.ndarray[cnp.float64_t, ndim=1] y0,
                   cnp.ndarray[cnp.float64_t, ndim=1] xout, double dx0, double atol, double rtol):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] yout = np.empty((xout.size, y0.size))
        yout[0, :] = y0
        self.thisptr.predefined(<PyObject*>y0, <PyObject*>xout, <PyObject*>yout,
                                dx0, atol, rtol)
        return yout

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


Integrator = {
    'rosenbrock4': OdeintRosenbrock4,
    'dopri5': OdeintDopri5,
}


def integrate_adaptive(rhs, jac, int ny, y0, x0, xend, dx0, double atol, double rtol,
                       stepper='rosenbrock4'):
    cdef size_t nsteps
    integr = Integrator[stepper](rhs, jac, ny)
    nsteps = integr.adaptive(np.asarray(y0), x0, xend, dx0, atol, rtol)
    return integr.get_xout(nsteps), integr.get_yout(nsteps)


def integrate_predefined(rhs, jac, int ny, y0, xout, double dx0, double atol, double rtol,
                         stepper='rosenbrock4'):
    integr = Integrator[stepper](rhs, jac, ny)
    yout = integr.predefined(
        np.asarray(y0, dtype=np.float64),
        np.asarray(xout, dtype=np.float64),
        dx0, atol, rtol)
    return yout
