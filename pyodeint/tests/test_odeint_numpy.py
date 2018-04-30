# -*- coding: utf-8 -*-
import os
import numpy as np
import pytest

from pyodeint import integrate_adaptive, integrate_predefined


decay_analytic = {
    0: lambda y0, k, t: (
        y0[0] * np.exp(-k[0]*t)),
    1: lambda y0, k, t: (
        y0[1] * np.exp(-k[1] * t) + y0[0] * k[0] / (k[1] - k[0]) *
        (np.exp(-k[0]*t) - np.exp(-k[1]*t))),
    2: lambda y0, k, t: (
        y0[2] * np.exp(-k[2] * t) + y0[1] * k[1] / (k[2] - k[1]) *
        (np.exp(-k[1]*t) - np.exp(-k[2]*t)) +
        k[1] * k[0] * y0[0] / (k[1] - k[0]) *
        (1 / (k[2] - k[0]) * (np.exp(-k[0]*t) - np.exp(-k[2]*t)) -
         1 / (k[2] - k[1]) * (np.exp(-k[1]*t) - np.exp(-k[2]*t))))
}


def decay_get_Cref(k, y0, tout):
    coeffs = list(k) + [0]*(3-len(k))
    return np.column_stack([
        decay_analytic[i](y0, coeffs, tout) for i in range(
            min(3, len(k)+1))])


def _get_f_j(k):
    k0, k1, k2 = k

    def f(t, y, fout):
        fout[0] = -k0*y[0]
        fout[1] = k0*y[0] - k1*y[1]
        fout[2] = k1*y[1] - k2*y[2]

    def j(t, y, jmat_out, dfdx_out):
        jmat_out[0, 0] = -k0
        jmat_out[0, 1] = 0
        jmat_out[0, 2] = 0
        jmat_out[1, 0] = k0
        jmat_out[1, 1] = -k1
        jmat_out[1, 2] = 0
        jmat_out[2, 0] = 0
        jmat_out[2, 1] = k1
        jmat_out[2, 2] = -k2
        dfdx_out[0] = 0
        dfdx_out[1] = 0
        dfdx_out[2] = 0
    return f, j

methods = [
    ('dopri5', False),
    ('bs', False),
    # rosenbrock4 suffered a massive performance regression:
    #   - https://github.com/headmyshoulder/odeint-v2/issues/189
    #   - https://github.com/bjodah/pyodeint/pull/16
    # this affects odeint provided by Boost 1.60 and 1.61
    ('rosenbrock4', True)
]


@pytest.mark.parametrize("method,use_jac", methods)
def test_integrate_adaptive(method, use_jac):
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0.3, 0.5]
    f, j = _get_f_j(k)
    if not use_jac:
        j = None
    kwargs = dict(x0=0, xend=3, dx0=1e-10, atol=1e-8, rtol=1e-8, method=method)
    # Run twice to catch possible side-effects:
    xout, yout, info = integrate_adaptive(f, j, y0, **kwargs)
    xout, yout, info = integrate_adaptive(f, j, y0, **kwargs)
    assert info['success']
    assert info['atol'] == 1e-8 and info['rtol'] == 1e-8
    assert info['nfev'] > 0
    if use_jac:
        assert info['njev'] > 0
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref)


@pytest.mark.parametrize("method,use_jac", methods)
def test_integrate_predefined(method, use_jac):
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0.3, 0.5]
    f, j = _get_f_j(k)
    if not use_jac:
        j = None
    xout = np.linspace(0, 3)
    dx0 = 1e-10
    # Run twice to catch possible side-effects:
    yout, info = integrate_predefined(f, j, y0, xout, 1e-9, 1e-9, dx0,
                                      method=method)
    yout, info = integrate_predefined(f, j, y0, xout, 1e-9, 1e-9, dx0,
                                      method=method)
    assert info['success']
    assert info['atol'] == 1e-9 and info['rtol'] == 1e-9
    assert info['nfev'] > 0
    if use_jac:
        assert info['njev'] > 0
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref)
    if os.name == 'posix':
        assert info['time_wall'] >= 0
        assert info['time_cpu'] >= 0


