# -*- mode: cython -*-
# -*- coding: utf-8 -*-

from libcpp.vector cimport vector
from libcpp.utility cimport pair
from odeint_anyode cimport StepType

cdef extern from "odeint_anyode_parallel.hpp" namespace "odeint_anyode_parallel":
    cdef vector[pair[vector[double], vector[double]]] multi_adaptive[U](
        vector[U*],
        double,
        double,
        StepType,
        const double * const,
        const double *,
        const double *,
        long int,
        double
    ) nogil except +

    cdef vector[pair[vector[int], vector[double]]] multi_predefined[U](
        vector[U*],
        double,
        double,
        StepType,
        const double * const,
        size_t,
        const double * const,
        double * const,
        long int,
        double
    ) nogil except +
