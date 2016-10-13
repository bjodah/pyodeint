#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Example program integrating an IVP problem of van der Pol oscillator
"""

from __future__ import (absolute_import, division, print_function)

import numpy as np
from pyodeint import integrate_adaptive, integrate_predefined


def get_f_and_j(mu):

    def f(t, y, dydt):
        dydt[0] = y[1]
        dydt[1] = -y[0] + mu*y[1]*(1 - y[0]**2)

    def j(t, y, Jmat, dfdt, fy=None):
        Jmat[0, 0] = 0
        Jmat[0, 1] = 1
        Jmat[1, 0] = -1 - mu*2*y[1]*y[0]
        Jmat[1, 1] = mu*(1 - y[0]**2)
        dfdt[:] = 0

    return f, j


def integrate_ivp(u0=1.0, v0=0.0, mu=1.0, tend=10.0, dt0=1e-8, nt=0,
                  t0=0.0, atol=1e-8, rtol=1e-8, plot=False, savefig='None',
                  method='rosenbrock4', dpi=100, verbose=False):
    f, j = get_f_and_j(mu)
    if nt > 1:
        tout = np.linspace(t0, tend, nt)
        yout, info = integrate_predefined(
            f, j, [u0, v0], tout, dt0, atol, rtol, method=method, nsteps=1000)
    else:
        tout, yout, info = integrate_adaptive(
            f, j, [u0, v0], t0, tend, dt0, atol, rtol, method=method, nsteps=1000)
    if verbose:
        print(info)
    if plot:
        import matplotlib.pyplot as plt
        plt.plot(tout, yout)
        if savefig == 'None':
            plt.show()
        else:
            plt.savefig(savefig, dpi=dpi)


if __name__ == '__main__':
    try:
        import argh
        argh.dispatch_command(integrate_ivp)
    except ImportError:
        integrate_ivp()