def test_adaptive_return_on_error():
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0.3, 0.5]
    atol, rtol = 1e-8, 1e-8
    kwargs = dict(x0=0, xend=3, dx0=1e-10, atol=atol, rtol=rtol,
                  method='rosenbrock4')
    f, j = _get_f_j(k)
    xout, yout, info = integrate_adaptive(f, j, y0, nsteps=7, return_on_error=True, **kwargs)
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref,
                       rtol=10*rtol,
                       atol=10*atol)
    assert xout.size == 8
    assert xout[-1] > 1e-6
    assert yout.shape[0] == xout.size
    assert info['nfev'] > 0
    assert info['njev'] > 0
    assert info['success'] is False
    assert xout[-1] < kwargs['xend']  # obviously not strict


def test_adaptive_autorestart():
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0.3, 0.5]
    atol, rtol = 1e-8, 1e-8
    kwargs = dict(x0=0, xend=3, dx0=1e-10, atol=atol, rtol=rtol,
                  method='rosenbrock4', nsteps=23, return_on_error=True,
                  autorestart=7+1)
    f, j = _get_f_j(k)
    xout, yout, info = integrate_adaptive(f, j, y0, **kwargs)
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref,
                       rtol=10*rtol,
                       atol=10*atol)
    assert xout[-1] > 1e-6
    assert yout.shape[0] == xout.size
    assert info['nfev'] > 0
    assert info['njev'] > 0
    assert info['success']
    assert xout[-1] == kwargs['xend']


def test_predefined_autorestart():
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0.3, 0.5]
    atol, rtol = 1e-8, 1e-8
    x0, xend = 0, 3
    kwargs = dict(dx0=1e-10, atol=atol, rtol=rtol,
                  method='rosenbrock4', nsteps=62,
                  autorestart=10)
    f, j = _get_f_j(k)
    xout = np.linspace(x0, xend)
    yout, info = integrate_predefined(f, j, y0, xout, **kwargs)
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref,
                       rtol=10*rtol,
                       atol=10*atol)
    assert xout[-1] > 1e-6
    assert yout.shape[0] == xout.size
    assert info['nfev'] > 0
    assert info['njev'] > 0
    assert info['success']
    assert xout[-1] == xend


def test_predefined_return_on_error():
    k = k0, k1, k2 = 2.0, 3.0, 4.0
    y0 = [0.7, 0., 0.]
    atol, rtol = 1e-8, 1e-8
    kwargs = dict(dx0=1e-10, atol=atol, rtol=rtol,
                  method='rosenbrock4', return_on_error=True, nsteps=12)
    f, j = _get_f_j(k)
    xout = np.logspace(-3, 1)
    yout, info = integrate_predefined(f, j, y0, xout, **kwargs)
    yref = decay_get_Cref(k, y0, xout - xout[0])
    assert np.allclose(yout[:info['nreached'], :], yref[:info['nreached'], :],
                       rtol=10*rtol,
                       atol=10*atol)
    assert 10 < info['nreached'] < 40
    assert yout.shape[0] == xout.size
    assert info['nfev'] > 0
    assert info['njev'] > 0
    assert info['success'] is False


def test_dx0cb():  # this test works for GSL and CVode, but it is a weak test for odeint
    k = 1e23, 3.0, 4.0
    y0 = [.7, .0, .0]
    x0, xend = 0, 5
    kwargs = dict(atol=1e-8, rtol=1e-8, method='rosenbrock4', dx0cb=lambda x, y: y[0]*1e-30)
    f, j = _get_f_j(k)
    xout, yout, info = integrate_adaptive(f, j, y0, x0, xend, **kwargs)
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref, atol=40*kwargs['atol'], rtol=40*kwargs['rtol'])
    assert info['nfev'] > 0
    assert info['njev'] > 0
    assert info['success'] is True
    assert xout[-1] == xend
