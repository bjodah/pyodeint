# -*- coding: utf-8 -*-
"""
Python binding for odeint from boost.
"""

from __future__ import absolute_import

from ._odeint_numpy import adaptive, predefined, requires_jac, steppers
from ._util import _check_callable, _check_indexing

from ._release import __version__


def get_include():
    from pkg_resources import resource_filename, Requirement
    return resource_filename(Requirement.parse(__name__),
                             '%s/include' % __name__)


def integrate_adaptive(rhs, jac, y0, x0, xend, dx0, atol, rtol,
                       check_callable=False, check_indexing=False, **kwargs):
    """
    Integrates a system of ordinary differential equations.

    Parameters
    ----------
    rhs: callable
        Function with signature f(t, y, fout) which modifies fout *inplace*.
    jac: callable
        Function with signature j(t, y, jmat_out, dfdx_out) which modifies
        jmat_out and dfdx_out *inplace*.
    y0: array_like
        initial values of the dependent variables
    x0: float
        initial value of the independent variable
    xend: float
        stopping value for the independent variable
    dx0: float
        initial step-size
    atol: float
        absolute tolerance
    rtol: float
        relative tolerance
    check_callable: bool (default: False)
        perform signature sanity checks on ``rhs`` and ``jac``
    check_indexing: bool (default: False)
        perform item setting sanity checks on ``rhs`` and ``jac``.
    \*\*kwargs:
         'method': str
            'rosenbrock4', 'dopri5' or 'bs'

    Returns
    -------
    (xout, yout, info):
        xout: 1-dimensional array of values for the independent variable
        yout: 2-dimensional array of the dependent variables (axis 1) for
            values corresponding to xout (axis 0)
        info: dictionary with information about the integration
    """
    # Sanity checks to reduce risk of having a segfault:
    if check_callable:
        _check_callable(rhs, jac, x0, y0)

    if check_indexing:
        _check_indexing(rhs, jac, x0, y0)

    return adaptive(rhs, jac, y0, x0, xend, dx0, atol, rtol, **kwargs)


def integrate_predefined(rhs, jac, y0, xout, dx0, atol, rtol,
                         check_callable=False, check_indexing=False, **kwargs):
    """
    Integrates a system of ordinary differential equations.

    Parameters
    ----------
    rhs: callable
        Function with signature f(t, y, fout) which modifies fout *inplace*.
    jac: callable
        Function with signature j(t, y, jmat_out, dfdx_out) which modifies
        jmat_out and dfdx_out *inplace*.
    y0: array_like
        initial values of the dependent variables
    xout: array_like
        values of the independent variable
    dx0: float
        initial step-size
    atol: float
        absolute tolerance
    rtol: float
        relative tolerance
    check_callable: bool (default: False)
        perform signature sanity checks on ``rhs`` and ``jac``
    check_indexing: bool (default: False)
        perform item setting sanity checks on ``rhs`` and ``jac``.
    \*\*kwargs:
         'method': str
            'rosenbrock4', 'dopri5' or 'bs'

    Returns
    -------
    (result, info):
        result: 2-dimensional array of the dependent variables (axis 1) for
            values corresponding to xout (axis 0)
        info: dictionary with information about the integration
    """
    # Sanity checks to reduce risk of having a segfault:
    if check_callable:
        _check_callable(rhs, jac, xout[0], y0)

    if check_indexing:
        _check_indexing(rhs, jac, xout[0], y0)

    return predefined(rhs, jac, y0, xout, dx0, atol, rtol, **kwargs)
