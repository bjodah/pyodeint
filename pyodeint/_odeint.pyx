# -*- coding: utf-8; mode: cython -*-
# distutils: language = c++
# distutils: extra_compile_args = -std=c++11

from cpython.ref cimport PyObject
cimport numpy as cnp
cnp.import_array()  # Numpy C-API initialization

import numpy as np

from anyode_numpy cimport PyOdeSys
from odeint_anyode cimport simple_adaptive, simple_predefined, styp_from_name

steppers = ('rosenbrock4', 'dopri5', 'bulirsch_stoer')
requires_jac = ('rosenbrock4',)


cdef dict get_last_info(PyOdeSys * odesys):
    info = {str(k.decode('utf-8')): v for k, v in dict(odesys.last_integration_info).items()}
    info.update({str(k.decode('utf-8')): v for k, v in dict(odesys.last_integration_info_dbl).items()})
    info['nfev'] = odesys.nfev
    info['njev'] = odesys.njev
    info['success'] = True
    return info


def adaptive(rhs, jac, cnp.ndarray[cnp.float64_t, ndim=1] y0, double x0, double xend,
             double dx0, double atol, double rtol, str method='rosenbrock4', int nsteps=500):
    cdef:
        int ny = y0.shape[y0.ndim - 1]
        PyOdeSys * odesys
    if method in requires_jac and jac is None:
        raise ValueError("Method requires explicit jacobian callback")
    if np.isnan(y0).any():
        raise ValueError("NaN found in y0")

    odesys = new PyOdeSys(ny, <PyObject *>rhs, <PyObject *>jac, NULL, NULL)
    try:
        xout, yout = map(np.asarray, simple_adaptive[PyOdeSys](
            odesys, atol, rtol, styp_from_name(method.lower().encode('UTF-8')),
            &y0[0], x0, xend, dx0, nsteps))
        return xout, yout.reshape(xout.size, ny), get_last_info(odesys)
    finally:
        del odesys


def predefined(rhs, jac,
               cnp.ndarray[cnp.float64_t, ndim=1] y0,
               cnp.ndarray[cnp.float64_t, ndim=1] xout,
               double dx0, double atol,
               double rtol, method='rosenbrock4', int nsteps=500):
    cdef:
        int ny = y0.shape[y0.ndim - 1]
        cnp.ndarray[cnp.float64_t, ndim=2] yout
        PyOdeSys * odesys
    if method in requires_jac and jac is None:
        raise ValueError("Method requires explicit jacobian callback")
    if np.isnan(y0).any():
        raise ValueError("NaN found in y0")
    odesys = new PyOdeSys(ny, <PyObject *>rhs, <PyObject *>jac, NULL, NULL)
    try:
        yout = np.empty((xout.size, ny))
        simple_predefined[PyOdeSys](odesys, atol, rtol, styp_from_name(method.lower().encode('UTF-8')),
                                    &y0[0], xout.size, &xout[0], &yout[0, 0],
                                    dx0, nsteps)
        return yout, get_last_info(odesys)
    finally:
        del odesys
