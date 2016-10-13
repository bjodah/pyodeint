# -*- coding: utf-8; mode: cython -*-

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.utility cimport pair
from libcpp cimport bool

cdef extern from "odeint_anyode.hpp" namespace "odeint_anyode":
    cdef cppclass StepType:
        pass

    cdef cppclass Integr[U]:
        double m_time_cpu, m_time_wall

    cdef void simple_predefined[U](
        U * const,
        const double,
        const double,
        const StepType,
        const double * const,
        const size_t,
        const double * const,
        double * const,
        long int,
        const double
    ) nogil except +

    cdef pair[vector[double], vector[double]] simple_adaptive[U](
        U * const,
        const double,
        const double,
        const StepType,
        const double * const,
        const double,
        const double,
        long int,
        const double
    ) nogil except +

    cdef StepType styp_from_name(string) nogil except +
    cdef bool requires_jacobian(StepType) nogil


cdef extern from "odeint_anyode.hpp" namespace "odeint_anyode::StepType":
    cdef StepType bulirsch_stoer
    cdef StepType rosenbrock4
    cdef StepType dopri5
