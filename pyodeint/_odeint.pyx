# -*- coding: utf-8; mode: cython -*-
# distutils: language = c++
# distutils: extra_compile_args = -std=c++11

from cpython.ref cimport PyObject
from libcpp cimport bool
cimport numpy as cnp
cnp.import_array()  # Numpy C-API initialization

import numpy as np

from anyode_numpy cimport PyOdeSys
from odeint_anyode cimport simple_adaptive, simple_predefined, styp_from_name

steppers = ('rosenbrock4', 'dopri5', 'bulirsch_stoer')
requires_jac = ('rosenbrock4',)


cdef dict get_last_info(PyOdeSys * odesys, success=True):
    info = {str(k.decode('utf-8')): v for k, v in dict(odesys.current_info.nfo_int).items()}
    info.update({str(k.decode('utf-8')): v for k, v in dict(odesys.current_info.nfo_dbl).items()})
    info['nfev'] = odesys.nfev
    info['njev'] = odesys.njev
    info['success'] = success
    return info


def adaptive(rhs, jac, cnp.ndarray[cnp.float64_t] y0, double x0, double xend,
             double atol, double rtol, double dx0=.0, double dx_max=.0, str method='rosenbrock4', int nsteps=500,
             int autorestart=0, bool return_on_error=False, dx0cb=None, dx_max_cb=None):
    cdef:
        int ny = y0.shape[y0.ndim - 1]
        PyOdeSys * odesys
        int mlower=-1, mupper=-1, nquads=0, nroots=0, nnz=-1

    if method in requires_jac and jac is None:
        raise ValueError("Method requires explicit jacobian callback")
    if np.isnan(y0).any():
        raise ValueError("NaN found in y0")

    odesys = new PyOdeSys(ny, <PyObject *>rhs, <PyObject *>jac, NULL, NULL, NULL, NULL,
                          mlower, mupper, nquads, nroots, <PyObject *> dx0cb, <PyObject *>dx_max_cb, nnz)
    try:
        xout, yout = map(np.asarray, simple_adaptive[PyOdeSys](
            odesys, atol, rtol, styp_from_name(method.lower().encode('UTF-8')),
            &y0[0], x0, xend, nsteps, dx0, dx_max, autorestart, return_on_error))
        nfo = get_last_info(odesys, False if return_on_error and xout[-1] != xend else True)
        nfo['atol'], nfo['rtol'] = atol, rtol
        return xout, yout.reshape(xout.size, ny), nfo
    finally:
        del odesys


def predefined(rhs, jac,
               cnp.ndarray[cnp.float64_t] y0,
               cnp.ndarray[cnp.float64_t, ndim=1] xout,
               double atol, double rtol, double dx0=.0, double dx_max=.0, method='rosenbrock4',
               int nsteps=500, int autorestart=0, bool return_on_error=False, dx0cb=None, dx_max_cb=None):
    cdef:
        int ny = y0.shape[y0.ndim - 1]
        int nreached
        cnp.ndarray[cnp.float64_t, ndim=2] yout
        PyOdeSys * odesys
        int mlower=-1, mupper=-1, nquads=0, nroots=0, nnz=-1

    if method in requires_jac and jac is None:
        raise ValueError("Method requires explicit jacobian callback")
    if np.isnan(y0).any():
        raise ValueError("NaN found in y0")
    odesys = new PyOdeSys(ny, <PyObject *>rhs, <PyObject *>jac, NULL, NULL, NULL, NULL, mlower, mupper,
                          nquads, nroots, <PyObject *> dx0cb, <PyObject *>dx_max_cb, nnz)
    try:
        yout = np.empty((xout.size, ny))
        nreached = simple_predefined[PyOdeSys](odesys, atol, rtol, styp_from_name(method.lower().encode('UTF-8')),
                                               &y0[0], xout.size, &xout[0], &yout[0, 0], nsteps, dx0, dx_max,
                                               autorestart, return_on_error)
        info = get_last_info(odesys, success=False if return_on_error and nreached < xout.size else True)
        info['nreached'] = nreached
        info['atol'], info['rtol'] = atol, rtol
        return yout, info
    finally:
        del odesys
