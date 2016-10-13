# -*- coding: utf-8 -*-
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
    yout, info = integrate_predefined(f, j, y0, xout, dx0, 1e-9, 1e-9,
                                      method=method)
    yout, info = integrate_predefined(f, j, y0, xout, dx0, 1e-9, 1e-9,
                                      method=method)
    assert info['success']
    assert info['nfev'] > 0
    if use_jac:
        assert info['njev'] > 0
    yref = decay_get_Cref(k, y0, xout)
    assert np.allclose(yout, yref)
    assert 1e-9 < info['time_wall'] < 1.0  # Takes a few ms on a 2012 desktop computer
    assert 1e-9 < info['time_cpu'] < 1.0  # Takes a few ms on a 2012 desktop computer
