v0.7.0
======
- Changed development status from alpha to beta.
- Jacobian callback has a new signature (fy=None, never passed by odeint, but by other integrators)
- Refactored wrappers, added "wall_time" entry in info-dict from integration.
- Refactored to use AnyODE base class (share code with pycvodes & pygslodeiv2)

v0.6.1
======
- Fixes to setup.py and conda recipe

v0.6.0
======
- Changes to info dict: rename 'nrhs' -> 'nfev', 'njac' -> 'njev', added 'cpu_time', 'success'

v0.5.0
======
- New function signature: integrate_predefined and integrate_adaptive now
  also return an info dict containing ``nrhs`` and ``njac`` conatining
  number of calls to each function made during last integration.
- Expose ``_odeint_numpy.steppers`` tuple at module level.
- check_callbable and check_indexing kwargs now defaults to False


v0.4.3
======
- Ship tests with package (e.g.: python -m pytest --pyargs pyodeint)

v0.4.2
======
- Less strict callback checks on python side.

v0.4.1
======
- jacobian callback may now be None for DOPRI5 and Bulirsch-Stoer (not used anyway).

v0.4
====
- Breaking Python API change: ny no longer needed in integrate_predefined/integrate_adaptive
- Excepctions thrown from C++ are now propagated to Python.

v0.3
====
- Breaking Python API change: "stepper" kwarg renamed to "method" (to follow SciPy better)
- Cython source rendered from template.

v0.2
====
- 'steppers' keyword added to integrate_adaptive
- integrate_predefined added
- integrate_adaptive have had its arguments reordered
- dopri5 stepper also available

v0.1
====
- Integration using adaptive step-size and the Rosenbrock4 stepper supported.
