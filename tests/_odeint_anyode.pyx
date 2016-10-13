# -*- coding: utf-8; mode: cython -*-
# distutils: language = c++
# distutils: extra_compile_args = -std=c++11
#
# pyximport ignores the above settings, see _odeint_anyode.pyxbld

from odeint_anyode cimport simple_adaptive, styp_from_name


cdef extern from "testing_utils.hpp":
    cppclass Decay:
        Decay(double)


cdef class PyDecay:
    cdef Decay *thisptr

    def __cinit__(self, double k):
        self.thisptr = new Decay(k)

    def __dealloc__(self):
        del self.thisptr

    def adaptive(self, double y0, double t, long int mxsteps=0,
                 str stepper_name='bulirsch_stoer'):
        cdef double dt0 = 1e-14
        cdef double t0 = 0.0
        return simple_adaptive[Decay](
            self.thisptr, 1e-10, 1e-10,
            styp_from_name(stepper_name.lower().encode('UTF-8')),
            &y0, t0, t, mxsteps, dt0)
