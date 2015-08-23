#!/usr/bin/env python
# -*- coding: utf-8 -*-

from pyodeint import integrate_adaptive


def get_f_and_j(mu):

    def f(t, y, dydt):
        dydt[0] = y[1]
        dydt[1] = -y[0] + mu*y[1]*(1 - y[0]**2)

    def j(t, y, Jmat, dfdt):
        Jmat[0, 0] = 0
        Jmat[0, 1] = 1
        Jmat[1, 0] = -1 - mu*2*y[1]*y[0]
        Jmat[1, 1] = mu*(1 - y[0]**2)
        dfdt[:] = 0

    return f, j


def integrate_ivp(u0=1.0, v0=0.0, mu=1.0, tend=10.0, dt0=1e-8,
                  t0=0.0, atol=1e-8, rtol=1e-8, plot=False, savefig='None'):
    """
    Example program integrating an IVP problem of van der Pol oscillator
    """
    ny = 2
    f, j = get_f_and_j(mu)
    tout, yout = integrate_adaptive(
        f, j, ny, atol, rtol, [u0, v0], t0, tend, dt0,
        check_indexing=False)  # dfdt[:] might only zero out length 1 vector
    if plot:
        import matplotlib.pyplot as plt
        plt.plot(tout, yout)
        if savefig == 'None':
            plt.show()
        else:
            plt.savefig(savefig)


if __name__ == '__main__':
    try:
        import argh
        argh.dispatch_command(integrate_ivp)
    except ImportError:
        integrate_ivp()
