========
pyodeint
========

.. image:: http://hera.physchem.kth.se:8080/github.com/bjodah/pyodeint/status.svg?branch=master
   :target: http://hera.physchem.kth.se:8080/github.com/bjodah/pyodeint
   :alt: Build status

``pyodeint`` provides a `Python <http://www.python.org>`_ binding to
`odeint <http://www.odint.com>`_. Currently, only the the 4th order
Rosenbrock stepper has been wrapped and exposed to python (using the
adpative integration routine).

Example
=======
The classic van der Pol oscillator (see `examples/van_der_pol.py <examples/van_der_pol.py>`_)

.. code:: python

   >>> from pyodeint import integrate_adaptive
   >>> mu = 1.0
   >>> def f(t, y, dydt):
   ...     dydt[0] = y[1]
   ...     dydt[1] = -y[0] + mu*y[1]*(1 - y[0]**2)
   ... 
   >>> def j(t, y, Jmat, dfdt):
   ...     Jmat[0, 0] = 0
   ...     Jmat[0, 1] = 1
   ...     Jmat[1, 0] = -1 -mu*2*y[1]*y[0]
   ...     Jmat[1, 1] = mu*(1 - y[0]**2)
   ...     dfdt[0] = 0
   ...     dfdt[1] = 0
   ...
   >>> y0 = [1; 0]; tend=10.0; dt0=1e-8; t0=0.0; atol=1e-8; rtol=1e-8
   >>> tout, yout = integrate_adaptive(f, j, y0, t0, tend, dt0, atol, rtol)


License
=======
The source code is Open Source and is released under the very permissive
"simplified (2-clause) BSD license". See ``LICENSE.txt`` for further details.
Contributors are welcome to suggest improvements at https://github.com/bjodah/pyodeint

Author
======
Björn I. Dahlgren, contact:

- gmail address: bjodah
- kth.se address: bda
